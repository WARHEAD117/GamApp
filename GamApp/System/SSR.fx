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
    MinFilter = linear;
    MagFilter = linear;
    MipFilter = linear;
};

sampler2D g_sampleRandomNormal =
sampler_state
{
	Texture = <g_RandomNormal>;
    MinFilter = linear;
    MagFilter = linear;
    MipFilter = linear;

    AddressU = wrap;
    AddressV = wrap;
};

sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
    MinFilter = linear;
	MagFilter = linear;
    MipFilter = linear;

    AddressU = Clamp;
    AddressV = Clamp;
};

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
    MinFilter = linear;
    MagFilter = linear;
    MipFilter = linear;

    AddressU = Border;
    AddressV = Border;
};

texture g_SSRBuffer;
sampler2D g_sampleSSR =
sampler_state
{
    Texture = <g_SSRBuffer>;
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

//----------------------------------------------------------------------------------
float Falloff(float DistanceSquare, float R)
{
    // 1 scalar mad instruction
    return DistanceSquare * ( -1.0f / (R * R)) + 1.0;
}

//----------------------------------------------------------------------------------
float2 RotateDirection(float2 Dir, float2 CosSin)
{
    return float2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                  Dir.x * CosSin.y + Dir.y * CosSin.x);
}


float M_PI = 3.1415926;


float3 getRandom(in float2 uv)
{
    return normalize(tex2D(g_sampleRandomNormal, /*g_screen_size*/float2(g_ScreenWidth, g_ScreenHeight) * uv / /*random_size*/float2(256, 256)).xyz );
}

float2 GetRayUV(float3 rayPos)
{
    
    float u = g_zNear * rayPos.x / rayPos.z / (g_zNear * g_ViewAngle_half_tan * g_ViewAspect);
    float v = g_zNear * rayPos.y / rayPos.z / (g_zNear * g_ViewAngle_half_tan);

    u = 0.5 * u + 0.5;
    v = 0.5 - 0.5 * v;

    return float2(u, v);
}

float4 MySSR(float2 TexCoord)
{
    float stepLength = 1.5;


	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos.z > g_zFar)
        return float4(0, 0, 0, 1);

	//观察空间法线
    float3 normal = GetNormal(TexCoord, g_sampleNormal);
    normal = normalize(normal);

	//随机纹理
    float3 rand = getRandom(TexCoord);

    stepLength *= rand.x;
    
    float3 viewDir = normalize(pos);
    //viewDir = normalize(float3(0,0,1));
    float3 reflectDir = reflect(viewDir, normal);
    
    reflectDir = normalize(reflectDir);

    //float rand_A_X = 2 * (rand.y - 0.5) * M_PI;
    //float rand_A_Y = 2 * (rand.z - 0.5) * M_PI;
    //reflectDir.xy = RotateDirection(reflectDir.xy, float2(cos(rand_A_X), sin(rand_A_X)));

    stepLength *= (abs(reflectDir.z) + 1);

    //return float4(reflectDir, 1);

    float4 sampleColor = float4(0,0,0,0);

    float2 reflectUV = float2(0, 0);

    bool flag = false;
    int iterations = 30;
    int forward = 0;
    float depth = 0;

    int test = 0;

    float4 withThickness = float4(0, 0, 0, 1);

    //reflectDir = float3(1, 1, 1);
    for (int i = 1; i < iterations; ++i)
    {
        float3 rayPos = pos + reflectDir * stepLength * i;
        float2 rayUV = GetRayUV(rayPos);
        float sampleDepth = GetDepth(rayUV, g_samplePosition);

        float3 lastRayPos = (pos + reflectDir * stepLength * (i - 1));
        float2 lastRayUV = GetRayUV(lastRayPos);
        float lastSampleDepth = GetDepth(lastRayUV, g_samplePosition);

        if (sampleDepth < rayPos.z)
        {
            test = 1;
        }

        //追踪的结果分两种
        //一种是射线击中深度，但是击中的位置是大于厚度的，这种情况下不能确定这个点是不是正确的反射位置，还需要后面的回溯
        //另一种是击中的位置小于厚度，这说明这个点肯定是大致正确的，所以可以提前确定颜色，回溯时会进一步得到精确结果
        if (sampleDepth < rayPos.z && !flag)
        {
            reflectUV = rayUV;
            flag = true;
            forward = i;
            depth = sampleDepth;
            if (rayPos.z - sampleDepth < 1)
            {
                sampleColor = tex2D(g_sampleMainColor, reflectUV);
                sampleColor.a = 1;

            }
        }
    }

    bool flag2 = false;

    

    if (flag > 0)
    {
        float3 rayPos = pos + reflectDir * stepLength * forward;

        //向反方向回溯
        float3 backDir = - reflectDir;
        backDir = normalize(backDir);

        //回溯的步数
        int backStepCount = 10;
        //回溯的步长
        float backStepLength = 1.0f * stepLength / backStepCount;
        
        float3 BackRayPos = rayPos;

        for (int j = 1; j < backStepCount; j++)
        {
            BackRayPos = rayPos + backDir * backStepLength * j;
                        
            float2 rayUV = GetRayUV(rayPos);
            
            float sampleDepth = GetDepth(rayUV, g_samplePosition);

            //如果发现射线深度小于深度图，说明回溯成功
            if ( sampleDepth > BackRayPos.z && flag2 == false )
            {
                reflectUV = rayUV;
                sampleColor = tex2D(g_sampleMainColor, reflectUV);
                sampleColor.a = 1;
                flag2 = true;
            }
        }
    }

    return sampleColor;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
    float4 SSR;

    SSR = MySSR(TexCoord);

    return SSR;

}

