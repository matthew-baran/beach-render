#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_dir;
uniform vec3 cam_pos;

out vec3 frag_pos;
out vec2 tex_coords;
out vec3 world_normal;
out vec3 tan_light_dir;
out vec3 tan_cam_pos;
out vec3 tan_frag_pos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.);

    frag_pos = vec3(model * vec4(aPos, 1.0));
	tex_coords = aTexCoords;

    vec3 tangent = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 world_normal = normalize(vec3(model * vec4(aNormal, 0.0)));

	// G-S Re-orthogonalize
    tangent = normalize(tangent - dot(tangent, world_normal) * world_normal);

    vec3 bitangent = cross(world_normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, world_normal);
	mat3 tbn_inv = transpose(tbn);

    tan_light_dir = tbn_inv * light_dir;
    tan_cam_pos = tbn_inv * cam_pos;
    tan_frag_pos = tbn_inv * frag_pos;
}