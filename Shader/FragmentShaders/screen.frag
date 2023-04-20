#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;

uniform float exposure;
uniform float gamma;

uniform bool bloomEnabled;
uniform bool gammaEnabled;
uniform bool exposureEnabled;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
    if (bloomEnabled)
        color += bloomColor; // additive blending

    vec3 result;

    if (exposureEnabled)    // reinhard tone mapping
		result = vec3(1.0) - exp(-color * exposure);
	else
		result = color;

    if (gammaEnabled)     // gamme corretion
        result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}