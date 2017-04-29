#include "common.fx"


#define STEPCOUNT 10
#define BISECTION_STEPCOUNT 10

float g_MaxMipLevel = 0;

float g_Roughness = 10.0;
float g_RayAngle = 0.1;

float g_StepLength = 3;
float g_MaxLenghFactor = 0.7;
float g_ScaleFactor = 20.0;
float g_ScaleFactor2 = 10.0;

int2 g_RandTexSize;

texture		g_NormalBuffer;
texture     g_RandTex;
texture		g_PositionBuffer;
texture		g_ViewDirBuffer;
texture     g_EnvBRDFLUT;

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
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;

    AddressU = wrap;
    AddressV = wrap;
};

sampler2D g_sampleEnvBRDFLUT =
sampler_state
{
    Texture = <g_EnvBRDFLUT>;
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
	MinFilter = None;
    MagFilter = None;
    MipFilter = None;


    AddressU = Border;
    AddressV = Border;
}; 

texture g_TemporalBuffer;
sampler2D g_sampleTemporal =
sampler_state
{
	Texture = <g_TemporalBuffer>;
	MinFilter = point;
	MagFilter = point;
	MipFilter = point;


	AddressU = clamp;
	AddressV = clamp;
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

float3 ImportanceSampleGGX(float2 rand, float roughness, float3 N, float3 baseDir, float3 V)
{
    //GGX
    float phi = 2 * rand.y * M_PI;
    float cosTheta = sqrt((1 - rand.x) / (1 + (roughness * roughness - 1) * rand.x));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);

    float H_x = cos(phi) * sinTheta;
    float H_y = sin(phi) * sinTheta;
    float H_z = cosTheta;
    
    //将切线空间的H转换回观察空间
    float3 TangentY = cross(N, baseDir);
    float3 TangentX = cross(TangentY, N);

    float3x3 TBN = float3x3(TangentX, TangentY, N);
    
    float3 H = mul(float3(H_x, H_y, H_z), TBN);
    H = normalize(H);

    return normalize(H);
}

float3 ComputeSampleDir(float2 rand, float roughness, float3 N, float3 baseDir, float3 V)
{
    float3 H = ImportanceSampleGGX(rand, roughness, N, baseDir, V);
    float3 SampleDir = 2 * dot(V, H) * H - V;
    return SampleDir;

}

bool RayMarching(float StepLength, int StepCount, int BisectionStepCount, float3 StartPos, float3 RayDir, out float3 hitPos)
{
    //检查正向射线追踪
    bool checkForward = false;
    
    //击中点的误差
    float hitDelta = 0;

	float StepDelta = StepLength;
    //正向的射线追踪
    for (int i = 1; i <= StepCount; ++i)
    {
        float3 rayPos = StartPos + RayDir * StepLength;
        float2 rayUV = GetRayUV(rayPos);
        float sampleDepth = GetDepth(rayUV, g_samplePosition);

        //初步的追踪
		if (sampleDepth < rayPos.z && !checkForward && rayUV.x<1 && rayUV.x >0 && rayUV.y < 1 && rayUV.y > 0)
        {
            checkForward = true;
            hitPos = rayPos;
            hitDelta = sampleDepth - rayPos.z;
            break;

        }
		else
		{
			StepDelta = StepLength;
			StepLength *= 2.0f;
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
        float3 BisectionDir = -RayDir;
        BisectionDir = normalize(BisectionDir);

        //起始的步长
		float BisectionStepLength = StepDelta;
        
        //射线的末端对应UV
        float2 hitUV;

        float sampleDepth;
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
            hitPos += BisectionDir * BisectionStepLength;
            //将射线末端转换为UV坐标
            hitUV = GetRayUV(hitPos);
            //采样深度图
            sampleDepth = GetDepth(hitUV, g_samplePosition);
            //取得当前深度图和射线深度的差值
            float delta = sampleDepth - hitPos.z;
            //两次步进差值符号相反，步进反向
            if (delta * hitDelta < 0)
            {
                BisectionDir = -BisectionDir;
            }
            //保存这次步进的差值
            hitDelta = delta;
        }

        
        //如果最后一次步进的差值小于一定的值，说明击中的点处于同一平面
        //需要研究到底该怎么选择这个值
        
        //现在的方法是:
        //额外采样一次深度图，既可以估算倾斜方向的梯度
        //通过梯度控制最终像素选择的距离误差，一定程度上可以降低噪点
        float sampleDepthLD = GetDepth(hitUV + 1.0 / g_ScreenSize, g_samplePosition);
        float d_depthXY = abs(sampleDepth - sampleDepthLD);

        //if (abs(hitDelta) <= 10 * BisectionStepLength)
        if (abs(hitDelta) <= g_ScaleFactor * (1.0 / (max(d_depthXY, 1)) * g_ScaleFactor2) * BisectionStepLength)
        {
            return true;
        }
    }

    hitPos = float3(0, 0, 0);
    return false;

}

