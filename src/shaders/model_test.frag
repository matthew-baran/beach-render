#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
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

vec3 directionalLight(DirLight light, vec3 normal, vec3 view_dir);

in vec3 TestColor;
in vec3 world_norm;

void main()
{
    vec3 out_color = vec3(0.0);

    vec3 view_dir = normalize(-FragPos);
    out_color += directionalLight(dir_light, Normal, view_dir);

    // vec3 test = (abs(normalize(world_norm))) / 2;
    // FragColor = vec4(out_color.x/2 + test.x, out_color.y, out_color.z/2 + test.z, 1.0);

    //	FragColor = vec4(TestColor, 1.0);
    FragColor = vec4(out_color, 1.0);
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
