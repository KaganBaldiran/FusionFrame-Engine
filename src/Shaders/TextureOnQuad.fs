#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D Viewport;
uniform vec2 SampleMultiplier;

uniform float Gamma;
uniform float Exposure;

void main()
{ 
    vec4 OutColor;
    OutColor.xyz = texture(Viewport, SampleMultiplier * TexCoords.xy).xyz;
	OutColor.xyz = vec3(1.0) - exp(-OutColor.xyz * Exposure);
    FragColor = vec4(pow(OutColor.xyz,vec3(1.0 / Gamma)),1.0f); 
}