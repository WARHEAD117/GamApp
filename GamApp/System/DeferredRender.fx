matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

texture		g_DiffuseBuffer;
texture		g_NormalDepthBuffer;

sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D g_sampleNormalDepth =
sampler_state
{
	Texture = <g_NormalDepthBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
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


float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	//纹理采样
	float4 Diffuse = tex2D(g_sampleDiffuse, TexCoord);
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);


	//DeferredRender
	//计算环境光
	float4 Ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);  //材质可理解为反射系数

	float4 NormalW = float4(normalize(NormalDepth.xyz), 1.0f);
	float4 ToEyeDirW = float4(0, 0, -1, 1.0f);
	float4 LightDir = float4(-1.0f, -1.0f, -1.0f,1.0f);
	//计算漫反射
	float DiffuseRatio = dot(-LightDir, NormalW);
	float4 DiffuseLight = float4(0.8f, 0.8f, 0.8f, 1.0f) * DiffuseRatio;

	//计算镜面反射

	//计算半角向量
	float4 H = normalize(ToEyeDirW - LightDir);

	//Phong光照
	//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
	//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

	//Blinn-Phong光照
	float SpecularRatio = max(dot(NormalW, H), 0);

	float4	Specular = pow(SpecularRatio, 12);
	//Specular = lightSpecular* (g_SpecularMaterial * Specular);

	//混合光照和纹理
	float4 finalColor = DiffuseLight * Diffuse + Specular + Ambient*0.1f;
	//输出颜色
	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);

	float4 final = Diffuse + NormalDepth;
	//输出颜色
	return final;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredRender
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}