#define		epsilon 0.0000001f

#include "common.fx"

texture		g_DiffuseBuffer;
texture		g_NormalBuffer;
texture		g_PositionBuffer;

texture		g_ShadowBuffer;
texture		g_PointShadowBuffer;

texture		g_ShadowResult;

sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture g_DiffuseLightBuffer;
sampler2D g_sampleDiffuseLight =
sampler_state
{
	Texture = <g_DiffuseLightBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture g_SpecularLightBuffer;
sampler2D g_sampleSpecularLight =
sampler_state
{
	Texture = <g_SpecularLightBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
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

//----------------------------
struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float3 view: TEXCOORD1;
};

OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	//将投影空间的的平面（距离屏幕为1）乘以观察投影的逆矩阵，转换到世界空间
	//这样就得到了天空平面
	float4 posW = outVS.posWVP;
	posW.z = 1;
	posW.w = 1;
	outVS.view = mul(posW, g_InverseProj);
	outVS.TexCoord = TexCoord;

	return outVS;
}

float4 ShadingPass(float2 TexCoord : TEXCOORD0, float3 view : TEXCOORD1) : COLOR
{
	//采样光照结果
	float4 DiffuseLightResult = tex2D(g_sampleDiffuseLight, TexCoord);
	float4 SpecularLightResult = tex2D(g_sampleSpecularLight, TexCoord);

	float4 diffuseLight = float4(DiffuseLightResult.xyz, 1.0f);
	float4 specularLight = float4(SpecularLightResult.xyz, 1.0f);

	//纹理采样
	float4 DiffuseBuffer = tex2D(g_sampleDiffuse, TexCoord);
	//纹理颜色
	float4 Texture = float4(DiffuseBuffer.rgb, 1.0f);
	//高光强度
	float  Specularintensity = DiffuseBuffer.a;

	float3 pos = GetPosition(TexCoord, g_samplePosition);
	if (pos.z > 100000)
	{
		float3 dir = normalize(view);
		float2 skyUV = float2((1 + atan2(dir.x, -dir.z) / M_PI ) /2 , acos(dir.y) / M_PI );
		Texture = tex2D(g_sampleSky, skyUV);

		//Texture = float4(view, 1);
	}
	//计算最终光照
	return Texture * diffuseLight + specularLight * Specularintensity;
}

technique DeferredRender
{
	pass p0 //着色
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 ShadingPass();
		AlphaBlendEnable = false;                        //设置渲染状态        
		//SrcBlend = ZERO;
		//DestBlend = SrcColor;
		ColorWriteEnable = 0xFFFFFFFF;
	}
}