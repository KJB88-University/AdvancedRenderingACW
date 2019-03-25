// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
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
	output.uvs = float2(mod(uvPos.x, 1.0f), mod(uvPos.y, 1.0f));

	// SPHERE
	float radius = 0.35f;
	float pi = 3.1415926 * 1.5f;
	float pi2 = 2.0f * pi;

	float a = 0.1f;
	float b = 0.3f;
	float c = 0.1f;

	float phi = pi * finalPos.x;
	float theta = pi * finalPos.y;

	float x = a * cos(phi) * sin(theta);
	float y = b * sin(phi) * sin(theta);
	float z = c * cos(theta);
	// END SPHERE

	finalPos = float3(x - 0.5f, y + 0.25, z - 1.0f);
	//finalPos = float3(x, y, z);

	output.pos = float4(finalPos, 1.0f);
	output.pos = mul(output.pos, model);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	return output;
}