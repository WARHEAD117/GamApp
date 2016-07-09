#include "SkinEntity.h"
#include "RenderPipeLine/RenderPipe.h"

SkinEntity::SkinEntity()
{
}


SkinEntity::~SkinEntity()
{
	SafeDelete(mSkinnedMesh);
}

void SkinEntity::OnBeginFrame()
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	mSkinnedMesh->update(dTime);
}

void SkinEntity::OnFrame()
{

}

void SkinEntity::OnEndFrame()
{

}

void SkinEntity::AssignRenderUtil()
{
	RENDERPIPE::Instance().PushSkinnedMesh(mSkinnedMesh);
}

void SkinEntity::BuildRenderUtil()
{
	AssignRenderUtil();
}

void SkinEntity::SetMeshFileName(std::string fileName)
{
	mSkinnedMesh = new SkinnedMesh(fileName);
	mSkinnedMesh->SetOwner(this);
	BuildRenderUtil();
}

void SkinEntity::SetMaterial(Material* material, int subMeshIndex /*= -1*/)
{
	//SkinnedMesh's subMesh feature is not ready!
	mSkinnedMesh->SetMaterial(material);

	/*
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
	*/
}

void SkinEntity::SetTexture(std::string fileName, int subMeshIndex /*= -1*/)
{
	//SkinnedMesh's subMesh feature is not ready!
	mSkinnedMesh->SetTexture(fileName);

	/*
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
	*/
}

void SkinEntity::SetNormalMap(std::string fileName, int subMeshIndex /*= -1*/)
{
	//SkinnedMesh's subMesh feature is not ready!
	mSkinnedMesh->SetNormalMap(fileName);

	/*
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
	*/
}

void SkinEntity::SetSpecularMap(std::string fileName, int subMeshIndex /*= -1*/)
{
	//SkinnedMesh's subMesh feature is not ready!
	mSkinnedMesh->SetSpecularMap(fileName);

	/*
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
	*/
}
