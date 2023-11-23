#version 330 core

layout (location = 0) in  vec3 PixelOffset;
//layout (location = 1) in  vec4 PixelColor;
 
out vec4 pixelColor;

uniform mat4 CamMat;
uniform vec2 ScreenSize;

void main()
{

  pixelColor = vec4(1.0f);
  gl_Position = CamMat * vec4(PixelOffset.x ,PixelOffset.y, 0.5f,1.0f);

}