#include "common.fx"

texture		g_DiffuseBuffer;
texture		g_NormalBuffer;
texture		g_PositionBuffer;

texture		g_ShadowBuffer;
texture		g_PointShadowBuffer;

texture		g_ShadowResult;

int			g_ShadowMapSize = 2048;
float		g_ShadowBias = 0.2f;

float		g_MinVariance = 0.02;
float		g_Amount = 1.0f;

float4		g_LightDir;
float4		g_LightPos;
float		g_LightRange;
float4		g_LightCosAngle;
float4		g_LightAttenuation;

//lightVolume================
matrix g_LightVolumeWVP;
matrix g_ToViewDirMatrix;
//===========================

bool		g_bUseShadow;
float4		g_LightColor;
float4		g_AmbientColor;

matrix g_ShadowView;
matrix g_ShadowProj;

matrix g_viewToLightProj;
matrix g_viewToLight;

sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = point;
	MagFilter = point;
	MipFilter = point;
};

sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = point;
	MagFilter = point;
	MipFilter = point;
};

sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
//----------------------------
//如果使用PCF算法，必须使用Point过滤
//如果使用VSM，就可使用别的过滤方式，比如ANISOTROPIC或者linear
sampler g_sampleShadow =
sampler_state
{
	Texture = <g_ShadowBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;// ANISOTROPIC;Point;
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler2D g_sampleShadowResult =
sampler_state
{
	Texture = <g_ShadowResult>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;// ANISOTROPIC;//GAUSSIANQUAD
	AddressU = Clamp;
	AddressV = Clamp;
}; 

texture g_Sky;
sampler2D g_sampleSky =
sampler_state
{
	Texture = <g_Sky>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
	AddressU = wrap;
	AddressV = wrap;
};

texture g_SkyCube;
samplerCUBE  g_sampleSkyCube =
sampler_state
{
	Texture = <g_SkyCube>;
	MinFilter = linear;
	MagFilter = linear;
	//MipFilter = linear;
	AddressU = wrap;
	AddressV = wrap;
};

texture g_SSRBuffer;
sampler2D g_sampleSSR =
sampler_state
{
	Texture = <g_SSRBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;

	AddressU = Border;
	AddressV = Border;
};

//----------------------------
struct OutputVS_Quad
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};
struct OutputVS_LightVolume
{
	float4 posWVP         : POSITION0;
	float4 TexCoord		: TEXCOORD0;
	float4 viewDir		: TEXCOORD1;
};

OutputVS_Quad VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS_Quad outVS = (OutputVS_Quad)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

OutputVS_LightVolume VShaderLightVolume(float4 posL       : POSITION0)
{
	OutputVS_LightVolume outVS = (OutputVS_LightVolume)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_LightVolumeWVP);

	outVS.TexCoord = outVS.posWVP;

	outVS.viewDir = mul(posL, g_ToViewDirMatrix);
	return outVS;
}


float linstep(float min, float max, float v)
{
	return clamp((v - min) / (max - min), 0, 1);
}

float ReduceLightBleeding(float p_max, float Amount)
{
	return linstep(Amount, 1, p_max);
}

float ChebyshevUpperBound(float2 Moments, float t)
{
	float p = (t <= Moments.x);

	float Variance = Moments.y - (Moments.x*Moments.x);
	Variance = max(Variance, g_MinVariance);

	float d = t - Moments.x;
	float p_max = Variance / (Variance + d*d);

	p_max = ReduceLightBleeding(p_max, g_Amount);

	return max(p, p_max);
}

float Vis_Smith(float Roughness, float NoV, float NoL)
{
	float a = pow(Roughness,2);
	float a2 = a*a;

	float Vis_SmithV = NoV + sqrt(NoV * (NoV - NoV * a2) + a2);
	float Vis_SmithL = NoL + sqrt(NoL * (NoL - NoL * a2) + a2);
	return rcp(Vis_SmithV * Vis_SmithL);
}

float rnd(float2 uv) {
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}

