#pragma once
#include "CommonUtil/D3D9Header.h"
#include "RenderUtil/RenderUtil.h"

class RenderPipe
{
public:
	RenderPipe();
	~RenderPipe();

	void	ClearRenderUtil();
	void	PushRenderUtil(RenderUtil* const renderUtil);

	void	ForwardRender();

	void	RenderDiffuse();
	void	RenderNormalDepth();
	void	DeferredRender();
	void	RenderPost();

	void	RenderAll();
	
	void	UpdateRenderState();

	LPDIRECT3DTEXTURE9	m_pDiffuseTarget;
	LPDIRECT3DTEXTURE9	m_pNormalDepthTarget;

private:
	std::vector<RenderUtil*> mRenderUtilList;

	

	LPDIRECT3DSURFACE9	m_pDIffuseSurface;
	LPDIRECT3DSURFACE9	m_pNormalDepthSurface;

	LPDIRECT3DSURFACE9	m_pOriSurface;
};


typedef CSingleton<RenderPipe> RENDERPIPE;