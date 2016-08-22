#include "common.fx"


#define STEPCOUNT 10
#define BISECTION_STEPCOUNT 10

float g_MaxMipLevel = 0;

float g_Roughness = 10.0;
float g_Thickness = 1;
float g_RayAngle = 0.1;

float g_Length = 3;
float g_ScaleFactor = 0.7;

int2 g_RandTexSize;

texture		g_NormalBuffer;
texture g_RandTex;
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

sampler2D g_sampleRandomTex =
sampler_state
{
    Texture = <g_RandTex>;
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
	MinFilter = Point;
    MagFilter = Point;
	MipFilter = Point;


    AddressU = Border;
    AddressV = Border;
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
    return normalize(tex2D(g_sampleRandomTex, float2(g_ScreenWidth, g_ScreenHeight) * uv / g_RandTexSize));
}

float2 GetRayUV(float3 rayPos)
{
    
    float u = g_zNear * rayPos.x / rayPos.z / (g_zNear * g_ViewAngle_half_tan * g_ViewAspect);
    float v = g_zNear * rayPos.y / rayPos.z / (g_zNear * g_ViewAngle_half_tan);

    u = 0.5 * u + 0.5;
    v = 0.5 - 0.5 * v;

    return float2(u, v);
}

float3 ComputeRandomDir(float4 rand, float roughness, float3 N, float3 baseDir, float3 V)
{
    //GGX
    float phi = 2 * rand.z * M_PI;
    float cosTheta = sqrt((1 - rand.y) / (1 + (roughness * roughness - 1) * rand.y));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);

    float H_x = cos(phi) * sinTheta;
    float H_y = sin(phi) * sinTheta;
    float H_z = cosTheta;
    
    //将切线空间的H转换回观察空间
    float3 TangentY = cross(N, baseDir);
    float3 TangentX = cross(TangentY, N);

    float3x3 TBN = float3x3(TangentX, TangentY, N);
    
    float3 randomH = mul(float3(H_x, H_y, H_z), TBN);
    randomH = normalize(randomH);

    //通过H和V计算出光照方向，也就是反射方向的重要性采样
    float3 H = randomH;
    float3 reflectDir = 2 * dot(V, H) * H - V;

    return normalize(reflectDir);
}

