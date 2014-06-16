#include "SSAO.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

SSAO::SSAO()
{
	m_intensity = 2;
	m_scale = 1;
	m_bias = 0.1;
	m_sample_rad = 0.1;
	m_AOEnable = true;
	m_BottonDown = false;
}


SSAO::~SSAO()
{
}

void SSAO::RenderPost()
{
	if (KEYDOWN('P'))
	{
		m_BottonDown = true;
	}
	if (m_BottonDown == true && KEYUP('P'))
	{
		m_BottonDown = false;
		m_AOEnable = !m_AOEnable;
	}

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);

	if (!m_AOEnable)
		return;

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	m_postEffect->SetTexture(NORMALDEPTHBUFFER, RENDERPIPE::Instance().m_pNormalDepthTarget);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);

	m_postEffect->SetTexture("g_RandomNormal", m_pRandomNormal);

	if (KEYDOWN('T'))
	{
		m_intensity += 0.0001;
	}
	if (KEYDOWN('Y'))
	{
		m_scale += 0.0001;
	}
	if (KEYDOWN('U'))
	{
		m_sample_rad += 0.0001;
	}
	if (KEYDOWN('I'))
	{
		m_bias += 0.0001;
	}

	if (KEYDOWN('G'))
	{
		m_intensity -= 0.0001;
	}
	if (KEYDOWN('H'))
	{
		m_scale -= 0.0001;
	}
	if (KEYDOWN('J'))
	{
		m_sample_rad -= 0.0001;
	}
	if (KEYDOWN('K'))
	{
		m_bias -= 0.0001;
	}

	if (KEYDOWN('R'))
	{
		m_intensity = 1;
		m_scale = 1;
		m_bias = 0.3;
		m_sample_rad = 0.03;
	}
	m_postEffect->SetFloat("g_intensity", m_intensity);
	m_postEffect->SetFloat("g_scale", m_scale);
	m_postEffect->SetFloat("g_bias", m_bias);
	m_postEffect->SetFloat("g_sample_rad", m_sample_rad);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();
}

void SSAO::CreatePostEffect()
{
	PostEffectBase::CreatePostEffect("System\\SSAO.fx");

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\randomNormal.dds", &m_pRandomNormal))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "randomNormal", MB_OK);
		abort();
	}
}
