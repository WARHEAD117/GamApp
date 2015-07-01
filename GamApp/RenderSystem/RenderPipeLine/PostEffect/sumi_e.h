#pragma once
#include "PostEffectBase.h"
class SumiE :
	public PostEffectBase
{
public:
	SumiE();
	~SumiE();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();
	
	void SetEdgeImage(LPDIRECT3DTEXTURE9 edgeImage);
private:
	LPDIRECT3DVERTEXDECLARATION9	mParticleDecl;
	LPDIRECT3DVERTEXBUFFER9			mParticleBuffer;

	LPDIRECT3DVERTEXBUFFER9			mParticleBuffer2;

	LPDIRECT3DTEXTURE9	m_ColorCoCBuffer;
	LPDIRECT3DTEXTURE9	m_ColorHorizontal;
	LPDIRECT3DTEXTURE9	m_ColorStep1;
	LPDIRECT3DTEXTURE9	m_ColorStep2;

	LPDIRECT3DTEXTURE9	m_pEdgeForward;
	LPDIRECT3DTEXTURE9	m_pEdgeBlur;
	LPDIRECT3DTEXTURE9	m_pEdgeImage;

	LPDIRECT3DTEXTURE9	m_pTestInk;
	LPDIRECT3DTEXTURE9	m_pInkTex;
	LPDIRECT3DTEXTURE9	m_pInkTex1;
	LPDIRECT3DTEXTURE9	m_pInkTex2;
	LPDIRECT3DTEXTURE9	m_pInkTex3;
	LPDIRECT3DTEXTURE9	m_pInkTex4;

	LPDIRECT3DTEXTURE9	m_pInkMask;
	LPDIRECT3DTEXTURE9	m_pInkCloud;

	LPDIRECT3DTEXTURE9	m_Garyscale;
	LPDIRECT3DTEXTURE9	m_BlurredGaryscale;

	LPDIRECT3DTEXTURE9	m_StrokesArea_1;
	LPDIRECT3DTEXTURE9	m_StrokesArea_2;
	LPDIRECT3DTEXTURE9	m_StrokesArea_3;
	LPDIRECT3DTEXTURE9	m_StrokesArea_4;
	LPDIRECT3DTEXTURE9	m_StrokesArea_5;


	LPDIRECT3DTEXTURE9 m_Diffusion1;
	LPDIRECT3DTEXTURE9 m_Diffusion2;

	LPDIRECT3DTEXTURE9 m_pContourTarget;
	LPDIRECT3DTEXTURE9 m_pInsideTarget;

	LPD3DXEFFECT		m_SynthesisEffect;

};

