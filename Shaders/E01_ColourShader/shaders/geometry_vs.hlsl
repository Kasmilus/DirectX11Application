// Grass shader
// Don't do anything here, everything's handled by geometry shader stage

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 positionSV : SV_POSITION;
};

OutputType main(InputType input)
{
	OutputType output;
	output.position = input.position;
	output.tex = input.tex;
	output.normal = input.normal;
	output.positionSV = 0;

	return output;
}