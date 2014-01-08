#include "RenderUtil.h"
#include "D3D9Device.h"

RenderUtil::RenderUtil()
{
}


RenderUtil::~RenderUtil()
{
}

void RenderUtil::BuildEffectInfo()
{
	D3DXMATRIX view;
	RENDERDEVICE::Instance().g_pD3DDevice->GetTransform(D3DTS_VIEW, &view);
	D3DXMATRIX proj;
	RENDERDEVICE::Instance().g_pD3DDevice->GetTransform(D3DTS_PROJECTION, &proj);
	mViewProj = view * proj;
}

void RenderUtil::Render()
{
	BuildEffectInfo();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, D3DXGetFVFVertexSize(mFVF));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		mEffectList[i]->SetMatrix("g_ViewProj", &mViewProj);

		UINT nPasses = 0;
		HRESULT r1 = mEffectList[i]->Begin(&nPasses, 0);
		HRESULT r2 = mEffectList[i]->BeginPass(0);

		if (&mSubMeshList[i].pMaterial)
			RENDERDEVICE::Instance().g_pD3DDevice->SetMaterial(&mSubMeshList[i].pMaterial);
		if (mSubMeshList[i].pTexture)
		{
			mEffectList[i]->SetTexture("g_mTexture", mSubMeshList[i].pTexture);
		}
		else
		{
			mEffectList[i]->SetTexture("g_mTexture", NULL);
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
