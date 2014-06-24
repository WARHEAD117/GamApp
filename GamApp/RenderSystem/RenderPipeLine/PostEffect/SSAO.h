#pragma once
#include "PostEffectBase.h"
class SSAO :
	public PostEffectBase
{
public:
	SSAO();
	~SSAO();

	void RenderPost();
	void CreatePostEffect();

private:
	float		m_intensity;
	float		m_scale;
	float		m_bias;
	float		m_sample_rad;
	bool		m_AOEnable;

	LPDIRECT3DTEXTURE9         m_pRandomNormal;
};