float4 MySSR(float2 TexCoord)
{
    //Roughness
    float Roughness = g_Roughness;
    Roughness = max(g_Roughness, 0);

    //最大MipMap等级
    int MaxMipLevel = g_MaxMipLevel;

    //射线追踪的步数
    int StepCount = STEPCOUNT;
    //原始步长
    float Length = g_Length;
    //假象厚度
    float Thickness = g_Thickness;

    //二分回溯的步数
    int BisectionStepCount = BISECTION_STEPCOUNT;
    //光线衰减的系数
    float ScaleFactor = g_ScaleFactor;

    //射线的立体角
    float rayAngle = g_RayAngle * M_PI;
    float tan_RayAngle = tan(rayAngle);
    
    float level = 10.0 / Roughness * MaxMipLevel;

	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);
    float3 V = normalize(-pos);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos.z > g_zFar)
        return float4(0, 0, 0, 1);

	//观察空间法线
    float3 normal = GetNormal(TexCoord, g_sampleNormal);
    normal = normalize(normal);
    
    //计算反射向量
    float3 reflectDir = reflect(pos, normal);
    reflectDir = normalize(reflectDir);

    float3 reflectDirBase = reflectDir;
	//随机纹理
    float4 rand = GetRandom(TexCoord);

    //计算随机角度(应该是用重要性采样来计算角度)
    reflectDir = ComputeRandomDir(rand, Roughness, normal, reflectDirBase, V);

    //这里其实应该重新生成方向
    int count = 0;
    while (dot(reflectDir, normal) < 0 && count < 10 )
    {
        rand = GetRandom(float2(rand.x, rand.y));
        reflectDir = ComputeRandomDir(rand, Roughness, normal, reflectDirBase, V);
        count++;
    }

    //reflectDir = reflect(pos, normal);
    //reflectDir = normalize(reflectDir);
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



    //检查正向射线追踪
    bool checkForward = false;

    //击中的位置
    float3 hitPos;

    //击中点的误差
    float hitDelta = 0;

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
            hitDelta = sampleDepth - rayPos.z;
            break;

        }
    }
    
    //反射的默认颜色
    float4 hitColor = float4(0, 0, 0, 0);
    //默认的输出UV颜色
    float4 hitUv = float4(0, 0, 0, 0);

    //二分法射线追踪
    if (checkForward > 0)
    {
        //二分的步进方向，起始方向是反射射线的相反方向
        float3 BisectionDir = -reflectDir;
        BisectionDir = normalize(BisectionDir);

        //起始的步长
        float BisectionStepLength = stepLength;
        
        //起始的射线起点
        float3 rayPos = hitPos;

        //射线的末端对应UV
        float2 rayUV;

        //从前一步得到的起点开始反向二分法追踪
        //每次将步长减半，并向着二分的查找方向前进
        //在前进到的位置取深度，计算深度图和射线深度的差
        //并且保存下来供下一次查找使用
        //当两次查找的差值符号相同（即两次步进的结果在深度图的同侧）
        //不改变步进方向
        //当两次查找的差值符号相反（即两次步进的结果在深度图的异侧）
        //改变步进方向
        //循环得到精确的二分查找结果
        for (int j = 1; j <= BisectionStepCount; j++)
        {
            //步长减半
            BisectionStepLength *= 0.5;
            //射线前进
            rayPos += BisectionDir * BisectionStepLength;
            //将射线末端转换为UV坐标
            rayUV = GetRayUV(rayPos);
            //采样深度图
            float sampleDepth = GetDepth(rayUV, g_samplePosition);
            //取得当前深度图和射线深度的差值
            float delta = sampleDepth - rayPos.z;
            //两次步进差值符号相反，步进反向
            if (delta * hitDelta < 0)
            {
                BisectionDir = -BisectionDir;
            }
            //保存这次步进的差值
            hitDelta = delta;
        }

        //如果最后一次步进的差值小于一定的值，说明击中的点处于同一平面
        //现在选择了步长的两倍，但是对于接几乎垂直于屏幕的面会出现误判
        //需要研究到底该怎么选择这个值
        if (abs(hitDelta) <= 2 * BisectionStepLength)
        {
            //保存击中的位置
            hitPos = rayPos;
            //击中的点到出发点的距离
            hitLengh = length(hitPos - pos);
            //距离系数
            float disFactor = hitLengh / maxLength * 3 * tan_RayAngle;

            //带有lod级别信息的UV坐标
            hitUv = float4(rayUV, 0, level * disFactor);
            //采样集中的颜色
            hitColor = tex2Dlod(g_sampleMainColor, hitUv);
            
            //根据最大长度计算衰减幅度
            float attenuation = max(maxLength - hitLengh, 0) / maxLength;

            //衰减集中的颜色
            hitColor = hitColor * attenuation * attenuation;

            hitColor.a = 1;

            //击中位置UV为负数或者超出1的像素为屏幕外的像素，需要标记为未击中
            if (hitUv.x < 0 || hitUv.y < 0 || hitUv.x > 1 || hitUv.y > 1)
            {
                hitUv.z = 0;
                hitColor.a = 0;
            }
            else
            {
                //通过b通道(z)保存是否击中的标记
                hitUv.z = 1;
                //通过alpha通道保存是否击中的标记
                hitColor.a = 1;
            }

        }
    }
    
    //return hitColor;
    return hitUv;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
    float4 SSR;

    SSR = MySSR(TexCoord);

    return SSR;

}


