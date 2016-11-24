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


texture		g_EdgeMap;
sampler2D g_sampleEdgeMap =
sampler_state
{
	Texture = <g_EdgeMap>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};


texture		g_lineMask;
sampler2D g_sampleLineMask =
sampler_state
{
	Texture = <g_lineMask>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
	AddressU = wrap;
	AddressV = wrap;
};

texture		g_UvTex;
sampler2D g_sampleUvTex =
sampler_state
{
	Texture = <g_UvTex>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
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

	depthN[1] = GetDepth(TexCoord + offset * float2(0, -1), g_samplePosition); //Up
	depthN[2] = GetDepth(TexCoord + offset * float2(1, 0), g_samplePosition); //Right
	depthN[3] = GetDepth(TexCoord + offset * float2(0, 1), g_samplePosition); //Down
	depthN[4] = GetDepth(TexCoord + offset * float2(-1, 0), g_samplePosition); //Left

	depthN[5] = GetDepth(TexCoord + offset * float2(1, -1), g_samplePosition); //RightUp
	depthN[6] = GetDepth(TexCoord + offset * float2(1, 1), g_samplePosition); //RightDown
	depthN[7] = GetDepth(TexCoord + offset * float2(-1, 1), g_samplePosition); //LeftDown
	depthN[8] = GetDepth(TexCoord + offset * float2(-1, -1), g_samplePosition); //LeftUp

	float depthNN[4];
	depthNN[0] = GetDepth(TexCoord + offset * float2(0, -2), g_samplePosition); //UpUp
	depthNN[1] = GetDepth(TexCoord + offset * float2(2, 0), g_samplePosition); //RightRight
	depthNN[2] = GetDepth(TexCoord + offset * float2(0, 2), g_samplePosition); //DownDown
	depthNN[3] = GetDepth(TexCoord + offset * float2(-2, 0), g_samplePosition); //LeftLeft

	float3 normalR[9];

	normalR[0] = normalize(GetNormal(g_sampleNormal, TexCoord));

	//return normalR[0].z;

	normalR[1] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, -1))); //Up
	normalR[2] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 0))); //Right
	normalR[3] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, 1))); //Down
	normalR[4] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0))); //Left

	normalR[5] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, -1))); //RightUp
	normalR[6] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 1))); //RIghtDown
	normalR[7] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 1))); //LeftDown
	normalR[8] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, -1))); //LeftUp

	float depth = depthN[0];

	if (depth > 1000)
	{
		return float4(1, 1, 1, 1);
	}
	float laplace = 4 * depth - (depthN[1] + depthN[2] + depthN[3] + depthN[4]);

	float laplaceN[5];
	laplaceN[1] = 4 * depthN[1] - (depthN[0] + depthN[5] + depthN[8] + depthNN[0]);
	laplaceN[2] = 4 * depthN[2] - (depthN[0] + depthN[5] + depthN[6] + depthNN[1]);
	laplaceN[3] = 4 * depthN[3] - (depthN[0] + depthN[6] + depthN[7] + depthNN[2]);
	laplaceN[4] = 4 * depthN[4] - (depthN[0] + depthN[7] + depthN[8] + depthNN[3]);


	bool isEdge = false;
	bool edgeList[9] = { false, false, false, false, false, false, false, false, false };

	if (g_switch == 1)
	{
		for (int i = 1; i<neighborNum + 1; i++)
		{
			if (laplace > 0.4)
			{
				//return float4(1, 1, 1, 1);
			}
		}
	}

	if (laplace < -0.6)
		isEdge = true;
	else
		isEdge = false;


	for (int i = 1; i< 5; i++)
	{
		if (laplaceN[i]  > 0.6)
		{
			edgeList[i] = true;
		}
	}
	for (int i = 1; i< 5; i++)
	{
		if (laplace > 0.4)
		{
			edgeList[i] = true;
		}
	}

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
	
	if (g_switch == 1)
	{
		du = dot(normalR[1], normalR[0]);
		dr = dot(normalR[0], normalR[2]);
		dd = dot(normalR[0], normalR[3]);
		dl = dot(normalR[4], normalR[0]);
	}

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


	float d[9] = { 0, du, dr, dd, dl, dru, drd, dld, dlu };
	dmax = -1000;
	dmin = 1000;
	int maxFlag = -1;
	int minFlag = -1;
	for (int i = 1; i < neighborNum + 1; i++)
	{
		if (edgeList[i] != true)
		{
			if (d[i] > dmax)
				maxFlag = i;
			dmax = max(d[i], dmax);

			if (d[i] < dmin)
				minFlag = i;
			dmin = min(d[i], dmin);
		}
	}

	dmax = (dmax + dmin) / 2;
	//dmax = dmax * dmin;
	float N = 1 - dmax / 3.14 * 0.8;



	N = N*N*N;
	if (isEdge)
	{
		N *= 0.5;
		//if (N > 0.6) N = 0.6;
	}

	float2 dir[8] =
	{
		float2(0, -1),
		float2(1, 0),
		float2(0, 1),
		float2(-1, 0),
		float2(1, -1),
		float2(1, 1),
		float2(-1, 1),
		float2(-1, -1)
	};
	dmax = -1000;
	dmin = 1000;
	maxFlag = -1;
	minFlag = -1;
	for (int i = 1; i < 8 + 1; i++)
	{
		if (edgeList[i] != true)
		{
			if (d[i] > dmax)
				maxFlag = i;
			dmax = max(d[i], dmax);

			if (d[i] < dmin)
				minFlag = i;
			dmin = min(d[i], dmin);
		}
	}
	float2 minDir = dir[minFlag];
		float2 maxDir = dir[maxFlag];

	float p1 = 0.0001;
	float p2 = 0.005;

	bool testR = false;
	bool testL = false; 
	
	bool testU = false;
	bool testD = false;
	{
		float3 T = float3(1, 0, 0);
		float3 N = normalR[0];
		float3 B = cross(T, N);
		T = cross(N, B);
		float3x3 TBN = float3x3(T, B, N);

			float3 NR = normalize(mul(TBN, normalR[2]));
			float3 NL = normalize(mul(TBN, normalR[4]));

			float3 NU = normalize(mul(TBN, normalR[1]));
			float3 ND = normalize(mul(TBN, normalR[3]));

		if (normalR[0].z < 0)
		{

			if (NR.x < -p1)
			{
				testR = true;
			}
			if (NL.x > p1)
			{
				testL = true;
			}
			if (NU.y < -p1)
			{
				testU = true;
			}
			if (ND.y > p1)
			{
				testD = true;
			}
		}
		else
		{
			if (NR.x < -p1)
			{
				testL = true;
			}
			if (NL.x > p1)
			{
				testR = true;
			}
			if (NU.y < -p1)
			{
				testD = true;
			}
			if (ND.y > p1)
			{
				testU = true;
			}
		}

	}
	if (g_switch == 1)
	{
		if ((testR || testL || testU || testD) && !isEdge)
		{
			float3 normal = normalR[0];
			float projectXY = sqrt(normal.x * normal.x + normal.y * normal.y);
			float cosA = normal.x / projectXY;
			float A = acos(cosA);
			if (normal.y < 0)
				A = -acos(cosA);
			A += 3.14159f / 2.0f;
			float2x2 rotationM = float2x2(float2(cos(A), -sin(A)), float2(sin(A), cos(A)));
				//TexCoord = mul(TexCoord - float2(0.5,0.5), rotationM) + float2(0.5,0.5);

			float2 uv = tex2D(g_sampleUvTex, TexCoord).zw;


			uv = mul(rotationM, uv - float2(0.5, 0.5)) + float2(0.5, 0.5);

				float4 c = tex2D(g_sampleLineMask, uv);
				//return tex2D(g_sampleLineMask, uv);
				//if (c.r < 0.5)
			{
				//N = -N * 10;
				//N = c.r;
			}
		}
	}
	else
	{

		float sinThetaX1 = normalize(cross(normalLProj, normalProjX)).y;
		float a = length(normalize(cross(normalLProj, normalProjX)));
		//float sinThetaX2 = normalize(cross(normalProjX, normalRProj)).y;
		float sinThetaY1 = normalize(cross(normalUProj, normalProjY)).x;
		float b = length(normalize(cross(normalUProj, normalProjY)));
		//float sinThetaY2 = normalize(cross(normalProjX, normalRProj)).x;

		//if (sinThetaX1 > 0.01 || sinThetaY1 > 0.01)
		//	return float4(-a, -a, -a, 0);

		if ((sinThetaX1 > p2 || sinThetaY1 > p2) && !isEdge)
		{
			float2 uv = tex2D(g_sampleUvTex, TexCoord).zw;
				float4 c = tex2D(g_sampleLineMask, uv);
				//return tex2D(g_sampleLineMask, uv);
				//if (c.r < 0.5)
			{
				//N = -N * 10;
				//N = c.r;
			}
		}
	}

	

	//float2 uv = tex2D(g_sampleUvTex, TexCoord).zw;
	//	return float4(uv,0,0);
	//return tex2D(g_sampleLineMask, uv);

	float alpha = 1;
	if (isEdge)
		alpha = 0;
	return float4(N, N, N, alpha);
}

