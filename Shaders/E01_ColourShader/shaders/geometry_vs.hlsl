// Grass shader
// Don't do anything here, everything's handled by geometry shader stage
// MAYBE ADD FRUSTRUM CULLING LATER ON

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

InputType main(InputType input)
{
	return input;
}