#include "RenderUtil.h"
#include "D3D9Device.h"
#include "EntityFeature/Entity.h"
#include "Light/LightManager.h"
#include "Light/DirectionLight.h"

#include "EffectParam.h"

RenderUtil::RenderUtil()
{
}


RenderUtil::~RenderUtil()
{
}

void RenderUtil::BuildEffectInfo()
{
	mWorldMat = mOwner->GetWorldTransform();
	D3DXMATRIX view = RENDERDEVICE::Instance().ViewMatrix;
	D3DXMATRIX proj = RENDERDEVICE::Instance().ProjMatrix;
	mViewProj = view * proj;
	mWorldViewProj = mWorldMat * mViewProj;
	
}

void RenderUtil::SetlightInfo(LPD3DXEFFECT effect)
{
// 	LIGHTMANAGER::Instance().
	BaseLight* pLight = LIGHTMANAGER::Instance().GetLight(0);
	DirectionLight* pDirLight = dynamic_cast<DirectionLight*>(pLight);
	D3DXVECTOR3 lightDir = pDirLight->GetLightDir();

	D3DXVECTOR3 viewPos = RENDERDEVICE::Instance().ViewPosition;
	effect->SetVector(LIGHTDIRECTION, &D3DXVECTOR4(lightDir.x, lightDir.y, lightDir.z, 1.0));
	effect->SetVector(VIEWPOSITION, &D3DXVECTOR4(viewPos.x, viewPos.y, viewPos.z, 1.0));
}

void RenderUtil::Render()
{
	BuildEffectInfo();
	

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, D3DXGetFVFVertexSize(mFVF));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		mMaterialList[i]->effect->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
		mMaterialList[i]->effect->SetMatrix(VIEWPROJMATRIX, &mViewProj);
		mMaterialList[i]->effect->SetMatrix(WORLDMATRIX, &mWorldMat);

		UINT nPasses = 0;
		HRESULT r1 = mMaterialList[i]->effect->Begin(&nPasses, 0);
		HRESULT r2 = mMaterialList[i]->effect->BeginPass(0);

		{
			mMaterialList[i]->effect->SetVector(AMBIENTMATERIAL, &mSubMeshList[i].pMaterial.Ambient);
			mMaterialList[i]->effect->SetVector(DIFFUSEMATERIAL, &mSubMeshList[i].pMaterial.Diffuse);
			mMaterialList[i]->effect->SetVector(SPECULARMATERIAL, &mSubMeshList[i].pMaterial.Specular);
			mMaterialList[i]->effect->SetFloat(SPECULARPOWER, (float)mSubMeshList[i].pMaterial.Power);
		}

		if (mSubMeshList[i].pTexture)
		{
			mMaterialList[i]->effect->SetTexture(DIFFUSETEXTURE, mSubMeshList[i].pTexture);
			
		}
		else
		{
			mMaterialList[i]->effect->SetTexture(DIFFUSETEXTURE, NULL);
		}

		SetlightInfo(mMaterialList[i]->effect);

		mMaterialList[i]->effect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);

		mMaterialList[i]->effect->EndPass();
		mMaterialList[i]->effect->End();
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

	mMaterialList.resize(mSubMeshList.size());
}

void RenderUtil::SetMaterialList(const std::vector<Material*>& materialList)
{
	mMaterialList = materialList;
}

void RenderUtil::SetEffect(int subMeshIndex, LPD3DXEFFECT effect)
{
	if (subMeshIndex >= mMaterialList.size())
		return;

	mMaterialList[subMeshIndex]->effect = effect;
}

void RenderUtil::SetOwner(Entity* owner)
{
	mOwner = owner;
}

void RenderUtil::SetMaterial(int subMeshIndex, Material* material)
{
	if (subMeshIndex >= mMaterialList.size())
		return;

	mMaterialList[subMeshIndex] = material;
}
