Texture2D dispMap : register (t0);
SamplerState Sampler;

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer DisplacementBuffer : register(b1)
{
	float4 displacementFactor;
};

struct HS_TRI_Tess_Param
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

static float3 TriPos[3] =
{
	float3(-1, 1, 0),
	float3(1, 1, 0),
	float3(0, -1, 0)
};
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uvs : TEXCOORD0;
};
float mod(float x, float y)
{
	return x - y * floor(x / y);

}

[domain("tri")]
VS_OUTPUT main(HS_TRI_Tess_Param input, float3 UVW : SV_DomainLocation)
{
	VS_OUTPUT output;

	float3 finalPos = UVW.x * TriPos[0] +
		UVW.y * TriPos[1] +
		UVW.z * TriPos[2];
	float3 uvPos = (1.0f - UVW.x) * finalPos + UVW.x * finalPos;


	// SPHERE
	float radius = 0.35f;
	float pi = 3.1415926 * 1.5f;
	float pi2 = 2.0f * pi;

	float phi = pi * finalPos.x;
	float theta = pi * finalPos.y;

	float x = radius * sin(phi) * cos(theta) * radius;
	float y = radius * sin(phi) * sin(theta) * radius;
	float z = radius * cos(phi) * radius;
	// END SPHERE

	finalPos = float3(x, y + 0.25, z);
	output.uvs = float2(mod(finalPos.x, 1.0f), mod(finalPos.z, 1.0f));

	float disp = dispMap.SampleLevel(Sampler, output.uvs, 0, 0);
	float dispScale = displacementFactor.x;
	finalPos += (disp * dispScale);

	//finalPos = float3(x, y, z);

	output.pos = float4(finalPos, 1.0f);
	output.pos = mul(output.pos, model);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	return output;
}