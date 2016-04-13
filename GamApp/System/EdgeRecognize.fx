#include "common.fx"

texture		g_NormalBuffer;
sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};


texture		g_PositionBuffer;
sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
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

float4 fliter(float3x3 _filter, sampler2D _image, float2 texCoord, float2 texSize)
{
	float2 filterOffset[3][3] = 
	{
		{ float2(-1.0f, -1.0f), float2( 0.0f, -1.0f), float2( 1.0f, -1.0f) },
		{ float2(-1.0f,  0.0f), float2( 0.0f,  0.0f), float2( 1.0f,  0.0f) },
		{ float2(-1.0f,  1.0f), float2( 0.0f,  1.0f), float2( 1.0f,  1.0f) }
	};

	float4 finalColor = float4(0, 0, 0, 0);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			float2 newTexCoord = texCoord + float2(filterOffset[i][j].x / texSize.x, filterOffset[i][j].y / texSize.y);
				finalColor += tex2D(_image, newTexCoord) * _filter[i][j];
		}
	}

	return finalColor;
}

float3 GetUnsharpMaskedNormal(float2 texCoords, sampler2D texSampler)
{
	float3 oriNormal = GetNormal(texCoords, texSampler);

	float weights[6] = { 0.00078633, 0.00655965, 0.01330373, 0.05472157, 0.11098164, 0.22508352 };

	float3 normal;
	float stepU = 1.0f / g_ScreenWidth;
	float stepV = 1.0f / g_ScreenHeight;

	//0,1,2,1,0
	//1,3,4,3,1
	//2,4,5,4,2
	//1,3,4,3,1
	//0,1,2,1,0
	normal = GetNormal(texSampler, texCoords + float2(-2 * stepU, -2 * stepV)) * (weights[0]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, -2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, -2 * stepV)) * (weights[2]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, -2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, -2 * stepV)) * (weights[0]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, -1 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, -1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, -1 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, -1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, -1 * stepV)) * (weights[1]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, 0 * stepV)) * (weights[2]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, 0 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, 0 * stepV)) * (weights[5]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, 0 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, 0 * stepV)) * (weights[2]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, 1 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, 1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, 1 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, 1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, 1 * stepV)) * (weights[1]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, 2 * stepV)) * (weights[0]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, 2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, 2 * stepV)) * (weights[2]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, 2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, 2 * stepV)) * (weights[0]);
	float3 blurredNormal = normalize(normal);

	float3 unsharpedMaskedNormal = oriNormal + 5*(oriNormal - blurredNormal);
	float3 importance = oriNormal - blurredNormal;

	if (importance.z < 0)
		return oriNormal;
	return normalize(unsharpedMaskedNormal);
}

int g_switch = 1;

