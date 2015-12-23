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


	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(p, g_WorldViewProj);
	

	//���ﲻ��ֱ�ӳ���WV������Ӧ�ó���world�����ת�ã������������ŵ�ʱ���߲��ܱ�֤��ƽ�洹ֱ
	//�۲�ռ��µķ���
	outVS.normalV = mul(n, g_WorldView);

	//�۲�ռ��µ�����
	outVS.tangentV = mul(t, g_WorldView);
	//�۲�ռ��µĸ�����
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

	//TBN�����������Ϊ�������������ڿռ������ת�����߿ռ�
	//����TBN��Ȼ���ǽ����߿ռ������ת�����������Ŀռ�
	//����ʹ�ù۲�ռ�����������������пռ�ķ��߳���TBN���ǹ۲�ռ�ķ�����
	float3x3 TBN = float3x3(TangentV, BinormalV, NormalV);
	//TBN = transpose(TBN);

	float3 sampledNormalT = tex2D(g_sampleNormalMap, TexCoord).rgb;
	sampledNormalT = 2.0f * sampledNormalT - 1.0f - 0.00392f;
	sampledNormalT = normalize(sampledNormalT);

	//�����ǵȼ۵�
	float3 sampledNormalV = mul(sampledNormalT, TBN);/* sampledNormalT.x * TBN[0] + sampledNormalT.y * TBN[1] + sampledNormalT.z * TBN[2];*/
	sampledNormalV = normalize(sampledNormalV);

	//sampledNormalV = (sampledNormalV + 1.0f) / 2.0f;
	sampledNormalV.xy = encode(sampledNormalV);
	sampledNormalV.xyz = float2ToFloat3(sampledNormalV.xy);

	//��պ�
	sampledNormalV = g_IsSky ? float3(0, 0, 0) : sampledNormalV;

	//�������
	float4 Texture = tex2D(g_sampleTexture, TexCoord);
	//�߹�ͼ����
	float4 Specular = tex2D(g_sampleSpecularMap, TexCoord);

	//��պ�
	posV = g_IsSky ? float4(1.0e6, 1.0e6, 1.0e6, 1.0e6) : posV;

	//ͶӰ��ķ��������
	//float DepthP = posP.z / posP.w;

	//�ж��Ƿ�����պ�
	float DepthP = g_IsSky ? 1.0e6 : posP.z / posP.w;

	//���ø߹���
	float Shininess = g_shininess;
	Shininess = 5.05f;

	//
	clip(Texture.a < 0.1f ? -1 : 1);

	Texture.xyz = g_IsSky ? Texture.xyz : Texture.xyz * g_ThicknessMaterial.xyz;

	//RGBͨ������������ɫ
	PsOut.diffuse.rgb = Texture.xyz / 2.0f;

	//Aͨ������߹�ǿ��
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