#pragma once
#include "CommonUtil/D3D9Header.h"
#include "RenderUtil/RenderUtil.h"



class PostEffectBase
{

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
protected:
	struct VERTEX
	{
		D3DXVECTOR3		position;
		FLOAT			tu, tv;
	};

public:
	PostEffectBase();
	~PostEffectBase();

	virtual void RenderPost();
	virtual void RenderPost(LPDIRECT3DTEXTURE9 lastBuffer);
	void CreatePostEffect(std::string effectName);
	LPDIRECT3DTEXTURE9 GetPostTarget();
private:
	

protected:
	LPDIRECT3DTEXTURE9	m_pPostTarget;
	LPD3DXEFFECT		m_postEffect;
	LPDIRECT3DSURFACE9	m_pPostSurface;
	LPDIRECT3DVERTEXBUFFER9		m_pBufferVex;

};

inline LPDIRECT3DTEXTURE9 PostEffectBase::GetPostTarget()
{
	return m_pPostTarget;
}
