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
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_ColorCoCBuffer, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_ColorHorizontal, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_ColorStep1, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_ColorStep2, NULL);
}


//通光孔直径（单位：m） 光圈值=焦距/通光孔直径
float g_aperture = 0.135f;
//焦距(单位：mm)
float g_focallength = 135;
//聚焦距离（单位：m）
float g_planeinfocus = 10;
//模糊范围缩放系数(为什么需要缩放系数？原始论文不带这个系数就行)
float g_scale = 0.04f;

void DOF::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	if (GAMEINPUT::Instance().KeyDown(DIK_T) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_aperture += 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_focallength += 0.01;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_planeinfocus += 0.01;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_scale += 0.001;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_T) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_aperture -= 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Y) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_focallength -= 0.01;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_U) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_planeinfocus -= 0.01;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_scale -= 0.001;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		g_aperture = 0.135;
		g_focallength = 135;
		g_planeinfocus = 1;
		g_scale = 0.04;
	}
	if (g_planeinfocus < CameraParam::zNear) g_planeinfocus = CameraParam::zNear;
	if (g_planeinfocus > CameraParam::zFar) g_planeinfocus = CameraParam::zFar;

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);

	//=============================================================================================================
	//alpha通道保存coc的主纹理
	PDIRECT3DSURFACE9 pSurfCoC = NULL;
	m_ColorCoCBuffer->GetSurfaceLevel(0, &pSurfCoC);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfCoC);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);
	m_postEffect->SetFloat("g_aperture", g_aperture);
	m_postEffect->SetFloat("g_focallength", g_focallength / 1000.0f);
	m_postEffect->SetFloat("g_planeinfocus", g_planeinfocus);
	m_postEffect->SetFloat("g_scale", g_scale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//=============================================================================================================
	//水平模糊
	PDIRECT3DSURFACE9 pSurfColorHorizontal = NULL;
	m_ColorHorizontal->GetSurfaceLevel(0, &pSurfColorHorizontal);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfColorHorizontal);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);
	m_postEffect->SetTexture("g_ColorCoCBuffer", m_ColorCoCBuffer);
	m_postEffect->SetFloat("g_angle", 0);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurfColorHorizontal);
	//=============================================================================================================
	//第一步向左偏移
	PDIRECT3DSURFACE9 pSurfColorStep1 = NULL;
	m_ColorStep1->GetSurfaceLevel(0, &pSurfColorStep1);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfColorStep1);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);
	m_postEffect->SetTexture("g_ColorCoCBuffer", m_ColorHorizontal);
	m_postEffect->SetFloat("g_angle", D3DX_PI / 3.0f);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurfColorStep1);
	//=============================================================================================================
	//第二步向右偏移
	PDIRECT3DSURFACE9 pSurfColorStep2 = NULL;
	m_ColorStep2->GetSurfaceLevel(0, &pSurfColorStep2);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurfColorStep2);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);
	m_postEffect->SetTexture("g_ColorCoCBuffer", m_ColorHorizontal);
	m_postEffect->SetFloat("g_angle", -D3DX_PI / 3.0f);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurfColorStep2);
	//=============================================================================================================
	//合并模糊
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetFloat("g_angle", D3DX_PI / 3.0f);

	m_postEffect->SetTexture("g_ColorStep1", m_ColorStep1);
	m_postEffect->SetTexture("g_ColorStep2", m_ColorStep2);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(2);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	m_postEffect->End();
}
