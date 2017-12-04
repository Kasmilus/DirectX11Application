// Depth Tessellation domain shader
// After tessellation the domain shader processes all of the vertices

Texture2D displacementTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer MatrixBuffer : register(cb0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer : register(cb1)
{
	float3 cameraPosition;
	float padding;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    float3 vertexPosition;
    OutputType output;
 
    // Determine variables for new vertices
	vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
	output.tex = uvwCoord.x * patch[0].tex + uvwCoord.y * patch[1].tex + uvwCoord.z * patch[2].tex;
	output.normal = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal;
	output.normal = normalize(output.normal);


	// --- DISPLACEMENT MAPPING --- //
	// Choose mipmap level
	const float mipInterval = 20.0f;
	float dist = distance(vertexPosition, cameraPosition);
	float mipLevel = clamp( (dist - mipInterval) / mipInterval, 0.0f, 6.0f );
	// Sample displacement map
	float height = displacementTexture.SampleLevel(SampleType, output.tex, mipLevel).x;
	// Offset vertex along the normal
	vertexPosition += (4 * (height - 1.0f)) * output.normal;


	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.depthPosition = output.position;

    return output;
}

