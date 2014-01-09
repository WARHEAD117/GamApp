#include "RenderUtil.h"
#include "D3D9Device.h"
#include "EntityFeature/Entity.h"

#include "EffectParam.h"

RenderUtil::RenderUtil()
{
}


RenderUtil::~RenderUtil()
{
}

void RenderUtil::BuildEffectInfo()
{
	D3DXMATRIX view = RENDERDEVICE::Instance().ViewMatrix;
	D3DXMATRIX proj = RENDERDEVICE::Instance().ProjMatrix;
	mViewProj = view * proj;

	mWorldMat = mOwner->GetWorldTransform();
}

void RenderUtil::Render()
{
	BuildEffectInfo();
	

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, D3DXGetFVFVertexSize(mFVF));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		mEffectList[i]->SetMatrix(VIEWPROJMATRIX, &mViewProj);
		mEffectList[i]->SetMatrix(WORLDMATRIX, &mWorldMat);

		UINT nPasses = 0;
		HRESULT r1 = mEffectList[i]->Begin(&nPasses, 0);
		HRESULT r2 = mEffectList[i]->BeginPass(0);

		if (&mSubMeshList[i].pMaterial)
			RENDERDEVICE::Instance().g_pD3DDevice->SetMaterial(&mSubMeshList[i].pMaterial);
		if (mSubMeshList[i].pTexture)
		{
			mEffectList[i]->SetTexture(DIFFUSETEXTURE, mSubMeshList[i].pTexture);
		}
		else
		{
			mEffectList[i]->SetTexture(DIFFUSETEXTURE, NULL);
		}

		mEffectList[i]->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);

		mEffectList[i]->EndPass();
		mEffectList[i]->End();
	}
	RENDERDEVICE::Instance().g_pD3DDevice->SetTexture(0, NULL);
}

void RenderUtil::SetVertexBuffer(const LPDIRECT3DVERTEXBUFFER9& vertexBuffer)
{
	mVertexBuffer = vertexBuffer;
}

void RenderUtil::SetIndexBuffer(const LPDIRECT3DINDEXBUFFER9& indexBuffer)
{
	mIndexBuffer = indexBuffer;
}

void RenderUtil::SetSubMeshList(const std::vector<SubMesh>& subMeshList)
{
	mSubMeshList = subMeshList;

	mEffectList.resize(mSubMeshList.size());
}

void RenderUtil::SetEffectList(const std::vector<LPD3DXEFFECT>& effectList)
{
	mEffectList = effectList;
}

void RenderUtil::SetEffect(int subMeshIndex, LPD3DXEFFECT effect)
{
	if (subMeshIndex >= mEffectList.size())
		return;

	mEffectList[subMeshIndex] = effect;
}

void RenderUtil::SetOwner(Entity* owner)
{
	mOwner = owner;
}