float g_randomOffset;

float4 MySSR(float2 TexCoord)
{
    //最大MipMap等级
    int MaxMipLevel = g_MaxMipLevel;

	float g_R = GetShininess(TexCoord, g_sampleNormal);
    //Roughness
	float Roughness = saturate(g_R);
    //射线追踪的步数
    int StepCount = STEPCOUNT;
    //二分回溯的步数
    int BisectionStepCount = BISECTION_STEPCOUNT;
    //原始步长
    float StepLength = g_StepLength;
    //光线衰减的系数
    float ScaleFactor = g_MaxLenghFactor;

    //射线的立体角
    float RayAngle = g_RayAngle * M_PI;
    float TanRayAngle = tan(RayAngle);
    

	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);
    if (pos.z > g_zFar)
        return float4(0, 0, 0, 0);

	float3 V = normalize(-pos);

		//观察空间法线
		float3 N = GetNormal(TexCoord, g_sampleNormal);

		//计算反射向量
		float3 reflectDirBase = normalize(reflect(pos, N));

		//随机纹理
		float4 rand = GetRandom(TexCoord + g_randomOffset);

	float bias = 0.1;
	//rand.x = lerp(rand.x, 1.0, ScaleFactor);
    //生成随机反射角度
    float3 reflectDir = ComputeSampleDir(rand, Roughness, N, reflectDirBase, V);;
    //反射方向在切平面以下的时候重新生成反射方向
    int count = 0;
    while ((dot(reflectDir, N) <= 0 && count < 10))
    {
        rand = GetRandom(float2(rand.y, rand.z));
        reflectDir = ComputeSampleDir(rand, Roughness, N, reflectDirBase, V);
        count++;
    }
    reflectDir = normalize(reflectDir);

    //return dot(reflectDir, normal) < 0 ? float4(0, 0, 0, 1) : return float4(0, 1, 0, 1);
    
    //生成随机步长，并且在垂直屏幕方向适当的加长步长，可以反射到更远的物体
    float RandStepLength = StepLength * (0.5 + 0.5 * rand.x) * (abs(reflectDir.z) + 1);

    //击中的位置
    float3 hitPos;
    bool rayHit = RayMarching(RandStepLength, StepCount, BisectionStepCount, pos, reflectDir, hitPos);

    
    //反射的默认颜色
    float4 hitColor = float4(0, 0, 0, 0);
    //默认的输出UV颜色
    float4 hitUV = float4(0, 0, 0, 0);

    if(rayHit)
    {
        
        //根据原始步长和步数计算光线的最大长度，使用系数调整
        float maxLength = StepLength * StepCount * 2 * g_MaxLenghFactor;
        
        //击中的点到出发点的距离
        //击中的距离
        float hitLengh = length(hitPos - pos);
            //距离系数
        float disFactor = hitLengh / maxLength * 3 * TanRayAngle;

        //将射线末端转换为UV坐标
        hitUV.xy = GetRayUV(hitPos);
        //带有lod级别信息的UV坐标
        float level = 10.0 / Roughness * MaxMipLevel;
        hitUV = float4(hitUV.xy, 1, level * disFactor);
        //采样集中的颜色
        hitColor = tex2Dlod(g_sampleMainColor, hitUV);
            
        //根据最大长度计算衰减幅度
        float attenuation = max(maxLength - hitLengh, 0) / maxLength;

        //衰减集中的颜色
        //hitColor = hitColor * attenuation * attenuation * attenuation;
        hitColor.a = 1;
        hitUV.w = 1;
    }

    hitUV.xyz = rayHit ? hitPos : reflectDir;


    //return hitColor;
    return hitUV;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
    float4 SSR;

    SSR = MySSR(TexCoord);

    return SSR;

}

struct PixelHit
{
    float3 hitPos;
    float hitPdf;
};

