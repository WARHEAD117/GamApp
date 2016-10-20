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

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\maskline.PNG", &m_pMask))
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
	PostEffectBase::RenderPost(mainBuffer);
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
