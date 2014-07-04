#pragma once
#include "CommonUtil/D3D9Header.h"
#include "RenderUtil/RenderUtil.h"



class PostEffectBase
{

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
protected:
	struct VERTEX
	{
		D3DXVECTOR3		position;
		FLOAT			tu, tv;
	};

public:
	PostEffectBase();
	~PostEffectBase();

	virtual void RenderPost(LPDIRECT3DTEXTURE9 lastBuffer = NULL);
	void CreatePostEffect(std::string effectName, D3DFORMAT postTargetFMT = D3DFMT_X8R8G8B8);
	LPDIRECT3DTEXTURE9 GetPostTarget();
private:
	

protected:
	LPDIRECT3DTEXTURE9	m_pPostTarget;
	LPD3DXEFFECT		m_postEffect;
	LPDIRECT3DSURFACE9	m_pPostSurface;
	LPDIRECT3DVERTEXBUFFER9		m_pBufferVex;


	//-----------------------------------------------------------------------------
	// Name: GaussianDistribution
	// Desc: Helper function for GetSampleOffsets function to compute the 
	//       2 parameter Gaussian distrubution using the given standard deviation
	//       rho
	//-----------------------------------------------------------------------------
	float GaussianDistribution(float x, float y, float rho)
	{
		float g = 1.0f / sqrtf(2.0f * D3DX_PI * rho * rho);
		g *= expf(-(x * x + y * y) / (2 * rho * rho));

		return g;
	}

