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

//----------------------------
struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};

OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

float4 ShadingPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//�������ս��
	float4 DiffuseLightResult = tex2D(g_sampleDiffuseLight, TexCoord);
	float4 SpecularLightResult = tex2D(g_sampleSpecularLight, TexCoord);

	float4 diffuseLight = float4(DiffuseLightResult.xyz, 1.0f);
	float4 specularLight = float4(SpecularLightResult.xyz, 1.0f);

	//�������
	float4 DiffuseBuffer = tex2D(g_sampleDiffuse, TexCoord);
	//������ɫ
	float4 Texture = float4(DiffuseBuffer.rgb, 1.0f);
	//�߹�ǿ��
	float  Specularintensity = DiffuseBuffer.a;

	//�������չ���
	return Texture * diffuseLight + specularLight * Specularintensity;
}

technique DeferredRender
{
	pass p0 //��ɫ
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 ShadingPass();
		AlphaBlendEnable = false;                        //������Ⱦ״̬        
		//SrcBlend = ZERO;
		//DestBlend = SrcColor;
		ColorWriteEnable = 0xFFFFFFFF;
	}
}