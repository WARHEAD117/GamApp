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

    AddressU = wrap;
    AddressV = wrap;
};

sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;

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

//================================================================

float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (dot(V1, V1) < dot(V2, V2)) ? V1 : V2;
}

//#endif //DEINTERLEAVED_TEXTURING

//----------------------------------------------------------------------------------
float Falloff(float DistanceSquare, float R)
{
    // 1 scalar mad instruction
    return DistanceSquare * ( -1.0f / (R * R)) + 1.0;
}

//----------------------------------------------------------------------------------
// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
//----------------------------------------------------------------------------------
float ComputeAO(float3 P, float3 N, float3 S, float bias, float R)
{
    float3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * rsqrt(VdotV);

    // Use saturate(x) instead of max(x,0.f) because that is faster on Kepler
    return saturate(NdotV - bias) * saturate(Falloff(VdotV, R));
}

//----------------------------------------------------------------------------------
float2 RotateDirection(float2 Dir, float2 CosSin)
{
    return float2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                  Dir.x * CosSin.y + Dir.y * CosSin.x);
}

//----------------------------------------------------------------------------------
float4 GetJitter()
{
#if !USE_RANDOM_TEXTURE
    return float4(1, 0, 1, 1);
#else
#if DEINTERLEAVED_TEXTURING
        // Get the current jitter vector from the per-pass constant buffer
        return g_Jitter;
#else
        // (cos(Alpha),sin(Alpha),rand1,rand2)
        return RandomTexture.Sample(PointWrapSampler, float3(IN.pos.xy / RANDOM_TEXTURE_WIDTH, 0));
#endif
#endif
}


float M_PI = 3.1415926;

//----------------------------------------------------------------------------------
float ComputeCoarseAO(float2 FullResUV, float RadiusPixels, float4 Rand, float3 ViewPosition, float3 ViewNormal, float R)
{
#if DEINTERLEAVED_TEXTURING
    RadiusPixels /= 4.0;
#endif
    
    int NUM_STEPS = 4;
    int NUM_DIRECTIONS = 6;

    //每一步前进几个像素（半径除以步数）
    // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
    float StepSizePixels = RadiusPixels / (NUM_STEPS + 1);

    const float Alpha = 2.0 * M_PI / NUM_DIRECTIONS;
    float AO = 0;

    //[unroll]
    for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
    {
        float Angle = Alpha * DirectionIndex;

        //这个方向是每一步前进的方向，按照Angle和一个随机值进行旋转
        // Compute normalized 2D direction
        float2 randomRotate = float2(cos(Rand.x), sin(Rand.x));
        float2 Direction = RotateDirection(float2(cos(Angle), sin(Angle)), randomRotate);

        //为什么要+1？
        // Jitter starting sample within the first step
        float RayPixels = (Rand.z * StepSizePixels + 1.0);

        //[unroll]
        for (float StepIndex = 0; StepIndex < NUM_STEPS; ++StepIndex)
        {
#if DEINTERLEAVED_TEXTURING
            float2 SnappedUV = round(RayPixels * Direction) * g_InvQuarterResolution + FullResUV;
            float3 S = FetchQuarterResViewPos(SnappedUV);
#else
            //根据前进的步长和方向算出整数的像素数，然后乘每个像素的uv大小。加上中心点的uv，就是最终的采样uv值
            float2 g_InvFullResolution = float2(1.0f / g_ScreenWidth, -1.0f / g_ScreenHeight);
            float2 SnappedUV = round(RayPixels * Direction) * g_InvFullResolution + FullResUV;
            float3 S = GetPosition(SnappedUV, g_samplePosition);
#endif
            //累加步长，得到下一步的位置
            RayPixels += StepSizePixels;

            float bias = 0.3;
            AO += ComputeAO(ViewPosition, ViewNormal, S, bias, R);
        }
    }
    float g_AOMultiplier = 1.8;
    AO *= g_AOMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
    return saturate(1.0 - AO * 2.0);
}

float NVIDIA_CoarseAO(float2 TexCoord)
{
    float R = 0.7;

	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);

	//观察空间法线
    float3 normal = GetNormal(TexCoord, g_sampleNormal);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos.z > g_zFar)
        return float4(1, 1, 1, 1);

    //下面的式子化简后就是只有一行的r_v
    //float dis = R * g_zNear / pos.z;
    //float height = 2 * g_zNear * g_ViewAngle_half_tan;
    //float r_v = dis / height;

    float r_v = 0.5 * R / pos.z / g_ViewAngle_half_tan;

    //计算出3D空间中的半径相当于多少个像素
    float RadiusPixels = r_v * g_ScreenHeight;

	//随机（现在的图不太好）
    /*
    float2 rand = getRandom(TexCoord);
    
    float projectXY = sqrt(rand.x * rand.x + rand.y * rand.y);
    float cosA = rand.x / projectXY;
    float A = acos(cosA);
    */

    float4 Rand = GetJitter();
    Rand = tex2D(g_sampleRandomNormal, float2(g_ScreenWidth, g_ScreenHeight) * TexCoord / float2(256, 256));
    float AO = ComputeCoarseAO(TexCoord, RadiusPixels, Rand, pos, normal, R);
    return float4(AO, AO, AO, 1);
}
//==========================================================