	//-----------------------------------------------------------------------------
	// Name: GetSampleOffsets_DownScale4x4
	// Desc: Get the texture coordinate offsets to be used inside the DownScale4x4
	//       pixel shader.
	//-----------------------------------------------------------------------------
	HRESULT GetSampleOffsets_DownScale4x4(DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[])
	{
		if (NULL == avSampleOffsets)
			return E_INVALIDARG;

		float tU = 1.0f / dwWidth;
		float tV = 1.0f / dwHeight;

		// Sample from the 16 surrounding points. Since the center point will be in
		// the exact center of 16 texels, a 0.5f offset is needed to specify a texel
		// center.
		int index = 0;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				avSampleOffsets[index].x = (x - 1.5f) * tU;
				avSampleOffsets[index].y = (y - 1.5f) * tV;

				index++;
			}
		}

		return S_OK;
	}




	//-----------------------------------------------------------------------------
	// Name: GetSampleOffsets_DownScale2x2
	// Desc: Get the texture coordinate offsets to be used inside the DownScale2x2
	//       pixel shader.
	//-----------------------------------------------------------------------------
	HRESULT GetSampleOffsets_DownScale2x2(DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[])
	{
		if (NULL == avSampleOffsets)
			return E_INVALIDARG;

		float tU = 1.0f / dwWidth;
		float tV = 1.0f / dwHeight;

		// Sample from the 4 surrounding points. Since the center point will be in
		// the exact center of 4 texels, a 0.5f offset is needed to specify a texel
		// center.
		int index = 0;
		for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				avSampleOffsets[index].x = (x - 0.5f) * tU;
				avSampleOffsets[index].y = (y - 0.5f) * tV;

				index++;
			}
		}

		return S_OK;
	}




	//-----------------------------------------------------------------------------
	// Name: GetSampleOffsets_GaussBlur5x5
	// Desc: Get the texture coordinate offsets to be used inside the GaussBlur5x5
	//       pixel shader.
	//-----------------------------------------------------------------------------
	HRESULT GetSampleOffsets_GaussBlur5x5(DWORD dwD3DTexWidth,
		DWORD dwD3DTexHeight,
		D3DXVECTOR2* avTexCoordOffset,
		D3DXVECTOR4* avSampleWeight,
		FLOAT fMultiplier = 1.0f)
	{
		float tu = 1.0f / (float)dwD3DTexWidth;
		float tv = 1.0f / (float)dwD3DTexHeight;

		D3DXVECTOR4 vWhite(1.0f, 1.0f, 1.0f, 1.0f);

		float totalWeight = 0.0f;
		int index = 0;
		for (int x = -2; x <= 2; x++)
		{
			for (int y = -2; y <= 2; y++)
			{
				// Exclude pixels with a block distance greater than 2. This will
				// create a kernel which approximates a 5x5 kernel using only 13
				// sample points instead of 25; this is necessary since 2.0 shaders
				// only support 16 texture grabs.
				if (abs(x) + abs(y) > 2)
					continue;

				// Get the unscaled Gaussian intensity for this offset
				avTexCoordOffset[index] = D3DXVECTOR2(x * tu, y * tv);
				avSampleWeight[index] = vWhite * GaussianDistribution((float)x, (float)y, 1.0f);
				totalWeight += avSampleWeight[index].x;

				index++;
			}
		}

		// Divide the current weight by the total weight of all the samples; Gaussian
		// blur kernels add to 1.0f to ensure that the intensity of the image isn't
		// changed when the blur occurs. An optional multiplier variable is used to
		// add or remove image intensity during the blur.
		for (int i = 0; i < index; i++)
		{
			avSampleWeight[i] /= totalWeight;
			avSampleWeight[i] *= fMultiplier;
		}

		return S_OK;
	}




	//-----------------------------------------------------------------------------
	// Name: GetSampleOffsets_Bloom
	// Desc: Get the texture coordinate offsets to be used inside the Bloom
	//       pixel shader.
	//-----------------------------------------------------------------------------
	HRESULT GetSampleOffsets_Bloom(DWORD dwD3DTexSize,
		float afTexCoordOffset[15],
		D3DXVECTOR4* avColorWeight,
		float fDeviation,
		float fMultiplier)
	{
		int i = 0;
		float tu = 1.0f / (float)dwD3DTexSize;

		// Fill the center texel
		float weight = fMultiplier * GaussianDistribution(0, 0, fDeviation);
		avColorWeight[0] = D3DXVECTOR4(weight, weight, weight, 1.0f);

		afTexCoordOffset[0] = 0.0f;

		// Fill the first half
		for (i = 1; i < 8; i++)
		{
			// Get the Gaussian intensity for this offset
			weight = fMultiplier * GaussianDistribution((float)i, 0, fDeviation);
			afTexCoordOffset[i] = i * tu;

			avColorWeight[i] = D3DXVECTOR4(weight, weight, weight, 1.0f);
		}

		// Mirror to the second half
		for (i = 8; i < 15; i++)
		{
			avColorWeight[i] = avColorWeight[i - 7];
			afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
		}

		return S_OK;
	}




	//-----------------------------------------------------------------------------
	// Name: GetSampleOffsets_Bloom
	// Desc: Get the texture coordinate offsets to be used inside the Bloom
	//       pixel shader.
	//-----------------------------------------------------------------------------
	HRESULT GetSampleOffsets_Star(DWORD dwD3DTexSize,
		float afTexCoordOffset[15],
		D3DXVECTOR4* avColorWeight,
		float fDeviation)
	{
		int i = 0;
		float tu = 1.0f / (float)dwD3DTexSize;

		// Fill the center texel
		float weight = 1.0f * GaussianDistribution(0, 0, fDeviation);
		avColorWeight[0] = D3DXVECTOR4(weight, weight, weight, 1.0f);

		afTexCoordOffset[0] = 0.0f;

		// Fill the first half
		for (i = 1; i < 8; i++)
		{
			// Get the Gaussian intensity for this offset
			weight = 1.0f * GaussianDistribution((float)i, 0, fDeviation);
			afTexCoordOffset[i] = i * tu;

			avColorWeight[i] = D3DXVECTOR4(weight, weight, weight, 1.0f);
		}

		// Mirror to the second half
		for (i = 8; i < 15; i++)
		{
			avColorWeight[i] = avColorWeight[i - 7];
			afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
		}

		return S_OK;
	}
	//-----------------------------------------------------------------------------
	// Name: GetTextureRect()
	// Desc: Get the dimensions of the texture
	//-----------------------------------------------------------------------------
	HRESULT GetTextureRect(PDIRECT3DTEXTURE9 pTexture, RECT* pRect)
	{
		HRESULT hr = S_OK;

		if (pTexture == NULL || pRect == NULL)
			return E_INVALIDARG;

		D3DSURFACE_DESC desc;
		hr = pTexture->GetLevelDesc(0, &desc);
		if (FAILED(hr))
			return hr;

		pRect->left = 0;
		pRect->top = 0;
		pRect->right = desc.Width;
		pRect->bottom = desc.Height;

		return S_OK;
	}


	// Texture coordinate rectangle
	struct CoordRect
	{
		float fLeftU, fTopV;
		float fRightU, fBottomV;
	};

	//-----------------------------------------------------------------------------
	// Name: GetTextureCoords()
	// Desc: Get the texture coordinates to use when rendering into the destination
	//       texture, given the source and destination rectangles
	//-----------------------------------------------------------------------------
	HRESULT GetTextureCoords(PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc,
		PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords)
	{
		HRESULT hr = S_OK;
		D3DSURFACE_DESC desc;
		float tU, tV;

		// Validate arguments
		if (pTexSrc == NULL || pTexDest == NULL || pCoords == NULL)
			return E_INVALIDARG;

		// Start with a default mapping of the complete source surface to complete 
		// destination surface
		pCoords->fLeftU = 0.0f;
		pCoords->fTopV = 0.0f;
		pCoords->fRightU = 1.0f;
		pCoords->fBottomV = 1.0f;

		// If not using the complete source surface, adjust the coordinates
		if (pRectSrc != NULL)
		{
			// Get destination texture description
			hr = pTexSrc->GetLevelDesc(0, &desc);
			if (FAILED(hr))
				return hr;

			// These delta values are the distance between source texel centers in 
			// texture address space
			tU = 1.0f / desc.Width;
			tV = 1.0f / desc.Height;

			pCoords->fLeftU += pRectSrc->left * tU;
			pCoords->fTopV += pRectSrc->top * tV;
			pCoords->fRightU -= (desc.Width - pRectSrc->right) * tU;
			pCoords->fBottomV -= (desc.Height - pRectSrc->bottom) * tV;
		}

		// If not drawing to the complete destination surface, adjust the coordinates
		if (pRectDest != NULL)
		{
			// Get source texture description
			hr = pTexDest->GetLevelDesc(0, &desc);
			if (FAILED(hr))
				return hr;

			// These delta values are the distance between source texel centers in 
			// texture address space
			tU = 1.0f / desc.Width;
			tV = 1.0f / desc.Height;

			pCoords->fLeftU -= pRectDest->left * tU;
			pCoords->fTopV -= pRectDest->top * tV;
			pCoords->fRightU += (desc.Width - pRectDest->right) * tU;
			pCoords->fBottomV += (desc.Height - pRectDest->bottom) * tV;
		}

		return S_OK;
	}
};

inline LPDIRECT3DTEXTURE9 PostEffectBase::GetPostTarget()
{
	return m_pPostTarget;
}
