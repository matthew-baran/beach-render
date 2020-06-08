#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 invmodel;

out vec3 FragPos;
out vec2 texCoords;
out mat3 tbn;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.);

    vec3 tangent = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 normal = normalize(vec3(model * vec4(aNormal, 0.0)));
    vec3 bitangent = cross(normal, tangent);

    tbn = mat3(tangent, bitangent, normal);
}