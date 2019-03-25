Texture2D floorTex : register (t0);
Texture2D floorNorm : register (t1);
SamplerState Sampler;

cbuffer ModelViewProjectionConstantBuffer : register(b1)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uvs : TEXCOORD0;
};

// Static light
//static const float3 light1Pos = float3(3.0f, 0.0f, 0.0f);
//static const float3 light1Intensity = float3(1.0f, 1.0f, 1.0f);
static const float3 eyePos = float3(0.0f, 0.7f, 1.5f);
static const float3 lookAt = float3(0.0f, -0.1f, 0.0f);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 finalColor = floorTex.Sample(Sampler, input.uvs);
	float4 normal = floorNorm.Sample(Sampler, input.uvs);
	finalColor.r = 0.5f;
	finalColor.b = 0.5f;

	// LIGHTING
	float specularPower = 20.0f;
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 color = float4(0.2f, 0.2f, 0.2f, 1.0f);
	float3 lightDir = float3(0.0f, 1.0f, 0.0f);
	float lightIntensity = saturate(dot(normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		color += (finalColor * lightIntensity);
		color = saturate(color);

		float reflection = normalize(2 * lightIntensity * normal - lightDir);
		specular = pow(saturate(dot(reflection, lookAt)), specularPower);
	}

	finalColor = color * finalColor;
	color = saturate(color + specular);
	// END LIGHTING

	//finalColor = (phongIllumination(normal, finalColor, specularColor, shininess, input.pos, eyePos), 1.0f);
	return float4(finalColor);
}
