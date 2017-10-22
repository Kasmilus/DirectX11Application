// Object pixel/fragment shader

Texture2D shaderTextures[4] : register(t0); // 0 - base, 1 - normal, 2 - metallic, 3 - roughness
SamplerState SampleType : register(s0);

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
};


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

	// ----- NORMAL MAPPING ----- //
	// Move normal sample into the range of [-1, 1]
	normalSample = (normalSample * 2.0f) - 1.0f;
	// Calculate normal for this fragment
	float3 normal = (normalSample.x * input.tangent) + (normalSample.y * input.binormal) + (normalSample.z * input.normal);
	normal = normalize(normal);

	// ----- NORMAL MAPPING ----- //
	// ----- LIGHTING ----- //
	//float3 lightDir = input.viewDirection - lightPosition;
	float lightIntensity = saturate(dot(normal, -lightPosition));
	float4 specular = 0;
	if (lightIntensity > 0)
	{
		// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
		float3 reflection = normalize(2 * lightIntensity * normal + lightPosition);

		// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
		float specularPower = 10;	// That should come from light
		specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
	}

	// ----- LIGHTING ----- //


	// Final colour calculations
	float4 ambientColour = saturate(baseColour * lightAmbientColour);
	float4 lightingColour = saturate(baseColour * lightDiffuseColour * lightIntensity);
	specular = specular * metallic;
	float4 colour = saturate(lightingColour + specular + ambientColour);
	//colour = float4(normal, 1);
	return colour;
}