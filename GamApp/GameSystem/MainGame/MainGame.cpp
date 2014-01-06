#include "MainGame.h"
#include "CommonUtil\Timer\GlobalTimer.h"
#include "RenderSystem\D3D9Device.h"

#include "CommonUtil\MeshLoader\MeshLoader.h"

MainGame::MainGame()
{
}


MainGame::~MainGame()
{
	if (g_DrawSkyBox)
		Delete(g_DrawSkyBox);
}

//Just for test-------------------------------------------------------------------
LPD3DXMESH mesh;
LPDIRECT3DTEXTURE9			planeTex;
MeshLoader loader;
//--------------------------------------------------------------------------------

void MainGame::GameLoad()
{
	//Just for test---------------------------------------------------------------------------------
	D3DXCreateSphere(RENDERDEVICE::Instance().g_pD3DDevice, 0.5, 32, 32, &mesh, NULL);
	D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "grass.jpg", &planeTex);
	//--初始化天空盒及其贴图
	g_DrawSkyBox = new CDrawSkyBox(RENDERDEVICE::Instance().g_pD3DDevice);

	g_DrawSkyBox->InitVB();
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\bottom.jpg", 0);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\left.jpg", 1);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\right.jpg", 2);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\top.jpg", 3);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\front.jpg", 5);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\back.jpg", 4);

	loader.LoadXMesh("Res\\Mesh\\tree3\\tree3.X");
	//===================================================================================================
}

void MainGame::GameLoop()
{
	GLOBALTIMER::Instance().StartFrame();
	Run();
	GLOBALTIMER::Instance().EndFrame();

}

void MainGame::Run()
{
	OnBeginFrame();
	OnFrame();
	Render();
	OnEndFrame();
}

void MainGame::OnBeginFrame()
{
	m_EntityManager.OnBeginFrame();
}

void MainGame::OnFrame()
{
	m_EntityManager.OnFrame();

	mCamera.OnFrame(ns_camera::posW, ns_camera::lookAtW, ns_camera::rightW, ns_camera::upW);
}

void MainGame::Render()
{
	RENDERDEVICE::Instance().g_pD3DDevice->BeginScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(1, 0, 0), 1.0f, 0);

	RENDERDEVICE::Instance().g_pD3DDevice->SetTexture(0, planeTex);

	g_DrawSkyBox->Render(D3DXVECTOR3(0, 0, 1));
	D3DXMATRIX  matWorld;
	//矩阵单位化
	D3DXMatrixIdentity(&matWorld);
	RENDERDEVICE::Instance().g_pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
	mesh->DrawSubset(0);
	RENDERDEVICE::Instance().g_pD3DDevice->EndScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Present(0, 0, 0, 0);
}
	
void MainGame::OnEndFrame()
{
	m_EntityManager.OnEndFrame();
}

void MainGame::GameStart()
{
	GLOBALTIMER::Instance().StartGameTimer();
}

void MainGame::GameEnd()
{
}
