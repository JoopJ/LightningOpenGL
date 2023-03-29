#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform bool bloom;
//uniform float exposure;

void main() {
    //const float gamma = 2.2;
    vec3 color = texture(screenTexture, TexCoords).rgb;
   if (bloom)
       color += texture(bloomBlur, TexCoords).rgb; // additive blending


    // reinhard tone mapping
    //vec3 result = vec3(1.0) - exp(-color * exposure);
    // gamme corretion
    //result = pow(result, vec3(1.0 / gamma));
    vec3 result = color;

    FragColor = vec4(result,1.0);
}