float2 Hammersley(uint i, uint N) 
{
	float sini = sin(float(i));
	float cosi = cos(float(i));
	float2 rand2 = float2(float(i) / float(N), rnd(float2(sini, cosi)));
	return rand2;
}
float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N) {
	float a = Roughness * Roughness;
	float Phi = 2 * M_PI * Xi.x; 
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y)); 
	float SinTheta = sqrt(1 - CosTheta * CosTheta);
	float3 H; H.x = SinTheta * cos(Phi); 
	H.y = SinTheta * sin(Phi); H.z = CosTheta;
	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0); 
	float3 TangentX = normalize(cross(UpVector, N)); 
	float3 TangentY = cross(N, TangentX); // Tangent to world space 
	return TangentX * H.x + TangentY * H.y + N * H.z;
}
float3 PrefilterEnvMap(float Roughness, float3 R) {
	float3 N = R; 
	float3 V = R;
	float3 PrefilteredColor = 0;
	const uint NumSamples = 196; 
	float TotalWeight = 0;
	for (uint i = 0; i < NumSamples; i++) 
	{
		float2 Xi = Hammersley(i, NumSamples); 
		float3 H = ImportanceSampleGGX(Xi, Roughness, N); 
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(dot(N, L)); 
		if (NoL > 0) 
		{ 
			//PrefilteredColor += EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb * NoL; TotalWeight += NoL;

			float2 skyUV = float2((1 + atan2(L.x, -L.z) / M_PI) / 2, acos(L.y) / M_PI);
			float r1 = (1 / M_PI)*acos(L.z) / sqrt(L.x * L.x + L.y * L.y);
			skyUV = float2(0.5, -0.5) * float2(L.x * r1 + 1, L.y * r1 + 1);

			float4 Texture = tex2Dlod(g_sampleSky, float4(skyUV,0,0));
			//线性空间和伽马空间的转换...不明白为什么
			//但是不转换的话，颜色不对会发红
			//DX的textureViewer也有同样的问题
			Texture = pow(Texture, 1 / 2.2);

			//Texture = texCUBE(g_sampleSkyCube, L);
			PrefilteredColor += Texture.rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}


void IBL_BRDF(float3 normal, float3 toEye, float3 diffuseColor, float3 specularColor, inout float3 DiffuseLight, inout float3 SpecularLight, in float Roughness)
{
	float3 V = normalize(toEye);
	float3 N = normalize(normal);
	//	float3 H = normalize(L + V);

	//	float NoH = saturate(dot(N, H));
	float NoV = saturate(dot(N, V));
	//float VoH = saturate(dot(V, H));

	//float cosTheta = saturate(dot(N, L));
	//float theta = acos(cosTheta);

	float a = Roughness * Roughness;

	// [ Lazarov 2013, "Getting More Physical in Call of Duty: Black Ops II" ]
	// Adaptation to fit our G term.		 
	float3 R = 2 * dot(V, N) * N - V;

	float4 R_W = mul(R, g_invView);

	float3 dir = (R_W);
	float2 skyUV = float2((1 + atan2(dir.x, -dir.z) / M_PI) / 2, acos(dir.y) / M_PI);
	float r1 = (1 / M_PI)*acos(dir.z) / sqrt(dir.x * dir.x + dir.y * dir.y);
	skyUV = float2(0.5,-0.5) * float2(dir.x * r1 + 1, dir.y * r1 + 1);

	float3 Texture = tex2D(g_sampleSky, skyUV).rgb;
	//Texture = texCUBE(g_sampleSkyCube, dir);
	Texture = PrefilterEnvMap(Roughness, dir);

	const half4 c0 = { -1, -0.0275, -0.572, 0.022 };
	const half4 c1 = { 1, 0.0425, 1.04, -0.04 };
	half4 r = Roughness * c0 + c1;
	half a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	half2 AB = half2(-1.04, 1.04) * a004 + r.zw;

	SpecularLight = Texture * (specularColor * AB.x + AB.y);
}

void LightFunc(float3 normal, float3 toLight, float3 toEye, float3 lightColor, float3 diffuseColor, float3 specularColor, inout float3 DiffuseLight, inout float3 SpecularLight, in float Roughness)
{
	//裁减掉背光的像素
	float NL = dot(toLight, normal);
	clip(NL);
	//计算漫反射光照
	float DiffuseRatio = max(NL, 0.0);
	DiffuseLight = lightColor * diffuseColor * DiffuseRatio / M_PI;


	float3 V = normalize(toEye);
	float3 L = normalize(toLight);
	float3 N = normalize(normal);
	float3 H = normalize(L + V);

	float NoH = saturate(dot(N, H));
	float NoV = saturate(dot(N, V));
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
	
	//G = Vis_Smith(Shininess, NoV, NoL);
	//G = G * VoH / (NoH * NoV);

	//F
	//float Fc = pow(1 - VoH, 5);
	//float F = (1 - Fc) * specularColor + Fc;

	//F Schlick
	float3 F = specularColor + (1 - specularColor) * pow(1 - LoH, 5);

	//BRDF
	float3 BRDF = D * G * F / (4 * NoL * NoV);

	//float BRDF = F * G * VoH/ (NoH * NoV);
	//BRDF = min(BRDF, 100000);
	//G_V = NoV + sqrt((NoV - NoV * Roughness) * NoV + Roughness);
	//G_L = NoV + sqrt((NoL - NoL * Roughness) * NoL + Roughness);
	//BRDF = (G_V * G_L);


	SpecularLight = lightColor * BRDF * DiffuseRatio;
}

void LightFunc2(float3 normal, float3 toLight, float3 toEye, float4 lightColor, inout float4 DiffuseLight, inout float4 SpecularLight, in float Shininess)
{
	//裁减掉背光的像素
	float NL = dot(toLight, normal);
	clip(NL);
	//计算漫反射光照
	float DiffuseRatio = max(NL, 0.0);
	DiffuseLight = lightColor * DiffuseRatio;

	//Blinn-Phong光照，已经是基于物理的了
	//计算半角向量
	float3 H = normalize(toLight + toEye);

	//遮挡项
	float lh = dot(toLight, H);
	float G = 1 / (lh*lh);

	float shininess = Shininess;
	float SpecularRatio = max(dot(normal, H), 0.0);
	//对Blinn-Phong的高光归一化，保证反射的能量守恒
	float PoweredSpecular = pow(SpecularRatio, shininess) * (shininess + 2.0f) / 8.0f;

	SpecularLight = lightColor * PoweredSpecular * DiffuseRatio * G;
}


//光照结果的MRT输出
struct OutputPS
{
	float4 Light		: COLOR0;
};

OutputPS ImageBasedLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.Light = 0;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 pos = GetPosition(TexCoord, viewDir, g_samplePosition);

	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float4 ToEyeDirV = float4(-pos, 0.0f);
		
	float3 DiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 SpecularLight = float3(0.0f, 0.0f, 0.0f);

	float3 Normal;// = GetNormal(TexCoord);
	float Shininess;// = GetShininess(TexCoord);
	GetNormalandShininess(TexCoord, Normal, Shininess, g_sampleNormal);
	
	//反射率
	float4 albedo = tex2D(g_sampleDiffuse, TexCoord);
		
	float metalness = albedo.a;

	float3 diffuseColor = albedo.rgb * (1 - metalness);
	float3 specularColor = lerp(0.04, albedo.rgb, metalness);

	IBL_BRDF(Normal, ToEyeDirV, diffuseColor, specularColor, DiffuseLight, SpecularLight, Shininess);

	outPs.Light = float4(DiffuseLight, 1.0f) + float4(SpecularLight, 1.0f);
	return outPs;
}

OutputPS DirectionLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.Light = 0;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 pos = GetPosition(TexCoord, viewDir, g_samplePosition);

	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float3 ToEyeDirV = normalize(pos * -1.0f);
	float3 toLight = normalize(-g_LightDir.xyz);

	float3 DiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 SpecularLight = float3(0.0f, 0.0f, 0.0f);

	float3 Normal;// = GetNormal(TexCoord);
	float Shininess;// = GetShininess(TexCoord);
	GetNormalandShininess(TexCoord, Normal, Shininess, g_sampleNormal);
	
	//反射率
	float4 albedo = tex2D(g_sampleDiffuse, TexCoord);
		
	float metalness = albedo.a;

	float3 diffuseColor = albedo.rgb * (1 - metalness);
	float3 specularColor = lerp(0.04, albedo.rgb, metalness);

	LightFunc(Normal, toLight, ToEyeDirV, g_LightColor, diffuseColor, specularColor, DiffuseLight, SpecularLight, Shininess);

	float shadowFinal = 1.0f;
	//shadowFinal = ShadowFunc(g_bUseShadow, pos);

	//阴影图采样
	shadowFinal = tex2D(g_sampleShadowResult, TexCoord);

	outPs.Light = float4(DiffuseLight * shadowFinal, 1.0f) + float4(SpecularLight * shadowFinal, 1.0f);
	return outPs;
}

