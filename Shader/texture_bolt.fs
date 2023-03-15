#version 460 core

out vec4 FragColor;

in vec3 Color;
in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = vec4(Color, 1.0);
}