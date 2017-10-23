// Depth buffer

struct InputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};


float4 main(InputType input) : SV_TARGET
{
	// Get depth of the pixel, subtract from 1 for better precision
	float depth = 1.0f - (input.depthPosition.z / input.depthPosition.w);
	// Logarythmic z-buffer
	// float depth = log(1* input.depthPosition.z + 1) / log(1* input.depthPosition.w + 1) * input.depthPosition.z*0.1f;
	// Write grayscale image, the brighter the closer to the camera
	float4 colour = float4(depth, depth, depth, 1.0f);

	return colour;
}