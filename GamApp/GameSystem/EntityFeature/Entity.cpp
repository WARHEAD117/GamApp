#include "Entity.h"
#include "RenderPipeLine/RenderPipe.h"

unsigned int Entity::baseFastIndex = 0;
Entity::Entity() :
	fastIndex(++baseFastIndex)
{
	D3DXMatrixIdentity(&mWorldTransform);
	mRenderUtil.SetOwner(this);
}


Entity::~Entity()
{
}

void Entity::OnBeginFrame()
{

}

void Entity::OnFrame()
{

}

void Entity::OnEndFrame()
{

}

void Entity::AssignRenderUtil()
{
	RENDERPIPE::Instance().PushRenderUtil(&mRenderUtil);
}

void Entity::BuildRenderUtil()
{
	LPDIRECT3DVERTEXBUFFER9	testVB;
	LPDIRECT3DINDEXBUFFER9	testIB;
	mMeshLoader.GetVertexBuffer(testVB);
	mMeshLoader.GetIndexBuffer(testIB);
	mRenderUtil.SetVertexBuffer(testVB);
	mRenderUtil.SetIndexBuffer(testIB);
	mRenderUtil.SetSubMeshList(mMeshLoader.GetSubMeshList());
	mRenderUtil.SetFVF(mMeshLoader.GetVertexFVF());

	for (int i = 0; i < mMeshLoader.GetSubMeshList().size(); i++)
	{
		mRenderUtil.SetMaterial(i, RENDERDEVICE::Instance().GetDefaultMaterial());
	}

	AssignRenderUtil();
}

void Entity::SetMeshFileName(std::string fileName)
{
	mMeshLoader.LoadXMesh(fileName);
}

void Entity::SetEffect(std::string fileName, int subMeshIndex)
{
	mEffectLoader.LoadFxEffect(fileName);
	if (subMeshIndex >= 0)
	{
		mRenderUtil.SetEffect(subMeshIndex, mEffectLoader.GetEffect());
	}
	else
	{
		int subMeshCount = mMeshLoader.GetSubMeshCount();
		for (int i = 0; i < subMeshCount; i++)
		{
			mRenderUtil.SetEffect(i, mEffectLoader.GetEffect());
		}
	}
}

void Entity::SetEffect(LPD3DXEFFECT effect, int subMeshIndex)
{
	if (subMeshIndex >= 0)
	{
		mRenderUtil.SetEffect(subMeshIndex, effect);
	}
	else
	{
		int subMeshCount = mMeshLoader.GetSubMeshCount();
		for (int i = 0; i < subMeshCount; i++)
		{
			mRenderUtil.SetEffect(i, effect);
		}
	}
}

void Entity::SetMaterial(Material* material, int subMeshIndex /*= -1*/)
{
	if (subMeshIndex >= 0)
	{
		mRenderUtil.SetMaterial(subMeshIndex, material);
	}
	else
	{
		int subMeshCount = mMeshLoader.GetSubMeshCount();
		for (int i = 0; i < subMeshCount; i++)
		{
			mRenderUtil.SetMaterial(i, material);
		}
	}
}
