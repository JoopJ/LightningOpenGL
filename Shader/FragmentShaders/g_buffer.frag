#version 460 core 

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gWorldPosition;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1_diffuse;
uniform sampler2D texture2_diffuse;
uniform int useTexture;
uniform vec3 color;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    // also store the per-fragment normals and diffuse:
    gNormal = normalize(Normal);
    // select the texture to use
    if (useTexture == 1) {
		gAlbedoSpec.rgb = texture(texture1_diffuse, TexCoords).rgb;
	} else if (useTexture == 2) {
        gAlbedoSpec.rgb = texture(texture2_diffuse, TexCoords).rgb;
    } else {
        // if not a texture, use the color uniform
        gAlbedoSpec.rgb = color;
    }

    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = 1.0f;
}