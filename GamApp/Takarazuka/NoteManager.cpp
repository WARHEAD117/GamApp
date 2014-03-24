#include "NoteManager.h"
#include "D3D9Device.h"
#include "EntityFeature/EntityManager.h"

int MAXNOTESIZE = 1000;
NoteManager::NoteManager()
{
}


NoteManager::~NoteManager()
{
	for (int i = 0; i < mNoteList.size(); i++)
		SafeDelete(mNoteList[i]);
	mNoteList.clear();
}

void NoteManager::InitNote()
{
	LoadTexture();
	LoadMaterial();
	mEffectLoader.LoadFxEffect("System\\commonDiffuse.fx");

	mVertexManager.BuildAllVertex();

	mNoteList.resize(MAXNOTESIZE);

	for (int i = 0; i < mNoteList.size(); i++)
	{
		mNoteList[i] = new NoteEntity();
		mNoteList[i]->SetTexture(mTextureList[0]);

		LPDIRECT3DVERTEXBUFFER9	tempVB;
		LPDIRECT3DINDEXBUFFER9	tempIB;
		mVertexManager.GetVertex(0, tempVB, tempIB);

		mNoteList[i]->SetIndexBuffer(tempIB);
		mNoteList[i]->SetVertexBuffer(tempVB);

		mNoteList[i]->InitSubMeshInfo();

		mNoteList[i]->SetEffect(RENDERDEVICE::Instance().GetDefaultEffect());

		mNoteList[i]->BuildRenderUtil();
		mNoteList[i]->AssignRenderUtil();

		ENTITYMANAGER::Instance().AddEntity(*mNoteList[i]);
	}
}

void NoteManager::LoadTexture()
{
	int textureCount = 1;
	mTextureList.resize(textureCount);

	for (int i = 0; i < mTextureList.size(); i++)
	{
		if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\Texture\\noteTest.png",
			&mTextureList[i])))
		{
			mTextureList[i] = NULL;
		}
	}
	
}

void NoteManager::LoadMaterial()
{
	int materialCount = 1;
	mTextureList.resize(materialCount);

	for (int i = 0; i < mMaterialList.size(); i++)
	{
		
	}
}
