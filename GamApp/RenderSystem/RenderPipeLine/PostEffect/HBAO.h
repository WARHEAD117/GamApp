#pragma once
#include "PostEffectBase.h"
class HBAO :
	public PostEffectBase
{
public:
	HBAO();
	~HBAO();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();
	void ComputeAOConfig();

private:
	float		m_intensity;
	float		m_scale;
	float		m_bias;
	float		m_sample_rad;
	float		m_rad_scale;
	float		m_rad_threshold;
	bool		m_AOEnable;

	LPDIRECT3DTEXTURE9			m_pRandomNormal;
	LPDIRECT3DTEXTURE9			m_pAoTarget;
	LPDIRECT3DSURFACE9			m_pAoSurface;
};

