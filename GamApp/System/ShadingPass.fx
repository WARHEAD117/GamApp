#define		epsilon 0.0000001f

matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

texture		g_DiffuseBuffer;
texture		g_NormalBuffer;
texture		g_PositionBuffer;

texture		g_ShadowBuffer;
texture		g_PointShadowBuffer;

texture		g_ShadowResult;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

int g_ScreenWidth;
int g_ScreenHeight;

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

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

float4 GaussianBlur(int mapWidth, int mapHeight, sampler2D texSampler, float2 texCoords)
{
	float weights[6] = { 0.00078633, 0.00655965, 0.01330373, 0.05472157, 0.11098164, 0.22508352 };

	float4 color;
	float stepU = 1.0f / mapWidth;
	float stepV = 1.0f / mapHeight;

	//0,1,2,1,0
	//1,3,4,3,1
	//2,4,5,4,2
	//1,3,4,3,1
	//0,1,2,1,0
	color = tex2D(texSampler, texCoords + float2(-2 * stepU, -2 * stepV)) * (weights[0]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, -2 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, -2 * stepV)) * (weights[0]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, -1 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, -1 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, -1 * stepV)) * (weights[1]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 0 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 0 * stepV)) * (weights[5]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 0 * stepV)) * (weights[2]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 1 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 1 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 1 * stepV)) * (weights[1]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 2 * stepV)) * (weights[0]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 2 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 2 * stepV)) * (weights[0]);

	return color;
}

float4 ShadingPass(float2 TexCoord : TEXCOORD0) : COLOR
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