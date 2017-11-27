// Geometry shader grass
// Just output texture without any lighting to keep it simple

Texture2D renderTexture : register(t0);
SamplerState sampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{
	float4 colour = renderTexture.Sample(sampleType,1 - input.tex);
	//float4 colour = float4(0, input.tex.y, 0, 0);

	return colour;
}