#include "NPR.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

NPR::NPR()
{
}


NPR::~NPR()
{
}

void NPR::RenderPost(LPDIRECT3DTEXTURE9 edgeTarget, LPDIRECT3DTEXTURE9 mainBuffer)
{
	m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	m_postEffect->SetTexture("g_edgeBuffer", edgeTarget);
	//m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	PostEffectBase::RenderPost(mainBuffer);
}