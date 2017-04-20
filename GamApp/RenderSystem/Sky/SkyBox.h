#pragma once
#include "CommonUtil/D3D9Header.h"
#include "CommonUtil/GlobalHeader.h"

class SkyBox
{
public:
	SkyBox();
	~SkyBox();

	void SetSkyTexture(const std::string filename, int flag);
	void BuildSkyBox();
	void BuildSkyQuad();
	void RenderInGBuffer(ID3DXEffect* pEffect);
	void RenderInShading(ID3DXEffect* pEffect);

private:
	LPDIRECT3DVERTEXBUFFER9    m_pBufferVex;
	LPDIRECT3DTEXTURE9         m_pSkyTextures[6];

	LPDIRECT3DVERTEXDECLARATION9	mVertexDecl;
	int								mVertexByteSize;

	LPDIRECT3DVERTEXBUFFER9    m_pBufferVexQuad;
	LPDIRECT3DVERTEXDECLARATION9	mVertexDecQuad;
	int								mVertexByteSizeQuad;
};

