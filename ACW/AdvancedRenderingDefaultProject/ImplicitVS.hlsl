// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VS_Canvas
{
	float4 pos : SV_POSITION;
	float2 canvasXY : TEXCOORD0;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

VS_Canvas main(VertexShaderInput input)
{
	VS_Canvas output;

	output.pos = float4(sign(input.pos.xy), 0, 1);

	float aRatio = projection._m11 / projection._m00;
	output.canvasXY = sign(input.pos.xy) * float2(aRatio, 1.0);

	return output;
}