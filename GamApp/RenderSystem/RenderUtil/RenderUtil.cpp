#include "RenderUtil.h"
#include "D3D9Device.h"
#include "EntityFeature/Entity.h"
#include "Light/DirectionLight.h"

#include "Camera/CameraParam.h"
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
	mViewMat = RENDERDEVICE::Instance().ViewMatrix;
	mProjMat = RENDERDEVICE::Instance().ProjMatrix;
	
	mWorldView		= mWorldMat * mViewMat;
	mViewProj		= mViewMat * mProjMat;
	mWorldViewProj	= mWorldMat * mViewProj;
}

void RenderUtil::BuildShadowEffectInfo(D3DXMATRIX lightViewMat, D3DXMATRIX lightProjMat)
{
	mWorldMat = mOwner->GetWorldTransform();

	mViewMat = lightViewMat;
	mProjMat = lightProjMat;

	mWorldView = mWorldMat * mViewMat;
	mViewProj = mViewMat * mProjMat;
	mWorldViewProj = mWorldMat * mViewProj;
}

void RenderUtil::SetlightInfo(LPD3DXEFFECT effect)
{
// 	LIGHTMANAGER::Instance().
	BaseLight* pLight = LIGHTMANAGER::Instance().GetLight(0);
	D3DXVECTOR3 lightDir = pLight->GetLightWorldDir();

	D3DXVECTOR3 viewPos = RENDERDEVICE::Instance().ViewPosition;
	effect->SetVector(LIGHTDIRECTION, &D3DXVECTOR4(lightDir.x, lightDir.y, lightDir.z, 1.0));
	effect->SetVector(VIEWPOSITION, &D3DXVECTOR4(viewPos.x, viewPos.y, viewPos.z, 1.0));
}

void RenderUtil::Render()
{
	BuildEffectInfo();
	

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, mVertexByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		LPD3DXEFFECT effect= mSubMeshList[i].pMaterial.effect;
		if (!effect)
		{
			effect = RENDERDEVICE::Instance().defaultEffect;
		}
		effect->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
		effect->SetMatrix(VIEWPROJMATRIX, &mViewProj);
		effect->SetMatrix(WORLDMATRIX, &mWorldMat);
		effect->SetMatrix(VIEWMATRIX, &mViewMat);

		UINT nPasses = 0;
		HRESULT r1 = effect->Begin(&nPasses, 0);
		HRESULT r2 = effect->BeginPass(0);

		{
			effect->SetVector(AMBIENTMATERIAL, &mSubMeshList[i].pMaterial.Ambient);
			effect->SetVector(DIFFUSEMATERIAL, &mSubMeshList[i].pMaterial.Diffuse);
			effect->SetVector(SPECULARMATERIAL, &mSubMeshList[i].pMaterial.Specular);
			effect->SetFloat(SPECULARPOWER, (float)mSubMeshList[i].pMaterial.Power);
		}

		if (mSubMeshList[i].pTexture)
		{
			effect->SetTexture(DIFFUSETEXTURE, mSubMeshList[i].pTexture);
			
		}
		else
		{
			effect->SetTexture(DIFFUSETEXTURE, RENDERDEVICE::Instance().GetDefaultTexture());
		}

		SetlightInfo(effect);

		effect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);

		effect->EndPass();
		effect->End();
	}
	RENDERDEVICE::Instance().g_pD3DDevice->SetTexture(0, NULL);
}

void RenderUtil::RenderNormal()
{
	BuildEffectInfo();

	RENDERDEVICE::Instance().GetNormalEffect()->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
	RENDERDEVICE::Instance().GetNormalEffect()->SetMatrix(WORLDVIEWMATRIX, &mWorldView);
	RENDERDEVICE::Instance().GetNormalEffect()->SetFloat("g_zNear", CameraParam::zNear);
	RENDERDEVICE::Instance().GetNormalEffect()->SetFloat("g_zFar", CameraParam::zFar);

	UINT nPasses = 0;
	HRESULT r1 = RENDERDEVICE::Instance().GetNormalEffect()->Begin(&nPasses, 0);
	HRESULT r2 = RENDERDEVICE::Instance().GetNormalEffect()->BeginPass(0);

	RENDERDEVICE::Instance().GetNormalEffect()->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, mVertexByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);
	}

	RENDERDEVICE::Instance().GetNormalEffect()->EndPass();
	RENDERDEVICE::Instance().GetNormalEffect()->End();
	RENDERDEVICE::Instance().g_pD3DDevice->SetTexture(0, NULL);
}

void RenderUtil::RenderDiffuse()
{
	BuildEffectInfo();


	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, mVertexByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		RENDERDEVICE::Instance().GetDiffuseEffect()->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
		
		UINT nPasses = 0;
		HRESULT r1 = RENDERDEVICE::Instance().GetDiffuseEffect()->Begin(&nPasses, 0);
		HRESULT r2 = RENDERDEVICE::Instance().GetDiffuseEffect()->BeginPass(0);

		if (mSubMeshList[i].pTexture)
		{
			RENDERDEVICE::Instance().GetDiffuseEffect()->SetTexture(DIFFUSETEXTURE, mSubMeshList[i].pTexture);

		}
		else
		{
			RENDERDEVICE::Instance().GetDiffuseEffect()->SetTexture(DIFFUSETEXTURE, RENDERDEVICE::Instance().GetDefaultTexture());
		}

		RENDERDEVICE::Instance().GetDiffuseEffect()->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);

		RENDERDEVICE::Instance().GetDiffuseEffect()->EndPass();
		RENDERDEVICE::Instance().GetDiffuseEffect()->End();
	}
	RENDERDEVICE::Instance().g_pD3DDevice->SetTexture(0, NULL);
}