OutputPS PointLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.Light = 0;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 pos = GetPosition(TexCoord, viewDir, g_samplePosition);
	
	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float3 DiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 SpecularLight = float3(0.0f, 0.0f, 0.0f);

	float3 toLight = g_LightPos.xyz - pos;
	float toLightDistance = length(toLight);
	
	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (toLightDistance > g_LightRange)
		return outPs;

	float disRange = clamp(toLightDistance / g_LightRange, 0.0f, 1.0f);


	float attenuation = (1.0f - disRange) / (g_LightAttenuation.x + g_LightAttenuation.y * disRange + g_LightAttenuation.z * disRange * disRange + 0.000001f);
	//attenuation = 1 - dot(toLight / g_LightRange, toLight / g_LightRange);

	float3 lightColor = attenuation * g_LightColor;

	toLight = normalize(toLight);

	float3 Normal;// = GetNormal(TexCoord);
	float Shininess;// = GetShininess(TexCoord);
	GetNormalandShininess(TexCoord, Normal, Shininess, g_sampleNormal);
	
	//反射率
	float4 albedo = tex2D(g_sampleDiffuse, TexCoord);
		
	float metalness = albedo.a;

	float3 diffuseColor = albedo.rgb * (1 - metalness);
	float3 specularColor = lerp(0.04, albedo.rgb, metalness);

	LightFunc(Normal, toLight, ToEyeDirV, lightColor, diffuseColor, specularColor, DiffuseLight, SpecularLight, Shininess);

	float shadowFinal = 1.0f;
	//shadowFinal = PointShadowFunc(g_bUseShadow, pos);

	//阴影图采样，因为点光源分辨率小，所以模糊一次
	//shadowFinal = tex2D(g_sampleShadowResult, TexCoord);
	shadowFinal = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleShadowResult, TexCoord);

	outPs.Light = float4(DiffuseLight * shadowFinal, 1.0f) + float4(SpecularLight * shadowFinal, 1.0f);
	return outPs;
}

