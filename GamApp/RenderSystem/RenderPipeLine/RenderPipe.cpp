#include "RenderPipe.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"

RenderPipe::RenderPipe()
{
}


RenderPipe::~RenderPipe()
{
}

void RenderPipe::RenderAll()
{
	RENDERDEVICE::Instance().g_pD3DDevice->BeginScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(1, 0, 0), 1.0f, 0);

	UpdateRenderState();

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i].Render();
	}

	RENDERDEVICE::Instance().g_pD3DDevice->EndScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Present(0, 0, 0, 0);
}

void RenderPipe::PushRenderUtil(const RenderUtil& renderUtil)
{
	mRenderUtilList.push_back(renderUtil);
}

void RenderPipe::ClearRenderUtil()
{
	mRenderUtilList.clear();
}

void RenderPipe::UpdateRenderState()
{
	if (KEYDOWN(VK_F1))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
	}
	if (KEYDOWN(VK_F2))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	if (KEYDOWN(VK_F3))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	if (KEYDOWN(VK_F4))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	}
	if (KEYDOWN(VK_F5))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	}
	if (KEYDOWN(VK_F6))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
	}
}