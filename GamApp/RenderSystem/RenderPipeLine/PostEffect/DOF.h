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
	LPDIRECT3DTEXTURE9	m_CoCBuffer;
};

