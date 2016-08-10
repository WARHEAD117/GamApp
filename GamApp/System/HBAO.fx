#include "common.fx"

int			g_mapWidth;
int			g_mapHeight;

float		g_intensity = 1;
float		g_scale = 1;
float		g_bias = 0;
float		g_sample_rad = 0.03f;

float		g_rad_scale = 0.3f;
float		g_rad_threshold = 4.0f;

texture		g_NormalBuffer;
texture		g_RandomNormal; 
texture		g_PositionBuffer;
texture		g_ViewDirBuffer;

sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

sampler2D g_sampleRandomNormal =
sampler_state
{
	Texture = <g_RandomNormal>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_AoBuffer;
sampler2D g_sampleAo =
sampler_state
{
	Texture = <g_AoBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
};

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};


OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

//Maria SSAO
float doAmbientOcclusion(in float2 cTexCoord, in float2 vec, in float3 cNormal, float3 cPos)
{
	float2 sampleCoord = cTexCoord + vec;
    float3 samplePos = GetPosition(sampleCoord, g_samplePosition);
    //return samplePos.z - cPos.z;

    
    float3 H_Vec = samplePos - cPos;
    if (dot(H_Vec, cNormal) < 0)
    //if (samplePos.z > cPos.z)
        return 0;
    float h = atan(H_Vec.z / length(H_Vec.xy));
	float a = atan(cNormal.y / cNormal.x);
	float b = dot(cNormal, float3(vec, 0)) / length(vec);
    float3 pn = b * normalize(float3(vec, 0));
    float t = atan(length(pn.xy) / cNormal.z);
    if (abs(h) < abs(t))
        return 0;
    float ao = sin(h) - sin(t);
    return abs(ao);
}

float doAO(float3 normal, float3 H_Vec, float2 vec)
{
    float h = atan(H_Vec.z / length(H_Vec.xy));
    float a = atan(normal.y / normal.x);
    float b = dot(normal, float3(vec, 0)) / length(vec);
    float3 pn = b * normalize(float3(vec, 0));
    float t = atan(length(pn.xy) / normal.z);

    if (abs(h) < abs(t)+ 3.1415 / 6)
        return 0;
    float ao = sin(h) - sin(t);
    return clamp(-ao, 0, 1);
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	const float2 vec[4] = { float2(1, 0), float2(-1, 0),float2(0, 1), float2(0, -1) };
    float R = 1;
    const float2 samplePoint[8][2] = 
    {
        { float2(1, 0),   float2(3, 0) },
        { float2(0, 1),   float2(0, 3) },
        { float2(-1, 0),  float2(-3, 0) },
        { float2(0, -1),  float2(0, -3) },
        { float2(1, 1),   float2(3, 3) },
        { float2(1, -1),  float2(3, -3) },
        { float2(-1, 1),  float2(-3, 3) },
        { float2(-1, -1), float2(-3, -3) }
    };
	//观察空间位置
	float3 pos = GetPosition(TexCoord, g_samplePosition);
	//return float4(pos.z, pos.z, pos.z, 1.0f);
    float dis = R * g_zNear / pos.z;

    float height = 2 * g_zNear * g_ViewAngle_half_tan;
    float width = 2 * g_zNear * g_ViewAngle_half_tan * g_ViewAspect;

    float2 r_uv = float2(dis / height, dis / width);

    int stepCount = 5;
    float2 step = r_uv / stepCount;

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos .z> g_zFar)
		return float4(1, 1, 1, 1);

	//观察空间法线
	float3 normal = GetNormal(TexCoord, g_sampleNormal);

    int iterations = 4;
    float totalAo = 0;
    for (int i = 0; i < iterations; ++i)
    {
        float wao = 0;
        float lastAo = 0;
        float maxAngle = 0;
        float ao = 0;
        float angle = 0;
        float2 sampleCoord = TexCoord;
        for (int j = 0; j < stepCount; j++)
        {
            sampleCoord += vec[i] * step;
            float3 samplePos = GetPosition(sampleCoord, g_samplePosition);
            float3 H_Vec = samplePos - pos;
            float l = length(H_Vec);
            float r = l / R;
            float wr = 1 - r * r;
            lastAo = ao;

            angle = dot(normalize(H_Vec), normalize(normal));
            if (l < R && angle > 0 && angle > maxAngle)
            {
                maxAngle = angle;
                ao = doAO(normal, H_Vec, vec[i]);
                wao += wr * (ao - lastAo);
            }


        }
        totalAo += wao;

    }
    totalAo = 1 - totalAo / (float) iterations;
    //ao = 0.5f;
    totalAo = clamp(totalAo, 0, 1);
    return float4(totalAo, totalAo, totalAo, 1);
}

float4 texture2DBilinear(sampler2D textureSampler, float2 uv)
{
	float stepU = 1.0f / g_mapWidth;
	float stepV = 1.0f / g_mapWidth;

	// in vertex shaders you should use texture2DLod instead of texture2D
	float4 tl = tex2D(textureSampler, uv);
	float4 tr = tex2D(textureSampler, uv + float2(stepU, 0));
	float4 bl = tex2D(textureSampler, uv + float2(0, stepV));
	float4 br = tex2D(textureSampler, uv + float2(stepU, stepV));
	float2 f = frac(uv.xy * float2(g_mapWidth, g_mapWidth)); // get the decimal part
	float4 tA = lerp(tl, tr, f.x); // will interpolate the red dot in the image
	float4 tB = lerp(bl, br, f.x); // will interpolate the blue dot in the image
	return lerp(tA, tB, f.y); // will interpolate the green dot in the image
}

float4 DrawMain(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 AO = tex2D(g_sampleAo, TexCoord);
	//float4 fianlColor = AO * tex2D(g_sampleMainColor, TexCoord);

    return AO;
}
technique HBAO
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}

	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DrawMain();
	}
}