OutputPS SpotLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.Light = 0;

	//outPs.diffuseLight = g_LightColor;
	//outPs.specularLight = g_LightColor;
	//return outPs;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 pos = GetPosition(TexCoord, viewDir, g_samplePosition);
	
	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float3 DiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 SpecularLight = float3(0.0f, 0.0f, 0.0f);

	float3 toLight = g_LightPos.xyz - pos;

	float toLightDistance = length(toLight);
	
	float disRange = clamp(toLightDistance / g_LightRange, 0.0f, 1.0f);

	float attenuation = (1.0f - disRange) / (g_LightAttenuation.x + g_LightAttenuation.y * disRange + g_LightAttenuation.z * disRange * disRange + 0.000001f);
	//不使用衰减参数，直接衰减到边缘
	//attenuation = 1 - dot(toLight / g_LightRange, toLight / g_LightRange);
	
	toLight = normalize(toLight);

	//光源到像素的向量与光源方向的夹角
	float cosAlpha = dot(-toLight, g_LightDir);
	//内锥角的一半的cos
	float cosTheta = g_LightCosAngle.y;
	//外锥角的一半的cos
	float cosPhi = g_LightCosAngle.x;

	//聚光灯外角以外和范围以外都没有光照(加入灯光体之后就可以省略掉这个判断了)
	if (toLightDistance > g_LightRange || cosAlpha < cosPhi)
		return outPs;
	
	float falloff = 1.0f;
	if (cosAlpha < cosTheta)
	{
		falloff = (cosAlpha - cosPhi) / (cosTheta - cosPhi);
	}
	//下面的公式可以简化掉if，但是看起来没有效率提升
	//falloff = 1 - (cosAlpha < cosTheta)* (1 - (cosAlpha - cosPhi) / (cosTheta - cosPhi));
	
	//灯光falloff的指数衰减
	//TODO：

	float3 lightColor = attenuation * g_LightColor * falloff;
	
	float3 Normal;// = GetNormal(TexCoord);
	float Shininess;// = GetShininess(TexCoord);
	GetNormalandShininess(TexCoord, Normal, Shininess, g_sampleNormal);
	
	//反射率
	float4 albedo = tex2D(g_sampleDiffuse, TexCoord);
		
	float metalness = albedo.a;

	float3 diffuseColor = albedo.rgb * (1 - metalness);
	float3 specularColor = lerp(0.04, albedo.rgb, metalness);

	LightFunc(Normal, toLight, ToEyeDirV, lightColor, diffuseColor, specularColor, DiffuseLight, SpecularLight, Shininess);

	float shadowFinal = 1.0f;
	//shadowFinal = ShadowFunc(g_bUseShadow, pos);
	//阴影图采样
	shadowFinal = tex2D(g_sampleShadowResult, TexCoord);

	outPs.Light = float4(DiffuseLight * shadowFinal, 1.0f) + float4(SpecularLight * shadowFinal, 1.0f);
	return outPs;
}


