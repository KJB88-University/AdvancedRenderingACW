// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer TimeBuffer : register(b1)
{
	float deltaTime;
	float3 padding;
};

struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};
[maxvertexcount(6)]
void GS_main(point GeometryShaderInput input[1], inout TriangleStream<PixelShaderInput> OutputStream)
{
	PixelShaderInput output;

	output.pos = input.pos;
	output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return output;
}