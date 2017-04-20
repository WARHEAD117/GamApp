#define		epsilon 0.0000001f
#define		M_PI 3.141592653f
matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldView;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;
matrix		g_invView;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

int			g_ScreenWidth;
int			g_ScreenHeight;

int2        g_ScreenSize;

float		g_ViewAngle_half_tan;
float		g_ViewAspect;

float2 encode(float3 n)
{
	float p = sqrt(-n.z * 8 + 8);
	return float2(n.xy / p + 0.5);
}

float3 decode(float2 enc)
{
	float2 fenc = enc * 4 - 2;
		float f = dot(fenc, fenc);
	float g = sqrt(1 - f / 4);
	float3 n;
	n.xy = fenc*g;
	n.z = -1 + f / 2;
	return n;
}

float3 float2ToFloat3(float2 input)
{
	float2 enc255 = input * 255;
		float2 residual = floor(frac(enc255) * 16);
		float3 output = float3(floor(enc255), residual.x * 16 + residual.y) / 255;
		return output;
}

float2 float3ToFloat2(float3 input)
{
	float nz = floor(input.z * 255) / 16;
	float2 output = input.xy + float2(floor(nz) / 16, frac(nz)) / 255;
	return output;
}

float3 GetPosition(in float2 uv, sampler2D samplePosition)
{
	float z = tex2D(samplePosition, uv).r;

	float u = uv.x * 2.0f - 1;
	float v = (1 - uv.y) * 2.0f - 1.0f;

	float y = g_ViewAngle_half_tan * v * z;
	float x = g_ViewAngle_half_tan * u * z * g_ViewAspect;
	return float3(x, y, z);
}

float3 GetNormal(in float2 uv, sampler2D sampleNormal)
{
	float4 normal_shininess = tex2D(sampleNormal, uv);

	normal_shininess.xy = float3ToFloat2(normal_shininess.xyz);

	float3 normal = decode(normal_shininess.xy);

	return normalize( normal );
}

float3 GetNormal(sampler2D sampleNormal, in float2 uv)
{
	float4 normal_shininess = tex2D(sampleNormal, uv);

		normal_shininess.xy = float3ToFloat2(normal_shininess.xyz);

	float3 normal = decode(normal_shininess.xy);

		return normal;
}

float4 GetColor(in float2 uv, sampler2D g_sampleColor)
{
	return tex2D(g_sampleColor, uv);
}


float GetShininess(in float2 uv, sampler2D sampleNormal)
{
	return tex2D(sampleNormal, uv).a;
}

void GetNormalandShininess(in float2 uv, inout float3 normal, inout float shininess, sampler2D sampleNormal)
{
	float4 normal_shininess = tex2D(sampleNormal, uv);

		normal_shininess.xy = float3ToFloat2(normal_shininess.xyz);

	normal = decode(normal_shininess.xy);
	shininess = normal_shininess.a;
}

float3 GetPosition(in float2 uv, in float4 viewDir, sampler2D samplePosition)
{
	float DepthV = tex2D(samplePosition, uv).r;

	float3 pos = viewDir * ((DepthV) / viewDir.z);

		return pos;
}

float GetDepth(in float2 uv, sampler2D samplePosition)
{
	float DepthV = tex2D(samplePosition, uv).r;

	return DepthV;
}

float4 GaussianBlur(int mapWidth, int mapHeight, sampler2D texSampler, float2 texCoords)
{
	float weights[6] = { 0.00078633, 0.00655965, 0.01330373, 0.05472157, 0.11098164, 0.22508352 };

	float4 color;
	float stepU = 1.0f / mapWidth;
	float stepV = 1.0f / mapHeight;

	//0,1,2,1,0
	//1,3,4,3,1
	//2,4,5,4,2
	//1,3,4,3,1
	//0,1,2,1,0
	color = tex2D(texSampler, texCoords + float2(-2 * stepU, -2 * stepV)) * (weights[0]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, -2 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, -2 * stepV)) * (weights[0]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, -1 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, -1 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, -1 * stepV)) * (weights[1]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 0 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 0 * stepV)) * (weights[5]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 0 * stepV)) * (weights[2]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 1 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 1 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 1 * stepV)) * (weights[1]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 2 * stepV)) * (weights[0]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 2 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 2 * stepV)) * (weights[0]);

	return color;
}

float4 texture2DBilinear(sampler2D textureSampler, float2 uv, int mapWidth, float mapHeight)
{
	float stepU = 1.0f / mapWidth;
	float stepV = 1.0f / mapHeight;

	// in vertex shaders you should use texture2DLod instead of texture2D
	float4 tl = tex2D(textureSampler, uv);
		float4 tr = tex2D(textureSampler, uv + float2(stepU, 0));
		float4 bl = tex2D(textureSampler, uv + float2(0, stepV));
		float4 br = tex2D(textureSampler, uv + float2(stepU, stepV));
		float2 f = frac(uv.xy * float2(mapWidth, mapHeight)); // get the decimal part
		float4 tA = lerp(tl, tr, f.x); // will interpolate the red dot in the image
		float4 tB = lerp(bl, br, f.x); // will interpolate the blue dot in the image
		return lerp(tA, tB, f.y); // will interpolate the green dot in the image
}