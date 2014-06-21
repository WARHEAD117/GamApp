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
	void	RenderNormal();
	void	RenderPosition();

	void	RenderGBuffer();

	void	RenderAO();
	void	DeferredRender_MultiPass();
	void	RenderFinalColor();

	void	RenderShadow();

	void	RenderAll();
	
	void	UpdateRenderState();

	LPDIRECT3DTEXTURE9	m_pDiffuseTarget;
	LPDIRECT3DTEXTURE9	m_pNormalTarget;
	LPDIRECT3DTEXTURE9	m_pPositionTarget;

	LPDIRECT3DTEXTURE9	m_pAOTarget;
	LPDIRECT3DTEXTURE9	m_pShadowTarget;

	LPDIRECT3DTEXTURE9	m_pMainColorTarget;
	LPDIRECT3DTEXTURE9	m_pPostTarget;

private:
	std::vector<RenderUtil*> mRenderUtilList;

	LPDIRECT3DSURFACE9	m_pDiffuseSurface;
	LPDIRECT3DSURFACE9	m_pNormalSurface;
	LPDIRECT3DSURFACE9	m_pPositionSurface;
	LPDIRECT3DSURFACE9	m_pAOSurface;

	LPDIRECT3DSURFACE9	m_pShadowSurface;
	LPDIRECT3DSURFACE9	m_pDepthStencilShadowSurface;

	LPDIRECT3DSURFACE9	m_pMainColorSurface;

	LPDIRECT3DSURFACE9	m_pOriSurface;
};


typedef CSingleton<RenderPipe> RENDERPIPE;