#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D Viewport;
uniform sampler2D DepthAttac;

uniform float FarPlane;
uniform float NearPlane;
uniform vec3 CamPos;

uniform float DOFdistanceFar;
uniform float DOFdistanceClose;
uniform bool DOFenabled;
uniform bool Debug;
uniform float DOFintensity;
uniform float Gamma;
uniform float Exposure;

void main()
{ 
    vec4 OutColor;
	if(DOFenabled)
    {
	    float DeltaDistance = FarPlane - NearPlane;
        float CamDistance = distance(CamPos,texture(DepthAttac, TexCoords).xyz)/DeltaDistance;
		
		if(CamDistance >= DOFdistanceFar || CamDistance <= DOFdistanceClose)
		{
			vec2 texelSize = 1.0f / vec2(textureSize(Viewport,0));
			vec3 result = vec3(0.0f);
            int TraversePixelCount = int(2.0f * DOFintensity); 
			for(int x = -TraversePixelCount ; x < TraversePixelCount ; ++x)
			{
				for(int y = -TraversePixelCount ; y < TraversePixelCount ; ++y)
				{
					vec2 offset = vec2(float(x), float(y)) * texelSize;
                    vec2 sampleCoords = TexCoords + offset;
                    sampleCoords = clamp(sampleCoords, 0.0, 1.0);
					result += texture(Viewport, sampleCoords).xyz; 
				}
			}
			result = result / vec3(pow(TraversePixelCount + TraversePixelCount,2));
            OutColor = vec4(result,1.0f);
		}
		else
		{ 
		   OutColor = texture(Viewport, TexCoords);
		}
	}
    else
	{
      OutColor = texture(Viewport, TexCoords);
    }
	
    OutColor.xyz = texture(Viewport, TexCoords.xy).xyz;
	OutColor.xyz = vec3(1.0) - exp(-OutColor.xyz * Exposure);
    FragColor = vec4(pow(OutColor.xyz,vec3(1.0 / Gamma)),1.0f); 
}