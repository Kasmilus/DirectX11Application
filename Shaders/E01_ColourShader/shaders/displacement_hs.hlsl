// Tessellation Hull Shader
// Prepares control points for tessellation

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float tesselationFactor : TESSFACTOR0;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;

    // Set the tessellation factors based on neighbour edges
	output.edges[0] = 0.5f * (inputPatch[1].tesselationFactor + inputPatch[2].tesselationFactor);
	output.edges[1] = 0.5f * (inputPatch[0].tesselationFactor + inputPatch[2].tesselationFactor);
	output.edges[2] = 0.5f * (inputPatch[1].tesselationFactor + inputPatch[0].tesselationFactor);

    // Set the tessellation factor for tessallating inside the triangle.
	output.inside = output.edges[0];

    return output;
}


[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Pass values down to domain shader
    output.position = patch[pointId].position;
	output.tex = patch[pointId].tex;
	output.normal = patch[pointId].normal;
	output.tangent = patch[pointId].tangent;
	output.binormal = patch[pointId].binormal;

    return output;
}