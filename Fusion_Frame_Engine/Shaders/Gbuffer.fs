#version 420 core

layout (location = 0) out vec4 AlbedoSpecularPass;
layout (location = 1) out vec4 NormalMetalicPass;
layout (location = 2) out vec4 PositionDepthPass;
layout (location = 3) out vec4 MetalicRoughnessPass;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

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

void main()
{
    
    float AlphaMap;
    if(disableclaymaterial[4] == 1)
    {
      AlphaMap = 1.0f;
    }
    else
    {
      AlphaMap = texture(texture_alpha0, FinalTexCoord * TilingCoeff).r;
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
      texturecolor = texture(texture_diffuse0, FinalTexCoord * TilingCoeff).rgb;
    }

    float roughnessmap;

    if(disableclaymaterial[1] == 1)
    {
      roughnessmap = roughness;
    }
    else
    {
      roughnessmap = texture(texture_specular0, FinalTexCoord * TilingCoeff).r;
    }

    vec3 resultnormal;

    if(disableclaymaterial[2] == 1)
    {
        resultnormal = normalize(Normal);
    }
    else
    {
        resultnormal = texture(texture_normal0,FinalTexCoord * TilingCoeff).rgb;
        resultnormal = resultnormal * 2.0f - 1.0f;
        resultnormal = normalize(TBN * resultnormal);
    }

    float metalicmap;

    if(disableclaymaterial[3] == 1)
    {
      metalicmap = metallic;
    }
    else
    {
      metalicmap = texture(texture_metalic0, FinalTexCoord * TilingCoeff).r;
    }

    AlbedoSpecularPass = vec4(texturecolor,1.0f);
    NormalMetalicPass = vec4(resultnormal,1.0f);
    PositionDepthPass = vec4(CurrentPos,1.0f);
    MetalicRoughnessPass = vec4(roughnessmap ,metalicmap,1.0f,1.0f);
}