// Renders given texture 
Texture2D renderTexture : register(t0);
SamplerState sampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
	float4 colour = renderTexture.Sample(sampleType,input.tex);

	return colour;
}