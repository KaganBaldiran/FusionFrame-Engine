#version 460 core

layout (location = 0) out vec4 AlbedoSpecularPass;
layout (location = 1) out vec4 NormalPass;
layout (location = 3) out vec4 MetalicRoughnessModelIDPass;

in mat4 ModelMat;
in mat4 ViewMat;
in vec4 PositionClipSpace;

uniform sampler2D WorldPositionBuffer;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_metalic0;
uniform sampler2D texture_alpha0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_metalic1;
uniform sampler2D texture_alpha1;

uniform vec4 albedo;
uniform float metallic;
uniform float roughness;

uniform float TilingCoeff;

uniform int disableclaymaterial[5];
uniform float ModelID;

void main()
{
	vec2 screenPos = PositionClipSpace.xy / PositionClipSpace.w;
	screenPos = screenPos * 0.5f + 0.5f; 

    vec4 WorldPosition = texture(WorldPositionBuffer , screenPos);
	vec4 SampledPosition = WorldPosition;
	SampledPosition = inverse(ModelMat) * SampledPosition;
	
	if(abs(SampledPosition.x) > 0.5f || abs(SampledPosition.y) > 0.5f || abs(SampledPosition.z) > 0.5f)
	{
		discard;
	}

    SampledPosition.xy += 0.5f; 

	float AlphaMap;
    if(disableclaymaterial[4] == 1)
    {
      AlphaMap = 1.0f;
    }
    else
    {
      AlphaMap = texture(texture_alpha0, SampledPosition.xy * TilingCoeff).r;
    }

    if(AlphaMap < 0.5f)
    {
      discard;
    }

    vec3 texturecolor;
    if(disableclaymaterial[0] == 1)
    {
      texturecolor = albedo.rgb;
    }
    else
    {
      texturecolor = texture(texture_diffuse0, SampledPosition.xy * TilingCoeff).rgb;
    }

    float roughnessmap;

    if(disableclaymaterial[1] == 1)
    {
      roughnessmap = roughness;
    }
    else
    {
      roughnessmap = texture(texture_specular0, SampledPosition.xy * TilingCoeff).r;
    }

    vec3 resultnormal;
    if(disableclaymaterial[2] != 1)
    {
        resultnormal = texture(texture_normal0,SampledPosition.xy * TilingCoeff).rgb;
        resultnormal = resultnormal * 2.0f - 1.0f;

        vec3 Dx = dFdx(WorldPosition.xyz);
        vec3 Dy = dFdy(WorldPosition.xyz);
        vec3 normal = normalize(cross(Dx,Dy));
        mat3 TBN;
        
        vec3 bitangent = normalize(Dx);
        vec3 tangent = normalize(Dy);
        
        TBN[0] = tangent;
        TBN[1] = bitangent;
        TBN[2] = normal;

        resultnormal = normalize(TBN * resultnormal);
        NormalPass = vec4(resultnormal,1.0f);
    }

    float metalicmap;

    if(disableclaymaterial[3] == 1)
    {
      metalicmap = metallic;
    }
    else
    {
      metalicmap = texture(texture_metalic0, SampledPosition.xy * TilingCoeff).r;
    }

    AlbedoSpecularPass = vec4(texturecolor,1.0f);
    MetalicRoughnessModelIDPass = vec4(roughnessmap ,metalicmap,ModelID,1.0f);
}