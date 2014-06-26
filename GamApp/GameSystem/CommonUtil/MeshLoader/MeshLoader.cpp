#include "MeshLoader.h"
#include "D3D9Device.h"

MeshLoader::MeshLoader()
{
}


MeshLoader::~MeshLoader()
{
// 	SafeRelease(m_pMesh);
// 	SafeRelease(mVertexBuffer);
// 	SafeRelease(mIndexBuffer);
}

std::string getFilePath(std::string in)
{
	int strSize = in.size();
	int splitIndex = in.find_last_of("\\");
	return in.substr(0, splitIndex+1);
}

HRESULT MeshLoader::LoadXMesh(std::string filePath)
{
	ID3DXBuffer* adjBuffer = 0;
	LPD3DXBUFFER pXBuffer;
	if (FAILED(D3DXLoadMeshFromX(filePath.c_str(), D3DXMESH_SYSTEMMEM, RENDERDEVICE::Instance().g_pD3DDevice,
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

	HRESULT hr;
	//generate normals
	if (hasNormals == false)
	{
		hr = D3DXComputeNormals(m_pMesh, NULL);
	}
		
	bool hasTangents = false;
	term = D3DDECL_END();
	for (int i = 0; i < MAX_FVF_DECL_SIZE; ++i)
	{
		// Did we reach D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}?
		if (elems[i].Stream == 0xff)
			break;

		if (elems[i].Type == D3DDECLTYPE_FLOAT3 &&
			elems[i].Usage == D3DDECLUSAGE_TANGENT &&
			elems[i].UsageIndex == 0)
		{
			hasTangents = true;
			break;
		}
	}
	
	DWORD* rgdwAdjacency = NULL;
	rgdwAdjacency = new DWORD[m_pMesh->GetNumFaces() * 3];

	if (rgdwAdjacency == NULL)
	{
		return E_OUTOFMEMORY;
	}
	m_pMesh->GenerateAdjacency(1e-6f, rgdwAdjacency);

	// Optimize the mesh for this graphics card's vertex cache 
	// so when rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     
	m_pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL);

	if (!hasTangents)// || !bHadBinormal)
	{
		ID3DXMesh* pNewMesh;

		// Compute tangents, which are required for normal mapping
		if (S_OK != D3DXComputeTangentFrameEx(m_pMesh, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0,
			D3DDECLUSAGE_BINORMAL, 0,
			D3DDECLUSAGE_NORMAL, 0, 0, rgdwAdjacency, -1.01f,
			-0.01f, -1.01f, &pNewMesh, NULL))
		{
			return E_FAIL;
		}

		SafeRelease(m_pMesh);
		m_pMesh = pNewMesh;
	}
	
	//Optimize the mesh
	m_pMesh->Optimize(D3DXMESH_MANAGED |
		D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, &m_pMesh);
	//ReleaseCOM(m_pMesh); // Done w/ system mesh.
	TRelease(adjBuffer); // Done with buffer.

	D3DXMATERIAL *Material = (D3DXMATERIAL *)pXBuffer->GetBufferPointer();
	m_Materials.resize(m_dwMtrlNum);
	m_pTextures.resize(m_dwMtrlNum);
	m_pNormalMaps.resize(m_dwMtrlNum);
	m_pSpecularMap.resize(m_dwMtrlNum);
	for (DWORD i = 0; i<m_dwMtrlNum; i++)
	{
		m_Materials[i] = Material[i].MatD3D;
		m_Materials[i].Ambient = m_Materials[i].Diffuse;
		m_pTextures[i] = NULL;  //纹理指针先清空 ;
		m_pTextures[i] = RENDERDEVICE::Instance().GetDefaultTexture();
		if (Material[i].pTextureFilename != NULL &&
			lstrlen(Material[i].pTextureFilename) > 0)
		{
			std::string	fileName = (char*)Material[i].pTextureFilename;
			std::string	texfilePath = getFilePath(filePath);
			std::string	texfinalPath = texfilePath + fileName;

			if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, texfinalPath.c_str(), &m_pTextures[i])))
			{
				m_pTextures[i] = RENDERDEVICE::Instance().GetDefaultTexture();
			}
			
		}
		m_pNormalMaps[i] = RENDERDEVICE::Instance().GetDefaultTexture();
		m_pSpecularMap[i] = RENDERDEVICE::Instance().GetDefaultTexture();
	}
	
	pXBuffer->Release();

	if (!SUCCEEDED(m_pMesh->GetVertexBuffer(&mVertexBuffer)))
	{
		return S_FALSE;
	}

	if (!SUCCEEDED(m_pMesh->GetIndexBuffer(&mIndexBuffer)))
	{
		return S_FALSE;
	}

	DWORD num_attr;
	m_pMesh->GetAttributeTable(NULL, &num_attr);
	m_AttTable.resize(num_attr);
	m_pMesh->GetAttributeTable(&m_AttTable[0], &num_attr);

	mSubMeshList.resize(num_attr);
	for (int i = 0; i < (int)num_attr; i++)
	{
		mSubMeshList[i].subMeshId = m_AttTable[i].AttribId;
		mSubMeshList[i].faceStart = m_AttTable[i].FaceStart;
		mSubMeshList[i].faceCount = m_AttTable[i].FaceCount;
		mSubMeshList[i].vertexStart = m_AttTable[i].VertexStart;
		mSubMeshList[i].vertexCount = m_AttTable[i].VertexCount;
		mSubMeshList[i].indexStart = m_AttTable[i].FaceStart*3;
		mSubMeshList[i].indexCount = m_AttTable[i].FaceCount*3;
		mSubMeshList[i].pTexture = m_pTextures[i];
		mSubMeshList[i].pNormalMap = m_pNormalMaps[i];
		mSubMeshList[i].pSpecularMap = m_pSpecularMap[i];
		mSubMeshList[i].pMaterial = m_Materials[i];
	}


	return S_OK;
}

