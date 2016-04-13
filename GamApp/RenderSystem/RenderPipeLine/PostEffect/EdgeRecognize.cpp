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

int useWhich = 1;
void EdgeRecognize::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	ConfigInput();

	m_postEffect->SetInt("g_switch", useWhich);

	m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
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
