#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"

class Entity;

struct SubMesh
{
	DWORD subMeshId;
	DWORD faceStart;
	DWORD faceCount;
	DWORD vertexStart;
	DWORD indexStart;
	DWORD vertexCount;
	DWORD indexCount;
	LPDIRECT3DTEXTURE9 pTexture;
	D3DMATERIAL9 pMaterial;
	ID3DXEffect* pEffect;
};

class RenderUtil
{
public:
	RenderUtil();
	~RenderUtil();

	void	Render();

	void	SetVertexBuffer(const LPDIRECT3DVERTEXBUFFER9& vertexBuffer);
	void	SetIndexBuffer(const LPDIRECT3DINDEXBUFFER9& indexBuffer);
	void	SetSubMeshList(const std::vector<SubMesh>& subMeshList);
	void	SetFVF(DWORD fvf);

	void	SetEffectList(const std::vector<LPD3DXEFFECT>& effectList);
	void	SetEffect(int subMeshIndex, LPD3DXEFFECT effect);

	void	SetOwner(Entity* owner);
private:
	LPDIRECT3DVERTEXBUFFER9		mVertexBuffer;
	LPDIRECT3DINDEXBUFFER9		mIndexBuffer;
	std::vector<SubMesh>		mSubMeshList;
	DWORD						mFVF;

	std::vector<LPD3DXEFFECT>	mEffectList;

	D3DXMATRIX					mViewProj;
	D3DXMATRIX					mWorldMat;

	Entity*						mOwner;
private:
	void	BuildEffectInfo();
};

inline void RenderUtil::SetFVF(DWORD fvf)
{
	mFVF = fvf;
}
