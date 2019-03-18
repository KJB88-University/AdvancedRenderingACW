// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
};

// Per-pixel color data passed through the pixel shader.
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

// Simple shader to do vertex processing on the GPU.
VS_OUTPUT main(VertexShaderInput input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1.0f);

	return output;
}
