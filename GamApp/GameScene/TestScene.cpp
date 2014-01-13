#include "TestScene.h"
#include "EntityFeature/Entity.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Input/Input.h"

TestScene::TestScene()
{
}


TestScene::~TestScene()
{
	if (g_DrawSkyBox)
		Delete(g_DrawSkyBox);
}


Entity testEntity;

void TestScene::OnLoad()
{
	//
	//Just for test---------------------------------------------------------------------------------
	//--初始化天空盒及其贴图
	g_DrawSkyBox = new CDrawSkyBox(RENDERDEVICE::Instance().g_pD3DDevice);

	g_DrawSkyBox->InitVB();
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\bottom.jpg", 0);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\left.jpg", 1);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\right.jpg", 2);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\top.jpg", 3);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\front.jpg", 5);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\back.jpg", 4);


	//===================================================================================================
	//BuildCamera
	mainCamera.Init();

	//Create Entity
	testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");
	testEntity.BuildRenderUtil();

	ENTITYMANAGER::Instance().AddEntity(testEntity);
}

void TestScene::OnBeginFrame()
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();

	D3DXMATRIX cW = mainCamera.GetWorldTransform();
	D3DXMATRIX moveMat;

	D3DXVECTOR3 move(0, 0, 0);
	if (KEYDOWN('W'))
	{
		move = D3DXVECTOR3(-1, 0, 0) * CameraParam::speed*(float)dTime;
	}

	if (KEYDOWN('S'))
	{
		move = D3DXVECTOR3(1, 0, 0) * CameraParam::speed*(float)dTime;
	}

	if (KEYDOWN('A'))
	{
		move = D3DXVECTOR3(0, 0, -1) * CameraParam::speed*(float)dTime;
	}

	if (KEYDOWN('D'))
	{
		move = D3DXVECTOR3(0, 0, 1) * CameraParam::speed*(float)dTime;
	}
	if (KEYDOWN('Q'))
	{
		move = D3DXVECTOR3(0, 1, 0) * CameraParam::speed*(float)dTime;
	}

	if (KEYDOWN('E'))
	{
		move = D3DXVECTOR3(0, -1, 0) * CameraParam::speed*(float)dTime;
	}
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	cW = cW * moveMat;
	mainCamera.SetWorldTransform(cW);
	mainCamera.OnFrame();
}

void TestScene::OnFrame()
{

}

void TestScene::OnEndFrame()
{

}
