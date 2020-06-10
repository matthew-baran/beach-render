#version 330 core

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_diffuse4;
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

vec2 parallaxMapping(vec3 view_dir, vec2 uv_coords);
vec3 skydomeLight(vec3 normal, vec3 view_dir, vec3 albedo, vec2 uv_coords);
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

    vec2 disp_coords = parallaxMapping(view_dir, tex_coords);

    vec3 albedo = texture(material.texture_diffuse1, disp_coords).rgb;
    vec3 tex_norm = texture(material.texture_diffuse2, disp_coords).rgb;
    tex_norm = 2.0 * tex_norm - 1.0;

    out_color += directionalLight(tan_light_dir, tex_norm, view_dir, albedo);
    out_color += skydomeLight(tex_norm, view_dir, albedo, disp_coords);

    // HDR stretch
    out_color = out_color / (out_color + vec3(1.0));
    out_color = pow(out_color, vec3(1.0 / 2.2));

    // FragColor = vec4(tex_norm, 1.0);
    FragColor = vec4(out_color, 1.0);
}

vec2 parallaxMapping(vec3 view_dir, vec2 uv_coords)
{
    float height_scale = 0.1;

    const float min_layers = 8;
    const float max_layers = 32;
    float num_layers = mix(max_layers, min_layers, max(dot(vec3(0.0, 0.0, 1.0), view_dir), 0.0));

    float depth_step = 1.0 / num_layers;
    float cur_depth = 0.0;
    vec2 delta_uv = (view_dir.xy / view_dir.z) * height_scale / num_layers;

    float depth_val = texture(material.texture_diffuse4, uv_coords).r;
    float depth_prev = depth_val;

    while (cur_depth < depth_val)
    {
        uv_coords -= delta_uv;
        depth_prev = depth_val;
        depth_val = texture(material.texture_diffuse4, uv_coords).r;
        cur_depth += depth_step;
    }

    vec2 prev_uv = uv_coords + delta_uv;
    float depth_after = depth_val - cur_depth;
    float depth_before = depth_prev - cur_depth + depth_step;

    float weight = depth_after / (depth_after - depth_before);
    uv_coords = prev_uv * weight + uv_coords * (1.0 - weight);

    return uv_coords;
}

vec3 skydomeLight(vec3 normal, vec3 view_dir, vec3 albedo, vec2 uv_coords)
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

    float ao = texture(material.texture_diffuse3, uv_coords).r;
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
    float metalness = 0.01; // assuming water is pure dielectric

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