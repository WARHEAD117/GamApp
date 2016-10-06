#pragma once
#include "PostEffectBase.h"
class EdgeRecognize :
	public PostEffectBase
{
	LPDIRECT3DTEXTURE9	m_pMask;

public:
	EdgeRecognize();
	~EdgeRecognize();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer = 0);
	void CreatePostEffect(std::string effectName);
private:
	void ConfigInput();
};

