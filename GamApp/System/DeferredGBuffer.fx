matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldView;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

texture		g_Texture;


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
	float4 posWVP			: POSITION;
	float3 normalV			: NORMAL;
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
				float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);
	

	//�۲�ռ��µķ���
	outVS.normalV = mul(normalL, g_WorldView);

	outVS.posP = outVS.posWVP;
	outVS.posV = mul(posL, g_WorldView);
	outVS.TexCoord = TexCoord;

	return outVS;
}

OutputPS PShader(float3 NormalV		: NORMAL,
				float2 TexCoord		: TEXCOORD0,
				float4 posP			: TEXCOORD1,
				float4 posV		: TEXCOORD2)
{
	OutputPS PsOut;

	NormalV = (normalize(NormalV) + float3(1.0f, 1.0f, 1.0f)) / 2;

	//��������
	float4 Texture = tex2D(g_sampleTexture, TexCoord);

	//ͶӰ��ķ��������
	float DepthP = posP.z / posP.w;

	PsOut.diffuse = Texture;
	PsOut.normal = float4(NormalV, 1.0f);
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