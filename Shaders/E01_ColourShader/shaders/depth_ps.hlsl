// Depth buffer

// R channel - regular depth, 0(closer to camera) - 1(further from camera)
// G channel - focus point for depth of field blurring
// B channel - depth

cbuffer DOFBuffer : register(cb0)
{
	float focalDistance;
	float focalRange;
	float2 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
	
	// Get depth of the pixel, subtract from 1 for better precision on long distances
    float depth = 1 - (input.depthPosition.z / input.depthPosition.w);
    float depthInv = (input.depthPosition.z / input.depthPosition.w);
	// Logarythmic z-buffer
    //depth = 1 - (log(1 * input.depthPosition.z + 1) / log(1 * input.depthPosition.w + 1) * input.depthPosition.z * 0.1f);

	// Write grayscale image, the brighter the closer to the camera
	float DOF = saturate(abs(-depth*100 + focalDistance) / focalRange);
    float4 colour = float4(depth, DOF, depthInv, 1.0f);

	return colour;
}