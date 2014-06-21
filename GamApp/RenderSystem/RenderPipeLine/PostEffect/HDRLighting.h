#pragma once
#include "PostEffectBase.h"
class HDRLighting :
	public PostEffectBase
{
public:
	HDRLighting();
	~HDRLighting();

	void RenderPost(LPDIRECT3DTEXTURE9 lastBuffer);
	void CreatePostEffect();

	LPDIRECT3DTEXTURE9 m_pScaledColor; //∫ÕMainColorœ‡Õ¨ ARGB16F
	LPDIRECT3DTEXTURE9 m_pBrightPass; //ARGB8
	LPDIRECT3DTEXTURE9 m_pAdaptedLuminanceCur; //R16F
	LPDIRECT3DTEXTURE9 m_pAdaptedLuminanceLast; //R16F
	LPDIRECT3DTEXTURE9 m_pStarTex; //ARGB8
	LPDIRECT3DTEXTURE9 m_pBloomTex; //ARGB8

};

