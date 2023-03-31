#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BlurColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;


uniform float far_plane;
uniform bool shadows; // enable/disable shadows

float ShadowCalculation(vec3 fragPos) {
    // TODO: PCF

    // get vector between light position and current fragment position
    vec3 lightToFrag = fragPos - lightPos;
    // sample from depth map
    float closestDepth = texture(depthMap, lightToFrag).r;
    // scale from [0,1] to [0,far_plane] (to it's original value)
    closestDepth *= far_plane;
    // current linear depth value
    float currentDepth = length(lightToFrag);
    // check if current fragment is in shadow or not
    float bias = 0.05f;  // bias to avoid shadow acne
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // display cubemap for debugging
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;
}

void main() {

    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.5);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;                      
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;   

    BlurColor = vec4(0,0,0,1);
    FragColor = vec4(result, 1.0);
}