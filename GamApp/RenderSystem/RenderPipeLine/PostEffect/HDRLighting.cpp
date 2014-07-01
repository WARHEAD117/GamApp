#include "HDRLighting.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "CommonUtil/Timer/GlobalTimer.h"

HDRLighting::HDRLighting()
{
}


HDRLighting::~HDRLighting()
{
}

//----------------------------------------------------------
// Star generation

// Define each line of the star.
typedef struct STARLINE
{
	int nPasses;
	float fSampleLength;
	float fAttenuation;
	float fInclination;

}*LPSTARLINE;


// Simple definition of the star.
typedef struct STARDEF
{
	TCHAR* szStarName;
	int nStarLines;
	int nPasses;
	float fSampleLength;
	float fAttenuation;
	float fInclination;
	bool bRotation;

}*LPSTARDEF;


// Simple definition of the sunny cross filter
typedef struct STARDEF_SUNNYCROSS
{
	TCHAR* szStarName;
	float fSampleLength;
	float fAttenuation;
	float fInclination;

}*LPSTARDEF_SUNNYCROSS;


// Star form library
enum ESTARLIBTYPE
{
	STLT_DISABLE = 0,

	STLT_CROSS,
	STLT_CROSSFILTER,
	STLT_SNOWCROSS,
	STLT_VERTICAL,
	NUM_BASESTARLIBTYPES,

	STLT_SUNNYCROSS = NUM_BASESTARLIBTYPES,

	NUM_STARLIBTYPES,
};


//----------------------------------------------------------
// Star generation object

class CStarDef
{
public:
	TCHAR               m_strStarName[256];

	int m_nStarLines;
	LPSTARLINE m_pStarLine;   // [m_nStarLines]
	float m_fInclination;
	bool m_bRotation;   // Rotation is available from outside ?

	// Static library
public:
	static CStarDef* ms_pStarLib;
	static D3DXCOLOR    ms_avChromaticAberrationColor[8];

	// Public method
public:
	CStarDef();
	CStarDef(const CStarDef& src);
	~CStarDef();

	CStarDef& operator =(const CStarDef& src)
	{
		Initialize(src);
		return *this;
	}

	HRESULT             Construct();
	void                Destruct();
	void                Release();

	HRESULT             Initialize(const CStarDef& src);

	HRESULT             Initialize(ESTARLIBTYPE eType)
	{
		return Initialize(ms_pStarLib[eType]);
	}

	/// Generic simple star generation
	HRESULT             Initialize(const TCHAR* szStarName,
		int nStarLines,
		int nPasses,
		float fSampleLength,
		float fAttenuation,
		float fInclination,
		bool bRotation);

	HRESULT             Initialize(const STARDEF& starDef)
	{
		return Initialize(starDef.szStarName,
			starDef.nStarLines,
			starDef.nPasses,
			starDef.fSampleLength,
			starDef.fAttenuation,
			starDef.fInclination,
			starDef.bRotation);
	}

	/// Specific star generation
	//  Sunny cross filter
	HRESULT             Initialize_SunnyCrossFilter(const TCHAR* szStarName = TEXT("SunnyCross"),
		float fSampleLength = 1.0f,
		float fAttenuation = 0.88f,
		float fLongAttenuation = 0.95f,
		float fInclination = D3DXToRadian(0.0f));


	// Public static method
public:
	/// Create star library
	static HRESULT      InitializeStaticStarLibs();
	static HRESULT      DeleteStaticStarLibs();

	/// Access to the star library
	static const CStarDef& GetLib(DWORD dwType)
	{
		return ms_pStarLib[dwType];
	}

	static const D3DXCOLOR& GetChromaticAberrationColor(DWORD dwID)
	{
		return ms_avChromaticAberrationColor[dwID];
	}
};




void HDRLighting::CreatePostEffect()
{
	PostEffectBase::CreatePostEffect("System\\HDRLighting.fx");

	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 4, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 4,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pScaledColor, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 4 + 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 4 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pBrightPass, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(1, 1,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F, D3DPOOL_DEFAULT,
		&m_pAdaptedLuminanceCur, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(1, 1,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F, D3DPOOL_DEFAULT,
		&m_pAdaptedLuminanceLast, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 4 + 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 4 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pStarSourceTex, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 8 + 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 8 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pBloomSourceTex, NULL);

	for (int i = 0; i < NUM_TONEMAP; i++)
	{
		int sampleSie = 1 << (2 * i);
		RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(sampleSie, sampleSie,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F, D3DPOOL_DEFAULT,
			&m_apToneMap[i], NULL);
	}

	for (int i = 1; i < NUM_BLOOM; i++)
	{
		int sampleSie = 1 << (2 * i);
		RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 8 + 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 8 + 2,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
			&m_apBloomTex[i], NULL);
	}
	// Create the final blooming effect texture
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 8, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 8,
		1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT, &m_apBloomTex[0], NULL);

	for (int i = 0; i < NUM_STAR; i++)
	{
		int sampleSie = 1 << (2 * i);
		RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 8 + 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 8 + 2,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
			&m_apStarTex[i], NULL);
	}

	// Set effect file variables
	m_postEffect->SetFloat("g_fBloomScale", 1.0f);
	m_postEffect->SetFloat("g_fStarScale", 0.5f);

}



