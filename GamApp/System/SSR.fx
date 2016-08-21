#include "common.fx"

int			g_mapWidth;
int			g_mapHeight;

float       g_Roughness = 10.0;
float       g_MaxMipLevel = 0;
float		g_Thickness = 1;

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


float4 GetRandom(in float2 uv)
{
    return normalize(tex2D(g_sampleRandomNormal, float2(g_ScreenWidth, g_ScreenHeight) * uv / /*random_size*/float2(256, 256)) );
}

float2 GetRayUV(float3 rayPos)
{
    
    float u = g_zNear * rayPos.x / rayPos.z / (g_zNear * g_ViewAngle_half_tan * g_ViewAspect);
    float v = g_zNear * rayPos.y / rayPos.z / (g_zNear * g_ViewAngle_half_tan);

    u = 0.5 * u + 0.5;
    v = 0.5 - 0.5 * v;

    return float2(u, v);
}

float3 ComputeRandomDir(float4 rand, float factor, float3 normal, float3 baseDir)
{
    float theta = (rand.y - 0.5) * M_PI * factor;
    float phi = 2 * (rand.z - 0.5) * M_PI;
    float x = cos(phi) * sin(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(theta);

    float3 b = cross(normal, baseDir);
    float3 t = cross(b, baseDir);

    float3x3 TBN = float3x3(t, b, baseDir);
    
    float3 randomDir = mul(float3(x, y, z), TBN);
    randomDir = normalize(randomDir);

    return randomDir;
}

float4 MySSR(float2 TexCoord)
{
    //Roughness
    float Roughness = 0.2;
    Roughness = g_Roughness;
    Roughness = max(g_Roughness, 0);

    //最大MipMap等级
    int MaxMipLevel = g_MaxMipLevel;

    //射线追踪的步数
    int StepCount = 20;
    //原始步长
    float Length = 1.5;
    //假象厚度
    float Thickness = 1;
    Thickness = g_Thickness;

    //回溯的步数
    int BackStepCount = 10;
    //光线衰减的系数
    float ScaleFactor = 0.7;

    
    float level = 1.0 / Roughness * MaxMipLevel;

	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos.z > g_zFar)
        return float4(0, 0, 0, 1);

	//观察空间法线
    float3 normal = GetNormal(TexCoord, g_sampleNormal);
    normal = normalize(normal);
    
    //计算反射向量
    float3 reflectDir = reflect(pos, normal);
    reflectDir = normalize(reflectDir);

	//随机纹理
    float4 rand = GetRandom(TexCoord);

    //计算随机角度(应该是用重要性采样来计算角度)
    float theta = (rand.y - 0.5) * M_PI * Roughness;
    theta = acos(pow(rand.y, (1.0 / (1.0 + Roughness))));

    float phi = 2 * M_PI * rand.z;
    float x = cos(phi) * sin(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(theta);

    float3 b = cross(normal, reflectDir);
    float3 t = cross(b, reflectDir);

    float3x3 TBN = float3x3(t, b, reflectDir);
    reflectDir = mul(float3(x,y,z), TBN);
    reflectDir = normalize(reflectDir);

    //这里其实应该重新生成方向
    int count = 0;
    while (dot(reflectDir, normal) < 0 && count < 10 )
    {
        rand = GetRandom(float2(rand.x, rand.y));
        reflectDir = ComputeRandomDir(rand, Roughness, normal, reflectDir);
        count++;
    }
    /*
    if (dot(reflectDir, normal) < 0)
    {
        return float4(0, 0, 0, 1);
    }
    else
    {
        return float4(0, 1, 0, 1);
    }
    */
    
    //生成随机步长，并限制步长的最小值
    float stepLength = Length * (0.5 + 0.5 * rand.x);
    //在垂直屏幕方向适当的加长步长，可以反射到更远的物体
    stepLength *= (abs(reflectDir.z) + 1);


    //反射的默认颜色
    float4 sampleColor = float4(0,0,0,0);

    //检查正向射线追踪
    bool checkForward = false;

    //击中的位置
    float3 hitPos;

    //击中的距离
    float hitLengh = 0;
    
    //根据原始步长和步数以及纵拉伸计算光线的最大长度，使用系数调整
    float maxLength = Length * StepCount * (abs(reflectDir.z) + 1) * ScaleFactor;

    //正向的射线追踪
    for (int i = 1; i <= StepCount; ++i)
    {
        float3 rayPos = pos + reflectDir * stepLength * i;
        float2 rayUV = GetRayUV(rayPos);
        float sampleDepth = GetDepth(rayUV, g_samplePosition);

        //初步的追踪
        if (sampleDepth < rayPos.z && !checkForward)
        {
            checkForward = true;
            hitPos = rayPos;
            break;

        }
    }

    //检查反向射线追踪
    bool checkBack = false;

    //反向射线追踪
    if (checkForward > 0)
    {
        //向反方向回溯
        float3 backDir = - reflectDir;
        backDir = normalize(backDir);

        //回溯的步长
        float BackStepLength = 1.0f * stepLength / BackStepCount;
        
        for (int j = 1; j <= BackStepCount; j++)
        {
            //起点为正向击中的位置
            float3 rayPos = hitPos + backDir * BackStepLength * j;
            float2 rayUV = GetRayUV(rayPos);
            float sampleDepth = GetDepth(rayUV, g_samplePosition);

            //如果发现射线深度小于深度图，说明回溯成功
            if (sampleDepth > rayPos.z && sampleDepth - rayPos.z < Thickness && checkBack == false)
            {
                checkBack = true;
                
                //更新击中的位置
                hitPos = rayPos;
                hitLengh = length(hitPos - pos);
                float disFactor = hitLengh / maxLength * 3;
                float4 lodUV = float4(rayUV, 0, level * disFactor * disFactor);
                sampleColor = tex2Dlod(g_sampleMainColor, lodUV);
                sampleColor.a = 1;


            }
        }
    }
    
    //根据最大长度计算衰减幅度
    float attenuation = max(maxLength - length(hitPos - pos), 0) / maxLength;

    //对于追踪到的像素应用衰减，没有追踪到的像素使用原来的值
    sampleColor = checkForward ? sampleColor * attenuation * attenuation : sampleColor;

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