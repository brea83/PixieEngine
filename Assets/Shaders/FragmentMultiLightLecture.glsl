#version 450 core
#define MAX_LIGHTS 6

layout(location = 1) out vec3 screenSpacePos;

layout (std140, binding = 1) uniform LightProjectionBlock
{
    vec4 mainLightPosition;
    mat4 lightViewMat;
    mat4 lightProjMat;
};

//// lightType enum in Component.h
// enum LightType
//    {
//        Directional, 0
//        Point,       1
//        Spot,        2
//        // END used for imgui combo windows
//        END
//    };
// MID GREEN DESATURATED (0.4, 0.5, 0.2, 1)
struct MaterialData
{
	sampler2D ColorTexture;
	vec3 BaseColor;

	sampler2D NormalMap;
	bool BUseNormalMap;

	sampler2D MetallicMap;
	bool BUseMetallicMap;
	bool BMapIsRoughness;

	vec3 Ao;
	
	float Smoothness;
	float SpecularPower;
};


out vec4 FragColor;

in VS_OUT
{
   vec3 Pos_WS;
   vec3 Pos_TS;
   vec3 Pos_CS;
   vec4 Pos_LS;
   vec3 Normal_WS;
   vec3 Normal_CS;
   vec2 UV;
   vec3 VertexColor;
   vec3 CameraPos_WS;
   vec3 CameraPos_TS; 
   vec3 EyeDirection_CS;
   mat3 TBN;
} IN;

uniform vec4 baseColor = vec4 (1.0, 1.0, 1.0, 1.0);

uniform MaterialData Material;

//uniform bool bUseShadowMap;
uniform sampler2D shadowMap;
uniform float lightNearPlane;
uniform float lightFarPlane;
uniform float shadowBiasMult = 0.000059; // values under 1 seem better, below zero creates a lot of shadow acne

uniform vec4 ambientLight = vec4(0.05, 0.05, 0.05, 1.0);
uniform int  lightTypes[MAX_LIGHTS];
uniform vec3 lightPosition[MAX_LIGHTS];
uniform vec3 lightDirection[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];

// light attnuation X constant Y linear, Z quadratic
uniform vec3 lightAttenuation[MAX_LIGHTS];
uniform float innerRadius[MAX_LIGHTS];
uniform float outerRadius[MAX_LIGHTS];

//uniform vec3 mainLightPosition;
uniform int activeLights;
//uniform DirectionalLightData MainLight;
uniform bool BUseLights;

uniform float Gamma = 2.0;// og srgb is 2.2

// required to view depth for debuging when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * lightNearPlane * lightFarPlane) / (lightFarPlane + lightNearPlane - z * (lightFarPlane - lightNearPlane));	
}

float ShadowCalculation(vec4 fragPosLightSpace, float nDotL)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	if(projCoords.z > 1)
	{
		//float shadow = 0.0;
		return 0.0;
	}

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	
	vec3 lightDir = normalize((mainLightPosition.xyz) - IN.Pos_WS);
    //float bias = max(0.05 * (1.0 - dot(normalizedNormal, lightDir)), 0.005);
	float bias = tan(acos(nDotL));
	bias *= shadowBiasMult;
	bias = clamp(bias, 0.0, 0.1);
    // check whether current frag pos is in shadow with edge softening 
	float shadow = 0.0;
//	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//	for(int x = -1; x <= 1; x++)
//	{
//		for(int y = -1; y <= 1; y++)
//		{
//			float depth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
//			shadow += currentDepth - bias > depth ? 1.0 : 0.0;
//			
//		}
//	}
//	shadow /= 9.0;
     
	shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

// until I start doing more pbr like stuff assume the diffuse intensity is just a little less than full reflection
vec3 GetPointLuminosity(float nDotL, vec3 lightColor, float lightDistance, vec3 attenuationConstants)
{
	float attenuation = 1.0 / (attenuationConstants.x + (attenuationConstants.y * lightDistance) + (attenuationConstants.z * pow(lightDistance, 2)));
	return  nDotL * lightColor * attenuation * 0.8;
}

vec3 GetDirectionalLuminosity(float nDotL, vec3 lightColor)
{
	return  nDotL * lightColor * 0.8;
}

vec3 GetSpotLuminosity(float nDotL, vec3 lightColor, vec3 attenuation, float cutoff)
{
	
	return vec3(0,0,0);
}

// until I start doing more pbr assume light intensity on spec is about a third of the main light color
vec3 GetPointSpecular(float nDotH, vec3 lightColor, float smoothness, float specularPower, float lightDistance, vec3 attenuationConstants)
{
	float attenuation = 1.0 / (attenuationConstants.x + (attenuationConstants.y * lightDistance) + (attenuationConstants.z * pow(lightDistance, 2)));
	float specBrightness = pow(max(nDotH, 0.0), specularPower * smoothness);

			
	return specBrightness * (lightColor * 0.3) * attenuation;
}

vec3 GetDirectionalSpecular(float nDotH, vec3 lightColor, float smoothness, float specularPower)
{
	float specBrightness = pow(max(nDotH, 0.0), smoothness * specularPower);

	return specBrightness * (lightColor * 0.3);

}

