#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D Viewport;
uniform sampler2D DepthAttac;
uniform sampler2D SSRtexture;
uniform sampler2D IDtexture;
uniform sampler2D TracedImage;

uniform sampler2DArray CascadeShadowMaps1024;

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
	//OutColor += texture(Viewport,texture(SSRtexture, TexCoords).xy);
    //OutColor /= 2.0f;
   // vec4 ReflectionUVcoords = texture(SSRtexture, TexCoords);
	vec3 TracedUV = texture(TracedImage, TexCoords.xy).xyz;
	OutColor.xyz =  TracedUV;
	if(Debug)
	{
      //FragColor = vec4(vec3(texture(CascadeShadowMaps1024,vec3(TexCoords,0)).r),1.0f); 
	  OutColor.xyz =  mix(texture(Viewport, TexCoords.xy).xyz,TracedUV,0.8f);
	}
    
	OutColor.xyz = vec3(1.0) - exp(-OutColor.xyz * Exposure);
    FragColor = vec4(pow(OutColor.xyz,vec3(1.0 / Gamma)),1.0f); 
	//vec2 Coords = TexCoords * 0.5f + vec2(0.5f,0.0f);

	//FragColor = vec4(texture(Viewport, ReflectionUVcoords.xy).xyz,1.0f);
    //FragColor = vec4(texture(SSRtexture, TexCoords).xyz,1.0f);
    //FragColor = vec4(vec3(texture(IDtexture, TexCoords).x / 10.0f),1.0f);
}