bool g_bToneMap = true;
bool g_bBlueShift = true;

float g_fKeyValue = 0.18;
bool g_bAdaptationInvalid = true;
void HDRLighting::RenderPost(LPDIRECT3DTEXTURE9 hdrBuffer)
{
	RenderMainToScaled(hdrBuffer);

	if (g_bToneMap)
		MeasureLuminance();

	// If FrameMove has been called, the user's adaptation level has also changed
	// and should be updated
	if (g_bAdaptationInvalid)
	{
		// Clear the update flag
		//g_bAdaptationInvalid = false;

		// Calculate the current luminance adaptation level
		CalculateAdaptation();
	}

	RenderScaledToBrightPass();
	RenderBrightPassToStarSource();
	RenderStarSourceToBloomSource();

	RenderBloom(hdrBuffer);
	RenderStar();

	m_postEffect->SetTechnique("FinalScenePass");
	m_postEffect->SetFloat("g_fMiddleGray", g_fKeyValue);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);


	m_postEffect->SetBool("g_bEnableToneMap", g_bToneMap);
	m_postEffect->SetBool("g_bEnableBlueShift", g_bBlueShift);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->SetTexture(MAINCOLORBUFFER, hdrBuffer);
	m_postEffect->SetTexture("g_BloomTex", m_apBloomTex[0]);// [0]);
	m_postEffect->SetTexture("g_StarTex", hdrBuffer);// [0]);
	m_postEffect->SetTexture("g_LuminanceCur", m_pAdaptedLuminanceCur);

// 	m_postEffect->SetTexture(MAINCOLORBUFFER, hdrBuffer);
// 	m_postEffect->SetTexture("g_BloomTex", m_apBloomTex[0]);// [0]);
// 	m_postEffect->SetTexture("g_StarTex", m_apToneMap[0]);// [0]);
// 	m_postEffect->SetTexture("g_LuminanceCur", m_pAdaptedLuminanceCur);
//	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
//	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	
}

void HDRLighting::MeasureLuminance()
{
	HRESULT hr = S_OK;
	UINT uiPassCount, uiPass;
	int i, x, y, index;
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];


	DWORD dwCurTexture = NUM_TONEMAP - 1;

	// Sample log average luminance
	PDIRECT3DSURFACE9 apSurfToneMap[NUM_TONEMAP] = { 0 };

	// Retrieve the tonemap surfaces
	for (i = 0; i < NUM_TONEMAP; i++)
	{
		hr = m_apToneMap[i]->GetSurfaceLevel(0, &apSurfToneMap[i]);
	}

	D3DSURFACE_DESC desc;
	m_apToneMap[dwCurTexture]->GetLevelDesc(0, &desc);


	// Initialize the sample offsets for the initial luminance pass.
	float tU, tV;
	tU = 1.0f / (3.0f * desc.Width);
	tV = 1.0f / (3.0f * desc.Height);

	index = 0;
	for (x = -1; x <= 1; x++)
	{
		for (y = -1; y <= 1; y++)
		{
			avSampleOffsets[index].x = x * tU;
			avSampleOffsets[index].y = y * tV;

			index++;
		}
	}


	// After this pass, the g_apTexToneMap[NUM_TONEMAP_TEXTURES-1] texture will contain
	// a scaled, grayscale copy of the HDR scene. Individual texels contain the log 
	// of average luminance values for points sampled on the HDR texture.
	m_postEffect->SetTechnique("SampleAvgLum");
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);

	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pScaledColor);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	dwCurTexture--;

	// Initialize the sample offsets for the iterative luminance passes
	while (dwCurTexture > 0)
	{
		m_apToneMap[dwCurTexture + 1]->GetLevelDesc(0, &desc);
		GetSampleOffsets_DownScale4x4(desc.Width, desc.Height, avSampleOffsets);


		// Each of these passes continue to scale down the log of average
		// luminance texture created above, storing intermediate results in 
		// g_apTexToneMap[1] through g_apTexToneMap[NUM_TONEMAP_TEXTURES-1].
		m_postEffect->SetTechnique("ResampleAvgLum");
		m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);
		m_postEffect->SetTexture(MAINCOLORBUFFER, m_apToneMap[dwCurTexture + 1]);
		RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);

		UINT numPasses = 0;
		m_postEffect->Begin(&numPasses, 0);
		m_postEffect->BeginPass(0);

		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

		m_postEffect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_postEffect->SetTexture(0, NULL);

		m_postEffect->EndPass();
		m_postEffect->End();


		dwCurTexture--;
	}

	// Downsample to 1x1
	m_apToneMap[1]->GetLevelDesc(0, &desc);
	GetSampleOffsets_DownScale4x4(desc.Width, desc.Height, avSampleOffsets);


	// Perform the final pass of the average luminance calculation. This pass
	// scales the 4x4 log of average luminance texture from above and performs
	// an exp() operation to return a single texel cooresponding to the average
	// luminance of the scene in g_apTexToneMap[0].
	m_postEffect->SetTechnique("ResampleAvgLumExp");
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, apSurfToneMap[0]);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_apToneMap[1]);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);

	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();


	for (i = 0; i < NUM_TONEMAP; i++)
	{
		SafeRelease(apSurfToneMap[i]);
	}
}