LPD3DXMESH& MeshLoader::GetD3DMesh()
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
	// 计算得到外接球球心及半径
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

HRESULT MeshLoader::LoadTexture(std::string filePath, int subMeshIndex)
{
	if (m_pTextures.size() <= subMeshIndex || mSubMeshList.size() <= subMeshIndex)
		return S_FALSE;
	if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, filePath.c_str(), &m_pTextures[subMeshIndex])))
	{
		m_pTextures[subMeshIndex] = RENDERDEVICE::Instance().GetDefaultTexture();
	}
	mSubMeshList[subMeshIndex].pTexture = m_pTextures[subMeshIndex];
	return S_OK;
}

HRESULT MeshLoader::LoadNormal(std::string filePath, int subMeshIndex)
{
	if (m_pNormalMaps.size() <= subMeshIndex || mSubMeshList.size() <= subMeshIndex)
		return S_FALSE;
	if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, filePath.c_str(), &m_pNormalMaps[subMeshIndex])))
	{
		m_pNormalMaps[subMeshIndex] = RENDERDEVICE::Instance().GetDefaultTexture();
	}
	mSubMeshList[subMeshIndex].pNormalMap = m_pNormalMaps[subMeshIndex];
	return S_OK;
}

HRESULT MeshLoader::LoadSpecular(std::string filePath, int subMeshIndex)
{
	if (m_pSpecularMap.size() <= subMeshIndex || mSubMeshList.size() <= subMeshIndex)
		return S_FALSE;
	if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, filePath.c_str(), &m_pSpecularMap[subMeshIndex])))
	{
		m_pSpecularMap[subMeshIndex] = RENDERDEVICE::Instance().GetDefaultTexture();
	}
	mSubMeshList[subMeshIndex].pSpecularMap = m_pSpecularMap[subMeshIndex];
	return S_OK;
}


