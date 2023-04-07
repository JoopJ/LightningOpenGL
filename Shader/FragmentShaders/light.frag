#version 460 core 

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BlurColor;

void main() {
    FragColor = vec4(1.0);
    BlurColor = vec4(0);
}