bool g_Switch1;
bool g_ReprojectPassSwitch;
bool g_HitReprojectSwitch;
float4 ColorResolve(float2 TexCoord : TEXCOORD0) : COLOR
{ 
	//return tex2D(g_sampleMainColor, TexCoord);
    //return tex2D(g_sampleSSR, TexCoord);

	float Roughness = GetShininess(TexCoord, g_sampleNormal);

    float2 PixelPos = floor(TexCoord * g_ScreenSize);
    int2 modPos = round(PixelPos % 2);
    int2 halfResPixelPos = floor(PixelPos * 0.5);
    int2 modHalfPos = round(halfResPixelPos % 2);

    float2 fullResStep = float2(1.0 / g_ScreenWidth, 1.0 / g_ScreenHeight);
    float2 halfResStep = float2(2.0 / g_ScreenWidth, 2.0 / g_ScreenHeight);
    float2 halfResTexCoord = halfResPixelPos * halfResStep + fullResStep;

    //return tex2D(g_sampleSSR, halfResTexCoord);

    //return float4(modHalfPos.x, 0, 0, 1);

    float2 offset[4] =
    { float2(0, 0), float2(-2, 2), float2(2, 2), float2(0, -2) };

    float3 pos = GetPosition(TexCoord, g_samplePosition);
    
    float3 V = normalize(-pos);

    float3 N = GetNormal(TexCoord, g_sampleNormal);
    
    float NoV = saturate(dot(N, V));

    //根据原始步长和步数以及纵拉伸计算光线的最大长度，使用系数调整
    float maxLength = g_StepLength * STEPCOUNT * 2 * g_MaxLenghFactor;

    float weightSum = 0;
	float4 resolveColor = float4(0, 0, 0, 0);

		int count = 4;
	if (g_Switch1)
	{
		count = 1;
	}
	else
	{
		count = 4;
	}

	float3 hit;

	int hitCount = 0;

	float fadeSum = 0;

	for (int i = 0; i < count; i++)
    {
		float4 PixelHit = tex2D(g_sampleSSR, TexCoord + fullResStep * offset[i]);
        bool RayHit = PixelHit.w < 0.5 ? false : true;

        if (RayHit)
		{
			hit = PixelHit.xyz;

            float3 hitPos = PixelHit.xyz;
            float2 hitUV = GetRayUV(hitPos);
            
			//因为是上一帧的图像，所以要进行一次重投影
			float4 posW = mul(float4(hitPos, 1.0f), g_invView);
			//上一帧的观察空间位置
			float4 lastPosV = mul(posW, g_LastView);
			float2 lastUV = GetRayUV(lastPosV.xyz);
			if (!g_HitReprojectSwitch)
				hitUV = lastUV;
			
            float4 hitColor = tex2Dlod(g_sampleMainColor, float4(hitUV, 0, 0));

            float2 fadeUVCoord = abs(hitUV * 2.0 - 1);
            float fade = max(fadeUVCoord.x, fadeUVCoord.y);
            float screenFade = 0.75;
            float fadeFactor = 1.0 - max(fade - screenFade, 0.0) / (1.0 - screenFade);
            //fadeFactor = min(fadeUVCoord.x, fadeUVCoord.y) > screenFade ? 1 - sqrt(pow(fadeUVCoord.x - screenFade, 2) + pow(fadeUVCoord.y - screenFade, 2)) / (1 - screenFade) : fadeFactor;
            fadeFactor = saturate(fadeFactor);
            
			fadeSum += fadeFactor;

            float3 L = RayHit ? hitPos - pos : hitPos - pos;
            
            //to do
            float RayLengh = length(L);
            //根据最大长度计算衰减幅度
            float attenuation = max(maxLength - RayLengh, 0) / maxLength;

            //对于追踪到的像素应用衰减，没有追踪到的像素使用原来的值
			hitColor = hitColor;// *attenuation * attenuation * fadeFactor;


            L = normalize(L);

            float3 H = normalize(L + V);

            float NoH = saturate(dot(N, H));
			float NoL = saturate(dot(N, L));
			float VoH = saturate(dot(V, H));
			float LoH = saturate(dot(L, H));
            
            float cosTheta = saturate(dot(N, L));
            float theta = acos(cosTheta);

			float a = Roughness * Roughness;

			//D
			a = max(a, 0.001);
			float D = a * a / (M_PI * pow(NoH * NoH * (a * a - 1) + 1, 2));

			//G
			float k = (Roughness + 1) * (Roughness + 1) / 8;
			k = a / 2;
			float G_V = NoV / (NoV * (1 - k) + k);
			float G_L = NoL / (NoL * (1 - k) + k);
			float G = G_L * G_V;

			//F Schlick
			float3 specularColor = float3(1, 1, 1);
			float3 F = specularColor + (1 - specularColor) * pow(1 - LoH, 5);
            
            //BRDF = D * G * F / (4 * dot(N,L) * dot(N,V))
            float BRDF = D * G * F / (4 * NoL * NoV);

            //PDF = D * dot(N,H) / (4 * dot(V,H))
			float PDF = D * NoH / (4 * VoH);
            
            float weight = 1;
            
			weight *= BRDF / PDF *cosTheta;

            resolveColor += hitColor * weight;
			//resolveColor += hitColor * F *G * VoH / (NoH * NoV);
           weightSum += weight;
		   hitCount++;
        }
    }

    //return weightSum;
    float2 EnvBRDF = tex2D(g_sampleEnvBRDFLUT, float2(NoV, Roughness));
    
	resolveColor /= weightSum;
	float avgFade = fadeSum / hitCount;
	resolveColor.a = 1.0f * hitCount / count * avgFade;

	float4 fianlColor = resolveColor;// *(EnvBRDF.x + EnvBRDF.y);
		
	//===================================================
	//reprojection
	//世界空间位置
	float3 hisPos = pos;
	/*
	float3 Hit = tex2D(g_sampleSSR, TexCoord);
	hisPos = Hit;
	float2 curUV = GetRayUV(Hit.xyz);
	*/
	//世界坐标
	float4 posW = mul(float4(hisPos, 1.0f), g_invView);
	//上一帧的观察空间位置
	float4 lastPosV = mul(posW, g_LastView);
	//float4 lastPosP = mul(lastPosV, g_Proj);//X
	//float2 lastUV = lastPosP.xy * float2(1, -1) + float2(0.5, 0.5);
	float2 lastUV = GetRayUV(lastPosV.xyz);
	/*
	float2 dUV = lastUV - curUV;
	lastUV = TexCoord - dUV;
	*/
	float4 historyColor = tex2D(g_sampleTemporal, lastUV);

	float4 neighborMin, neighborMax;
	// calculate neighborMin, neighborMax by
	// iterating through 9 pixels in neighborhood
	//historyColor = clamp(historyColor, neighborMin, neighborMax);

	float4 final = lerp(historyColor, fianlColor, 0.05f);

	if (g_ReprojectPassSwitch)
		return fianlColor;
	else
		return final;
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

bool g_ClampHistorySwitch;
float4 Temporal(float2 TexCoord : TEXCOORD0) : COLOR
{
	//观察空间位置
	float3 pos = GetPosition(TexCoord, g_samplePosition);
	//世界空间位置
	float4 posW = mul(float4(pos, 1.0f), g_invView);
	//上一帧的观察空间位置
	float4 lastPosV = mul(posW, g_LastView);
	float2 lastUV = GetRayUV(lastPosV.xyz);

	float4 historyColor = tex2D(g_sampleTemporal, lastUV);

	float2 fullResStep = float2(1.0 / g_ScreenWidth, 1.0 / g_ScreenHeight);

	float4 minColor, maxColor;
	float4 SSR = tex2D(g_sampleSSR, TexCoord);
	if (g_ClampHistorySwitch)
	{
		minColor = SSR;
		maxColor = SSR;
		int num = 4;
		for (int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; j++)
			{
				float2 offset = float2(i - num / 2, j - num / 2);
					offset *= fullResStep;
				float4 curColor = tex2D(g_sampleSSR, TexCoord + offset);


				if (curColor.x != 0 && curColor.y != 0 && curColor.z != 0)
				{
					maxColor = max(maxColor, curColor);
					minColor = min(minColor, curColor);
				}
			}
		}
		historyColor = clamp(historyColor, minColor, maxColor);
	}
	
	float4 final = lerp(historyColor, SSR, 0.05f);
	//return SSR; 
	return final;
}

float4 DrawMain(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 ssrColor = tex2D(g_sampleSSR, TexCoord);
	float g_R = GetShininess(TexCoord, g_sampleNormal);

	float4 final = ssrColor;
	//return final * (1 - g_R) + mainColor * g_R;// *SSR.a + (1 - SSR.a) * float4(0, 0, 0, 1);
	ssrColor.rgb *= (1 - g_R);
	return ssrColor;
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
	pass p4
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 Temporal();
	}
}