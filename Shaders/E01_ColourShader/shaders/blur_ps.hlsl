/*
	Depth of field blur - gaussian blur pixels which are really close and far away
	Do that in 2 passes. First horizontal then vertical
*/
Texture2D shaderTexture : register(t0);
Texture2D depthTexture : register(t1);
SamplerState SampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float2 texCoordLeft1 : TEXCOORD1;
	float2 texCoordLeft2 : TEXCOORD2;
	float2 texCoordLeft3 : TEXCOORD3;
	float2 texCoordRight1 : TEXCOORD4;
	float2 texCoordRight2 : TEXCOORD5;
	float2 texCoordRight3 : TEXCOORD6;
	float2 texCoordUp1 : TEXCOORD7;
	float2 texCoordUp2 : TEXCOORD8;
	float2 texCoordUp3 : TEXCOORD9;
	float2 texCoordDown1 : TEXCOORD10;
	float2 texCoordDown2 : TEXCOORD11;
	float2 texCoordDown3 : TEXCOORD12;
	int isHorizontal : TEXCOORD13;
};

float4 main(InputType input) : SV_TARGET
{
	float4 colour;
	float depth;
	
	// Get depth of this pixel
	depth = depthTexture.Sample(SampleType, input.tex).x;

	// Weights
	// SHOULD change depending on depth value!!!!
	float weight0 = 0.382928f;
	float weight1 = 0.241732f;
	float weight2 = 0.060598f;
	float weight3 = 0.006206f;

	// Get pixel colour
	colour = shaderTexture.Sample(SampleType, input.tex) * weight0;

	/*if (depth < 0.3f  || depth > 0.7f)
	{*/
	if (input.isHorizontal == 1) {
		colour += shaderTexture.Sample(SampleType, input.texCoordUp3) * weight3;
		colour += shaderTexture.Sample(SampleType, input.texCoordUp2) * weight2;
		colour += shaderTexture.Sample(SampleType, input.texCoordUp1) * weight1;
		colour += shaderTexture.Sample(SampleType, input.texCoordDown1) * weight1;
		colour += shaderTexture.Sample(SampleType, input.texCoordDown2) * weight2;
		colour += shaderTexture.Sample(SampleType, input.texCoordDown3) * weight3;

	}
	else if (input.isHorizontal == 0) {
		colour += shaderTexture.Sample(SampleType, input.texCoordLeft3) * weight3;
		colour += shaderTexture.Sample(SampleType, input.texCoordLeft2) * weight2;
		colour += shaderTexture.Sample(SampleType, input.texCoordLeft1) * weight1;
		colour += shaderTexture.Sample(SampleType, input.texCoordRight1) * weight1;
		colour += shaderTexture.Sample(SampleType, input.texCoordRight2) * weight2;
		colour += shaderTexture.Sample(SampleType, input.texCoordRight3) * weight3;
	}
		
	//}


	// Set the alpha channel to one.
	colour.a = 1.0f;

	return colour;
}