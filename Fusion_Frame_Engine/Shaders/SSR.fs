#version 460 core

layout(location = 0) out vec4 FragColor;

noperspective  in vec2 TexCoords;
uniform sampler2D AlbedoSpecularPass;
uniform sampler2D NormalMetalicPass;
uniform sampler2D PositionDepthPass;
uniform sampler2D MetalicRoughnessPass;

uniform float FarPlane;
uniform float NearPlane;
uniform vec3 CameraPos;
uniform vec2 WindowSize;

uniform mat4 ViewMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
uniform mat4 ProjectionMatrix;

#define RAY_STEP 0.1f
#define MIN_RAY_STEP 0.1f
#define MAX_RAY_STEP 50
#define SEARCH_DISTANCE 5
#define BINARY_SEARCH_STEPS 10
#define MAX_DISTANCE 1000.0f
#define Scale vec3(.8, .8, .8)
const float reflectionSpecularFalloffExponent = 3.0;
#define K 19.19

vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}

vec3 BinarySearch(inout vec3 direction, inout vec3 HitCoord, inout float dDepth)
{
   float depth;
   vec4 ProjectedCoord;
   
   for(int i = 0; i < BINARY_SEARCH_STEPS; i++)
   {
        ProjectedCoord = ProjectionMatrix * vec4(HitCoord, 1.0);
        ProjectedCoord.xy /= ProjectedCoord.w;
        ProjectedCoord.xy = ProjectedCoord.xy * 0.5 + 0.5;
   
        vec4 WorldPos = texture(PositionDepthPass, ProjectedCoord.xy);
		WorldPos = ViewMatrix * vec4(WorldPos.xyz , 1.0f);
	    //WorldPos = WorldPos / WorldPos.w;
        depth = WorldPos.z;

        dDepth = HitCoord.z - depth;
		
		direction *= 0.5f;
		
		if(dDepth > 0.0f)
		{
		  HitCoord += direction;
		}
		else
		{
		  HitCoord -= direction;
		}
   }
   
   ProjectedCoord = ProjectionMatrix * vec4(HitCoord, 1.0);
   ProjectedCoord.xy /= ProjectedCoord.w;
   ProjectedCoord.xy = ProjectedCoord.xy * 0.5 + 0.5;
   
   return vec3(ProjectedCoord.xy , depth);
}

vec4 RayCast(in vec3 direction , inout vec3 HitCoord , out float ReflectionDepth)
{
   direction *= RAY_STEP;
   
   float Depth = 0.0f;
   int steps = 0;
   vec4 ProjectedCoord = vec4(0.0f);

   for(int i=0; i < MAX_RAY_STEP ; i++)
   {
      HitCoord += direction;
	  
	  ProjectedCoord = ProjectionMatrix * vec4(HitCoord , 1.0f);
	  ProjectedCoord.xy = ProjectedCoord.xy / ProjectedCoord.w;
	  ProjectedCoord.xy = ProjectedCoord.xy * 0.5f + 0.5f;
	  
	  vec4 WorldPos = texture(PositionDepthPass, ProjectedCoord.xy);
      WorldPos = ViewMatrix * vec4(WorldPos.xyz , 1.0f);
	  //WorldPos = WorldPos / WorldPos.w;
	  Depth = WorldPos.z;
	  
      if(Depth > MAX_DISTANCE)
	  {
	    continue;
	  }
	  
	  ReflectionDepth = HitCoord.z - Depth; 
	  
	  if((direction.z - ReflectionDepth) < 1.2)
	  {
	     if(ReflectionDepth <= 0.0f)
		 {
		    vec4 Result;
			Result = vec4(BinarySearch(direction,HitCoord,ReflectionDepth),1.0f);
		    return Result;
		 }
	  }
	  
   }
   return vec4(ProjectedCoord.xy,Depth,0.0f);
}

void main()
{
   vec2 texCoord = gl_FragCoord.xy / WindowSize;

   vec4 MetalicRoughness = texture(MetalicRoughnessPass, TexCoords);
   float SceneAlpha = MetalicRoughness.b;

   if(SceneAlpha < 1.0f)
   {
      discard;
   }

   vec4 AlbedoSpecular = texture(AlbedoSpecularPass, TexCoords);
   vec4 NormalMetalic = texture(NormalMetalicPass, TexCoords);
   vec4 PositionDepth = texture(PositionDepthPass, TexCoords);

   vec3 Albedo = AlbedoSpecular.rgb;
   vec3 Normal = NormalMetalic.rgb;
   float roughness = MetalicRoughness.r;
   float Metallic = MetalicRoughness.g;
   vec3 Position = PositionDepth.rgb;

   //if(Metalic < 0.1)
        //discard;

   vec4 PosView = ViewMatrix * vec4(Position,1.0f);
   //Normal = Normal * 0.5f + 0.5f;
   vec3 NormalView = transpose(inverse(mat3(ViewMatrix))) * Normal;
   //vec3 NormalView =  Normal;

   //vec4 NormalView = ViewMatrix * vec4(Normal,1.0f);
   //vec4 NormalView = transpose(InverseViewMatrix) * vec4(Normal,1.0f);
   //vec3 NormalView = vec3(vec4(Normal,1.0f) * InverseViewMatrix);
   //NormalView = NormalView * 0.5f + 0.5f;

   vec3 Reflected = normalize(reflect(normalize(PosView.xyz),normalize(NormalView.xyz)).xyz);
   vec3 HitPosition = PosView.xyz + NormalView * 0.005f;

   float ReflectionDepth = 0.0f;

   //vec3 Reflection = texture(AlbedoSpecularPass,).xyz;
   vec3 jitt = mix(vec3(0.0), vec3(hash(Position)), roughness);
   vec4 Coords = vec4(RayCast(normalize(vec3(jitt) + Reflected * max(MIN_RAY_STEP, -PosView.z)),HitPosition,ReflectionDepth).xyz,1.0f);
   
   vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - Coords.xy));
 
 
   float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

   float ReflectionMultiplier =
                screenEdgefactor * 
                -Reflected.z;

   //FragColor = vec4(vec3(ReflectionMultiplier),1.0f);
   //FragColor = vec4(vec3(ReflectionMultiplier),1.0f);
   FragColor = Coords;
   //FragColor = vec4(NormalView.xyz,1.0f);
}