void RenderUtil::RenderPosition()
{
	BuildEffectInfo();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, mVertexByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		RENDERDEVICE::Instance().GetPositionEffect()->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
		RENDERDEVICE::Instance().GetPositionEffect()->SetMatrix(WORLDVIEWMATRIX, &mWorldView);

		UINT nPasses = 0;
		HRESULT r1 = RENDERDEVICE::Instance().GetPositionEffect()->Begin(&nPasses, 0);
		HRESULT r2 = RENDERDEVICE::Instance().GetPositionEffect()->BeginPass(0);

		RENDERDEVICE::Instance().GetPositionEffect()->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);

		RENDERDEVICE::Instance().GetPositionEffect()->EndPass();
		RENDERDEVICE::Instance().GetPositionEffect()->End();
	}
}

void RenderUtil::RenderShadow(D3DXMATRIX lightViewMat, D3DXMATRIX lightProjMat, LightType lightType)
{
	BuildShadowEffectInfo(lightViewMat, lightProjMat);

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, mVertexByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		RENDERDEVICE::Instance().GetShadowEffect()->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
		RENDERDEVICE::Instance().GetShadowEffect()->SetMatrix(WORLDVIEWMATRIX, &mWorldView);

		if (lightType == ePointLight)
			RENDERDEVICE::Instance().GetShadowEffect()->SetBool("g_IsPointLight", true);
		else
			RENDERDEVICE::Instance().GetShadowEffect()->SetBool("g_IsPointLight", false);

		UINT nPasses = 0;
		HRESULT r1 = RENDERDEVICE::Instance().GetShadowEffect()->Begin(&nPasses, 0);
		HRESULT r2 = RENDERDEVICE::Instance().GetShadowEffect()->BeginPass(0);

		RENDERDEVICE::Instance().GetShadowEffect()->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);

		RENDERDEVICE::Instance().GetShadowEffect()->EndPass();
		RENDERDEVICE::Instance().GetShadowEffect()->End();
	}
}



void RenderUtil::RenderDeferredGeometry(ID3DXEffect* pEffect)
{
	BuildEffectInfo();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mVertexBuffer, 0, mVertexByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(mIndexBuffer);
	for (DWORD i = 0; i < mSubMeshList.size(); i++)
	{
		pEffect->SetMatrix(WORLDVIEWPROJMATRIX, &mWorldViewProj);
		pEffect->SetMatrix(WORLDVIEWMATRIX, &mWorldView);
		pEffect->SetMatrix(WORLDMATRIX, &mWorldMat);
		pEffect->SetMatrix(VIEWMATRIX, &mViewMat);

		if (mSubMeshList[i].pTexture)
		{
			pEffect->SetTexture(DIFFUSETEXTURE, mSubMeshList[i].pTexture);
		}
		else
		{
			pEffect->SetTexture(DIFFUSETEXTURE, RENDERDEVICE::Instance().GetDefaultTexture());
		}

		if (mSubMeshList[i].pNormalMap)
		{
			pEffect->SetTexture(NORMALMAP, mSubMeshList[i].pNormalMap);
		}
		else
		{
			pEffect->SetTexture(NORMALMAP, RENDERDEVICE::Instance().GetDefaultNormalMap());
		}

		if (mSubMeshList[i].pSpecularMap)
		{
			pEffect->SetTexture(SPECULARMAP, mSubMeshList[i].pSpecularMap);
		}
		else
		{
			pEffect->SetTexture(SPECULARMAP, RENDERDEVICE::Instance().GetDefaultTexture());
		}

		pEffect->SetFloat("g_shininess", mSubMeshList[i].pMaterial.Power);

		pEffect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			mSubMeshList[i].vertexCount, mSubMeshList[i].indexStart, mSubMeshList[i].faceCount);
	}
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

	//mMaterialList.resize(mSubMeshList.size());
}

void RenderUtil::SetMaterialList(const std::vector<Material*>& materialList)
{
	if (materialList.size() != mSubMeshList.size())
		return;
	for (int i = 0; i < mSubMeshList.size(); i++)
	{
		mSubMeshList[i].pMaterial = *materialList[i];
	}
	//mMaterialList = materialList;
}

void RenderUtil::SetEffect(int subMeshIndex, LPD3DXEFFECT effect)
{
	if (subMeshIndex >= mSubMeshList.size())
		return;

	mSubMeshList[subMeshIndex].pMaterial.effect = effect;

	//if (subMeshIndex >= mMaterialList.size())
	//	return;

	//mMaterialList[subMeshIndex]->effect = effect;
}

void RenderUtil::SetOwner(Entity* owner)
{
	mOwner = owner;
}

void RenderUtil::SetMaterial(int subMeshIndex, Material* material)
{
	if (subMeshIndex >= mSubMeshList.size())
		return;

	mSubMeshList[subMeshIndex].pMaterial = *material;

	//if (subMeshIndex >= mMaterialList.size())
	//	return;

	//mMaterialList[subMeshIndex] = material;
}

const Material* RenderUtil::GetMaterial(int subMeshIndex)
{
	if (subMeshIndex >= mSubMeshList.size())
	{
		return NULL;
	}

	return &mSubMeshList[subMeshIndex].pMaterial;
}
