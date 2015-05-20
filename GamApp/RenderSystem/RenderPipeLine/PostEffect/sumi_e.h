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

private:
	LPDIRECT3DTEXTURE9	m_ColorCoCBuffer;
	LPDIRECT3DTEXTURE9	m_ColorHorizontal;
	LPDIRECT3DTEXTURE9	m_ColorStep1;
	LPDIRECT3DTEXTURE9	m_ColorStep2;

	LPDIRECT3DTEXTURE9	m_pTestInk;

	LPDIRECT3DTEXTURE9	m_Garyscale;
	LPDIRECT3DTEXTURE9	m_BlurredGaryscale;

	LPDIRECT3DTEXTURE9	m_StrokesArea_1;
	LPDIRECT3DTEXTURE9	m_StrokesArea_2;
	LPDIRECT3DTEXTURE9	m_StrokesArea_3;
	LPDIRECT3DTEXTURE9	m_StrokesArea_4;
	LPDIRECT3DTEXTURE9	m_StrokesArea_5;


	LPDIRECT3DTEXTURE9 m_Diffusion1;
	LPDIRECT3DTEXTURE9 m_Diffusion2;

};

