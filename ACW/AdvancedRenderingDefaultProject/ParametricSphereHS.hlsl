struct HS_TRI_Tess_Param
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

HS_TRI_Tess_Param ConstantHS_TRI(InputPatch<VS_OUTPUT, 3> ip)
{
	HS_TRI_Tess_Param output;

	float TessAmount = 30.0f;

	output.Edges[0] = TessAmount;
	output.Edges[1] = TessAmount;
	output.Edges[2] = TessAmount;
	output.Inside = TessAmount;

	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS_TRI")]
VS_OUTPUT main(InputPatch<VS_OUTPUT, 3> patch, uint id : SV_OutputControlPointID)
{
	VS_OUTPUT output;
	output.pos = patch[id].pos;

	return output;
}