void HDRLighting::CalculateAdaptation()
{
	HRESULT hr = S_OK;
	UINT uiPass, uiPassCount;

	// Swap current & last luminance
	PDIRECT3DTEXTURE9 pTexSwap = m_pAdaptedLuminanceLast;
	m_pAdaptedLuminanceLast = m_pAdaptedLuminanceCur;
	m_pAdaptedLuminanceCur = pTexSwap;

	PDIRECT3DSURFACE9 pSurfAdaptedLum = NULL;
	m_pAdaptedLuminanceCur->GetSurfaceLevel(0, &pSurfAdaptedLum);

	// This simulates the light adaptation that occurs when moving from a 
	// dark area to a bright area, or vice versa. The g_pTexAdaptedLuminance
	// texture stores a single texel cooresponding to the user's adapted 
	// level.
	m_postEffect->SetTechnique("CalculateAdaptedLum");
	m_postEffect->SetFloat("g_fElapsedTime", GLOBALTIMER::Instance().GetFrameTime());// DXUTGetElapsedTime());

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfAdaptedLum);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pAdaptedLuminanceLast);
	m_postEffect->SetTexture("g_LuminanceCur", m_apToneMap[0]);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();


	SafeRelease(pSurfAdaptedLum);
}

void HDRLighting::RenderMainToScaled( LPDIRECT3DTEXTURE9 hdrBuffer )
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	GetSampleOffsets_DownScale4x4(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight, avSampleOffsets);
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfScaledScene = NULL;
	HRESULT hr = m_pScaledColor->GetSurfaceLevel(0, &pSurfScaledScene);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfScaledScene);
	m_postEffect->SetTexture(MAINCOLORBUFFER, hdrBuffer);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	m_postEffect->SetTechnique("DownScale4x4");
	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	SafeRelease(pSurfScaledScene);
}

void HDRLighting::RenderScaledToBrightPass()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBrightPass = NULL;
	HRESULT hr = m_pBrightPass->GetSurfaceLevel(0, &pSurfBrightPass);

	
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfBrightPass);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pScaledColor);
	m_postEffect->SetTexture("g_LuminanceCur", m_pAdaptedLuminanceCur);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect(m_pBrightPass, &rectDest);
	InflateRect(&rectDest, -1, -1);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetScissorRect(&rectDest);

	m_postEffect->SetTechnique("BrightPassFilter");
	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	SafeRelease(pSurfBrightPass);
}

void HDRLighting::RenderBrightPassToStarSource()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfStarSource = NULL;
	HRESULT hr = m_pStarSourceTex->GetSurfaceLevel(0, &pSurfStarSource);


	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfStarSource);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pBrightPass);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect(m_pStarSourceTex, &rectDest);
	InflateRect(&rectDest, -1, -1);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetScissorRect(&rectDest);

	D3DSURFACE_DESC desc;
	hr = m_pBrightPass->GetLevelDesc(0, &desc);
	GetSampleOffsets_GaussBlur5x5(desc.Width, desc.Height, avSampleOffsets, avSampleWeights);
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
	m_postEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));

	// The gaussian blur smooths out rough edges to avoid aliasing effects
	// when the star effect is run
	m_postEffect->SetTechnique("GaussBlur5x5");

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	SafeRelease(pSurfStarSource);
}

