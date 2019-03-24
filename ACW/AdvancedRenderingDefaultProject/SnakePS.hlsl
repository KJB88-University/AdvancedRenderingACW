Texture2D snakeTex : register (t0);
SamplerState Sampler;

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uvs : TEXCOORD0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 finalColor = snakeTex.Sample(Sampler, input.uvs, 0);
	return float4(finalColor);
}