float4 Laplace_Edge(float2 TexCoord)
{
	int neighborNum = 4;
	//neighborNum = 8;

	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);

	float depthN[9];

	depthN[0] = GetDepth(TexCoord, g_samplePosition);

	depthN[1] = GetDepth(TexCoord + offset * float2( 0,-1), g_samplePosition); //Up
	depthN[2] = GetDepth(TexCoord + offset * float2( 1, 0), g_samplePosition); //Right
	depthN[3] = GetDepth(TexCoord + offset * float2( 0, 1), g_samplePosition); //Down
	depthN[4] = GetDepth(TexCoord + offset * float2(-1, 0), g_samplePosition); //Left

	depthN[5] = GetDepth(TexCoord + offset * float2( 1,-1), g_samplePosition); //RightUp
	depthN[6] = GetDepth(TexCoord + offset * float2( 1, 1), g_samplePosition); //RightDown
	depthN[7] = GetDepth(TexCoord + offset * float2(-1, 1), g_samplePosition); //LeftDown
	depthN[8] = GetDepth(TexCoord + offset * float2(-1,-1), g_samplePosition); //LeftUp

	float depthNN[4];
	depthNN[0] = GetDepth(TexCoord + offset * float2( 0, -2), g_samplePosition); //UpUp
	depthNN[1] = GetDepth(TexCoord + offset * float2( 2,  0), g_samplePosition); //RightRight
	depthNN[2] = GetDepth(TexCoord + offset * float2( 0,  2), g_samplePosition); //DownDown
	depthNN[3] = GetDepth(TexCoord + offset * float2(-2,  0), g_samplePosition); //LeftLeft

	float3 normalR[9];

	normalR[0] = normalize(GetNormal(g_sampleNormal, TexCoord));

	normalR[1] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 0,-1))); //Up
	normalR[2] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 1, 0))); //Right
	normalR[3] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 0, 1))); //Down
	normalR[4] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0))); //Left

	normalR[5] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 1,-1))); //RightUp
	normalR[6] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 1, 1))); //RIghtDown
	normalR[7] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 1))); //LeftDown
	normalR[8] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1,-1))); //LeftUp

	float depth = depthN[0];

	float deltaD_Inner = (depth / 20.0f)*(depth / 20.0f);
	float deltaD_Outer = depth / 20.0f;

	for (int i = 1; i<neighborNum + 1; i++)
	{
		if (depth - depthN[i] > deltaD_Outer)
		{
			return float4(1, 1, 1, 1);
		}
	}
	if (depth > 1000)
	{
		return float4(1, 1, 1, 1);
	}

	bool isEdge = false;
	bool edgeList[9] = { false, false, false, false, false, false, false, false, false };
	for (int i = 1; i< neighborNum + 1; i++)
	{
		if (depthN[i] - depth  > deltaD_Inner)
		{
			isEdge = true;
			edgeList[i] = true;
		}
	}

	float laplace = 4 * depth - (depthN[1] + depthN[2] + depthN[3] + depthN[4]);

	float laplaceN[5];
	laplaceN[1] = 4 * depth - (depthN[0] + depthN[5] + depthN[8] + depthNN[0]);
	laplaceN[2] = 4 * depth - (depthN[0] + depthN[5] + depthN[6] + depthNN[1]);
	laplaceN[3] = 4 * depth - (depthN[0] + depthN[6] + depthN[7] + depthNN[2]);
	laplaceN[4] = 4 * depth - (depthN[0] + depthN[7] + depthN[8] + depthNN[3]);

	

	if (g_switch == 1)
	{
		for (int i = 1; i< 5; i++)
		{
			if (laplaceN[i]  > 0.6)
			{
				//isEdge = true;
				edgeList[i] = true;
			}
		}
	}

	if (laplace < -0.4)
		isEdge = true;
	else
		isEdge = false;

	//if (isEdge)
	//	return float4(0, 0, 0, 1);
	//else
	//	return float4(1, 1, 1, 1);

	//return float4(maxD, maxD, maxD, 1);


	float3 vecXY = normalize(float3(1, 1, 0));
	float3 vecYX = normalize(float3(1, -1, 0));

	float3 normalProjX = normalize(float3(normalR[0].x, 0, normalR[0].z));
	float3 normalProjY = normalize(float3(0, normalR[0].y, normalR[0].z));

	float3 normalProjXY = dot(normalR[0], vecXY) * vecXY + float3(0, 0, normalR[0].z);
	float3 normalProjYX = dot(normalR[0], vecYX) * vecYX + float3(0, 0, normalR[0].z);

	float3 normalUProj = normalize(float3(0, normalR[1].y, normalR[1].z));
	float3 normalRProj = normalize(float3(normalR[2].x, 0, normalR[2].z));
	float3 normalDProj = normalize(float3(0, normalR[3].y, normalR[3].z));
	float3 normalLProj = normalize(float3(normalR[4].x, 0, normalR[4].z));

	float3 normalRUProjXY = dot(normalR[5], vecXY) * vecXY + float3(0, 0, normalR[5].z);
	float3 normalRDProjYX = dot(normalR[6], vecYX) * vecYX + float3(0, 0, normalR[6].z);
	float3 normalLDProjXY = dot(normalR[7], vecXY) * vecXY + float3(0, 0, normalR[7].z);
	float3 normalLUProjYX = dot(normalR[8], vecYX) * vecYX + float3(0, 0, normalR[8].z);

	float du = dot(normalUProj, normalProjY);
	float dr = dot(normalProjX, normalRProj);
	float dd = dot(normalProjY, normalDProj);
	float dl = dot(normalLProj, normalProjX);

	float dru = dot(normalRUProjXY, normalProjXY);
	float drd = dot(normalProjYX, normalRDProjYX);
	float dld = dot(normalProjXY, normalLDProjXY);
	float dlu = dot(normalLUProjYX, normalProjYX);

	dl = acos(dl);
	dr = acos(dr);
	du = acos(du);
	dd = acos(dd);
	dru = acos(dru);
	drd = acos(drd);
	dld = acos(dld);
	dlu = acos(dlu);
	float dmax = (max(max(max(dl, dr), du), dd));
	float dmin = (min(min(min(dl, dr), du), dd));

	float d[9] = {0, du, dr, dd, dl, dru, drd, dld, dlu };
	dmax = -1000;
	dmin = 1000;
	for (int i = 1; i < neighborNum + 1; i++)
	{
		if (edgeList[i] != true)
		{
			dmax = max(d[i], dmax);
			dmin = min(d[i], dmin);
		}
	}
	
	dmax = (dmax + dmin) / 2;
	//dmax = dmax * dmin;
	float N = 1 - dmax / 3.14 * 0.8;

	

	N = N*N*N ;
	if (isEdge)
	{
		N *= 0.5;
		//if (N > 0.6) N = 0.6;
	}
	float alpha = 1;
	if (isEdge)
		alpha = 0;
	return float4(N, N, N, alpha);
}


float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	return Laplace_Edge(TexCoord);	
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}