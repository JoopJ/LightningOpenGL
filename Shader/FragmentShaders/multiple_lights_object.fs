#version 460 core 

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;
};

struct PointLightProperties {
    // point light options
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    // attenuation
    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 100

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform PointLightProperties pointLightProperties;

vec3 CalcPointLight(PointLight light, PointLightProperties properties, vec3 normal, vec3 FragPos, vec3 viewDir);

void main() {
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], pointLightProperties, norm, FragPos, viewDir);

    // check whether result is higher than some threshold, if so, output as bloom threshol color
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else 
        BrightColor = vec4(0, 0, 0, 1);

    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, PointLightProperties properties, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDiff = light.position - fragPos;
    vec3 lightDir = normalize(lightDiff);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(lightDiff);
    float attenuation = 1.0 / (properties.constant + properties.linear * distance + properties.quadratic * (distance * distance));
   
    // combine results
    vec3 ambient = properties.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = properties.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = properties.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}
