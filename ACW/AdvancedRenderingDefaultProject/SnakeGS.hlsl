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

[maxvertexcount(112)]
void GS_main(line GeometryShaderInput input[2], inout TriangleStream<PixelShaderInput> OutputStream)
{
	PixelShaderInput output = (PixelShaderInput)0;

		for (int i = 0; i < 11; i++)
		{
			float angle = 3.14f * 2.0f / 10.0f * i;
			float4 offset;
			if (input[0].pos.z < -0.7f)
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.015;
			}
			else
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.05f;
			}
			output.pos = input[0].pos + offset;

			output.pos = mul(output.pos, model);
			output.pos = mul(output.pos, view);
			output.pos = mul(output.pos, projection);
			OutputStream.Append(output);

			if (input[1].pos.z > 0.1f)
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.05f;
			}
			else
			{
				offset = float4(cos(angle), -sin(angle), 0.0f, 0.0f) * 0.025f;
			}

			output.pos = input[1].pos + offset;

			output.pos = mul(output.pos, model);
			output.pos = mul(output.pos, view);
			output.pos = mul(output.pos, projection);

			OutputStream.Append(output);
		}
}



//
//	//output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
//
//	//OutputStream.Append(output);
//	//float4 vPos1 = input[0].pos;
//	//float4 vPos2 = input[1].pos;
//	//vPos1 = mul(vPos1, model);
//	//vPos1 = mul(vPos1, view);
//	//vPos2 = mul(vPos2, model);
//	//vPos2 = mul(vPos2, view);
//
//
//
//	//for (int i = 0; i < 100; i++)
//	//{
//	//	float angle = 2.0f * PI / 100.0f * i;
//
//	//	float4 offset = float4(cos(angle) * 0.1f, -sin(angle) * 1.0f, i, 0.0f);
//	//	output.pos = vPos1 + offset;
//	//	output.pos = mul(output.pos, projection);
//	//}
//
//	//for (int i = 0; i <= 100; i++)
//	//{
//	//	float ang = 3.14f * 2.0f / 100.0f * i;
//	//	float4 vPos = input[0].pos;
//	//	vPos = mul(vPos, model);
//	//	vPos = mul(vPos, view);
//
//	//	 Offset from center point
//	//	float4 offset = float4(cos(ang) * 0.1f , 1.0f, -sin(ang) * 0.1f, 0.0f);
//	//	output.pos = input[0].pos + offset;
//
//	//	output.pos = mul(output.pos, projection);
//
//	//	OutputStream.Append(output);
//	//}
//
//	//float radius = 2.0f;
//	//// V1
//	//float4 vPos = input[0].pos;
//	//vPos = mul(vPos, model);
//	//vPos = mul(vPos, view);
//
//	////output.pos = vPos + float4(radius * cos(3.14f * 2.0f), radius * sin(3.14f * 2.0f), vPos.z, 0.0f);
//	//output.pos = mul(output.pos, projection);
//	//output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
//
//	//OutputStream.Append(output);
//
//	//// V2
//	//vPos = input[1].pos;
//	//vPos = mul(vPos, model);
//	//vPos = mul(vPos, view);
//
//	////output.pos = vPos + float4(radius * cos(3.14f * 2.0f), radius * sin(3.14f * 2.0f), vPos.z, 0.0f);
//	//output.pos = mul(output.pos, projection);
//	//output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
//
//	//OutputStream.Append(output);
//}