float4 ColorResolve(float2 TexCoord : TEXCOORD0) : COLOR
{
    //return tex2D(g_sampleSSR, TexCoord);

    float2 PixelPos = floor(TexCoord * g_ScreenSize);
    float2 modPos = PixelPos % 2;
    float2 halfResPixelCoord = floor(PixelPos * 0.5);

    float2 halfResStep = float2(2.0 / g_ScreenWidth, 2.0 / g_ScreenHeight);
    float2 halfResTexCoord = halfResPixelCoord * halfResStep;

    //return float4(modPos.x, 0, 0, 1);

    float4 neighborUV[4];
    
    neighborUV[0] = tex2D(g_sampleSSR, halfResTexCoord + halfResStep * float2(modPos.x - 1, modPos.y - 1));
    neighborUV[1] = tex2D(g_sampleSSR, halfResTexCoord + halfResStep * float2(modPos.x, modPos.y - 1));
    neighborUV[2] = tex2D(g_sampleSSR, halfResTexCoord + halfResStep * float2(modPos.x - 1, modPos.y));
    neighborUV[3] = tex2D(g_sampleSSR, halfResTexCoord + halfResStep * float2(modPos.x, modPos.y));

    float3 pos = GetPosition(TexCoord, g_samplePosition);
    float3 normal = GetNormal(TexCoord, g_sampleNormal);

    float weightSum = 0;
    float4 resolveColor = float4(0, 0, 0, 0);
    for (int i = 0; i < 4; i++)
    {
        if (neighborUV[i].z > 0.5)
        {
            float3 hitPos = GetPosition(neighborUV[i].xy, g_samplePosition);

            float3 Ray = hitPos - pos;
            float RayLengh = length(Ray);
            Ray = normalize(Ray);
            float3 View = normalize(-pos);
        
            float3 H = normalize(Ray + View);

            float cosTheta = saturate(dot(normal, Ray));
            float theta = acos(cosTheta);
        
            float NoH = saturate(dot(normal, H));
            float NoV = saturate(dot(normal, View));
            float NoL = saturate(dot(normal, Ray));
            float VoH = saturate(dot(View, H));

            float D = g_Roughness * g_Roughness / (M_PI * M_PI * pow(NoH * NoH * (g_Roughness * g_Roughness - 1) + 1, 2));
            float k = (g_Roughness + 1) * (g_Roughness + 1) / 8;
            float G_L = NoV / (NoV * (1 - k) + k);
            float G_V = NoL / (NoL * (1 - k) + k);
            float G = G_L * G_V;
            float3 specularColor = float3(1, 1, 1);
            float Fc = pow(1 - VoH, 5);
            float F = (1 - Fc) * specularColor + Fc;

        //PDF = D * dot(N,H) / (4 * dot(V,H))
            float PDF = D * NoH / (4 * VoH);
        //BRDF = D * G * F / (4 * dot(N,L) * dot(N,V))
            float BRDF = D * G * F / (4 * NoL * NoV);
        
            float weight = 1;
        //lodUV[i].z;

            weight *= BRDF / PDF * cosTheta;

        //return weight;
        
        //根据原始步长和步数以及纵拉伸计算光线的最大长度，使用系数调整
            float maxLength = g_Length * STEPCOUNT * (abs(normalize(Ray).z) + 1) * g_ScaleFactor;

        //根据最大长度计算衰减幅度
            float attenuation = max(maxLength - RayLengh, 0) / maxLength;

        //使用标记使没有击中的像素变成黑色
            float4 hitColor = tex2Dlod(g_sampleMainColor, neighborUV[i]);

        //对于追踪到的像素应用衰减，没有追踪到的像素使用原来的值
            hitColor = neighborUV[i].z ? hitColor * attenuation * attenuation : hitColor;

            resolveColor += hitColor * weight;
            weightSum += weight;
        }
    }

    //return weightSum;

    resolveColor /= weightSum;

    float4 fianlColor = resolveColor;

    return fianlColor;
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
    //SSR = NoDarkBlur(g_sampleSSR, TexCoord);
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
    pass p3
    {
        vertexShader = compile vs_3_0 VShader();
        pixelShader = compile ps_3_0 ColorResolve();
    }
}