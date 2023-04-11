#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

const int NR_LIGHTS = 1;
// layout (std140, binding = 2), uniform LightPositions { // explicitly specify binding point
layout (std140) uniform LightPositions {
    vec3 lightPositions;  
};

uniform vec3 viewPos;

// attenuation and color parameters are constatnt for all lights 
// (waste of space to pass with each Light object)
uniform float Linear;
uniform float Quadratic;
uniform vec3 Color;
void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting = Albedo * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lightPositions - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * Color;
        
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = Specular * spec * Color;

        // attenuation
        float distance = length(lightPositions - FragPos);
        float attenuation = 1.0 / (1.0 + Linear * distance + Quadratic * distance * distance);

        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    
    FragColor = vec4(lighting, 1.0);
}  