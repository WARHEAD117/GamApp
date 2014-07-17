#pragma once
#include "PostEffectBase.h"
class SSAO :
	public PostEffectBase
{
public:
	SSAO();
	~SSAO();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();
	void ComputeAOConfig();

private:
	float		m_intensity;
	float		m_scale;
	float		m_bias;
	float		m_sample_rad;
	bool		m_AOEnable;

	LPDIRECT3DTEXTURE9			m_pRandomNormal;
	LPDIRECT3DTEXTURE9			m_pAoTarget;
	LPDIRECT3DSURFACE9			m_pAoSurface;

	LPDIRECT3DTEXTURE9			m_pViewDirTarget;
	LPDIRECT3DSURFACE9			m_pViewDirSurface;
};

