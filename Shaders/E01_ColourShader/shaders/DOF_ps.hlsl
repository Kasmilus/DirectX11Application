/*
	Depth of field blur - interpolate between scene texture without blur and the one with blur depending on values stored in DOF depth map
*/
Texture2D sceneTexture : register(t0);
Texture2D depthTexture : register(t1);
Texture2D blurTexture : register(t2);
SamplerState SampleType : register(s0);

cbuffer ScreenSizeBuffer : register(cb0)
{
	float2 screenResolution;
	float2 padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float2 screenPos : TEXCOORD1;
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

	// Calculate final blur
	finalColour = lerp(sceneColour, blurColour, depth);

	// --- VIGNETE --- //
	// Basic
	float2 pos = (input.position.xy / screenResolution) - float2(0.5f, 0.5f);
	float len = length(pos);
	// Circle inside shouldn't be affected
	const float RADIUS = 0.9f;
	const float INNER_RADIUS = 0.5f;
	float vignetteRatio = smoothstep(RADIUS, INNER_RADIUS, len);

	// Add vignette to the blurred image
	finalColour = finalColour * vignetteRatio;

	// Set the alpha channel to one.
	finalColour.a = 1.0f;

	return finalColour;
}