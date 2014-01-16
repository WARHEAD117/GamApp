matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_mWorldInv;

float4		g_LightDir;
float4		g_ViewPos;

texture		g_Texture;

float4 lightDiffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightSpecular = float4(0.5f, 0.5f, 0.5f, 1.0f);

float4 materialAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 materialDiffuse = float4(0.5f, 0.5f, 0.5f, 1.0f);
float4 materialSpecular = float4(1.0f, 1.0f, 1.0f, 1.0f);

sampler2D g_sampleTexture =
sampler_state
{
	Texture = <g_Texture>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

struct OutputVS
{
	float4 posH         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float2 normal		: NORMAL0;
	float4 toEyeDir		: TEXCOORD1;
};


OutputVS VShader(float4 posL       : POSITION0,
	float3 normalL : NORMAL0,  // Assumed to be unit length
	float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	// Transform to homogeneous clip space.
	//matrix matWVP = mul(g_ViewProj, g_World);
	outVS.posH = mul(posL, g_ViewProj);

	// Pass on texture coordinates to be interpolated
	// in rasterization.
	outVS.TexCoord = TexCoord;
	outVS.normal = normalL;
	outVS.toEyeDir = g_LightDir - posL;
	return outVS;
}


float4 PShader(float4 posH			:	POSITION0,
	float2 TexCoord : TEXCOORD0,
	float3 Normal : NORMAL0,
	float4 ToEyeDir : TEXCOORD1) : COLOR
{

	//纹理采样
	float4 Texture = tex2D(g_sampleTexture, TexCoord);



	//计算环境光
	float4 Ambient = lightAmbient * materialAmbient;  //材质可理解为反射系数

	//计算漫反射
	float DiffuseRatio = dot(g_LightDir, Normal);
	float4 Diffuse = lightDiffuse * (materialDiffuse * DiffuseRatio);

	Normal = normalize(Normal);
	float4 LightDir = normalize(g_LightDir);
	//计算镜面反射
	float4 Reflect = float4(normalize(LightDir.xyz - 2 * DiffuseRatio * Normal), 1.0f);
	float SpecularRatio = dot(Reflect, ToEyeDir);
	float4 Specular = lightSpecular* (materialSpecular * SpecularRatio);

	//Specular = normalize(Specular); 
	//Diffuse = normalize(Diffuse);
	//混合光照和纹理
	float4 finalColor = Texture * (Ambient *0.3f + Diffuse);// + Specular;// ;
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