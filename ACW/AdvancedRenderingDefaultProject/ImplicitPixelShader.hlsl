/*
REFERENCE:
Pixel Shader adapted from tutorial labs provided by Dr Qingde Li.
Pixel Shader adapted from tutorials provided by Jamie Wong.
Distance Functions provided by Íñigo Quílez.

Dist Functions: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
Jamie Wong's Tutorial: https://www.shadertoy.com/view/XtcGWn
and: http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/

*/

cbuffer TimeBuffer : register(b0)
{
	float deltaTime;
}

//cbuffer ControlBuffer : register(b1)
//{
//	float3 repDefFrac;
//}

struct PixelShaderInput
{
	float4 pos : POSITION;
	float2 canvasXY : TEXCOORD0;
};

struct Ray
{
	float3 origin;
	float3 direction;
};

// CAMERA
static const float3 eyePos = float3(0.0f, 10.0f, 20.0f);
static const float nearPlane = 0.01f;
static const float farPlane = 1000.0f;

// LIGHTING
// TODO - Cleanup
static const float3 lightPos1;
static const float3 lightIntensity1;
static const float3 lightPos2;
static const float3 lightIntensity2;

static const float3 AMBIENT_COLOR;
static const float3 DIFFUSE_COLOR;
static const float3 SPECULAR_COLOR;

// RAYMARCHING
static const int MAX_MARCH = 255;
static const float EPSILON = 0.0001f;


// Distance Func for Sphere
float sphereDistFunc(float3 samplePoint, float radius)
{
	return length(samplePoint) - radius;
}

// Distance Func for Cube
float cubeDistFunc(float3 samplePoint)
{
	// Distance func
	float3 dist = abs(samplePoint) - float3(1.0f, 1.0f, 1.0f);

	// Distance from surface (inside)
	float insideDist = min(max(dist, max(dist.y, dist.z)), 0.0f);

	// Distance from surface (outside)
	float outsideDist = length(max(dist, 0.0f));

	return insideDist + outsideDist;
}

float torusDistFunc(float3 pos, float2 radii)
{
	float2 q = float2(length(pos.xz) - radii.x, pos.y);
	return length(q) - radii.y;
}

float hexDF(float3 pos, float2 h)
{
	const float3 k = float3(-0.8660254f, 0.5f, 0.57735f);
	pos = abs(pos);

	pos.xy -= 2.0f * min(dot(k.xy, pos.xy), 0.0f) * k.xy;

	float2 d = float2(
		length(pos.xy - float2(clamp(pos.x, -k.z * h.x, k.z * h.x), h.x)) * sign(pos.y - h.x),
		pos.z - h.y
		);

	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
}

float octahedronDF(float3 pos, float size)
{
	pos = abs(pos);
	float m = pos.x + pos.y + pos.z - size;

	float3 q;
	if (3.0f * pos.x < m)
	{
		q = pos.xyz;
	}
	else if (3.0f * pos.y < m)
	{
		q = pos.yzx;
	}
	else if (3.0f * pos.z < m)
	{
		q = pos.zxy;
	}
	else
	{
		return m * 0.57735027f;
	}

	float k = clamp(0.5f * (q.z - q.y + size), 0.0f, size);
	
	return length(float3(q.x, q.y - size + k, q.z - k));
}

// Intersection OP
float intersectDF(float distanceA, float distanceB)
{
	return max(distanceA, distanceB);
}

// Union OP
float unionDF(float distanceA, float distanceB)
{
	return min(distanceA, distanceB);
}

// Differential OP
float diffDF(float distanceA, float distanceB)
{
	return max(distanceA, -distanceB);
}

// Scene Sampling
float sceneDistFunc(float3 samplePoint)
{
	// NOTE: Multiplying values by sin or cos will result in a wavey masking

	float cubeOffsetX = -3.0f;
	float torusOffsetX = 5.0f;
	float octaOffset = 3.0f;
	float hexOffset = 3.0f;

	float sphere = sphereDistFunc(samplePoint / 1.2f, 1.0) *  1.2;
	float cube = cubeDistFunc(samplePoint + cubeOffsetX);
	float torus = torusDistFunc(samplePoint + torusOffsetX, float2(1.5f, 0.5f));
	float octa = octahedronDF(float3(samplePoint.x - octaOffset, samplePoint.y + octaOffset, samplePoint.z), 1.0f);
	float hex = hexDF(float3(samplePoint.x + hexOffset, samplePoint.y - hexOffset, samplePoint.z), float2(1.0f, 1.0f));

	float final = unionDF(sphere, cube);
	final = unionDF(final, torus);
	final = unionDF(final, octa);
	final = unionDF(final, hex);
	return final;

	/*return ellipsoidDF(samplePoint, float3(2.0f, 1.0f, 2.0f));
	return cylinderDF(samplePoint, float3(0.1f, 0.1f, 0.1f));
	return torusDistFunc(samplePoint, float2(1.0f, 1.0f));
	float sphereDist = sphereDistFunc(samplePoint / 1.2f, 1.0f) *  1.2;
	float cubeDist = cubeDistFunc(samplePoint);

	return intersectDF(cubeDist, sphereDist);

	return sphereDistFunc(samplePoint, 1.0f);
	return cubeDistFunc(samplePoint);*/
}

