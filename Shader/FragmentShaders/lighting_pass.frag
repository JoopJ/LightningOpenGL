#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform samplerCubeArray depthMapArray;

const int MAX_NUM_LIGHTS = 10;
uniform vec3 lightPositions[MAX_NUM_LIGHTS];    // light positions and depth cubemaps
uniform vec3 viewPos;
uniform float far_plane;
uniform int numLightsActive;
uniform bool shadows;   // Toggle shadows

// attenuation and color parameters are constatnt for all lights 
uniform float Linear;
uniform float Quadratic;
uniform vec3 lightColor;

float ShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCubeArray depthMap, int lightIndex);

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    vec3 ambient = 0.05 * Diffuse;
    
    // then calculate lighting
    vec3 lighting = vec3(0);
    vec3 viewDir = normalize(viewPos - FragPos);
    for(int i = 0; i < numLightsActive; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
        
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = Specular * spec * lightColor;

        // attenuation
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (1.0 + Linear * distance + Quadratic * distance * distance);

        // calculate shadow
        float shadow = shadows ? ShadowCalculation(FragPos, lightPositions[i], depthMapArray, i) : 0.0;

        diffuse *= attenuation;
        specular *= attenuation;
        //lighting += diffuse + specular;
        lighting += (ambient + (1.0 - shadow) * (diffuse + specular));
    }

    // FragColor = vec4(FragPos, 1.0); // visualize positions
    // FragColor = vec4(Normal, 1.0); // visualize normals
    // FragColor = vec4(Diffuse, 1.0); // visualize diffuse
    lighting = lighting / float(numLightsActive); // average the light colors
    FragColor = vec4(lighting, 1.0);
}

// shadow calculation for a single light, from an array of depth cubemaps
float ShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCubeArray depthMapArray, int lightIndex) {
    // get the vector betweent the fragment position and the light positions
    vec3 lightToFrag = fragPos-lightPos;

    // use the vector to sample from the depthMap
    float closestDepth = texture(depthMapArray, vec4(lightToFrag, lightIndex)).r;

    // closestDepth is in linear range [0,1] so we need to convert it to 
    // the original depth value[0, far_plane]
    closestDepth *= far_plane;

    // get the current linear depth from the length between light and 
    // fragment positions
    float currentDepth = length(lightToFrag);

    // test for shadow:
    float bias = 0.05; // bias is larger since depth is in [near_plane, far_plane] range
    // if the currentDepth is larger (further from lightPos) than the closestDepth, then
    // the fragment is in shadow.
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // display closestDepth to visualize the depth cubemap
    // FragColor = vec4(vec3(closestDepth/far_plane), 1.0);

    return shadow;
}
