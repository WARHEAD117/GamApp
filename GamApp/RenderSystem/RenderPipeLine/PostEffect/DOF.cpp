#include "DOF.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

DOF::DOF()
{
}


DOF::~DOF()
{
}

void DOF::CreatePostEffect()
{
	PostEffectBase::CreatePostEffect("System\\DOF.fx");

	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R16F, D3DPOOL_DEFAULT,
		&m_CoCBuffer, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth/2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight/2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_scaledBuffer, NULL);
}


//通光孔直径（单位：m） 光圈值=焦距/通光孔直径
float g_aperture = 0.135f;
//焦距(单位：mm)
float g_focallength = 135;
//聚焦距离（单位：m）
float g_planeinfocus = 10;
//模糊范围缩放系数 分辨率*（聚焦距离-焦距）/（聚焦距离*焦距）/cos（FOV/2） 暂且近似为1000
float g_scale = 1000;

void DOF::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	

	D3DXVECTOR2 avSampleOffsets[16];
	D3DXVECTOR4 avSampleWeights[16];

	D3DSURFACE_DESC desc;
	HRESULT hr = mainBuffer->GetLevelDesc(0, &desc);
	GetSampleOffsets_GaussBlur5x5(desc.Width, desc.Height, avSampleOffsets, avSampleWeights);

	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
	m_postEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));

	PDIRECT3DSURFACE9 pSurfCoC = NULL;
	hr = m_CoCBuffer->GetSurfaceLevel(0, &pSurfCoC);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfCoC);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	if (GAMEINPUT::Instance().KeyDown(DIK_T) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_aperture += 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_focallength += 0.1;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_planeinfocus += 0.01;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_scale += 0.1;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_T) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_aperture -= 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_focallength -= 0.1;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_planeinfocus -= 0.01;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_scale -= 0.1;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		g_aperture = 0.135;
		g_focallength = 135;
		g_planeinfocus = 10;
		g_scale = 1000;
	}
	if (g_planeinfocus < CameraParam::zNear) g_planeinfocus = CameraParam::zNear;
	if (g_planeinfocus > CameraParam::zFar) g_planeinfocus = CameraParam::zFar;

	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);

	m_postEffect->BeginPass(0);

	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	//=============================================================================================================
	GetSampleOffsets_DownScale4x4(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight, avSampleOffsets);
	m_postEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfScaledScene = NULL;
	hr = m_scaledBuffer->GetSurfaceLevel(0, &pSurfScaledScene);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfScaledScene);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->BeginPass(2);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();

	SafeRelease(pSurfScaledScene);
	//=============================================================================================================
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_scaledBuffer", m_scaledBuffer);
	m_postEffect->SetTexture("g_CoCBuffer", m_CoCBuffer);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->SetFloat("g_aperture", g_aperture);
	m_postEffect->SetFloat("g_focallength", g_focallength / 1000.0f);
	m_postEffect->SetFloat("g_planeinfocus", g_planeinfocus);
	m_postEffect->SetFloat("g_scale", g_scale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);

	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();

	m_postEffect->End();
}