float4 R(float2 TexCoord)
{
	int neighborNum = 4;
	//neighborNum = 8;

	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);

		float depthN[9];

	depthN[0] = GetDepth(TexCoord, g_samplePosition);

	depthN[1] = GetDepth(TexCoord + offset * float2(0, -1), g_samplePosition); //Up
	depthN[2] = GetDepth(TexCoord + offset * float2(1, 0), g_samplePosition); //Right
	depthN[3] = GetDepth(TexCoord + offset * float2(0, 1), g_samplePosition); //Down
	depthN[4] = GetDepth(TexCoord + offset * float2(-1, 0), g_samplePosition); //Left

	depthN[5] = GetDepth(TexCoord + offset * float2(1, -1), g_samplePosition); //RightUp
	depthN[6] = GetDepth(TexCoord + offset * float2(1, 1), g_samplePosition); //RightDown
	depthN[7] = GetDepth(TexCoord + offset * float2(-1, 1), g_samplePosition); //LeftDown
	depthN[8] = GetDepth(TexCoord + offset * float2(-1, -1), g_samplePosition); //LeftUp

	float depthNN[4];
	depthNN[0] = GetDepth(TexCoord + offset * float2(0, -2), g_samplePosition); //UpUp
	depthNN[1] = GetDepth(TexCoord + offset * float2(2, 0), g_samplePosition); //RightRight
	depthNN[2] = GetDepth(TexCoord + offset * float2(0, 2), g_samplePosition); //DownDown
	depthNN[3] = GetDepth(TexCoord + offset * float2(-2, 0), g_samplePosition); //LeftLeft

	float3 normalR[9];

	normalR[0] = normalize(GetNormal(g_sampleNormal, TexCoord));

	//return normalR[0].z;

	normalR[1] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, -1))); //Up
	normalR[2] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 0))); //Right
	normalR[3] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, 1))); //Down
	normalR[4] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0))); //Left

	normalR[5] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, -1))); //RightUp
	normalR[6] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 1))); //RIghtDown
	normalR[7] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 1))); //LeftDown
	normalR[8] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, -1))); //LeftUp

	float depth = depthN[0];

	if (depth > 1000)
	{
		return float4(1, 1, 1, 1);
	}
	float laplace = 4 * depth - (depthN[1] + depthN[2] + depthN[3] + depthN[4]);

	float laplaceN[5];
	laplaceN[1] = 4 * depthN[1] - (depthN[0] + depthN[5] + depthN[8] + depthNN[0]);
	laplaceN[2] = 4 * depthN[2] - (depthN[0] + depthN[5] + depthN[6] + depthNN[1]);
	laplaceN[3] = 4 * depthN[3] - (depthN[0] + depthN[6] + depthN[7] + depthNN[2]);
	laplaceN[4] = 4 * depthN[4] - (depthN[0] + depthN[7] + depthN[8] + depthNN[3]);


	bool isEdge = false;
	bool edgeList[9] = { false, false, false, false, false, false, false, false, false };

	if (g_switch == 1)
	{
		for (int i = 1; i<neighborNum + 1; i++)
		{
			if (laplace > 0.4)
			{
				//return float4(1, 1, 1, 1);
			}
		}
	}

	if (laplace < -0.6)
		isEdge = true;
	else
		isEdge = false;


	for (int i = 1; i< 5; i++)
	{
		if (laplaceN[i]  > 0.6)
		{
			edgeList[i] = true;
		}
	}
	for (int i = 1; i< 5; i++)
	{
		if (laplace > 0.4)
		{
			edgeList[i] = true;
		}
	}

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

	if (g_switch == 1)
	{
		du = dot(normalR[1], normalR[0]);
		dr = dot(normalR[0], normalR[2]);
		dd = dot(normalR[0], normalR[3]);
		dl = dot(normalR[4], normalR[0]);
	}

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


	float d[9] = { 0, du, dr, dd, dl, dru, drd, dld, dlu };
	dmax = -1000;
	dmin = 1000;
	int maxFlag = -1;
	int minFlag = -1;
	for (int i = 1; i < neighborNum + 1; i++)
	{
		if (edgeList[i] != true)
		{
			if (d[i] > dmax)
				maxFlag = i;
			dmax = max(d[i], dmax);

			if (d[i] < dmin)
				minFlag = i;
			dmin = min(d[i], dmin);
		}
	}

	///dmax = (dmax + dmin) / 2;
	
	return float4(dmax, dmax, dmax, dmax);
}


