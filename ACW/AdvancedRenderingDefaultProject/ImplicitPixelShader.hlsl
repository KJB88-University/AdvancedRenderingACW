struct VS_QUAD
{
	float4 pos : SV_POSITION;
	float2 canvasXY : TEXCOORD0;
};

float4 main (VS_QUAD input) : SV_Target
{
	float4 rgbColor = float4(input.canvasXY, 0.0f, 1.0f);
	
	if (length(input.canvasXY) < 0.5f)
	{
		rgbColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		rgbColor = (float4)0.2f;
	}

	return rgbColor;
}