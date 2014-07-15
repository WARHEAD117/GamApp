matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldView;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

bool		g_IsSky;

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
};


struct OutputPS
{
	float4 diffuse			: COLOR0;
	float4 normal			: COLOR1;
	float4 position			: COLOR2;
};

OutputVS VShader(float4 posL		: POSITION,
				float3 normalL : NORMAL,
				float3 tangentL : TANGENT,
				float3 binormalL : BINORMAL,
				float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);
	

	//���ﲻ��ֱ�ӳ���WV������Ӧ�ó���world�����ת�ã������������ŵ�ʱ���߲��ܱ�֤��ƽ�洹ֱ
	//�۲�ռ��µķ���
	outVS.normalV = mul(normalL, g_WorldView);

	//�۲�ռ��µ�����
	outVS.tangentV = mul(tangentL, g_WorldView);
	//�۲�ռ��µĸ�����
	outVS.binormalV = mul(binormalL, g_WorldView);

	outVS.posP = outVS.posWVP;
	outVS.posV = mul(posL, g_WorldView);
	outVS.TexCoord = TexCoord;

	return outVS;
}

OutputPS PShader(float3 NormalV		: NORMAL,
				 float3 TangentV		: TANGENT,
				 float3 BinormalV	: BINORMAL,
				 float2 TexCoord		: TEXCOORD0,
				 float4 posP			: TEXCOORD1,
				 float4 posV			: TEXCOORD2)
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
	

	//if (abs(sampledNormalT.x - 0) < 0.02 && abs(sampledNormalT.y - 0) < 0.02 && abs(sampledNormalT.z - 1) < 0.02)
	//	sampledNormalV = float3(1, 1, 1);

	sampledNormalV = (sampledNormalV + 1.0f) / 2.0f;
	
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

	//RGBͨ������������ɫ
	PsOut.diffuse.rgb = Texture.xyz;
	//Aͨ������߹�ǿ��
	PsOut.diffuse.a = Specular.x;
	PsOut.normal = float4(sampledNormalV, 1.0f);
	PsOut.position = float4(posV.xyz, DepthP);
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