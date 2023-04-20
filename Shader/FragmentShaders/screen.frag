#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;
uniform bool bloom;
uniform float exposure;

void main() {
    const float gamma = 2.2;
    vec3 color = texture(screenTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
    if (bloom)
        color += bloomColor; // additive blending


    // reinhard tone mapping
    vec3 result = vec3(1.0) - exp(-color * exposure);
    // gamme corretion
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}