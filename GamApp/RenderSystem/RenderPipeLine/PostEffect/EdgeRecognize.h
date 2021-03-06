#pragma once
#include "PostEffectBase.h"
class EdgeRecognize :
	public PostEffectBase
{
	LPDIRECT3DTEXTURE9	m_pMask; 
	
	LPDIRECT3DTEXTURE9	m_pEdgeTarget;
	LPDIRECT3DSURFACE9	m_pEdgeSurface;

	LPDIRECT3DTEXTURE9	m_pEdge2Target;
	LPDIRECT3DSURFACE9	m_pEdge2Surface;

public:
	EdgeRecognize();
	~EdgeRecognize();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer = 0);
	void CreatePostEffect(std::string effectName);
private:
	void ConfigInput();
};

