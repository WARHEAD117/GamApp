#include "common.fx"

#define		epsilon 0.0000001f

bool		g_IsSky;

float		g_shininess = 1.0f;

float4		g_ThicknessMaterial = float4(1, 1, 1, 1);
int			g_MatIndex = 1;


//========================================
uniform extern float4x4 gFinalXForms[54];




texture		g_Texture;
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

texture		g_NormalMap;
sampler2D g_sampleNormalMap =
sampler_state
{
	Texture = <g_NormalMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

texture		g_SpecularMap;
sampler2D g_sampleSpecularMap =
sampler_state
{
	Texture = <g_SpecularMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct OutputVS
{
	float4 posWVP			: POSITION;
	float3 normalV			: NORMAL;
	float3 tangentV			: TANGENT;
	float3 binormalV		: BINORMAL;

	float2 TexCoord			: TEXCOORD0;
	float4 posP				: TEXCOORD1;
	float4 posV				: TEXCOORD2;

	float isEdge			: TEXCOORD3;
};


struct OutputPS
{
	float4 diffuse			: COLOR0;
	float4 normal			: COLOR1;
	float4 position			: COLOR2;
	float4 silhouette		: COLOR3;
};

OutputVS VShader(float4 posL		: POSITION,
				float3 normalL : NORMAL,
				float3 tangentL : TANGENT,
				float3 binormalL : BINORMAL,
				float2 TexCoord : TEXCOORD0,
				float4 weight0 : BLENDWEIGHT0,
				int4 boneIndex : BLENDINDICES0)
{
	OutputVS outVS = (OutputVS)0;

	// Do the vertex blending calculation for posL and normalL.
	float weight1 = 1.0f - weight0.x;

	float4 p = weight0.x * mul(posL, gFinalXForms[boneIndex[0]]);
	p += weight0.y * mul(posL, gFinalXForms[boneIndex[1]]);
	p += weight0.z * mul(posL, gFinalXForms[boneIndex[2]]);
	p += (1 - weight0.x - weight0.y - weight0.z) * mul(posL, gFinalXForms[boneIndex[3]]);
	//p		+= weight2 * mul(float4(posL, 1.0f), gFinalXForms[boneIndex[2]]);
	p.w = 1.0f;

	// We can use the same matrix to transform normals since we are assuming
	// no scaling (i.e., rigid-body).
	float4 n = weight0.x * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[0]]);
	n += weight0.y * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[1]]);
	n += weight0.z * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[2]]);
	n += (1 - weight0.x - weight0.y - weight0.z) * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[3]]);
	//n		+= weight2 * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[2]]);
	n.w = 0.0f;

	// We can use the same matrix to transform normals since we are assuming
	// no scaling (i.e., rigid-body).
	float4 b = weight0.x * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[0]]);
		b += weight0.y * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[1]]);
	b += weight0.z * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[2]]);
	b += (1 - weight0.x - weight0.y - weight0.z) * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[3]]);
	//n		+= weight2 * mul(float4(normalL, 0.0f), gFinalXForms[boneIndex[2]]);
	b.w = 0.0f;

	float4 t = float4(cross(n.xyz, b.xyz),0.0f);


	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(p, g_WorldViewProj);
	

	//这里不该直接乘以WV，而是应该乘以world的逆的转置，这样在有缩放的时候法线才能保证与平面垂直
	//观察空间下的法线
	outVS.normalV = mul(n, g_WorldView);

	//观察空间下的切线
	outVS.tangentV = mul(t, g_WorldView);
	//观察空间下的副法线
	outVS.binormalV = mul(b, g_WorldView);

	outVS.posP = outVS.posWVP;
	outVS.posV = mul(p, g_WorldView);
	outVS.TexCoord = TexCoord;

	float flag = dot(normalize(outVS.normalV), -normalize(outVS.posV));
	if (flag >= 0 && flag <= 0.2)
		outVS.isEdge = flag;
	else
		outVS.isEdge = 0;

	return outVS;
}