// Calculate Normals
float3 calcNormals(float3 pos)
{
	return normalize(float3
		(
			sceneDistFunc(float3(pos.x + EPSILON, pos.y, pos.z)) - sceneDistFunc(float3(pos.x - EPSILON, pos.y, pos.z)),
			sceneDistFunc(float3(pos.x, pos.y + EPSILON, pos.z)) - sceneDistFunc(float3(pos.x, pos.y - EPSILON, pos.z)),
			sceneDistFunc(float3(pos.x, pos.y, pos.z + EPSILON)) - sceneDistFunc(float3(pos.x, pos.y, pos.z - EPSILON))
			));
}
// RAY MARCH
float shortestDistanceToSurface(Ray ray, float start, float end)
{
	float depth = start;

	for (int i = 0; i < MAX_MARCH; i++)
	{
		float dist = sceneDistFunc(ray.origin + depth * ray.direction);
		if (dist < EPSILON)
		{
			return depth;
		}
		else
		{
			depth += dist;
			if (depth >= end)
			{
				return end;
			}
		}
	}

	return end;
}

// CONTRIBUTION OF PHONG LIGHT
float3 phongLightObstruction(float3 diffuseFactor, float3 specularFactor, float shininess, float3 pos, float3 eyePos, float3 lightPos, float3 lightIntensity)
{
	float3 normal = calcNormals(pos);
	float3 lightDir = normalize(lightPos - pos);
	float3 viewDir = normalize(eyePos - pos);
	float3 reflectVector = normalize(reflect(-lightDir, normal));

	float dotLN = dot(lightDir, normal);
	float dotRV = dot(reflectVector, viewDir);

	if (dotLN < 0.0f)
	{
		// Obstruction from pixel pos to light
		return float3(0.0f, 0.0f, 0.0f);
	}

	if (dotRV < 0.0f)
	{
		// Reflection along view dir vector, only diffuse lighting
		return lightIntensity * (diffuseFactor * dotLN);
	}

	return lightIntensity * (diffuseFactor * dotLN + specularFactor * pow(dotRV, shininess));
}

// PHONG ILLUMINATION
float3 phongIllumination(float3 ambientFactor, float3 diffuseFactor, float3 specularFactor, float shininess, float3 pos, float3 eyePos)
{
	float3 ambientLight = 0.5f * float3(1.0f, 1.0f, 1.0f);
	float3 color = ambientLight * ambientFactor;

	float3 light1Pos = float3(4.0f, 2.0f, 4.0f);
	float3 light1Intensity = float3(0.4, 0.4f, 0.4f);

	color += phongLightObstruction(diffuseFactor, specularFactor, shininess, pos, eyePos, light1Pos, light1Intensity);

	float3 light2Pos = float3(2.0, 2.0f, 2.0f);
	float3 light2Intensity = float3(0.4f, 0.4f, 0.4f);

	color += phongLightObstruction(diffuseFactor, specularFactor, shininess, pos, eyePos, light2Pos, light2Intensity);

	return color;
}

float4 main(PixelShaderInput input) : SV_Target
{
	float4 output = (float4)0;

	float zoom = 5.0f;
	float2 xy = zoom * input.canvasXY;
	float distEye2Canvas = nearPlane;
	float3 pixelPos = float3(xy, -distEye2Canvas);

	Ray eyeRay;
	eyeRay.origin = eyePos;
	eyeRay.direction = normalize(pixelPos - eyePos);

	float distance = shortestDistanceToSurface(eyeRay, nearPlane, farPlane);

	if (distance > farPlane - EPSILON)
	{
		output = float4(0.0f, 0.0f, 0.0f, 0.0f);
		return output;
	}

	float3 pos = eyeRay.origin + distance * eyeRay.direction;

	float3 ambientFactor = (calcNormals(pos) + (float3)1.0f) / 2.0f;
	float3 diffuseFactor = ambientFactor;
	float3 specularFactor = float3(1.0f, 1.0f, 1.0f);
	float shininess = 10.0f;

	float3 color = phongIllumination(ambientFactor, diffuseFactor, specularFactor, shininess, pos, eyePos);
	output = float4(color, 1.0f);
	return output;
}