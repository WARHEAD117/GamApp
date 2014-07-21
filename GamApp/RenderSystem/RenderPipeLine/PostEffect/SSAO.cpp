#include "SSAO.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

SSAO::SSAO()
{
	m_intensity = 2;
	m_scale = 0.5;
	m_bias = 0.2;
	m_sample_rad = 0.03;
	m_rad_scale = 3;
	m_AOEnable = true;
}


SSAO::~SSAO()
{
}

void SSAO::CreatePostEffect()
{
	PostEffectBase::CreatePostEffect("System\\SSAO.fx", D3DFMT_A16B16G16R16F);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth/2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight/2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pAoTarget, NULL);
	HRESULT hr = m_pAoTarget->GetSurfaceLevel(0, &m_pAoSurface);

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\randomNormal.dds", &m_pRandomNormal))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "randomNormal", MB_OK);
		abort();
	}
}


void SSAO::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	if (GAMEINPUT::Instance().KeyPressed(DIK_P))
	{
		m_AOEnable = !m_AOEnable;
	}

	ComputeAOConfig();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);

	
	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	float angle = tan(CameraParam::FOV/2);
	m_postEffect->SetFloat("g_angle", angle);
	float aspect = RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	m_postEffect->SetFloat("g_aspect", aspect);

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
	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pAoSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);
	if (m_AOEnable)
	{
		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(0);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_postEffect->EndPass();
	}

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);

	m_postEffect->SetTexture("g_AoBuffer", m_pAoTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);
	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	m_postEffect->End();
}

void SSAO::ComputeAOConfig()
{
	if (GAMEINPUT::Instance().KeyDown(DIK_T) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_intensity += 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_scale += 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_sample_rad += 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_bias += 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_O) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_rad_scale += 0.001;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_T) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_intensity -= 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_scale -= 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_sample_rad -= 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_bias -= 0.0001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_O) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_rad_scale -= 0.001;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		m_intensity = 2;
		m_scale = 0.5;
		m_bias = 0.2;
		m_sample_rad = 0.03;
		m_rad_scale = 3;
	}
}
