#include "EdgeRecognize.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

EdgeRecognize::EdgeRecognize()
{
}


EdgeRecognize::~EdgeRecognize()
{
}

void EdgeRecognize::CreatePostEffect(std::string effectName)
{
	PostEffectBase::CreatePostEffect(effectName, D3DFMT_X8R8G8B8);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
		&m_pEdgeTarget, NULL);
	HRESULT hr = m_pEdgeTarget->GetSurfaceLevel(0, &m_pEdgeSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
		&m_pEdge2Target, NULL);
	hr = m_pEdge2Target->GetSurfaceLevel(0, &m_pEdge2Surface);

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\maskline_bold_wave3.PNG", &m_pMask))//maskline //maskline_wave //maskline_wave2 // maskline_bold //maskline_bold_wave //maskline_bold_wave2
	{
		MessageBox(GetForegroundWindow(), "TextureError", "m_pMask", MB_OK);
		abort();
	}
}

int useWhich = 1;
void EdgeRecognize::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	ConfigInput();

	m_postEffect->SetInt("g_switch", useWhich);

	m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture("g_lineMask", m_pMask);
	m_postEffect->SetTexture("g_UvTex", RENDERPIPE::Instance().m_pGrayscaleTarget);

	//---------------------------------------------------

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

	m_postEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	float angle = tan(CameraParam::FOV / 2);
	m_postEffect->SetFloat("g_ViewAngle_half_tan", angle);
	float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	m_postEffect->SetFloat("g_ViewAspect", aspect);


	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	if (mainBuffer != NULL)
		m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);
	else
		m_postEffect->SetTexture(MAINCOLORBUFFER, RENDERPIPE::Instance().m_pMainColorTarget);
	m_postEffect->SetTexture(DIFFUSEBUFFER, RENDERPIPE::Instance().m_pDiffuseTarget);


	//----------------------------------------------------------------------------
	UINT numPasses = 0;
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetTexture("g_EdgeMap", m_pEdgeTarget);
	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();

	m_postEffect->End();

	//PostEffectBase::RenderPost(mainBuffer);
}

void EdgeRecognize::ConfigInput()
{
	if (GAMEINPUT::Instance().KeyDown(DIK_O) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		useWhich = 1;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_L) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		useWhich = 2;
	}
}
