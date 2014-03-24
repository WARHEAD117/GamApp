#pragma once
#include "NoteEntity.h"
#include "VertexManager.h"

class NoteManager
{
public:
	NoteManager();
	~NoteManager();

	void InitNote();
	void LoadTexture();
	void LoadMaterial();
	void LoadNoteConfig();
private:
	VertexManager mVertexManager;

	std::vector<NoteEntity*> mNoteList;

	std::vector<LPDIRECT3DTEXTURE9> mTextureList;
	std::vector<Material*> mMaterialList;


	EffectLoader mEffectLoader;
};

