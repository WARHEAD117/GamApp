#pragma once
#include "PostEffectBase.h"
class HDRLighting :
	public PostEffectBase
{

#define NUM_BLOOM 3
#define NUM_STAR 12
#define NUM_TONEMAP 4
#define MAX_SAMPLES 16
public:
	HDRLighting();
	~HDRLighting();

	void RenderPost(LPDIRECT3DTEXTURE9 hdrBuffer);
	void RenderMainToScaled(LPDIRECT3DTEXTURE9 hdrBuffer);
	void RenderScaledToBrightPass();
	void RenderBrightPassToStarSource();
	void RenderStarSourceToBloomSource();
	void CreatePostEffect();

	void MeasureLuminance();
	void CalculateAdaptation();

	void RenderBloom(LPDIRECT3DTEXTURE9 hdrBuffer);
	void RenderStar();

	LPDIRECT3DTEXTURE9 m_pScaledColor; //和输入buffer（hdrBuffer）相同 ARGB16F
	LPDIRECT3DTEXTURE9 m_pBrightPass; //ARGB8
	LPDIRECT3DTEXTURE9 m_pAdaptedLuminanceCur; //R16F
	LPDIRECT3DTEXTURE9 m_pAdaptedLuminanceLast; //R16F
	LPDIRECT3DTEXTURE9 m_pStarSourceTex; //ARGB8
	LPDIRECT3DTEXTURE9 m_pBloomSourceTex; //ARGB8

	LPDIRECT3DTEXTURE9 m_apBloomTex[NUM_BLOOM];
	LPDIRECT3DTEXTURE9 m_apStarTex[NUM_STAR];
	LPDIRECT3DTEXTURE9 m_apToneMap[NUM_TONEMAP];

};

