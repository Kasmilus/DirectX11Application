// skybox pixel/fragment shader

TextureCube shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float3 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
	float4 colour = shaderTexture.Sample(SampleType, input.position);

	return colour;
}