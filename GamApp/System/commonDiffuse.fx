#include "common.fx"

float4		g_LightDir;
float4		g_ViewPos;

texture		g_Texture;

float4 lightDiffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightSpecular = float4(1.0f, 1.0f, 1.0f, 1.0f);

float4 g_AmbientMaterial = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 g_DiffuseMaterial = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 g_SpecularMaterial = float4(1.0f, 1.0f, 1.0f, 1.0f);
float g_SpecularPower = 1.0f;

sampler2D g_sampleTexture =
sampler_state
{
	Texture = <g_Texture>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float3 normalW		: NORMAL0;
	float3 toEyeDirW		: TEXCOORD1;
};


OutputVS VShader(float4 posL       : POSITION0,
	float3 normalL : NORMAL0,  // Assumed to be unit length
	float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	outVS.normalW = mul(normalL, g_World);

	outVS.TexCoord = TexCoord;
	outVS.toEyeDirW = g_ViewPos.xyz - mul(posL, g_World).xyz;
	return outVS;
}


float4 PShader( float2 TexCoord : TEXCOORD0,
				float3 NormalW : NORMAL0,
				float3 ToEyeDirW : TEXCOORD1) : COLOR
{
	//纹理采样
	float4 Texture = tex2D(g_sampleTexture, TexCoord);


	//计算环境光
	float4 Ambient = lightAmbient * g_AmbientMaterial;  //材质可理解为反射系数

	NormalW = normalize(NormalW);
	ToEyeDirW = normalize(ToEyeDirW);
	//计算漫反射
	float DiffuseRatio = max(dot(-g_LightDir.xyz, NormalW),0);
	float4 Diffuse = lightDiffuse * (g_DiffuseMaterial * DiffuseRatio);

	//计算镜面反射

	//计算半角向量
	float3 H = normalize(ToEyeDirW - g_LightDir.xyz);

	//Phong光照
	//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
	//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

	//Blinn-Phong光照
	float SpecularRatio = max(dot(NormalW, H),0);
	
	float4	Specular = pow(SpecularRatio, 12);
		//Specular = lightSpecular* (g_SpecularMaterial * Specular);

	//混合光照和纹理
	float4 finalColor = Texture * (Diffuse + Ambient*0.1f) + Specular;
	//输出颜色
	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique CommonDiffuse
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}