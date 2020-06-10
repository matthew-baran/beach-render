#version 330 core

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
};
uniform Material material;

uniform samplerCube env_map;
uniform sampler2D brdf_map;

in vec2 brdf_coords;
in vec3 frag_pos;
in vec2 tex_coords;
in vec3 world_normal;
in vec3 tan_light_dir;
in vec3 tan_cam_pos;
in vec3 tan_frag_pos;

const float PI = 3.14159265359;

out vec4 FragColor;

vec3 skydomeLight(vec3 normal, vec3 view_dir, vec3 albedo);
vec3 directionalLight(vec3 light_dir, vec3 normal, vec3 view_dir, vec3 albedo);

// PBR
float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySchlickGGX(float NdotV, float k);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{
    vec3 out_color = vec3(0.0);

    vec3 view_dir = normalize(tan_cam_pos - tan_frag_pos);

    vec3 albedo = texture(material.texture_diffuse1, tex_coords).rgb;
    // vec3 tex_norm = texture(material.texture_diffuse2, tex_coords).rbg;     // Flip Z-up to Y-up -- make this a uniform optional?
    // tex_norm = 2.0 * tex_norm - 1.0;
    vec3 tex_norm = vec3(0, 1, 0);

    out_color += directionalLight(tan_light_dir, tex_norm, view_dir, albedo);
    out_color += skydomeLight(tex_norm, view_dir, albedo);

    // HDR stretch
    out_color = out_color / (out_color + vec3(1.0));
    out_color = pow(out_color, vec3(1.0 / 2.2));

    // FragColor = vec4(tex_norm, 1.0);
    FragColor = vec4(out_color, 1.0);
}

vec3 skydomeLight(vec3 normal, vec3 view_dir, vec3 albedo)
{
    // These should be uniforms
    float roughness = 0.95;
    float metalness = 0.01;
    vec3 F0 = vec3(0.02); // water Fresnel normal incidence

    // Indirect diffuse
    F0 = mix(F0, albedo, metalness);
    vec3 kS = fresnelSchlickRoughness(max(dot(normal, view_dir), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;

    vec3 irradiance = texture(env_map, world_normal).rgb;
    vec3 diffuse = irradiance * albedo;

    // Indirect specular
    vec3 N = normal;
    vec3 R = reflect(-view_dir, normal);
    vec3 V = R;

    vec3 prefilteredColor = irradiance; // technically should use LOD/sampled map
    vec2 brdf = texture(brdf_map, vec2(max(dot(normal, view_dir), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

    float ao = texture(material.texture_diffuse3, tex_coords).r;
    vec3 ambient = (kD * diffuse + specular) * ao;

    return ambient;
}

vec3 directionalLight(vec3 light_dir, vec3 normal, vec3 view_dir, vec3 albedo)
{
    light_dir = normalize(-light_dir);

    // These should be uniform defined variables
    vec3 light_color = vec3(23.47, 21.31, 20.79) * .2;
    float roughness = 0.95; // water roughness
    vec3 F0 = vec3(0.02);   // water Fresnel normal incidence
    float metalness = 0.01;  // assuming water is pure dielectric

    vec3 halfway_dir = normalize(light_dir + view_dir);

    F0 = mix(F0, albedo, metalness);

    float D = DistributionGGX(normal, halfway_dir, roughness);
    float G = GeometrySmith(normal, view_dir, light_dir, roughness);
    vec3 kS = fresnelSchlick(dot(halfway_dir, view_dir), F0);
    
    vec3 numerator = D * G * kS;
    float denominator = 4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float NdotL = max(dot(normal, light_dir), 0.0);
    vec3 radiance = light_color;

    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    return Lo;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}