float4 texture2DBilinear(sampler2D textureSampler, float2 uv)
{
	float stepU = 1.0f / g_ScreenWidth;
	float stepV = 1.0f / g_ScreenHeight;

	// in vertex shaders you should use texture2DLod instead of texture2D
	float4 tl = tex2D(textureSampler, uv);
	float4 tr = tex2D(textureSampler, uv + float2(stepU, 0));
	float4 bl = tex2D(textureSampler, uv + float2(0, stepV));
	float4 br = tex2D(textureSampler, uv + float2(stepU, stepV));
    float2 f = frac(uv.xy * float2(g_ScreenWidth, g_ScreenHeight)); // get the decimal part
	float4 tA = lerp(tl, tr, f.x); // will interpolate the red dot in the image
	float4 tB = lerp(bl, br, f.x); // will interpolate the blue dot in the image
	return lerp(tA, tB, f.y); // will interpolate the green dot in the image
}

#define SAMPLE_COUNT 15

// 偏移数组
float2 g_SampleOffsets[SAMPLE_COUNT];
// 权重数组
float g_SampleWeights[SAMPLE_COUNT];

float4 Blur(sampler2D textureSampler, float2 TexCoord)
{
    float4 c = 0;

    float depthCenter = tex2D(g_samplePosition, TexCoord).r;
	// 按偏移及权重数组叠加周围颜色值到该像素
	// 相对原理，即可理解为该像素颜色按特定权重发散到周围偏移像素
    float totalWeight = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        float weight = g_SampleWeights[i];

        c += tex2D(textureSampler, TexCoord + g_SampleOffsets[i]) * weight;
        totalWeight += weight;
    }
    c = c / totalWeight;
    return c;
}

float4 DrawBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
    float4 SSR = Blur(g_sampleSSR, TexCoord);

    return SSR;
}

float4 NoDarkBlur(sampler2D textureSampler, float2 uv)
{
    float stepU = 1.0f / g_ScreenWidth;
    float stepV = 1.0f / g_ScreenHeight;
    float2 step = (stepU, stepV);
    float4 color[9];
    
    color[0] = tex2D(textureSampler, uv);
    
    color[1] = tex2D(textureSampler, uv + step * float2(-1, 0));
    color[2] = tex2D(textureSampler, uv + step * float2(1, 0));
    color[3] = tex2D(textureSampler, uv + step * float2(0, -1));
    color[4] = tex2D(textureSampler, uv + step * float2(0, 1));

    color[5] = tex2D(textureSampler, uv + step * float2(-1, -1));
    color[6] = tex2D(textureSampler, uv + step * float2(1, -1));
    color[7] = tex2D(textureSampler, uv + step * float2(-1, 1));
    color[8] = tex2D(textureSampler, uv + step * float2(1, 1));
    

    float weight[9] = {2, 1, 1, 1, 1, 0.7, 0.7, 0.7, 0.7};

    int count = 0;
    float weightSum = 0;
    float4 colorSum = float4(0, 0, 0, 1);
    for (int i = 0; i < 5; i++)
    {
        if (color[i].a != 0)
        {
            colorSum += color[i] * weight[i];
            count++;
            weightSum += weight[i];
        }
        
    }
    colorSum /= weightSum;
        

    return colorSum;
}

float4 DrawMain(float2 TexCoord : TEXCOORD0) : COLOR
{
    float4 SSR = tex2D(g_sampleSSR, TexCoord);
    SSR = NoDarkBlur(g_sampleSSR, TexCoord);
    float4 fianlColor = SSR + tex2D(g_sampleMainColor, TexCoord);

    return SSR;
}

technique SSR
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
    pass p1
    {
        vertexShader = compile vs_3_0 VShader();
        pixelShader = compile ps_3_0 DrawBlur();
    }
	pass p2
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DrawMain();
	}
}