float2 ComputeMoments(float Depth)
{
	float2 Moments;

	Moments.x = Depth;

	float dx = ddx(Depth);
	float dy = ddy(Depth);

	Moments.y = Depth*Depth + 0.25 * (dx*dx + dy*dy);

	return Moments;
}

float ShadowFunc(bool useShadow, float3 objViewPos)
{
	float shadowContribute = 1.0f;
	//if (useShadow)
	{
		//Shadow
		float4 lightViewPos = mul(float4(objViewPos, 1.0f), g_viewToLight);
			float4 lightProjPos = mul(float4(objViewPos, 1.0f), g_viewToLightProj);
			float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
		float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;

		float2 ShadowTexCoord = float2(lightU, lightV);

			//裁减掉shadowMap的uv值0-1范围之外的部分
			//也可以使用border，但是对于FP16的值，需要先缩放才能使用border，否则设置的borderColor最大值仅为1，没法使用
			clip(float2(1, 1) - ShadowTexCoord);
		clip(ShadowTexCoord);

		//float2 Moments = tex2D(g_sampleShadow, ShadowTexCoord);
		//float2 Moments = GaussianBlur(g_ShadowMapSize, g_ShadowMapSize, g_sampleShadow, ShadowTexCoord);
		float2 Moments = ComputeMoments(GaussianBlur(g_ShadowMapSize, g_ShadowMapSize, g_sampleShadow, ShadowTexCoord).x);

			shadowContribute = ChebyshevUpperBound(Moments, lightViewPos.z);
	}
	return shadowContribute;
}

float PointShadowFunc(bool useShadow, float3 objViewPos)
{
	float shadowContribute = 1.0f;
	//if (useShadow)
	{
		//Shadow
		float3 toObj = objViewPos - g_LightPos.xyz;

			float4 worldToObj = mul(float4(toObj, 0.0f), g_invView);

			float2 Moments = texCUBE(g_sampleShadow, normalize(worldToObj.xyz));


			shadowContribute = ChebyshevUpperBound(Moments, length(worldToObj.xyz));
	}
	return shadowContribute;
}

