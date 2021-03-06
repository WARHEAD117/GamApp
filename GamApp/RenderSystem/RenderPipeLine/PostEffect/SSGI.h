#pragma once
#include "PostEffectBase.h"
class SSGI :
	public PostEffectBase
{
public:
	SSGI();
	~SSGI();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();
	void ComputeGIConfig();

private:
	float		m_intensity;
	float		m_scale;
	float		m_bias;
	float		m_sample_rad;
	float		m_rad_scale;
	float		m_rad_threshold;

	LPDIRECT3DTEXTURE9			m_pRandomNormal;
	LPDIRECT3DTEXTURE9			m_pGiTarget;
	LPDIRECT3DSURFACE9			m_pGiSurface;
	LPDIRECT3DTEXTURE9			m_pScaledMainColorTarget;
	LPDIRECT3DSURFACE9			m_pScaledMainColorSurface;
	LPDIRECT3DTEXTURE9			m_pScaledMainColorTarget2;
	LPDIRECT3DSURFACE9			m_pScaledMainColorSurface2;
};

