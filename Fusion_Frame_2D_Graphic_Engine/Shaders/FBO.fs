#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D Viewport;
uniform sampler2D DepthAttac;
uniform sampler2D ID;

uniform float FarPlane;
uniform float NearPlane;
uniform vec3 CamPos;

uniform float DOFdistance;
uniform bool DOFenabled;
uniform float DOFintensity;

void main()
{ 
    
	if(DOFenabled)
    {
	    float DeltaDistance = FarPlane - NearPlane;
        float CamDistance = distance(CamPos,texture(DepthAttac, TexCoords).xyz)/DeltaDistance;

		if(CamDistance >= DOFdistance)
		{
			vec2 texelSize = DOFintensity / vec2(textureSize(Viewport,0));
			vec3 result = vec3(0.0f);
			for(int x = -2 ; x < 2 ; ++x)
			{
				for(int y = -2 ; y < 2 ; ++y)
				{
					vec2 offset = vec2(float(x) , float(y)) * texelSize;
					result += texture(Viewport,TexCoords + offset).xyz; 
				}
			}

			result = result / vec3(4.0f * 4.0f);
			FragColor = vec4(pow(result.xyz.xyz,vec3(0.9)),1.0f);
		}
		else
		{ 
		   vec4 OutColor = texture(Viewport, TexCoords);
		   FragColor = vec4(pow(OutColor.xyz.xyz,vec3(0.9)),OutColor.w);
		}
	}
    else
	{
      vec4 OutColor = texture(Viewport, TexCoords);
      FragColor = vec4(pow(OutColor.xyz.xyz,vec3(0.9)),OutColor.w);

	  //float OutColor = texture(ID , TexCoords).r / 10.0f;
      //FragColor = vec4(pow(vec3(OutColor),vec3(0.9)),1.0f);
    }
}