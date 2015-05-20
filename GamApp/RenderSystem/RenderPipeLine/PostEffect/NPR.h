#pragma once
#include "PostEffectBase.h"
class NPR :
	public PostEffectBase
{
public:
	NPR();
	~NPR();

	void RenderPost(LPDIRECT3DTEXTURE9 edgeTarget, LPDIRECT3DTEXTURE9 mainBuffer = 0);
};

