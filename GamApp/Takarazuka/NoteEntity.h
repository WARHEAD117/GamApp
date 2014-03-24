#pragma once
#include "EntityFeature\Entity.h"
#include "CommonUtil\D3D9Header.h"

class NoteEntity :
	public Entity
{
public:
	NoteEntity();
	~NoteEntity();

	void SetTexture(LPDIRECT3DTEXTURE9 texture);
	void InitSubMeshInfo();
	void SetEffect(LPD3DXEFFECT effect);
	void SetMaterial(Material& material);
	void SetVertexBuffer(LPDIRECT3DVERTEXBUFFER9& vertexBuffer);
	void SetIndexBuffer(LPDIRECT3DINDEXBUFFER9& indexBuffer);

	virtual void BuildRenderUtil();
	virtual void AssignRenderUtil();
private:
	LPD3DXEFFECT mEffect;
	std::vector<SubMesh> mSubMeshList;
	LPDIRECT3DINDEXBUFFER9 mIndexBuffer;
	LPDIRECT3DVERTEXBUFFER9 mVertexBuffer;
};

