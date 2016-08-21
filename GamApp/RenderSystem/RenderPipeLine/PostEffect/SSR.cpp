#include "SSR.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

SSR::SSR()
{
	m_Roughness = 10;
	m_Thickness = 1.0f;
	m_bias = 0.2f;
	m_sample_rad = 0.03f;
	m_rad_scale = 0.3f;
	m_rad_threshold = 4.0f;
	m_SSREnable = true;
}


SSR::~SSR()
{
}

void SSR::CreatePostEffect()
{
	PostEffectBase::CreatePostEffect("System\\SSR.fx", D3DFMT_A16B16G16R16F);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pSSRTarget, NULL);
	HRESULT hr = m_pSSRTarget->GetSurfaceLevel(0, &m_pSSRSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pBlurXTarget, NULL);
	hr = m_pBlurXTarget->GetSurfaceLevel(0, &m_pBlurXSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pBlurYTarget, NULL);
	hr = m_pBlurYTarget->GetSurfaceLevel(0, &m_pBlurYSurface);

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\noiseColor.png", &m_pRandomNormal))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "randomNormal", MB_OK);
		abort();
	}

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		6, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pMainTargetWithMip, NULL);
}

int sampleCount = 15;
float m_SampleWeights[15];
float m_SampleOffsets[30];

bool SSR::BuildMipMap(const LPDIRECT3DTEXTURE9 src, LPDIRECT3DTEXTURE9 dest)
{
	if (!src || !dest)
		return false;

	int maxlevel = dest->GetLevelCount();

	LPDIRECT3DSURFACE9 srcSurface;
	src->GetSurfaceLevel(0, &srcSurface);
	for (int i = 0; i < maxlevel; i++)
	{
		LPDIRECT3DSURFACE9 destSurface;
		dest->GetSurfaceLevel(i, &destSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->StretchRect(srcSurface, NULL, destSurface, NULL, D3DTEXF_LINEAR);
	}

	return true;
}

void SSR::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	BuildMipMap(mainBuffer, m_pMainTargetWithMip);
	int maxMipLevel = m_pMainTargetWithMip->GetLevelCount();

	ComputeSSRConfig();

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
	m_postEffect->SetFloat("g_ViewAngle_half_tan", angle);
	float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	m_postEffect->SetFloat("g_ViewAspect", aspect);

	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture("g_RandomNormal", m_pRandomNormal);

	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pMainTargetWithMip);
	
	m_postEffect->SetFloat("g_Roughness", m_Roughness);
	m_postEffect->SetInt("g_MaxMipLevel", maxMipLevel);
	m_postEffect->SetFloat("g_Thickness", m_Thickness);

	m_postEffect->SetFloat("g_bias", m_bias);
	m_postEffect->SetFloat("g_sample_rad", m_sample_rad);
	m_postEffect->SetFloat("g_rad_scale", m_rad_scale);
	m_postEffect->SetFloat("g_rad_threshold", m_rad_threshold);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pSSRSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	if (false)
	{
		//================================================================================================================
		//Blur X
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pBlurXSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

		m_postEffect->SetTexture("g_SSRBuffer", m_pSSRTarget);
		m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

		SetGaussian(m_postEffect, 1.0f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, 0, "g_SampleWeights", "g_SampleOffsets");

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(1);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_postEffect->EndPass();

		//================================================================================================================
		//Blur Y
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pSSRSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

		m_postEffect->SetTexture("g_SSRBuffer", m_pBlurXTarget);
		m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);


		SetGaussian(m_postEffect, 0, 1.0f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight, "g_SampleWeights", "g_SampleOffsets");

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(1);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_postEffect->EndPass();


		//================================================================================================================
	}
	
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

	m_postEffect->SetTexture("g_SSRBuffer", m_pSSRTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pMainTargetWithMip);

	SetGaussian(m_postEffect, 1.0f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, 0, "g_SampleWeights", "g_SampleOffsets");
	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(2);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	m_postEffect->End();
}

void SSR::ComputeSSRConfig()
{
	if (GAMEINPUT::Instance().KeyDown(DIK_M) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_Roughness += 0.8f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_N) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_Thickness += 0.01f;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_U) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_sample_rad += 0.0001f;
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

	if (GAMEINPUT::Instance().KeyDown(DIK_M) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_Roughness -= 0.8f;
		m_Roughness = m_Roughness < 0 ? 0 : m_Roughness;

	}
	if (GAMEINPUT::Instance().KeyDown(DIK_N) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_Thickness -= 0.01f;
		m_Thickness = m_Thickness < 0 ? 0 : m_Thickness;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_U) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_sample_rad -= 0.0001f;
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
		m_Roughness = 10.0f;
		m_Thickness = 1.0f;

		m_bias = 0.2f;
		m_sample_rad = 0.03f;
		m_rad_scale = 0.3f;
		m_rad_threshold = 4.0f;
	}
}

void SSR::SetGaussian(LPD3DXEFFECT effect, float deltaX, float deltaY, std::string weightArrayName, std::string offsetArrayName)
{
	//===
	m_SampleWeights[0] = ComputeGaussianWeight(0);
	m_SampleOffsets[0] = 0.0f;
	m_SampleOffsets[1] = 0.0f;

	float totalWeights = m_SampleWeights[0];

	for (int i = 0; i < sampleCount / 2; i++)
	{
		float weight = ComputeGaussianWeight(i + 1);
		m_SampleWeights[i * 2 + 1] = weight;
		m_SampleWeights[i * 2 + 2] = weight;
		totalWeights += weight * 2;

		float sampleOffset = i * 2 + 1.5f;
		D3DXVECTOR2 delta = D3DXVECTOR2(deltaX, deltaY) * sampleOffset;

		m_SampleOffsets[i * 4 + 1] = delta.x;
		m_SampleOffsets[i * 4 + 2] = delta.y;
		m_SampleOffsets[i * 4 + 3] = -delta.x;
		m_SampleOffsets[i * 4 + 4] = -delta.y;
	}

	for (int i = 0; i < sampleCount; i++)
	{
		m_SampleWeights[i] /= totalWeights;
	}

	// �����������ݵ�GaussianBlur��Ч
	effect->SetFloatArray(weightArrayName.c_str(), m_SampleWeights, sampleCount);
	effect->SetFloatArray(offsetArrayName.c_str(), m_SampleOffsets, sampleCount * 2);
	//==
}

float m_BlurAmount = 5.5;
float SSR::ComputeGaussianWeight(float n)
{
	//��˹�������㹫ʽ
	float theta = m_BlurAmount;
	return (float)((1.0 / sqrt(2 * D3DX_PI * theta)) *
		exp(-(n * n) / (2 * theta * theta)));
}