OutputPS PShader(float3 NormalV		: NORMAL,
				 float3 TangentV		: TANGENT,
				 float3 BinormalV	: BINORMAL,
				 float2 TexCoord		: TEXCOORD0,
				 float4 posP			: TEXCOORD1,
				 float4 posV			: TEXCOORD2,
				 float isEdge : TEXCOORD3)
{
	OutputPS PsOut;

	NormalV = normalize(NormalV);
	BinormalV = normalize(BinormalV);
	TangentV = normalize(TangentV);
	
	//float tW = TangentL.w;
	//TangentL3 = normalize(TangentL3);
	//float3 BinormalL3 = cross(NormalL, TangentL3) *tW;
	//BinormalL3 = normalize(BinormalL3);

	//TBN的逆矩阵作用为把三个分量所在空间的向量转到切线空间
	//所以TBN自然就是将切线空间的向量转回三个分量的空间
	//这里使用观察空间的三个分量，所以切空间的法线乘以TBN就是观察空间的法线了
	float3x3 TBN = float3x3(TangentV, BinormalV, NormalV);
	//TBN = transpose(TBN);

	float3 sampledNormalT = tex2D(g_sampleNormalMap, TexCoord).rgb;
	sampledNormalT = 2.0f * sampledNormalT - 1.0f - 0.00392f;
	sampledNormalT = normalize(sampledNormalT);

	//两种是等价的
	float3 sampledNormalV = mul(sampledNormalT, TBN);/* sampledNormalT.x * TBN[0] + sampledNormalT.y * TBN[1] + sampledNormalT.z * TBN[2];*/
	sampledNormalV = normalize(sampledNormalV);

	//sampledNormalV = (sampledNormalV + 1.0f) / 2.0f;
	sampledNormalV.xy = encode(sampledNormalV);
	sampledNormalV.xyz = float2ToFloat3(sampledNormalV.xy);

	//天空盒
	sampledNormalV = g_IsSky ? float3(0, 0, 0) : sampledNormalV;

	//纹理采样
	float4 Texture = tex2D(g_sampleTexture, TexCoord);
	//高光图采样
	float4 Specular = tex2D(g_sampleSpecularMap, TexCoord);

	//天空盒
	posV = g_IsSky ? float4(1.0e6, 1.0e6, 1.0e6, 1.0e6) : posV;

	//投影后的非线性深度
	//float DepthP = posP.z / posP.w;

	//判断是否是天空盒
	float DepthP = g_IsSky ? 1.0e6 : posP.z / posP.w;

	//设置高光幂
	float Shininess = g_shininess;
	Shininess = 5.05f;

	//
	clip(Texture.a < 0.1f ? -1 : 1);

	Texture.xyz = g_IsSky ? Texture.xyz : Texture.xyz * g_ThicknessMaterial.xyz;

	//RGB通道储存纹理颜色
	PsOut.diffuse.rgb = Texture.xyz / 2.0f;

	//A通道储存高光强度
	PsOut.diffuse.a = g_MatIndex / 255.0f; // Specular.x;
	PsOut.normal = float4(sampledNormalV.xyz, 1.0f);
	PsOut.normal.a = 1.0f / (Shininess + epsilon);
	PsOut.position = posV.zzzz;

	PsOut.silhouette = float4(1, 1, 1, 1);

	return PsOut;

	if (isEdge >= 0)
		PsOut.diffuse = float4(isEdge * 5, 1, PsOut.diffuse.b, PsOut.diffuse.a);
	else
		PsOut.diffuse = float4(0, 1, PsOut.diffuse.b, PsOut.diffuse.a);
	if (g_IsSky)PsOut.diffuse = float4(0, 1, PsOut.diffuse.b, PsOut.diffuse.a);

	return PsOut;
}

technique DeferredGBuffer
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}