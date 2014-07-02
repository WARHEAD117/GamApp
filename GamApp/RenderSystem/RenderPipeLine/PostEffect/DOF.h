#pragma once
#include "PostEffectBase.h"
class DOF :
	public PostEffectBase
{
public:
	DOF();
	~DOF();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();

private:
	LPDIRECT3DTEXTURE9	m_ColorCoCBuffer;
	LPDIRECT3DTEXTURE9	m_ColorHorizontal;
	LPDIRECT3DTEXTURE9	m_ColorStep1;
	LPDIRECT3DTEXTURE9	m_ColorStep2;
};

