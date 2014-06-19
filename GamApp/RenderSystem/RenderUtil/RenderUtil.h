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
	Material pMaterial;
	ID3DXEffect* pEffect;
};

class RenderUtil
{
public:
	RenderUtil();
	~RenderUtil();

	void	Render();
	void	RenderNormalDepth();
	void	RenderDiffuse();
	void	RenderPosition();
	void	RenderShadow(int lightIndex);

	void	RenderDeferredGeometry(ID3DXEffect* pEffect);

	void	SetVertexBuffer(const LPDIRECT3DVERTEXBUFFER9& vertexBuffer);
	void	SetIndexBuffer(const LPDIRECT3DINDEXBUFFER9& indexBuffer);
	void	SetSubMeshList(const std::vector<SubMesh>& subMeshList);
	void	SetFVF(DWORD fvf);

	void	SetMaterialList(const std::vector<Material*>& materialList);
	void	SetEffect(int subMeshIndex, LPD3DXEFFECT effect);
	void	SetMaterial(int subMeshIndex, Material* material);

	void	SetOwner(Entity* owner);

private:
	void	SetlightInfo(LPD3DXEFFECT effect);
private:
	LPDIRECT3DVERTEXBUFFER9		mVertexBuffer;
	LPDIRECT3DINDEXBUFFER9		mIndexBuffer;
	std::vector<SubMesh>		mSubMeshList;
	DWORD						mFVF;

	std::vector<Material*>		mMaterialList;

	D3DXMATRIX					mWorldMat;
	D3DXMATRIX					mViewMat;
	D3DXMATRIX					mProjMat;
	D3DXMATRIX					mWorldView;
	D3DXMATRIX					mViewProj;
	D3DXMATRIX					mWorldViewProj;

	Entity*						mOwner;
private:
	void	BuildEffectInfo();
	void BuildShadowEffectInfo(int lightIndex);
};

inline void RenderUtil::SetFVF(DWORD fvf)
{
	mFVF = fvf;
}