float4 Laplace_Edge2(float2 TexCoord, float2 R_d)
{
	int neighborNum = 4;
	//neighborNum = 8;

	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);

		float depthN[9];

	depthN[0] = GetDepth(TexCoord, g_samplePosition);

	depthN[1] = GetDepth(TexCoord + offset * float2(0, -1), g_samplePosition); //Up
	depthN[2] = GetDepth(TexCoord + offset * float2(1, 0), g_samplePosition); //Right
	depthN[3] = GetDepth(TexCoord + offset * float2(0, 1), g_samplePosition); //Down
	depthN[4] = GetDepth(TexCoord + offset * float2(-1, 0), g_samplePosition); //Left

	depthN[5] = GetDepth(TexCoord + offset * float2(1, -1), g_samplePosition); //RightUp
	depthN[6] = GetDepth(TexCoord + offset * float2(1, 1), g_samplePosition); //RightDown
	depthN[7] = GetDepth(TexCoord + offset * float2(-1, 1), g_samplePosition); //LeftDown
	depthN[8] = GetDepth(TexCoord + offset * float2(-1, -1), g_samplePosition); //LeftUp

	float depthNN[4];
	depthNN[0] = GetDepth(TexCoord + offset * float2(0, -2), g_samplePosition); //UpUp
	depthNN[1] = GetDepth(TexCoord + offset * float2(2, 0), g_samplePosition); //RightRight
	depthNN[2] = GetDepth(TexCoord + offset * float2(0, 2), g_samplePosition); //DownDown
	depthNN[3] = GetDepth(TexCoord + offset * float2(-2, 0), g_samplePosition); //LeftLeft

	float3 normalR[9];

	normalR[0] = normalize(GetNormal(g_sampleNormal, TexCoord));

	//return normalR[0].z;

	normalR[1] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, -1))); //Up
	normalR[2] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 0))); //Right
	normalR[3] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, 1))); //Down
	normalR[4] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0))); //Left

	normalR[5] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, -1))); //RightUp
	normalR[6] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 1))); //RIghtDown
	normalR[7] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 1))); //LeftDown
	normalR[8] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, -1))); //LeftUp

	float depth = depthN[0];

	if (depth > 1000)
	{
		return float4(1, 1, 1, 1);
	}
	float laplace = 4 * depth - (depthN[1] + depthN[2] + depthN[3] + depthN[4]);

	float laplaceN[5];
	laplaceN[1] = 4 * depthN[1] - (depthN[0] + depthN[5] + depthN[8] + depthNN[0]);
	laplaceN[2] = 4 * depthN[2] - (depthN[0] + depthN[5] + depthN[6] + depthNN[1]);
	laplaceN[3] = 4 * depthN[3] - (depthN[0] + depthN[6] + depthN[7] + depthNN[2]);
	laplaceN[4] = 4 * depthN[4] - (depthN[0] + depthN[7] + depthN[8] + depthNN[3]);


	bool isEdge = false;
	bool edgeList[9] = { false, false, false, false, false, false, false, false, false };

	if (g_switch == 1)
	{
		for (int i = 1; i<neighborNum + 1; i++)
		{
			if (laplace > 0.4)
			{
				//return float4(1, 1, 1, 1);
			}
		}
	}

	if (laplace < -0.6)
		isEdge = true;
	else
		isEdge = false;


	for (int i = 1; i< 5; i++)
	{
		if (laplaceN[i]  > 0.6)
		{
			edgeList[i] = true;
		}
	}
	for (int i = 1; i< 5; i++)
	{
		if (laplace > 0.4)
		{
			edgeList[i] = true;
		}
	}

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

	if (g_switch == 1)
	{
		du = dot(normalR[1], normalR[0]);
		dr = dot(normalR[0], normalR[2]);
		dd = dot(normalR[0], normalR[3]);
		dl = dot(normalR[4], normalR[0]);
	}

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


	float d[9] = { 0, du, dr, dd, dl, dru, drd, dld, dlu };
	dmax = -1000;
	dmin = 1000;
	int maxFlag = -1;
	int minFlag = -1;
	for (int i = 1; i < neighborNum + 1; i++)
	{
		if (edgeList[i] != true)
		{
			if (d[i] > dmax)
				maxFlag = i;
			dmax = max(d[i], dmax);

			if (d[i] < dmin)
				minFlag = i;
			dmin = min(d[i], dmin);
		}
	}

	dmax = (dmax + dmin) / 2;
	//dmax = dmax * dmin;
	float N = 1 - dmax / 3.14 * 0.8;

	du = dot(normalUProj, normalProjY);
	dr = dot(normalProjX, normalRProj);
	dd = dot(normalProjY, normalDProj);
	dl = dot(normalLProj, normalProjX);
	dl = acos(dl);
	dr = acos(dr);
	du = acos(du);
	dd = acos(dd);
	d[1] = du;
	d[2] = dr;
	d[3] = dd;
	d[4] = dl;

	N = N*N*N;
	if (isEdge)
	{
		N *= 0.5;
		//if (N > 0.6) N = 0.6;
	}

	float2 dir[8] =
	{
		float2(0, -1),
		float2(1, 0),
		float2(0, 1),
		float2(-1, 0),
		float2(1, -1),
		float2(1, 1),
		float2(-1, 1),
		float2(-1, -1)
	};

	float dirR[8] =
	{
		0,
		1,
		0,
		1,
		3.1415 /2.0f,
		-3.1415 / 2.0f,
		3.1415 / 2.0f,
		3.1415 / 2.0f
	};



	dmax = -1000;
	dmin = 1000;
	maxFlag = -1;
	minFlag = -1;
	for (int i = 1; i < 8 + 1; i++)
	{
		if (edgeList[i] != true)
		{
			if (d[i] > dmax)
				maxFlag = i;
			dmax = max(d[i], dmax);

			if (d[i] < dmin)
				minFlag = i;
			dmin = min(d[i], dmin);
		}
	}
	float2 minDir = dir[minFlag];
		float2 maxDir = dir[maxFlag];

		float p1 = 0.0001;
	float p2 = 0.005;

	bool testR = false;
	bool testL = false;

	bool testU = false;
	bool testD = false;
	{
		float3 T = float3(1, 0, 0);
			float3 N = normalR[0];
			float3 B = cross(T, N);
			T = cross(N, B);
		float3x3 TBN = float3x3(T, B, N);

			float3 NR = normalize(mul(TBN, normalR[2]));
			float3 NL = normalize(mul(TBN, normalR[4]));

			float3 NU = normalize(mul(TBN, normalR[1]));
			float3 ND = normalize(mul(TBN, normalR[3]));

		if (normalR[0].z < 0)
		{

			if (NR.x < -p1)
			{
				testR = true;
			}
			if (NL.x > p1)
			{
				testL = true;
			}
			if (NU.y < -p1)
			{
				testU = true;
			}
			if (ND.y > p1)
			{
				testD = true;
			}
		}
		else
		{
			if (NR.x < -p1)
			{
				testL = true;
			}
			if (NL.x > p1)
			{
				testR = true;
			}
			if (NU.y < -p1)
			{
				testD = true;
			}
			if (ND.y > p1)
			{
				testU = true;
			}
		}

	}
	//if (g_switch == 1)
	{
		if ((testR || testL || testU || testD) && !isEdge)
		{
			float3 normal = normalR[0];
				float projectXY = sqrt(normal.x * normal.x + normal.y * normal.y);
			float cosA = normal.x / projectXY;
			cosA = dirR[maxFlag];// R_d.x / sqrt(R_d.x * R_d.x + R_d.y * R_d.y);;
			float A = acos(cosA);
			if (normal.y < 0)
				A = -acos(cosA);
			A += 3.14159f / 2.0f;
			float2x2 rotationM = float2x2(float2(cos(A), -sin(A)), float2(sin(A), cos(A)));
				//TexCoord = mul(TexCoord - float2(0.5,0.5), rotationM) + float2(0.5,0.5);

				float2 uv = tex2D(g_sampleUvTex, TexCoord).zw;


				uv = mul(rotationM, uv - float2(0.5, 0.5)) + float2(0.5, 0.5);

			float4 c = tex2D(g_sampleLineMask, uv);
				//return tex2D(g_sampleLineMask, uv);
				//if (c.r < 0.5)
			{
				//N = -N * 10;
				//N = c.r;
			}
		}
	}
	//else
	{

		float sinThetaX1 = normalize(cross(normalLProj, normalProjX)).y;
		float a = length(normalize(cross(normalLProj, normalProjX)));
		//float sinThetaX2 = normalize(cross(normalProjX, normalRProj)).y;
		float sinThetaY1 = normalize(cross(normalUProj, normalProjY)).x;
		float b = length(normalize(cross(normalUProj, normalProjY)));
		//float sinThetaY2 = normalize(cross(normalProjX, normalRProj)).x;

		//if (sinThetaX1 > 0.01 || sinThetaY1 > 0.01)
		//	return float4(-a, -a, -a, 0);

		if ((sinThetaX1 > p2 || sinThetaY1 > p2) && !isEdge)
		{
			float2 uv = tex2D(g_sampleUvTex, TexCoord).zw;
				float4 c = tex2D(g_sampleLineMask, uv);
				//return tex2D(g_sampleLineMask, uv);
				//if (c.r < 0.5)
			{
				//N = -N * 10;
				//N = c.r;
			}
		}
	}



	//float2 uv = tex2D(g_sampleUvTex, TexCoord).zw;
	//	return float4(uv,0,0);
	//return tex2D(g_sampleLineMask, uv);

	float alpha = 1;
	if (isEdge)
		alpha = 0;

	float gray = tex2D(g_sampleUvTex, TexCoord).x;
	int J = 1;
	if (gray < 0.4)
	{
		J = 0;
	}
	else
	{
		J = 1;
	}

	return float4(N, J, N, alpha);
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	return R(TexCoord);
	//return Laplace_Edge(TexCoord);	
}

float4 PShader2(float2 TexCoord : TEXCOORD0) : COLOR
{
	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);
	float R = tex2D(g_sampleEdgeMap, TexCoord);
	//return R;
	float R_L = tex2D(g_sampleEdgeMap, TexCoord + offset * float2(-1, 0));
	float R_R = tex2D(g_sampleEdgeMap, TexCoord + offset * float2(1, 0));
	float R_U = tex2D(g_sampleEdgeMap, TexCoord + offset * float2(0, -1));
	float R_D = tex2D(g_sampleEdgeMap, TexCoord + offset * float2(0, 1));

	float3 N_U = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, -1))); //Up
	float3 N_R = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, 0))); //Right
	float3 N_D = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, 1))); //Down
	float3 N_L = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0))); //Left
	float dx = R - R_R;
	float dy = R - R_D;

	float2 d = normalize(float2(dx, dy));

	//return float4(d,0,0);
	float4 edge = Laplace_Edge2(TexCoord, d);
	return edge;
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}

	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader2();
	}
}