float PCFShadowFunc(bool useShadow, float3 objViewPos)
{
	float shadowContribute = 1.0f;
	//if (useShadow)
	{
		//Shadow
		float4 lightViewPos = mul(float4(objViewPos, 1.0f), g_viewToLight);
			float4 lightProjPos = mul(float4(objViewPos, 1.0f), g_viewToLightProj);
			float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
		float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;

		int SHADOWMAP_SIZE = g_ShadowMapSize;
		float2 ShadowTexCoord = float2(lightU, lightV);
			float2 texturePos = ShadowTexCoord * SHADOWMAP_SIZE;
			float2 lerps = frac(texturePos);

			float shadowBias = g_ShadowBias;

		float shadowVal[4];
		shadowVal[0] = (tex2D(g_sampleShadow, ShadowTexCoord) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;
		shadowVal[1] = (tex2D(g_sampleShadow, ShadowTexCoord + float2(1.0f / SHADOWMAP_SIZE, 0.0f)) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;
		shadowVal[2] = (tex2D(g_sampleShadow, ShadowTexCoord + float2(0.0f, 1.0f / SHADOWMAP_SIZE)) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;
		shadowVal[3] = (tex2D(g_sampleShadow, ShadowTexCoord + float2(1.0f / SHADOWMAP_SIZE, 1.0f / SHADOWMAP_SIZE)) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;

		shadowContribute = lerp(lerp(shadowVal[0], shadowVal[1], lerps.x), lerp(shadowVal[2], shadowVal[3], lerps.x), lerps.y);

	}
	return shadowContribute;
}

float PCFPointShadowFunc(bool useShadow, float3 worldViewPos)
{
	float shadowContribute = 1.0f;
	//if (useShadow)
	{
		//Shadow
		//g_LightPos是view空间下的灯光位置
		float3 toObjVec = worldViewPos - g_LightPos.xyz;
			float4 worldToObjVec = mul(float4(toObjVec, 0.0f), g_invView);

			float shadowBias = g_ShadowBias;
		shadowContribute = (texCUBE(g_sampleShadow, normalize(worldToObjVec.xyz)) + shadowBias < length(toObjVec)) ? 0.0f : 1.0f;
	}
	return shadowContribute;
}

float4 ShadowPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1) : COLOR
{
	//return float4(0.5, 0.0f, 0.0f, 1.0f);

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);
	float3 pos = GetPosition(TexCoord, viewDir, g_samplePosition);


	float shadowFinal = 1.0f;
	//shadowFinal = PCFShadowFunc(g_bUseShadow, pos);
	shadowFinal = ShadowFunc(g_bUseShadow, pos);

	//输出阴影结果
	float4 finalColor = float4(shadowFinal, shadowFinal, shadowFinal, shadowFinal);
	return finalColor;
}

float4 PointShadowPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1) : COLOR
{
	//return float4(0.5, 0.0f, 0.0f, 1.0f);

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 pos = GetPosition(TexCoord, viewDir, g_samplePosition);


	float shadowFinal = 1.0f;
	//shadowFinal = PCFPointShadowFunc(g_bUseShadow, pos);
	shadowFinal = PointShadowFunc(g_bUseShadow, pos);
	//输出阴影结果
	float4 finalColor = float4(shadowFinal, shadowFinal, shadowFinal, shadowFinal);
	return finalColor;
}

float4 StencilPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1) : COLOR
{
	return 0;
}

float4 AmbientPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//计算环境光
	float4 Ambient = g_AmbientColor;

	//反射率
	float4 albedo = tex2D(g_sampleDiffuse, TexCoord);

	float metalness = albedo.a;

	float3 diffuseColor = albedo.rgb * (1 - metalness);

	//临时增加屏幕空间镜面反射
	float3 ssrColor = tex2D(g_sampleSSR, TexCoord);
	float3 specularColor = lerp(0.04, ssrColor.rgb, metalness);

	return Ambient * float4(diffuseColor , 1.0f);
}

technique DeferredRender
{
	pass p0 //渲染灯光
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 DirectionLightPass();//ImageBasedLightPass//DirectionLightPass
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
		ColorWriteEnable = 0xFFFFFFFF;
	}

	pass p1 //PointLight
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 PointLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
		ColorWriteEnable = 0xFFFFFFFF;
	}

	pass p2 //SpotLight
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 SpotLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
		ColorWriteEnable = 0xFFFFFFFF;
	}

	pass p3 //DirShadow
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 ShadowPass();
		AlphaBlendEnable = false;
		ColorWriteEnable = 0xFFFFFFFF;
	}

	pass p4 //PointShadow
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 PointShadowPass();
		AlphaBlendEnable = false;
		ColorWriteEnable = 0xFFFFFFFF;
	}

	pass p5 //添加环境光
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 AmbientPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
		ColorWriteEnable = 0xFFFFFFFF;
	}
	pass p6 //Stencil Pass
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 StencilPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ZERO;
		DestBlend = ONE;
		ColorWriteEnable = 0;
	}
	pass p7 //渲染灯光
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 ImageBasedLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
		ColorWriteEnable = 0xFFFFFFFF;
	}
}