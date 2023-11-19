#version 330 core

in vec2 TexCoord;
in vec3 VColor; 

out vec4 FragColor;

uniform sampler2D texture0;
uniform vec2 ScreenSize;

void main()
{
	vec2 uv = gl_FragCoord.xy / ScreenSize;
	FragColor = vec4(VColor,1.0f) * texture(texture0,uv);
}