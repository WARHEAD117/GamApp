#pragma once
#include "PostEffectBase.h"
class EdgeRecognize :
	public PostEffectBase
{
public:
	EdgeRecognize();
	~EdgeRecognize();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer = 0);

private:
	float m_depthFactor;
	float m_normalFactor;
	void ConfigInput();
};

