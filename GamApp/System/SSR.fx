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
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;

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
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;

    MinFilter = linear;
    MagFilter = linear;
    MipFilter = linear;

    AddressU = Border;
    AddressV = Border;
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


float4 MySSR(float2 TexCoord)
{
    float stepLength = 1.1;


	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos.z > g_zFar)
        return float4(1, 1, 1, 1);

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

    //return float4(reflectDir, 1);

    float4 sampleColor = float4(0,0,0,1);

    float2 reflectUV = float2(0, 0);

    int flag = -1;
    int iterations = 40;
    int forward = 0;
    float depth = 0;

    int test = 0;

    //reflectDir = float3(1, 1, 1);
    for (int i = 1; i < iterations; ++i)
    {
        float3 rayPos = pos + reflectDir * stepLength * i;
        float lasrRayPos = (pos + reflectDir * stepLength * (i - 1)).z;

        float u = g_zNear * rayPos.x / rayPos.z / (g_zNear * g_ViewAngle_half_tan * g_ViewAspect);
        float v = g_zNear * rayPos.y / rayPos.z / (g_zNear * g_ViewAngle_half_tan);

        u = 0.5 * u + 0.5;
        v = 0.5 - 0.5 * v;

        float sampleDepth = GetDepth(float2(u,v), g_samplePosition);

        
        if (sampleDepth < rayPos.z)
        {
            test = 1;
        }
        /*
        if (
            reflectDir.z >= 0 && sampleDepth < rayPos.z && sampleDepth > lasrRayPos + 0.1
         || reflectDir.z < 0 && sampleDepth > rayPos.z && lasrRayPos > sampleDepth + 0.1
            )
        */
        if (sampleDepth < rayPos.z && rayPos.z - sampleDepth < 1 )
        {
            reflectUV = float2(u, v);
            flag = 1;
            forward = i;
            depth = sampleDepth;
            break;
        }


    }

    //return float4(test, 0, 0, 1);

    bool flag2 = false;

    if(flag > 0)
    {
        
        float3 rayPos = pos + reflectDir * stepLength * forward;
        float target = depth - rayPos.z;
        target = target / abs(target);
        float3 backDir = target * reflectDir;

        int backStepCount = 10;
        float backStepLength = 1.0f * stepLength / backStepCount;
        
        for (int j = 1; j < backStepCount; j++)
        {
            float3 BackRayPos = rayPos + backDir * backStepLength * j;
            
            float lasrRayPos = (pos + reflectDir * stepLength * (j - 1)).z;
            
            float u = g_zNear * BackRayPos.x / BackRayPos.z / (g_zNear * g_ViewAngle_half_tan * g_ViewAspect);
            float v = g_zNear * BackRayPos.y / BackRayPos.z / (g_zNear * g_ViewAngle_half_tan);

            u = 0.5 * u + 0.5;
            v = 0.5 - 0.5 * v;
            
            float sampleDepth = GetDepth(float2(u, v), g_samplePosition);

            if (
            (reflectDir.z > 0 && sampleDepth < rayPos.z && lasrRayPos + 0.001 < sampleDepth
            || reflectDir.z < 0 && sampleDepth > rayPos.z && lasrRayPos > sampleDepth + 0.001)
             && flag2 == false)
            {
                reflectUV = float2(u, v);
                flag2 = true;

            }
        }
    }
    
    //return float4(flag2, 0, 0, 1);

    sampleColor = flag > 0 ? tex2D(g_sampleMainColor, reflectUV) : float4(0, 0, 0, 1);
    //sampleColor = tex2D(g_sampleMainColor, reflectUV);

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
	float4 fianlColor = AO * tex2D(g_sampleMainColor, TexCoord);

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