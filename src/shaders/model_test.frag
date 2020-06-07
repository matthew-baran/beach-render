#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};
uniform Material material;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dir_light;

struct PointLight {
	vec3 position;
	vec3 attenuation;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
//#define NR_POINT_LIGHTS 1
//uniform PointLight point_lights[NR_POINT_LIGHTS];

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 attenuation;
	float cutoff;
	float outer_cutoff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
//uniform SpotLight spot_light;

vec3 directionalLight(DirLight light, vec3 normal, vec3 view_dir);
vec3 pointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 spotLight(SpotLight light);

in vec3 TestColor;

void main()
{
	vec3 output = vec3(0.0);

	vec3 view_dir = normalize(-FragPos);
	output += directionalLight(dir_light, Normal, view_dir);

//	for (int i = 0; i < NR_POINT_LIGHTS; ++i)
//	{
//		output += pointLight(point_lights[i], Normal, FragPos, view_dir);
//	}

//	output += spotLight(spot_light);
//	FragColor = vec4(TestColor, 1.0);
	FragColor = vec4(output, 1.0);
}

vec3 directionalLight(DirLight light, vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-light.direction);

	float diff = max(dot(normal, light_dir), 0.0);

	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0f);

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
	return (ambient + diffuse + specular);
}

vec3 pointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir = normalize(light.position - frag_pos);

	float diff = max(dot(normal, light_dir), 0.0);

	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0f);

	float d = length(light.position - frag_pos);
	float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y*d + light.attenuation.z*d*d);

	vec3 ambient = attenuation * light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 diffuse = attenuation * light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 specular = attenuation * light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
	return (ambient + diffuse + specular);
}

vec3 spotLight(SpotLight light)
{
	float d = length(light.position - FragPos);
	float att_factor = 1 / (light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d);

	vec3 ambient = att_factor * light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));

	vec3 light_dir = normalize(light.direction);
	vec3 light_vec = normalize(light.position - FragPos);
	float light_angle = dot(light_vec, light_dir);
	float epsilon = light.cutoff - light.outer_cutoff;
	float spot_strength = clamp((light_angle - light.outer_cutoff) / epsilon, 0.0, 1.0);

	vec3 norm = normalize(Normal);	
	float diff = spot_strength * max(dot(light_dir, norm), 0.0);

	vec3 diffuse = att_factor * light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));

	vec3 cam_vec = normalize(-FragPos);
	vec3 view_dir = normalize(reflect(-light_dir, norm));
	float spec = spot_strength * pow(max(dot(view_dir, cam_vec), 0.0), 32.0f);

	vec3 specular = att_factor * light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
	
	return (ambient + diffuse + specular);
}
