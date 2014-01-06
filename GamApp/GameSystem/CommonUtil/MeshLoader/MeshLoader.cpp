#include "MeshLoader.h"


MeshLoader::MeshLoader()
{
}


MeshLoader::~MeshLoader()
{
}

std::string getFilePath(std::string in)
{
	int strSize = in.size();
	int splitIndex = in.find_last_of("\\");
	return in.substr(0, splitIndex+1);
}

HRESULT MeshLoader::LoadXMesh(char* filePath)
{
	ID3DXBuffer* adjBuffer = 0;
	LPD3DXBUFFER pXBuffer;
	if (FAILED(D3DXLoadMeshFromX(filePath, D3DXMESH_SYSTEMMEM, RENDERDEVICE::Instance().g_pD3DDevice,
		&adjBuffer, &pXBuffer, NULL, &m_dwMtrlNum, &m_pMesh)))
		return E_FAIL;
	
	//Check normals
	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	m_pMesh->GetDeclaration(elems);

	bool hasNormals = false;
	D3DVERTEXELEMENT9 term = D3DDECL_END();
	for (int i = 0; i < MAX_FVF_DECL_SIZE; ++i)
	{
		// Did we reach D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}?
		if (elems[i].Stream == 0xff)
			break;

		if (elems[i].Type == D3DDECLTYPE_FLOAT3 &&
			elems[i].Usage == D3DDECLUSAGE_NORMAL &&
			elems[i].UsageIndex == 0)
		{
			hasNormals = true;
			break;
		}
	}

	//generate normals
	if (hasNormals == false)
		D3DXComputeNormals(m_pMesh, 0);

	//Optimize the mesh
	m_pMesh->Optimize(D3DXMESH_MANAGED |
		D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, &m_pMesh);
	//ReleaseCOM(m_pMesh); // Done w/ system mesh.
	Release(adjBuffer); // Done with buffer.

	D3DXMATERIAL *Material = (D3DXMATERIAL *)pXBuffer->GetBufferPointer();
	m_Materials.resize(m_dwMtrlNum);
	m_pTextures.resize(m_dwMtrlNum);
	for (DWORD i = 0; i<m_dwMtrlNum; i++)
	{
		m_Materials[i] = Material[i].MatD3D;
		m_Materials[i].Ambient = m_Materials[i].Diffuse;
		m_pTextures[i] = NULL;  //����ָ������� ;
		if (Material[i].pTextureFilename != NULL &&
			lstrlen(Material[i].pTextureFilename) > 0)
		{
			std::string	fileName = (char*)Material[i].pTextureFilename;
			std::string	texfilePath = getFilePath(filePath);
			std::string	texfinalPath = texfilePath + fileName;

			if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, texfinalPath.c_str(), &m_pTextures[i])))
			{
				m_pTextures[i] = NULL;
			}
		}
	}
	
	pXBuffer->Release();
	return S_OK;
}

LPD3DXMESH MeshLoader::GetD3DMesh()
{
	return m_pMesh;
}

void MeshLoader::ComputeBoundingBox()
{
	HRESULT hr = 0;
	BYTE* v = 0;
	D3DXVECTOR3 min, max;

	m_pMesh->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		m_pMesh->GetNumVertices(),
		D3DXGetFVFVertexSize(m_pMesh->GetFVF()),
		&min,
		&max);
	m_pMesh->UnlockVertexBuffer();
	if (FAILED(hr))
		return;

	mBoundingBoxInfo.max = max;
	mBoundingBoxInfo.min = min;
}

void MeshLoader::ComputeBoundingSphere()
{
	HRESULT hr = 0;
	BYTE* v = 0;
	D3DXVECTOR3 center; float radius;
	// ����õ���������ļ��뾶
	m_pMesh->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		m_pMesh->GetNumVertices(),
		D3DXGetFVFVertexSize(m_pMesh->GetFVF()),
		&center,
		&radius);
	m_pMesh->UnlockVertexBuffer();
	if (FAILED(hr))
		return;

	mBoundingSphereInfo.center = center;
	mBoundingSphereInfo.radius = radius;
}