vec3 GetSpotSpecular(float nDotH, vec3 lightColor, float smoothness, float specularPower, float lightDistance, vec3 attenuation, float cutoff)
{
	return vec3(0,0,0);
}

void main()
{
	screenSpacePos = gl_FragCoord.xyz;
	FragColor = vec4(0, 0, 0, 1);
	vec3 textureColor = texture(Material.ColorTexture, IN.UV).rgb;
	//textureColor = pow(textureColor, vec3(Gamma));

	//FragColor.xyz += ambientLight.xyz ; // mult in Material ambience when I have it
	if(BUseLights)
	{
	// set up Material stuff that is the same for all light types
		float smoothness = 1;
		vec3 diffuse = vec3(0, 0, 0);
		vec3 specular = vec3(0, 0, 0);

		if(Material.BUseMetallicMap)
		{
			float mapSmoothness = 1.0 - texture(Material.MetallicMap, IN.UV).g;
			//mapSmoothness = Material.bMapIsRoughness ? 1.0 - mapSmoothness : mapSmoothness;
			smoothness = mapSmoothness * Material.Smoothness;
		}
		else
		{
			smoothness = Material.Smoothness;
		}
		smoothness = clamp(smoothness, 0.1, 1.0);

		// ambient lighting
		//FragColor.rgb += ambientLight.xyz * textureColor;

		// set up for diffuse and specular lighting

		vec3 N;
		vec3 V;

		if(Material.BUseNormalMap)
		{
			vec3 sampledNormal = texture(Material.NormalMap, IN.UV).rgb;
			sampledNormal = normalize((sampledNormal * 2.0) - 1.0);
			N = sampledNormal;
			V = IN.TBN * normalize(IN.CameraPos_WS.xyz - IN.Pos_WS);
		}
		else
		{
			N = normalize(IN.Normal_WS);
			V = normalize(IN.CameraPos_WS.xyz - IN.Pos_WS);
		}

		vec3 accumulatedDiffuse = vec3(0,0,0);
		vec3 accumulatedSpecular = vec3(0,0,0);

		float shadowMask = 0.0; // 1 == shadow 0 == no shadow

		// light loop
		for(int i = 0; i < activeLights; i++)
		{
			if(lightTypes[i] == 0) // is directional light
			{
				vec3 direction = normalize(-lightDirection[i]);
				if(Material.BUseNormalMap)
				{
					direction = IN.TBN * direction;
					direction = normalize(direction);
				}

				float nDotL = clamp(dot(N, direction), 0.0, 1.0);
				vec3 H = normalize((direction + V));
				float nDotH = clamp(dot(N, H), 0.0, 1.0);

				shadowMask = ShadowCalculation(IN.Pos_LS, nDotL);

				accumulatedDiffuse += GetDirectionalLuminosity(nDotL, lightColor[i]);
				accumulatedSpecular += GetDirectionalSpecular(nDotH, lightColor[i], smoothness, Material.SpecularPower);
				continue;
			}

			vec3 direction =  normalize(lightPosition[i] - IN.Pos_WS);
			if(Material.BUseNormalMap)
			{
				direction = IN.TBN * direction;
				direction = normalize(direction);
			}

			float nDotL = clamp(dot(N, direction), 0.0, 1.0);

			vec3 H = normalize((direction + V));
			float nDotH = max(dot(N, H), 0.0);
			float lightDistance = length(lightPosition[i] - IN.Pos_WS);

			if(lightTypes[i] == 1) // is point light
			{
				accumulatedDiffuse += GetPointLuminosity(nDotL, lightColor[i], lightDistance, lightAttenuation[i]);
				accumulatedSpecular += GetPointSpecular(nDotH, lightColor[i], smoothness, Material.SpecularPower, lightDistance, lightAttenuation[i]);
				continue;
			}

			if(lightTypes[i] == 2) // is spot light
			{
				float theta = dot(direction, normalize(-lightDirection[i]));
				float epsilon = innerRadius[i] - outerRadius[i];
				float intensity = clamp((theta - outerRadius[i]) / epsilon, 0.0, 1.0);

				accumulatedDiffuse +=  intensity * GetPointLuminosity(nDotL, lightColor[i], lightDistance, lightAttenuation[i]);
				accumulatedSpecular += intensity * GetPointSpecular(nDotH, lightColor[i], smoothness, Material.SpecularPower, lightDistance, lightAttenuation[i]);
				continue;
			}
			
		}	
		FragColor.rgb = (ambientLight.xyz + (1.0 - shadowMask) * (accumulatedDiffuse + accumulatedSpecular)) * textureColor;
		//FragColor.rgb = (ambientLight.xyz + (accumulatedDiffuse + accumulatedSpecular)) * textureColor;

	}
	else
	{
		FragColor.rgb = textureColor;
	}

	
    FragColor.rgb = clamp(pow(FragColor.rgb, vec3(1.0/Gamma)), 0.0, 1.0);
}