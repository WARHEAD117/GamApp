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
	float		m_intensity;
	float		m_scale;
	float		m_bias;
	float		m_sample_rad;
	float		m_rad_scale;
	float		m_rad_threshold;
	bool		m_SSREnable;

	LPDIRECT3DTEXTURE9			m_pRandomNormal;
	LPDIRECT3DTEXTURE9			m_pSSRTarget;
	LPDIRECT3DSURFACE9			m_pSSRSurface;


	LPDIRECT3DTEXTURE9			m_pBlurXTarget;
	LPDIRECT3DSURFACE9			m_pBlurXSurface;
	LPDIRECT3DTEXTURE9			m_pBlurYTarget;
	LPDIRECT3DSURFACE9			m_pBlurYSurface;

	void SetGaussian(LPD3DXEFFECT effect, float deltaX, float deltaY, std::string weightArrayName, std::string offsetArrayName);
	float ComputeGaussianWeight(float n);
};

