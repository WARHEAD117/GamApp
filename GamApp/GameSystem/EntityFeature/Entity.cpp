#include "Entity.h"
#include "RenderPipeLine/RenderPipe.h"

unsigned int Entity::baseFastIndex = 0;
Entity::Entity() :
	fastIndex(++baseFastIndex)
{
	D3DXMatrixIdentity(&mWorldTramsform);
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
	RENDERPIPE::Instance().PushRenderUtil(mRenderUtil);
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
		mRenderUtil.SetEffect(i, RENDERDEVICE::Instance().GetDefaultEffect());
	}

	AssignRenderUtil();
}

void Entity::SetMeshFileName(std::string fileName)
{
	mMeshLoader.LoadXMesh(fileName);
}

void Entity::SetEffectFileName(int subMeshIndex, std::string fileName)
{
	mEffectLoader.LoadFxEffect(fileName);
	mRenderUtil.SetEffect(subMeshIndex, mEffectLoader.GetEffect());
}
