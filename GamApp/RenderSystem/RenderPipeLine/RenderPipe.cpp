#include "RenderPipe.h"
#include "D3D9Device.h"

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
