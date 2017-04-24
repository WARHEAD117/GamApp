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

texture g_LightBuffer;
sampler2D g_sampleLight =
sampler_state
{
	Texture = <g_LightBuffer>;
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
	float4 LightResult = tex2D(g_sampleLight, TexCoord);

	float4 Light = float4(LightResult.xyz, 1.0f);

	float3 pos = GetPosition(TexCoord, g_samplePosition);
	if (pos.z > 100000)
	{
		float3 dir = normalize(view);
		float2 skyUV = float2((1 + atan2(dir.x, -dir.z) / M_PI ) /2 , acos(dir.y) / M_PI );
		float r1 = (1 / M_PI)*acos(dir.z) / sqrt(dir.x * dir.x + dir.y * dir.y);
		skyUV = -0.5 * float2(dir.x * r1 + 1, dir.y * r1 + 1);
		float4 Texture = tex2D(g_sampleSky, skyUV);

		//线性空间和伽马空间的转换...不明白为什么
		//但是不转换的话，颜色不对会发红
		//DX的textureViewer也有同样的问题
		Texture = pow(Texture, 1 / 2.2);
		//Texture = float4(view, 1);
		return Texture;
	}

	return Light;
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