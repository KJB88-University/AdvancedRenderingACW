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
	float2 uv : TEXCOORD0;
};

float mod(float x, float y)
{
	return x - y * floor(x / y);
}

[maxvertexcount(112)]
void GS_main(line GeometryShaderInput input[2], inout TriangleStream<PixelShaderInput> OutputStream)
{
	PixelShaderInput output = (PixelShaderInput)0;

	float amp = 0.1f;
	float length = 5.5f;
	float freq = 3.0f;

	// Define additional vertices
		for (int i = 0; i < 11; i++)
		{
			float angle = 3.14f * 2.0f / 5.0f * i;
			float4 offset;

			// TAIL
			// Taper
			if (input[0].pos.z < -0.7f)
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.001f;
			}
			// Fatten Head
			else if (input[0].pos.z >= 0.4f)
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.075f;
			}
			// Normal
			else
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.05f;
			}
			
			output.pos = input[0].pos + offset;
			output.uv = float2(mod(output.pos.x, 1.0), mod(output.pos.z, 1.0));

			output.pos = mul(output.pos, model);
			output.pos = mul(output.pos, view);
			output.pos = mul(output.pos, projection);

			output.pos.x += sin((deltaTime * freq) + (output.pos.x * length)) * amp,

			OutputStream.Append(output);

			// HEAD
			// Taper
			if (input[1].pos.z > 0.5f)
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.001f;
			}
			// Fatten Head
			else if (input[1].pos.z >= 0.4f)
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.075f;
			}
			// Normal
			else
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.05f;
			}

			output.pos = input[1].pos + offset;
			output.uv = float2(mod(output.pos.x, 1.0), mod(output.pos.z, 1.0));

			output.pos = mul(output.pos, model);
			output.pos = mul(output.pos, view);
			output.pos = mul(output.pos, projection);

			output.pos.x += sin((deltaTime * freq) + (output.pos.x * length)) * amp;

			OutputStream.Append(output);
		}
}
