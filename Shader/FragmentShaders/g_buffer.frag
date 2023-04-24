#version 460 core 

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gWorldPosition;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;

uniform bool useDiffuse;
uniform bool useNormal;
uniform bool useSpecular;

uniform vec3 color;

void main()
{    
    // Position
    gPosition = FragPos;

    // Normal
    if (useNormal) {
            gNormal = texture(texture_normal1, TexCoords).rgb;
    } else {
        gNormal = normalize(Normal);
	}

    // Diffuse
    if (useDiffuse) {
		gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
	} else {
        gAlbedoSpec.rgb = color;
    }

    // Specualar
    if (useSpecular) {
		gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
	} else {
		gAlbedoSpec.a = 1.0f;
	}
}