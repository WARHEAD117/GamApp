#include "NoteEntity.h"
#include "RenderPipeLine/RenderPipe.h"

NoteEntity::NoteEntity()
{
	mSubMeshList.resize(1);
}


NoteEntity::~NoteEntity()
{
}

void NoteEntity::SetTexture(LPDIRECT3DTEXTURE9 texture)
{
	if (texture)
	{
		mSubMeshList[0].pTexture = texture;
	}
}

void NoteEntity::SetMaterial(Material& material)
{
	mSubMeshList[0].pMaterial = material;
}

void NoteEntity::SetVertexBuffer(LPDIRECT3DVERTEXBUFFER9& vertexBuffer)
{
	if (vertexBuffer)
	{
		mVertexBuffer = vertexBuffer;
	}
}

void NoteEntity::SetIndexBuffer(LPDIRECT3DINDEXBUFFER9& indexBuffer)
{
	if (indexBuffer)
	{
		mIndexBuffer = indexBuffer;
	}
}

void NoteEntity::InitSubMeshInfo()
{
	mSubMeshList[0].subMeshId = 0;
	mSubMeshList[0].faceStart = 0;
	mSubMeshList[0].faceCount = 2;
	mSubMeshList[0].vertexStart = 0;
	mSubMeshList[0].vertexCount = 4;
	mSubMeshList[0].indexStart = 0 * 3;
	mSubMeshList[0].indexCount = 2 * 3;
}

//test
#define D3DFVF_VERTEX (D3DFVF_XYZ |D3DFVF_NORMAL|D3DFVF_TEX1| D3DFVF_DIFFUSE)

void NoteEntity::BuildRenderUtil()
{
	mRenderUtil.SetVertexBuffer(mVertexBuffer);
	mRenderUtil.SetIndexBuffer(mIndexBuffer);
	mRenderUtil.SetSubMeshList(mSubMeshList);
	mRenderUtil.SetFVF(D3DFVF_VERTEX);

	mRenderUtil.SetMaterial(0, RENDERDEVICE::Instance().GetDefaultMaterial());

	AssignRenderUtil();
}

void NoteEntity::AssignRenderUtil()
{
	RENDERPIPE::Instance().PushRenderUtil(&mRenderUtil);
}

void NoteEntity::SetEffect(LPD3DXEFFECT effect)
{
	if (effect)
	{
		mEffect = effect;
	}
}
