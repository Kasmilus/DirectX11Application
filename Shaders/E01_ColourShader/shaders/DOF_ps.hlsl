/*
	Depth of field blur - interpolate between scene texture without blur and the one with blur depending on values stored in DOF depth map
*/
Texture2D sceneTexture : register(t0);
Texture2D depthTexture : register(t1);
Texture2D blurTexture : register(t2);
SamplerState SampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
	float4 finalColour;
	float4 sceneColour;
	float4 blurColour;
	float depth;

	// Get pixel colour in all textures
	sceneColour = sceneTexture.Sample(SampleType, input.tex);
	blurColour = blurTexture.Sample(SampleType, input.tex);
	depth = depthTexture.Sample(SampleType, input.tex).y;

	// Calculate final colour
	finalColour = lerp(sceneColour, blurColour, depth);

	// Set the alpha channel to one.
	finalColour.a = 1.0f;

	return finalColour;
}