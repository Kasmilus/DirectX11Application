// Depth buffer

// R channel - regular depth, 0(closer to camera) - 1(further from camera)
// G channel - focus point for depth of field blurring
// B channel - 

cbuffer DOFBuffer : register(cb0)
{
	float focalDistance;
	float focalRange;
	float2 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{

	// Get depth of the pixel, subtract from 1 for better precision on long distances
	float depth = (input.depthPosition.z / input.depthPosition.w);

	// Write grayscale image, the brighter the closer to the camera
    float DOF = saturate(abs(-(1 - depth) * 100 + focalDistance) / focalRange);
    float4 colour = float4(depth, DOF, depth, 1.0f);

	return colour;
}