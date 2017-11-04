// Tessellation vertex shader.
// Doesn't do much, could manipulate the control points
// Pass forward data, strip out some values not required for example.

cbuffer MatrixBuffer : register(cb0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer TessellationBufferType : register(cb1)
{
	float minTesselationDistance;
	float maxTesselationDistance;
	float minTesselationFactor;
	float maxTesselationFactor;
};

cbuffer CameraBuffer : register(cb2)
{
	float3 cameraPosition;
	float padding;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float tesselationFactor : TESSFACTOR0;
};

OutputType main(InputType input)
{
    OutputType output;

	// transform to world space - needed to correctly calculate distance
	input.position.w = 1.0f;
	output.position = mul(input.position, worldMatrix);
	//output.position = mul(output.position, viewMatrix);
	//output.position = mul(output.position, projectionMatrix);
	float dist = distance(output.position.xyz, cameraPosition);

	 // Pass the vertex position into the hull shader.
    output.position = input.position;

	// Pass texture coordinates
	output.tex = input.tex;

	// Calculate tesselation factor based on distance
	// it's simple heuristic which suits my needs as it's used on mesh which has evenly sized triangles
	// otherwise I'd determine tessellation factor based on screen size of the triangle
	float tess = saturate( (dist - minTesselationDistance) / (maxTesselationDistance - minTesselationDistance) );	// Normalized distance based on min and max tesselation distance
	output.tesselationFactor = minTesselationFactor + tess * maxTesselationFactor;
	output.tesselationFactor = clamp(output.tesselationFactor, 1, 64);
	
	// Calculate the normal vector against the world matrix then normalize
	output.normal = mul(input.normal, (float3x3)worldMatrix);	// normal
	output.normal = normalize(output.normal);
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);	// Tangent
	output.tangent = normalize(output.tangent);
	output.binormal = mul(input.binormal, (float3x3)worldMatrix);	// Binormal
	output.binormal = normalize(output.binormal);

    return output;
}
