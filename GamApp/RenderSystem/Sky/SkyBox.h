#pragma once
#include "CommonUtil/D3D9Header.h"
#include "CommonUtil/GlobalHeader.h"

class SkyBox
{

protected:
	struct VERTEX
	{
		D3DXVECTOR3		position;
		D3DXVECTOR2		uv;
		D3DXVECTOR3		normal;
		D3DXVECTOR3		tangent;
		D3DXVECTOR3		binormal;
		
	};

public:
	SkyBox();
	~SkyBox();

	void SetSkyTexture(const std::string filename, int flag);
	void BuildSkyBox();
	void RenderInGBuffer(ID3DXEffect* pEffect);

private:
	LPDIRECT3DVERTEXBUFFER9    m_pBufferVex;
	LPDIRECT3DTEXTURE9         m_pSkyTextures[6];

	LPDIRECT3DVERTEXDECLARATION9	mVertexDecl;
	int								mVertexByteSize;
};

