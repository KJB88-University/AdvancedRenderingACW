Texture2D dispMap : register (t0);
SamplerState Sampler;

// Floor Quad
static float3 QuadPos[4] =
{
	float3(-1, 0, -1),
	float3(-1, 0, 1),
	float3(1, 0, -1),
	float3(1, 0, 1)
};

struct HS_Quad_Tess_Param
{
	float Edges[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};

struct HS_TRI_Tess_Param
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer TimeBuffer : register (b1)
{
	float deltaTime;
	float3 padding;
}

[domain("quad")]
VS_OUTPUT DS_QuadTess(HS_Quad_Tess_Param input, float2 uv : SV_DomainLocation)
{
	VS_OUTPUT output;

	float3 vPos1 = (1.0f - uv.y) * QuadPos[0] + uv.y * QuadPos[1].xyz;
	float3 vPos2 = (1.0f - uv.y) * QuadPos[2] + uv.y * QuadPos[3].xyz;
	float3 uvPos = (1.0f - uv.x) * vPos1 + uv.x * vPos2;

	output.pos = float4(uvPos.x, uvPos.y, uvPos.z, 1);

	output.pos = mul(output.pos, model);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	return output;
}

[domain("tri")]
VS_OUTPUT DS_TriTess(HS_TRI_Tess_Param input, float3 UVW : SV_DomainLocation)
{
	VS_OUTPUT output;

	float3 finalPos = UVW.x * QuadPos[0].xyz + UVW.y * QuadPos[2].xyz + UVW.z * QuadPos[1].xyz;

	output.pos = float4(0.6f * finalPos, 1.0f);

	return output;
}