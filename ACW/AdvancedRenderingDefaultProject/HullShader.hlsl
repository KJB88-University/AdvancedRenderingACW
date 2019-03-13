struct HS_Quad_Tess_Factors
{
	float Edges[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

struct HS_TRI_Tess_Param
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

HS_TRI_Tess_Param ConstantHS_Tri(InputPatch<VS_OUTPUT, 3> ip)
{
	HS_TRI_Tess_Param output;

	float TessAmount = 30.0f;

	output.Edges[0] = output.Edges[1] = output.Edges[2] = TessAmount;
	output.Inside = TessAmount;

	return output;
}

HS_Quad_Tess_Factors ConstantHS(InputPatch <VS_OUTPUT, 4> ip)
{
	HS_Quad_Tess_Factors output;

	float TessAmount = 31.0f;

	output.Edges[0] = TessAmount;
	output.Edges[1] = TessAmount;
	output.Edges[2] = TessAmount;
	output.Edges[3] = TessAmount;

	output.Inside[0] = TessAmount;
	output.Inside[1] = TessAmount;

	return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
VS_OUTPUT HS_QuadTess(InputPatch<VS_OUTPUT, 4> patch, uint i : SV_OutputControlPointID)
{
	VS_OUTPUT output;

	output.pos = patch[i].pos;

	return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS_Tri")]
VS_OUTPUT HS_TriTess(InputPatch<VS_OUTPUT, 3> p, uint i : SV_OutputControlPointID)
{
	VS_OUTPUT output;

	output.pos = p[i].pos;
	
	return output;
}