void HDRLighting::RenderStarSourceToBloomSource()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBloomSource = NULL;
	HRESULT hr = m_pBloomSourceTex->GetSurfaceLevel(0, &pSurfBloomSource);


	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfBloomSource);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pStarSourceTex);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect(m_pBloomSourceTex, &rectDest);
	InflateRect(&rectDest, -1, -1);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetScissorRect(&rectDest);


	// Get the sample offsets used within the pixel shader
	D3DSURFACE_DESC desc;
	hr = m_pStarSourceTex->GetLevelDesc(0, &desc);

	GetSampleOffsets_DownScale2x2(desc.Width, desc.Height, avSampleOffsets);
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));


	// The gaussian blur smooths out rough edges to avoid aliasing effects
	// when the star effect is run
	m_postEffect->SetTechnique("DownScale2x2");

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	SafeRelease(pSurfBloomSource);
}



void HDRLighting::RenderBloom( LPDIRECT3DTEXTURE9 hdrBuffer )
{
	HRESULT hr = S_OK;

	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	FLOAT afSampleOffsets[MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

	PDIRECT3DSURFACE9 pSurfScaledHDR;
	m_pScaledColor->GetSurfaceLevel(0, &pSurfScaledHDR);
	
	PDIRECT3DSURFACE9 pSurfBloom;
	m_apBloomTex[0]->GetSurfaceLevel(0, &pSurfBloom);
	
	PDIRECT3DSURFACE9 pSurfHDR;
	hdrBuffer->GetSurfaceLevel(0, &pSurfHDR);

	PDIRECT3DSURFACE9 pSurfTempBloom;
	m_apBloomTex[1]->GetSurfaceLevel(0, &pSurfTempBloom);

	PDIRECT3DSURFACE9 pSurfBloomSource;
	m_apBloomTex[2]->GetSurfaceLevel(0, &pSurfBloomSource);

	// Clear the bloom texture
	RENDERDEVICE::Instance().g_pD3DDevice->ColorFill(pSurfBloom, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));

// 	if (g_GlareDef.m_fGlareLuminance <= 0.0f ||
// 		g_GlareDef.m_fBloomLuminance <= 0.0f)
// 	{
// 		hr = S_OK;
// 		goto LCleanReturn;
// 	}

	RECT rectSrc;
	GetTextureRect(m_pBloomSourceTex, &rectSrc);
	InflateRect(&rectSrc, -1, -1);

	RECT rectDest;
	GetTextureRect(m_apBloomTex[2], &rectDest);
	InflateRect(&rectDest, -1, -1);

	CoordRect coords;
	GetTextureCoords(m_pBloomSourceTex, &rectSrc, m_apBloomTex[2], &rectDest, &coords);

	D3DSURFACE_DESC desc;
	hr = m_pBloomSourceTex->GetLevelDesc(0, &desc);


	m_postEffect->SetTechnique("GaussBlur5x5");

	hr = GetSampleOffsets_GaussBlur5x5(desc.Width, desc.Height, avSampleOffsets, avSampleWeights, 1.0f);

	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
	m_postEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfBloomSource);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pBloomSourceTex);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetScissorRect(&rectDest);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();

	hr = m_apBloomTex[2]->GetLevelDesc(0, &desc);

	hr = GetSampleOffsets_Bloom(desc.Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		avSampleOffsets[i] = D3DXVECTOR2(afSampleOffsets[i], 0.0f);
	}


	m_postEffect->SetTechnique("Bloom");
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
	m_postEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfTempBloom);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_apBloomTex[2]);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetScissorRect(&rectDest);


	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();


	hr = m_apBloomTex[1]->GetLevelDesc(0, &desc);

	hr = GetSampleOffsets_Bloom(desc.Height, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		avSampleOffsets[i] = D3DXVECTOR2(0.0f, afSampleOffsets[i]);
	}


	GetTextureRect(m_apBloomTex[1], &rectSrc);
	InflateRect(&rectSrc, -1, -1);

	GetTextureCoords(m_apBloomTex[1], &rectSrc, m_apBloomTex[0], NULL, &coords);


	m_postEffect->SetTechnique("Bloom");
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
	m_postEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfBloom);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_apBloomTex[1]);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();


	hr = S_OK;

	SafeRelease(pSurfBloomSource);
	SafeRelease(pSurfTempBloom);
	SafeRelease(pSurfBloom);
	SafeRelease(pSurfHDR);
	SafeRelease(pSurfScaledHDR);
}

void HDRLighting::RenderStar()
{
}
