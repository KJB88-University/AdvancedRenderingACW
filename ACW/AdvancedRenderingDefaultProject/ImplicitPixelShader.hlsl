// BUFFERS
cbuffer CameraBuffer : register(b0)
{
	float4 eyePos;
	float4 lookAt;
	float4 clipPlanes;
};

// INPUT
struct VS_QUAD
{
	float4 pos : SV_POSITION;
	float2 canvasXY : TEXCOORD0;
};

// RAY
struct Ray
{
	float3 origin;
	float3 direction;
};

//static float4 eyePos = float4(0, 0, 15, 1);
//static float nearPlane = 0.1f;
//static float farPlane = 1000.0f;

// STATIC LIGHTING
static float4 lightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
static float3 lightPos = float3(0.0f, 100.0f, 0.0f);
static float4 backgroundColor = float4(0.1f, 0.2f, 0.3f, 1.0f);

float SphereIntersection(Ray ray, out bool hit)
{
	float t;
	float3 v = -ray.origin;
	float A = dot(v, ray.direction);
	float B = dot(v, v) - A * A;

	float R = 2;

	// Intersection check
	if (B > R *R)
	{
		hit = false;
		t = clipPlanes.y;
	}
	else
	{
		float disc = sqrt(R*R - B);
		t = A - disc;

		if (t < 0.0f)
		{
			hit = false;
		}
		else
		{
			hit = true;
		}
	}

	return t;
}

float4 RayCasting(Ray eyeRay)
{
	bool hit = false;
	float t = SphereIntersection(eyeRay, hit);
	float3 interP = eyeRay.origin + t * normalize(eyeRay.direction);

	// RENDER
	float4 RTColor = (float4)0;

	if (!hit)
	{
		RTColor = backgroundColor;
	}
	else
	{
		float3 c = lightColor.rgb;
		float3 N = normalize(interP);
		N = normalize(N);
		float3 L = normalize(lightPos - interP);
		float3 V = normalize(eyePos.xyz - interP);
		float3 R = reflect(-L, N);
		float r = max(0.5 * dot(N, L), 0.2f);
		r += pow(max(0.1f, dot(R, V)), 50.0f);
		RTColor = float4(1.5f * r * c, 1.0f);
	}

	return RTColor;
}

float4 main(VS_QUAD input) : SV_Target
{
	float zoom = 5.0f;
	float2 xy = zoom * input.canvasXY;
	float distEye2Canvas = clipPlanes.x;
	float3 pixelPos = float3(xy, -distEye2Canvas);

	Ray eyeRay;
	eyeRay.origin = eyePos.xyz;
	eyeRay.direction = normalize(pixelPos - eyePos.xyz);

	return RayCasting(eyeRay);
	//float4 rgbColor = float4(input.canvasXY, 0.0f, 1.0f);
	//
	//if (length(input.canvasXY) < 0.5f)
	//{
	//	rgbColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	//}
	//else
	//{
	//	rgbColor = (float4)0.2f;
	//}

	//return rgbColor;
}