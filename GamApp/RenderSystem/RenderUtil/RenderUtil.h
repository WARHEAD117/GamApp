#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"
#include "Light/LightManager.h"

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
	LPDIRECT3DTEXTURE9 pNormalMap;
	LPDIRECT3DTEXTURE9 pSpecularMap;
	Material pMaterial;
	ID3DXEffect* pEffect;
};

class RenderUtil
{
public:
	RenderUtil();
	~RenderUtil();

	void	Render();
	void	RenderNormal();
	void	RenderDiffuse();
	void	RenderPosition();
	void	RenderShadow(D3DXMATRIX lightViewMat, D3DXMATRIX lightProjMat, LightType lightType);

	void	RenderDeferredGeometry(ID3DXEffect* pEffect);

	void	SetVertexBuffer(const LPDIRECT3DVERTEXBUFFER9& vertexBuffer);
	void	SetIndexBuffer(const LPDIRECT3DINDEXBUFFER9& indexBuffer);
	void	SetSubMeshList(const std::vector<SubMesh>& subMeshList);
	void	SetFVF(DWORD fvf);
	void	SetVertexDecl(LPDIRECT3DVERTEXDECLARATION9 vertexDecl);
	void	SetVertexByteSize(int vbs);

	void	SetMaterialList(const std::vector<Material*>& materialList);
	void	SetEffect(int subMeshIndex, LPD3DXEFFECT effect);
	void	SetMaterial(int subMeshIndex, Material* material);
	const Material*	GetMaterial(int subMeshIndex);

	void	SetOwner(Entity* owner);

private:
	void	SetlightInfo(LPD3DXEFFECT effect);
private:
	LPDIRECT3DVERTEXBUFFER9		mVertexBuffer;
	LPDIRECT3DINDEXBUFFER9		mIndexBuffer;
	std::vector<SubMesh>		mSubMeshList;
	DWORD						mFVF;
	LPDIRECT3DVERTEXDECLARATION9 mVertexDecl;
	int							mVertexByteSize;

	//std::vector<Material*>		mMaterialList;

	D3DXMATRIX					mWorldMat;
	D3DXMATRIX					mViewMat;
	D3DXMATRIX					mProjMat;

	D3DXMATRIX					mLastWorldMat
;
	D3DXMATRIX					mWorldView;
	D3DXMATRIX					mViewProj;
	D3DXMATRIX					mWorldViewProj;

	Entity*						mOwner;
private:
	void	BuildEffectInfo();
	void BuildShadowEffectInfo(D3DXMATRIX lightViewMat, D3DXMATRIX lightProjMat);
};

inline void RenderUtil::SetFVF(DWORD fvf)
{
	mFVF = fvf;
}

inline void RenderUtil::SetVertexDecl(LPDIRECT3DVERTEXDECLARATION9 vertexDecl)
{
	mVertexDecl = vertexDecl;
}

inline void RenderUtil::SetVertexByteSize(int vbs)
{
	mVertexByteSize = vbs;
}