matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

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

texture hmapTex;
sampler2D hmapSampler = sampler_state {
	Texture = <hmapTex>;
	MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture nmapTex;
sampler2D nmapSampler = sampler_state {
	Texture = <hmapTex>;
	MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

float4 tex_brdf(float3 normalW,
				float3 toEyeDirW,
				float4 LampColor) 
{
	float3 Nn = normalize(normalW);
	float3 Vn = normalize(toEyeDirW);
	float3 Ln = normalize(g_LightDir.xyz);
	float3 Hn = normalize(Vn + Ln);
	float2 huv = float2(0.5 + dot(Ln, Hn) / 2.0,
		1.0 - (0.5 + dot(Nn, Hn) / 2.0));
	float2 nuv = float2(0.5 + dot(Ln, Nn) / 2.0,
		1.0 - (0.5 + dot(Nn, Vn) / 2.0));
	float4 ht = tex2D(hmapSampler, huv);
	float4 nt = tex2D(nmapSampler, nuv);
	float4 nspec = ht * nt * LampColor;
	return nspec;
}

float4 bank_brdf(float3 normalW,
	float3 toEyeDirW,
	float4 Diffuse)
{
	float4 specular = float4(0.0, 0.0, 0.0, 0.0);

	//计算半角向量
	float3 H = normalize(toEyeDirW - g_LightDir.xyz);

	float3 V = toEyeDirW;

	bool back = (dot(V, float4(normalW, 1.0f))>0) && (dot(g_LightDir.xyz, normalW));

	float Ks = 0.6;
	float shininess = 10;
	if (back)
	{
		float3 T = normalize(cross(normalW, -V));   // 计算顶点切向量 

			float a = dot(g_LightDir.xyz, T);

		float b = dot(V, T);

		float c = sqrt(1 - pow(a, 2.0))* sqrt(1 - pow(b, 2.0)) - a*b; // 计算 Bank BRDF 系数 

		float brdf = Ks* pow(c, shininess);

		specular = brdf *Diffuse;// *lightColor;
	}

	return specular;
}


float4 PShader(float2 TexCoord : TEXCOORD0,
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



		float4 Specular = float4(0.0, 0.0, 0.0, 0.0);

		Specular = bank_brdf(NormalW, ToEyeDirW, Diffuse);
		//Specular = tex_brdf(NormalW, ToEyeDirW, float4(1.0f, 1.0f, 1.0f, 1.0f));
		//混合光照和纹理
		float4 finalColor = Specular + Texture * Diffuse;// +Ambient*0.1f;
		//输出颜色
		return finalColor;
}

technique CommonDiffuse
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}