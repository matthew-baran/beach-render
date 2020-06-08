#version 330 core

struct Wave
{
    vec2 wave_dirs;
    float freq;
    float phase;
    float phase_offset;
    float amplitude;
    float chop;
};
#define NUM_TEX_WAVES 32
uniform Wave[NUM_TEX_WAVES] tex_waves;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
};
uniform Material material;

struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dir_light;

uniform vec3 cam_pos;
uniform samplerCube env_map;
uniform sampler2D brdf_map;

in vec2 TexCoords;
in vec2 brdf_coords;
in vec3 Normal;
in vec3 WorldPos;
in vec3 FragPos;

in vec3 Color;
in float depth;
in float att_factor;
in float surface_elev;
in float particle_phase;
in float wave_width;

const float PI = 3.14159265359;

out vec4 FragColor;

vec3 skydomeLight(vec3 normal, vec3 view_dir);
vec3 directionalLight(DirLight light, vec3 normal, vec3 view_dir);
vec3 getTexNormal();

// PBR
float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySchlickGGX(float NdotV, float k);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{
    vec3 out_color = vec3(0.0);

    //	vec3 new_norm = normalize(Normal);
    vec3 new_norm = getTexNormal();

    vec3 view_dir = normalize(cam_pos - WorldPos);

    out_color += directionalLight(dir_light, new_norm, view_dir);
    out_color += skydomeLight(new_norm, view_dir);

    vec3 refract_ray = refract(view_dir, new_norm, 1.0 / 1.33);
    float cos_val = dot(refract_ray, vec3(0, -1, 0));
    float alpha;
    if (cos_val > 0)
    {
        float refract_length = depth / cos_val;
        alpha = clamp(1 - exp(-refract_length * 5), 0, 1);
    }
    else
    {
        alpha = 1.0;
    }

    out_color = out_color / (out_color + vec3(1.0));
    out_color = pow(out_color, vec3(1.0 / 2.2));

    // Foam texture will need its own PBR prior to HDR tonemapping, but... let's get it working
    // first
    float max_foam_depth = 0.5;
    float foam_alpha = 0;
    //	if (surface_elev + max_foam_depth > 0)
    if (TexCoords.y >= 0 && TexCoords.y <= 1)
    {
        float v = 1 - clamp((max_foam_depth + surface_elev) / max_foam_depth, 0, 1);
        float u = WorldPos.z / 5;
        //		vec4 foam_color = texture(material.texture_diffuse2, vec2(u, v));
        vec4 foam_color = texture(material.texture_diffuse2, TexCoords);

        // warning, warning, this only works with single wave
        float fade_time = 0.15 * 2 * PI;
        float blend_factor = mod(particle_phase, 2 * PI);
        float f1 = 1 - clamp((blend_factor - PI / 2) / fade_time, 0, 1);
        float f2 = 1 - clamp(-(blend_factor - 3 * PI / 2) / fade_time, 0, 1);

        foam_alpha = min(foam_color.a, max(f1, f2));
        out_color = foam_color.rgb * foam_alpha + out_color * (1 - foam_alpha);
    }

    //	if (surface_elev > -.15)
    if (TexCoords.y < 0)
    {
        alpha = 0.0;
    }
    else
    {
        alpha = max(foam_alpha, alpha);
        //		alpha = 1.0;
    }

    //	alpha = clamp((depth-0.15)/0.5, 0, 1);
    //	alpha = 1;

    //	vec3 debug = vec3(sign(new_norm.y) + 1) / 2;
    //	FragColor = vec4(debug,1);
    //	FragColor = vec4(texture(brdf_map, vec2(WorldPos.x/50, -WorldPos.z/50)).rg, 0.0, 1.0);
    FragColor = vec4(out_color, alpha);
}

vec3 skydomeLight(vec3 normal, vec3 view_dir)
{
    vec3 albedo = Color;

    float roughness = 0.25;
    float metalness = 0.01;
    vec3 F0 = vec3(0.02); // water Fresnel normal incidence
    F0 = mix(F0, albedo, metalness);
    vec3 kS = fresnelSchlickRoughness(max(dot(normal, view_dir), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;

    vec3 irradiance = texture(env_map, normal).rgb;
    vec3 diffuse = irradiance * albedo;

    // instead of sampling/integrating - assume normal as halfway_dir....
    vec3 light_dir = reflect(view_dir, normal);
    vec3 halfway_dir = normal;

    vec3 F = kS;

    ///////////////// specular experiment
    vec3 N = normal;
    vec3 R = reflect(-view_dir, normal);
    vec3 V = R;

    vec3 prefilteredColor = irradiance; // technically should use LOD/sampled map
    vec2 brdf = texture(brdf_map, vec2(max(dot(normal, view_dir), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular);

    return ambient;
    //	return (ambient + diffuse + specular);
}

vec3 directionalLight(DirLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);

    vec3 light_color = vec3(23.47, 21.31, 20.79) * .2;

    vec3 albedo = Color;

    vec3 halfway_dir = normalize(light_dir + view_dir);

    float roughness = 0.25; // water roughness
    vec3 F0 = vec3(0.02);   // water Fresnel normal incidence

    float metalness = 0.1; // assuming water is pure dielectric
    F0 = mix(F0, albedo, metalness);

    float D = DistributionGGX(normal, halfway_dir, roughness);
    float G = GeometrySmith(normal, view_dir, light_dir, roughness);
    vec3 F = fresnelSchlick(dot(halfway_dir, view_dir), F0);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metalness;

    float NdotL = max(dot(normal, light_dir), 0.0);
    vec3 radiance = light_color;

    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // attenuate SSS based on wave width
    float scatter_strength = clamp(exp(-wave_width / 5), 0, 1);

    // allow up to 30 degrees of backscatter
    float dir_strength = clamp(dot(light_dir, -normalize(Normal)) + sin(radians(22.)), 0, 1);

    Lo +=
        25 * dir_strength * scatter_strength * radiance * albedo * max(dot(normal, view_dir), 0.0);

    return Lo;
}

vec3 getTexNormal()
{
    vec3 new_norm = Normal;
    for (int i = 0; i < NUM_TEX_WAVES; ++i)
    {
        float amp = att_factor * tex_waves[i].amplitude;

        // Gerstner
        float angle =
            tex_waves[i].freq * dot(tex_waves[i].wave_dirs, vec2(WorldPos.x, -WorldPos.z)) -
            tex_waves[i].phase;
        new_norm.x -= tex_waves[i].wave_dirs.x * tex_waves[i].freq * amp * cos(angle);
        new_norm.z -= -tex_waves[i].wave_dirs.y * tex_waves[i].freq * amp * cos(angle);
        new_norm.y -= tex_waves[i].chop * tex_waves[i].freq * amp * sin(angle);

        // Power Sines
        //		float k = 3;
        //		float angle = tex_waves[i].freq*dot(tex_waves[i].wave_dirs, vec2(WorldPos.x,
        //-WorldPos.z)) - tex_waves[i].phase; 		vec3 tmp; 		new_norm.x -= k *
        //tex_waves[i].freq * tex_waves[i].wave_dirs.x * amp * cos(angle) * pow((sin(angle)+1)/2,
        // k-1); 		new_norm.z -= k
        //* tex_waves[i].freq * -tex_waves[i].wave_dirs.y * amp * cos(angle) * pow((sin(angle)+1)/2,
        // k-1); 		new_norm.y += amp;
    }

    return normalize(new_norm);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) { return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); }

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}