float doAO(float3 normal, float3 H_Vec, float2 vec, float3 tangent)
{
    float h = atan(-H_Vec.z / length(H_Vec.xy));
   // float x = -1 * H_Vec.y / H_Vec.x;
    //float3 n = normalize(float3(x, 1, 0));
    float b = dot(normal, float3(vec, 0)) / length(vec);
    float3 pn = b * normalize(float3(vec, 0));
    float t = -atan(length(pn.xy) / normal.z);
   // t = atan(tangent.z / length(tangent.xy));
    
    if (dot(normal, H_Vec) > 0.80)
        return 0.0f;

    if (h < -t+ 3.1415 / 6)
        return 0;
    float ao = sin(h) - sin(t);
    return clamp(ao, 0, 1);
}

float3 getRandom(in float2 uv)
{
    return normalize(tex2D(g_sampleRandomNormal, /*g_screen_size*/float2(g_ScreenWidth, g_ScreenHeight) * uv / /*random_size*/float2(256, 256)).xyz );
}

float3 tangent_eye_pos(float2 uv, float z, float4 tangentPlane)
{
    // view vector going through the surface point at uv
    float3 V = float3(uv, z);
    float NdotV = dot(tangentPlane.xyz, V);
    // intersect with tangent plane except for silhouette edges
    if (NdotV < 0.0)
        V *= (tangentPlane.w / NdotV);
    return V;
}

float3 min_diff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (length(V1) < length(V2)) ? V1 : V2;
}

float3 tangent_vector(float2 deltaUV, float3 dPdu, float3 dPdv)
{
    return deltaUV.x * dPdu + deltaUV.y * dPdv;
}

float4 MyAO(float2 TexCoord)
{
    float R = 0.7;
    int stepCount = 6;

	//观察空间位置
    float3 pos = GetPosition(TexCoord, g_samplePosition);
	//return float4(pos.z, pos.z, pos.z, 1.0f);
    float dis = R * g_zNear / pos.z;

    float height = 2 * g_zNear * g_ViewAngle_half_tan;
    float width = 2 * g_zNear * g_ViewAngle_half_tan * g_ViewAspect;

    float2 r_uv = float2(dis / width, dis / height);

    float RadiusPixels = r_uv.x * g_ScreenWidth;

    float step = RadiusPixels / (stepCount + 1);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
    if (pos.z > g_zFar)
        return float4(1, 1, 1, 1);

	//观察空间法线
    float3 normal = GetNormal(TexCoord, g_sampleNormal);
    normal = normalize(normal);

	//随机法线
    float3 rand = getRandom(TexCoord);
    

    int iterations = 6;
    float totalAo = 0;
    for (int i = 0; i < iterations; ++i)
    {
        float2 dir = float2(cos(i * 3.14159 / 3), sin(i * 3.14159 / 3));
        dir = RotateDirection(dir, float2(cos(rand.x), sin(rand.x)));
        dir = normalize(dir);

        float RayPixel = (rand.z * step + 1.0);
        //RayPixel = 1.0;

        
        float b = dot(float3(dir, 0), normal);
        b = normal.z > 0 ? -1 : 1;

        float4 tangentPlane;
        tangentPlane = float4(normal, dot(pos, normal));
        float3 tangent = cross(float3(RotateDirection(dir, float2(cos(-0.5 * M_PI), sin(-0.5 * M_PI))), 0), normal);

        tangent = normalize(tangent);
        
        //return float4(tangent, 1.0f);

        float3 pn = b * float3(dir, 0);
        float sign = b < 0 ? -1 : 1;
        float t = b * atan(length(pn.xy) / normal.z);
        t = atan(-tangent.z / length(tangent.xy));

        float wao = 0;
        float lastAo = 0;
        float maxAngle = t + M_PI / 6.0f;
        maxAngle = clamp(maxAngle, -0.5 * M_PI, 0.5 * M_PI);

        float ao = 0;
        float h = 0;
        float2 sampleCoord = TexCoord;
        for (int j = 0; j < stepCount; j++)
        {
            //根据前进的步长和方向算出整数的像素数，然后乘每个像素的uv大小。加上中心点的uv，就是最终的采样uv值
            float2 g_InvFullResolution = float2(1.0f / g_ScreenWidth, -1.0f / g_ScreenHeight);
            float2 SnappedUV = round(RayPixel * dir) * g_InvFullResolution + TexCoord;

            float3 samplePos = GetPosition(SnappedUV, g_samplePosition);

            RayPixel += step;

            float3 H_Vec = samplePos - pos;

            //用来计算衰减
            float l = length(H_Vec);
            float r = l / R;
            float wr = 1 - r * r;

            //保存上一次的ao结果
            lastAo = ao;

            h = atan(-H_Vec.z / length(H_Vec.xy));

            //if (l < R && h > maxAngle && dot(H_Vec, normal) > 0.0)
            if (l < R )
            {
                //maxAngle = h;
                //ao = doAO(normal, H_Vec, dir, tangent);

                //这个是改进后，法线方向半球的遮挡计算，0.3是偏移值
                //ao = saturate(dot(normalize(H_Vec), normalize(normal)) - 0.3);

                //这个是原始的计算方式...
                ao = sin(h) - sin(t);
                if(h < t)
                {
                    //ao = 0;
                    //return float4(1, 0, 0, 1);
                }
                //ao = max(ao, 0);
                
            }
            
            wao += wr * (ao - lastAo);

        }
        totalAo += wao;

    }
    totalAo = 1 - totalAo / (float) iterations;

    totalAo = clamp(totalAo, 0, 1);
    return float4(totalAo, totalAo, totalAo, 1);
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
    float4 AO;

    //AO = NVIDIA_CoarseAO(TexCoord);
    AO = MyAO(TexCoord);

    return AO;

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