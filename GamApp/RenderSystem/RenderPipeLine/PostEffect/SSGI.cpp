#include "SSGI.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

SSGI::SSGI()
{
	m_intensity = 2;
	m_scale = 0.5f;
	m_bias = 0.2f;
	m_sample_rad = 1.53f;
	m_rad_scale = 0.3f;
	m_rad_threshold = 4.0f;
}


SSGI::~SSGI()
{
}

void SSGI::CreatePostEffect()
{
	PostEffectBase::CreatePostEffect("System\\SSGI.fx", D3DFMT_A16B16G16R16F);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pGiTarget, NULL);
	HRESULT hr = m_pGiTarget->GetSurfaceLevel(0, &m_pGiSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 4, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 4,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pGiSourceTarget, NULL);
	hr = m_pGiSourceTarget->GetSurfaceLevel(0, &m_pGiSourceSurface);
	
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 4, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 4,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pGiSourceBlurTarget, NULL);
	hr = m_pGiSourceBlurTarget->GetSurfaceLevel(0, &m_pGiSourceBlurSurface);

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\123.png", &m_pRandomNormal))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "randomNormal", MB_OK);
		abort();
	}
}


void SSGI::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	// 	if (GAMEINPUT::Instance().KeyPressed(DIK_1))
	// 	{
	// 		m_AOEnable = !m_AOEnable;
	// 	}

	ComputeGIConfig();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);


	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	float angle = tan(CameraParam::FOV / 2);
	m_postEffect->SetFloat("g_ViewAngle_half_tan", angle);
	float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	m_postEffect->SetFloat("g_ViewAspect", aspect);

	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture("g_RandomNormal", m_pRandomNormal);

	

	m_postEffect->SetFloat("g_intensity", m_intensity);
	m_postEffect->SetFloat("g_scale", m_scale);
	m_postEffect->SetFloat("g_bias", m_bias);
	m_postEffect->SetFloat("g_sample_rad", m_sample_rad);
	m_postEffect->SetFloat("g_rad_scale", m_rad_scale);
	m_postEffect->SetFloat("g_rad_threshold", m_rad_threshold);
	m_postEffect->CommitChanges();

	//-------------------------------------------------------------------------
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pGiSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//-------------------------------------------------------------------------
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

	m_postEffect->SetTexture("g_AoBuffer", m_pGiTarget);
	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	m_postEffect->End();
}

void SSGI::ComputeGIConfig()
{
	if (GAMEINPUT::Instance().KeyDown(DIK_T) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_intensity += 0.0001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_scale += 0.0001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_sample_rad += 0.01f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_bias += 0.0001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_O) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_rad_scale += 0.0001f;
	}
	//if (GAMEINPUT::Instance().KeyDown(DIK_P) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	//{
	//	m_rad_threshold += 0.0001f;
	//}

	if (GAMEINPUT::Instance().KeyDown(DIK_T) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_intensity -= 0.0001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_scale -= 0.0001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_sample_rad -= 0.01f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_bias -= 0.0001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_O) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_rad_scale -= 0.0001f;
	}
	//if (GAMEINPUT::Instance().KeyDown(DIK_P) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	//{
	//	m_rad_threshold -= 0.0001f;
	//}

	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		m_intensity = 2;
		m_scale = 0.5f;
		m_bias = 0.2f;
		m_sample_rad = 1.03f;
		m_rad_scale = 0.3f;
		m_rad_threshold = 4.0f;
	}
}
