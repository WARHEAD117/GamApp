#include "TakarazukaScene.h"
#include "Light/LightManager.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "CommonUtil/Input/Input.h"

TakarazukaScene::TakarazukaScene()
{
}


TakarazukaScene::~TakarazukaScene()
{
}

void TakarazukaScene::OnLoad()
{
	mNoteManager.InitNote();

	//LIGHTMANAGER::Instance().AddLight(&mDirLight);

	//BuildCamera
	mainCamera.Init();
}

void ComputeMoveTest(D3DXVECTOR3& move)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();

	float speed = CameraParam::speed;
	if (KEYDOWN(VK_SHIFT))
	{
		speed *= 5;
	}

	if (KEYDOWN('W'))
	{
		move = D3DXVECTOR3(-1, 0, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('S'))
	{
		move = D3DXVECTOR3(1, 0, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('A'))
	{
		move = D3DXVECTOR3(0, 0, -1) * speed*(float)dTime;
	}

	if (KEYDOWN('D'))
	{
		move = D3DXVECTOR3(0, 0, 1) * speed*(float)dTime;
	}
	if (KEYDOWN('Q'))
	{
		move = D3DXVECTOR3(0, 1, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('E'))
	{
		move = D3DXVECTOR3(0, -1, 0) * speed*(float)dTime;
	}
}

void TakarazukaScene::OnBeginFrame()
{
	D3DXMATRIX cW = mainCamera.GetWorldTransform();
	D3DXMATRIX moveMat;

	D3DXVECTOR3 move(0, 0, 0);
	ComputeMoveTest(move);

	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	cW = cW * moveMat;
	mainCamera.SetWorldTransform(cW);
}

void TakarazukaScene::OnFrame()
{
	mainCamera.OnFrame();
}

void TakarazukaScene::OnEndFrame()
{

}