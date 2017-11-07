// Object pixel/fragment shader

Texture2D shaderTextures[5] : register(t0); // 0 - base, 1 - normal, 2 - metallic, 3 - roughness, 4 - shadow
SamplerState SampleType : register(s0);
SamplerState SampleTypeClampPoint : register(s1);
SamplerComparisonState SampleTypeComparison : register(s2);

// Light Buffer
cbuffer LightBuffer : register(cb0)
{
	float4 lightDiffuseColour;
	float4 lightAmbientColour;
	float3 lightPosition;
	float padding;
};

// Input
struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 viewDirection : TEXCOORD1;
	float4 lightViewPosition : TEXCOORD2;
	float3 lightPos : TEXCOORD3;
};

























#define BLOCKER_SEARCH_NUM_SAMPLES 16
#define PCF_NUM_SAMPLES 16
#define NEAR_PLANE 9.5
#define LIGHT_WORLD_SIZE 1.5
#define LIGHT_FRUSTUM_WIDTH 3.75
// Assuming that LIGHT_FRUSTUM_WIDTH == LIGHT_FRUSTUM_HEIGHT
#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH)

float2 poissonDisk[16] = {
	float2(-0.94201624, -0.39906216),
	float2(0.94558609, -0.76890725),
	float2(-0.094184101, -0.92938870),
	float2(0.34495938, 0.29387760),
	float2(-0.91588581, 0.45771432),
	float2(-0.81544232, -0.87912464),
	float2(-0.38277543, 0.27676845),
	float2(0.97484398, 0.75648379),
	float2(0.44323325, -0.97511554),
	float2(0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023),
	float2(0.79197514, 0.19090188),
	float2(-0.24188840, 0.99706507),
	float2(-0.81409955, 0.91437590),
	float2(0.19984126, 0.78641367),
	float2(0.14383161, -0.14100790)
};


float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation
{
	return (zReceiver - zBlocker) / zBlocker;
}
void FindBlocker(out float avgBlockerDepth,
	out float numBlockers,
	float2 uv, float zReceiver)
{
	Texture2D shadowMapTex = shaderTextures[4];
	//This uses similar triangles to compute what
	//area of the shadow map we should search
	float searchWidth = LIGHT_SIZE_UV * (zReceiver - NEAR_PLANE) / zReceiver;
	float blockerSum = 0;
	numBlockers = 0;

	for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i)
	{
		float shadowMapDepth = shadowMapTex.SampleLevel(
			SampleTypeClampPoint,
			uv + poissonDisk[i] * searchWidth,
			0);
		if (shadowMapDepth < zReceiver) {
			blockerSum += shadowMapDepth;
			numBlockers++;
		}
	}
	avgBlockerDepth = blockerSum / numBlockers;
}
float PCF_Filter(float2 uv, float zReceiver, float filterRadiusUV)
{
	Texture2D shadowMapTex = shaderTextures[4];

	float sum = 0.0f;
	for (int i = 0; i < PCF_NUM_SAMPLES; ++i)
	{
		float2 offset = poissonDisk[i] * filterRadiusUV;
		sum += shadowMapTex.SampleCmpLevelZero(SampleTypeComparison, uv + offset, zReceiver);
	}
	return sum / PCF_NUM_SAMPLES;
}
float PCSS(float4 coords)
{
	float2 uv = coords.xy;
	float zReceiver = coords.z; // Assumed to be eye-space z in this code

								// STEP 1: blocker search
	float avgBlockerDepth = 0;
	float numBlockers = 0;
	FindBlocker(avgBlockerDepth, numBlockers, uv, zReceiver);
	if (numBlockers < 1)
		//There are no occluders so early out (this saves filtering)
		return 1.0f;
	// STEP 2: penumbra size
	float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);
	float filterRadiusUV = penumbraRatio * LIGHT_SIZE_UV * NEAR_PLANE / coords.z;

	// STEP 3: filtering
	return PCF_Filter(uv, zReceiver, filterRadiusUV);
}





































float4 main(InputType input) : SV_TARGET
{
	// Textures
	Texture2D textureBase = shaderTextures[0];
	Texture2D textureNormal = shaderTextures[1];
	Texture2D textureMetallic = shaderTextures[2];
	Texture2D textureRoughness = shaderTextures[3];
	// Textures samples for this fragment
	float4 baseColour = textureBase.Sample(SampleType, input.tex);
	float3 normalSample = textureNormal.Sample(SampleType, input.tex).rgb;
	float roughness = textureRoughness.Sample(SampleType, input.tex).r;
	float metallic = textureRoughness.Sample(SampleType, input.tex).r;
	// Light
	float lightIntensity = 0;
	float4 specular = float4(0,0,0,0);
	float4 colour = float4(0,0,0,0);
	float4 ambientColour = float4(0, 0, 0, 0);
	float4 lightingColour = float4(0, 0, 0, 0);
	// Shadows
	float bias;
	float4 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	/*
	bias = 0.0005f;

	// Calculate prjected coordinates, then into UV range
	projectTexCoord.xyz = input.lightViewPosition.xyz / input.lightViewPosition.z;

	// Calculate the projected texture coordinates.
	projectTexCoord.x = (projectTexCoord.x / 2.0f) + 0.5f;
	projectTexCoord.y = (-projectTexCoord.y / 2.0f) + 0.5f;

	// Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
	//if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	//{
		// Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
		//depthValue = shaderTextures[4].Sample(SampleTypeClampPoint, projectTexCoord).x;
		//projectTexCoord.z = input.lightViewPosition.z;
		depthValue = PCSS(projectTexCoord);

		// Calculate the depth of the light.
		lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		// Subtract the bias from the lightDepthValue.
		lightDepthValue = lightDepthValue - bias;
		*/
		// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
		// If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
		//if (lightDepthValue < depthValue)
		//{
			// ----- NORMAL MAPPING ----- //
			// Move normal sample into the range of [-1, 1]
			normalSample = (normalSample * 2.0f) - 1.0f;
			// Calculate normal for this fragment
			float3 normal = (normalSample.x * input.tangent) + (normalSample.y * input.binormal) + (normalSample.z * input.normal);
			normal = normalize(normal);

			// ----- NORMAL MAPPING ----- //
			// ----- LIGHTING ----- //
			//float3 lightDir = input.viewDirection - lightPosition;
			lightIntensity = saturate(dot(normal, -lightPosition));
			//lightIntensity = 1;	// testing shadows
			if (lightIntensity > 0)
			{
				// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
				float3 reflection = normalize(2 * lightIntensity * normal + lightPosition);

				// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
				float specularPower = 100;	// That should come from light
				specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
			}
		//}
	//}
	// ----- LIGHTING ----- //
	

	// Final colour calculations

	ambientColour = saturate(baseColour * lightAmbientColour);
	lightingColour = saturate(baseColour * lightDiffuseColour * lightIntensity);
	specular = specular * metallic;
	colour = saturate(lightingColour + specular + ambientColour);
	//colour = saturate(lightingColour + ambientColour);
	//colour = float4(depthValue, depthValue, depthValue, 1);
	//colour = specular;
	//colour = float4(input.binormal, 1);
	//colour = float4(input.tex, 0, 1);
	//colour = float4(normal, 1.0f);
	
	return colour;
}































