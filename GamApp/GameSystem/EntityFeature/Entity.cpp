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
	mRenderUtil.SetVertexDecl(mMeshLoader.GetVertexDecl());
	mRenderUtil.SetVertexByteSize(mMeshLoader.GetVertexByteSize());

	for (int i = 0; i < mMeshLoader.GetSubMeshList().size(); i++)
	{
		mRenderUtil.SetEffect(i, RENDERDEVICE::Instance().GetDefaultEffect());
	}

	AssignRenderUtil();
}

void Entity::SetMeshFileName(std::string fileName)
{
	mMeshLoader.LoadXMesh(fileName);
	BuildRenderUtil();
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

void Entity::SetTexture(std::string fileName, int subMeshIndex /*= -1*/)
{
	if (subMeshIndex >= 0)
	{
		mMeshLoader.LoadTexture(fileName, subMeshIndex);
	}
	else
	{
		int subMeshCount = mMeshLoader.GetSubMeshCount();
		for (int i = 0; i < subMeshCount; i++)
		{
			mMeshLoader.LoadTexture(fileName, i);
		}
	}
	mRenderUtil.SetSubMeshList(mMeshLoader.GetSubMeshList());
}

void Entity::SetNormalMap(std::string fileName, int subMeshIndex /*= -1*/)
{
	if (subMeshIndex >= 0)
	{
		mMeshLoader.LoadNormal(fileName, subMeshIndex);
	}
	else
	{
		int subMeshCount = mMeshLoader.GetSubMeshCount();
		for (int i = 0; i < subMeshCount; i++)
		{
			mMeshLoader.LoadNormal(fileName, i);
		}
	}
	mRenderUtil.SetSubMeshList(mMeshLoader.GetSubMeshList());
}

void Entity::SetSpecularMap(std::string fileName, int subMeshIndex /*= -1*/)
{
	if (subMeshIndex >= 0)
	{
		mMeshLoader.LoadSpecular(fileName, subMeshIndex);
	}
	else
	{
		int subMeshCount = mMeshLoader.GetSubMeshCount();
		for (int i = 0; i < subMeshCount; i++)
		{
			mMeshLoader.LoadSpecular(fileName, i);
		}
	}
	mRenderUtil.SetSubMeshList(mMeshLoader.GetSubMeshList());
}
