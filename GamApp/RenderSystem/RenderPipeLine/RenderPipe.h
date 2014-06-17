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
	void	RenderPosition();
	void	RenderAO();
	void	DeferredRender();
	void	DeferredRender_MultiPass();
	void	RenderMainColor();

	void	RenderShadow();

	void	RenderAll();
	
	void	UpdateRenderState();

	LPDIRECT3DTEXTURE9	m_pDiffuseTarget;
	LPDIRECT3DTEXTURE9	m_pNormalDepthTarget;
	LPDIRECT3DTEXTURE9	m_pPositionTarget;
	LPDIRECT3DTEXTURE9	m_pAOTarget;
	LPDIRECT3DTEXTURE9	m_pShadowTarget;

	LPDIRECT3DTEXTURE9	m_pMainColorTarget;

private:
	std::vector<RenderUtil*> mRenderUtilList;

	LPDIRECT3DSURFACE9	m_pDIffuseSurface;
	LPDIRECT3DSURFACE9	m_pNormalDepthSurface;
	LPDIRECT3DSURFACE9	m_pPositionSurface;
	LPDIRECT3DSURFACE9	m_pAOSurface;
	LPDIRECT3DSURFACE9	m_pShadowSurface;

	LPDIRECT3DSURFACE9	m_pMainColorSurface;

	LPDIRECT3DSURFACE9	m_pOriSurface;
};


typedef CSingleton<RenderPipe> RENDERPIPE;