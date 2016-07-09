#include "common.fx"

float4		g_ViewPos;

bool		g_IsPointLight;

uniform extern float4x4 gFinalXForms[54];

struct OutputVS
{
	float4 posWVP			: POSITION;
	float3 posWV			: TEXCOORD0;
};


OutputVS VShader(float4 posL		: POSITION0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	outVS.posWV = mul(posL, g_WorldView);

	return outVS;
}

OutputVS VSkinnedShader(
    float4 posL : POSITION0,
	float4 weight0 : BLENDWEIGHT0,
	int4 boneIndex : BLENDINDICES0)
{
    OutputVS outVS = (OutputVS) 0;

    // Do the vertex blending calculation for posL and normalL.
    float weight1 = 1.0f - weight0.x;

    float4 p = weight0.x * mul(posL, gFinalXForms[boneIndex[0]]);
    p += weight0.y * mul(posL, gFinalXForms[boneIndex[1]]);
    p += weight0.z * mul(posL, gFinalXForms[boneIndex[2]]);
    p += (1 - weight0.x - weight0.y - weight0.z) * mul(posL, gFinalXForms[boneIndex[3]]);
	//p	+= weight2 * mul(float4(posL, 1.0f), gFinalXForms[boneIndex[2]]);
    p.w = 1.0f;

    
	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
    outVS.posWVP = mul(p, g_WorldViewProj);
    outVS.posWV = mul(p, g_WorldView);

    return outVS;
}

float2 ComputeMoments(float Depth)
{
	float2 Moments;

	Moments.x = Depth;

	float dx = ddx(Depth);
	float dy = ddy(Depth);

	Moments.y = Depth*Depth + 0.25 * (dx*dx + dy*dy);

	return Moments;
}

float4 PShader(float3 posWV : TEXCOORD0) : COLOR
{
	//输出颜色
	float4 Color = posWV.z;
	if (g_IsPointLight)
	{
		Color.x = length(posWV);
	}
	//这里有个严重的问题，VSM的计算需要知道一个物体周边的物体的深度
	//也就是说。单独渲染每个物体时计算的monents本质上没有任何使用价值
	//会导致阴影的起点虚化，效果非常差
	//因此一种是只保存普通的灯光深度图，然后在最后渲染阴影的时候直接计算Moments（现在就是这种）
	//另一种方案是在计算出完整阴影深度图后再处理一次，进行Moments的计算，最后渲染的时候就和以前一样
	//前者的好处是可以让阴影图的深度（大小）减小，只需要R32的纹理就可以，不需要保存Moments
	//甚至可以直接用DepthBuffer（如果可以用的话，到现在都不知道怎么用呢）
	//缺点是最后的延迟渲染计算量会进一步增大，可能压力有点大
	//后者的好处延迟渲染的时候和原来一样，但是图需要过两次，可能计算量也不小

	//所以下面这行代码现在其实没有必要留着，但是等想好再处理吧
	float2 monents = ComputeMoments(Color.x);
	Color = float4(monents, 0.0f, 1.0f);

	return Color;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredShadowVSM
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
    }
    pass p1
    {
        vertexShader = compile vs_3_0 VSkinnedShader();
        pixelShader = compile ps_3_0 PShader();
    }
}