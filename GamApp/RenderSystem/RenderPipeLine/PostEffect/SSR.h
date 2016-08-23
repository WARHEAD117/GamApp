#pragma once
#include "PostEffectBase.h"
class SSR :
	public PostEffectBase
{
public:
	SSR();
	~SSR();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();
	void ComputeSSRConfig();

private:
	float		m_Roughness;
	float		m_RayAngle;
	float		m_StepLength;
	float		m_ScaleFactor;
	float		m_ScaleFactor2;
	float		m_rad_threshold;
	bool		m_SSREnable;

	LPDIRECT3DTEXTURE9			m_pRandomTex;
	LPDIRECT3DTEXTURE9			m_pSSRTarget;
	LPDIRECT3DSURFACE9			m_pSSRSurface;

	LPDIRECT3DTEXTURE9			m_pResolveTarget;
	LPDIRECT3DSURFACE9			m_pResolveSurface;

	LPDIRECT3DTEXTURE9			m_pMainTargetWithMip;

	LPDIRECT3DTEXTURE9			m_pBlurXTarget;
	LPDIRECT3DSURFACE9			m_pBlurXSurface;
	LPDIRECT3DTEXTURE9			m_pBlurYTarget;
	LPDIRECT3DSURFACE9			m_pBlurYSurface;

	void SetGaussian(LPD3DXEFFECT effect, float deltaX, float deltaY, std::string weightArrayName, std::string offsetArrayName);
	float ComputeGaussianWeight(float n);
	bool BuildMipMap(const LPDIRECT3DTEXTURE9 src, LPDIRECT3DTEXTURE9 dest);
};

