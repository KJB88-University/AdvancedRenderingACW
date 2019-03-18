Texture2D grassTex : register (t0);
SamplerState Sampler;

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

[maxvertexcount(4)]
void GS_main(point GeometryShaderInput input[1], inout TriangleStream<PixelShaderInput> OutputStream)
{
	// Init and default
	PixelShaderInput output = (PixelShaderInput)0;

	float4 vPos = input[0].pos;
	vPos = mul(vPos, model);
	vPos = mul(vPos, view);

	static const float3 g_positions[4] =
	{
	float3(-1, -1, 0),
	float3(-1, 1, 0),
	float3(1, -1, 0),
	float3(1, 1, 0)
	};

	//static const float3 g_positions[3] =
	//{
	//	float3(0, 2, 0),
	//	float3(0.5, -0.5, 0),
	//	float3(-0.5, -0.5, 0)
	//};

	// Tri1
	float size = 0.02f;
	float windSpeed = 1.0f;
	float waveAmplitude = 0.1f;
	float dampening = 0.25f;

	float4 texColor = grassTex.SampleLevel(Sampler, 0.0f, 0.0f);

	// V1
	output.pos = vPos + float4(size * g_positions[0], 0.0f);
	output.pos = mul(output.pos, projection);

	output.color = texColor;

	OutputStream.Append(output);

	// V2
	output.pos = vPos + float4(size * g_positions[1], 0.0f);

	// Apply animation
	output.pos.z += sin(windSpeed * deltaTime)  * waveAmplitude * dampening;
	output.pos.x += cos(windSpeed * deltaTime) * waveAmplitude * dampening;

	output.pos = mul(output.pos, projection);

	output.color = texColor;

	OutputStream.Append(output);

	// V3
	output.pos = vPos + float4(size * g_positions[2], 0.0f);
	output.pos = mul(output.pos, projection);

	//output.color = input[0].color;
	output.color = texColor;

	OutputStream.Append(output);

	// V2
	output.pos = vPos + float4(size * g_positions[3], 0.0f);

	// Apply animation
	output.pos.z += sin(windSpeed * deltaTime)  * waveAmplitude * dampening;
	output.pos.x += cos(windSpeed * deltaTime) * waveAmplitude * dampening;

	output.pos = mul(output.pos, projection);

	output.color = texColor;

	OutputStream.Append(output);
}