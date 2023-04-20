#version 460 core 

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BlurColor;

uniform vec3 color;
uniform float alpha;

void main() {
    BlurColor = vec4(color, alpha);
    FragColor = vec4(color, alpha);
}