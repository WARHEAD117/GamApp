#pragma once
#include "CommonUtil/D3D9Header.h"
#include "RenderUtil/RenderUtil.h"

class RenderPipe
{
public:
	RenderPipe();
	~RenderPipe();

	void	BuildScreenQuad();
	void	BuildBuffers();
	void	BuildEffects();

	void	ClearRenderUtil();
	void	PushRenderUtil(RenderUtil* const renderUtil);

	void	ForwardRender();

	void	RenderDiffuse();
	void	RenderNormal();
	void	RenderPosition();

	void	RenderGBuffer();

	void	ComputeLightPassIndex(LightType type, UINT& lightPassIndex, UINT& shadowPassIndex);
	void	DeferredRender_MultiPass();
	void	DeferredRender_Shading();
	void	DeferredRender_Lighting();
	void	RenderFinalColor();

	void	RenderShadow();

	void	RenderAll();
	
	void	UpdateRenderState();

	LPDIRECT3DTEXTURE9	m_pDiffuseTarget;
	LPDIRECT3DTEXTURE9	m_pNormalTarget;
	LPDIRECT3DTEXTURE9	m_pPositionTarget;

	LPDIRECT3DTEXTURE9	m_pDiffuseLightTarget;
	LPDIRECT3DTEXTURE9	m_pSpecularLightTarget;

	LPDIRECT3DTEXTURE9	m_pShadowTarget;

	LPDIRECT3DTEXTURE9	m_pMainColorTarget;
	LPDIRECT3DTEXTURE9	m_pPostTarget;
	LPDIRECT3DTEXTURE9	m_pTempTarget;

private:
	std::vector<RenderUtil*> mRenderUtilList;

	LPDIRECT3DSURFACE9	m_pDiffuseSurface;
	LPDIRECT3DSURFACE9	m_pNormalSurface;
	LPDIRECT3DSURFACE9	m_pPositionSurface;

	LPDIRECT3DSURFACE9	m_pDiffuseLightSurface;
	LPDIRECT3DSURFACE9	m_pSpecularLightSurface;

	LPDIRECT3DSURFACE9	m_pShadowSurface;

	LPDIRECT3DSURFACE9	m_pMainColorSurface;

	LPDIRECT3DSURFACE9	m_pOriSurface;

private:
	bool	m_enableAO;
	bool	m_enableDOF;
	bool	m_enableHDR;
	bool	m_enableGI;
	bool	m_enableFXAA;
	bool	m_enableDither;
	bool	m_enableColorChange;

	bool	m_enableEdgeRecognize;

#ifdef RENDER_DEBUG
	enum DEBUG_MODE
	{
		NONE,
		ShowNormal,
		ShowPosition,
		ShowDiffuse,
		ShowDiffuseLight,
		ShowSpecularLight,
		ShowShadowResult,
	};

	DEBUG_MODE m_debugMode;
	bool	m_showNormal;
	bool	m_showPosition;
	bool	m_showDiffuse;
	bool	m_showDiffuseLight;
	bool	m_showSpecularLight;
	bool	m_showShadowResult;
#endif // RENDER_DEBUG
};


typedef CSingleton<RenderPipe> RENDERPIPE;