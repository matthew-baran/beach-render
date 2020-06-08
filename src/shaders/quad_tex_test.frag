#version 330 core

uniform sampler2D tex;

in vec2 TexCoords;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(tex, TexCoords).rg, 0.0, 1.0);
    //	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}