#include "TestScene.h"
#include "EntityFeature/Entity.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Input/Input.h"

#include "Light/DirectionLight.h"
#include "Light/LightManager.h"

TestScene::TestScene()
{
}


TestScene::~TestScene()
{
	if (g_DrawSkyBox)
		Delete(g_DrawSkyBox);
}


Entity* krisEntity;
Entity* shevaEntity;
Material testMat1;
EffectLoader effectLoader;
BaseLight* dirLight1;
BaseLight* dirLight2;
BaseLight* pointLight1;

BaseLight* spotLight1;
BaseLight* spotLight2;
BaseLight* spotLight3;
BaseLight* spotLight4;
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
	//krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Cube.X");
	krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\kris.X");
	//shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\teapot.X");
	shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\sheva.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\car\\car25.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");

	Entity* planeEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\room.x");
	D3DXMATRIX planeM;
	D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&planeM, move.x, move.y, move.z); 
	planeEntity->SetWorldTransform(planeM);

	effectLoader.LoadFxEffect("System\\BankBRDFLight.fx");
	testMat1.effect = effectLoader.GetEffect();
	krisEntity->SetMaterial(&testMat1);

	//--------------------------------------------------------------------------
	D3DXMATRIX lightMoveMat;
	D3DXMATRIX lightRot1Mat;
	D3DXMATRIX lightRot2Mat;
	dirLight1 = LIGHTMANAGER::Instance().CreateLight(eDirectionLight);
	dirLight1->SetLightColor(D3DXCOLOR(0.3, 0.3, 0.3, 1.0f));
	dirLight1->SetUseShadow(true);

	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 0);
	dirLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	dirLight2 = LIGHTMANAGER::Instance().CreateLight(eDirectionLight);
	dirLight2->SetLightColor(D3DXCOLOR(0.0, 0.0, 0.3, 1.0f));
	dirLight2->SetUseShadow(false);
	
	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25 * D3DX_PI);

	dirLight2->SetWorldTransform(lightRot1Mat*lightMoveMat);
	//--------------------------------------------------------------------------
	pointLight1 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	pointLight1->SetLightColor(D3DXCOLOR(5.7,5.7,5.7, 1.0f));
	pointLight1->SetUseShadow(true);
	pointLight1->SetLightRange(3);
	
	D3DXMatrixTranslation(&lightMoveMat, 0, -4, 0);
	pointLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight1 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight1->SetLightRange(30);
	spotLight1->SetLightColor(D3DXCOLOR(1.0, 0.0, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 5, 5, 5);
	spotLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight2 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight2->SetLightRange(30);
	spotLight2->SetLightColor(D3DXCOLOR(0.0, 1.0, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 5, 5, -5);
	spotLight2->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight3 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight3->SetLightRange(30);
	spotLight3->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.0, 1.0f));
	spotLight3->SetUseShadow(false);
	spotLight3->SetLightRange(100);
	D3DXMatrixTranslation(&lightMoveMat, -10, 10, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25f * D3DX_PI);
	spotLight3->SetWorldTransform(lightMoveMat*lightRot1Mat);
	//--------------------------------------------------------------------------
	spotLight4 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight4->SetLightRange(30);
	spotLight4->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, -10, 10, -10);
	D3DXMatrixRotationX(&lightRot1Mat, -0.25f * D3DX_PI);
	spotLight4->SetWorldTransform(lightMoveMat*lightRot1Mat);
	
}

void ComputeMove(D3DXVECTOR3& move)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	
	float speed = CameraParam::speed;
	if (KEYDOWN(VK_SHIFT))
	{
		speed *= 5;
	}

	if (KEYDOWN('W'))
	{
		move = D3DXVECTOR3(0, 0, 1) * speed*(float)dTime;
	}

	if (KEYDOWN('S'))
	{
		move = D3DXVECTOR3(0, 0, -1) * speed*(float)dTime;
	}

	if (KEYDOWN('A'))
	{
		move = D3DXVECTOR3(-1, 0, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('D'))
	{
		move = D3DXVECTOR3(1, 0, 0) * speed*(float)dTime;
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

void ComputeRotate(D3DXMATRIX& rot)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();

	float speed = 0.1;
	if (KEYDOWN(VK_SHIFT))
	{
		speed *= 5;
	}

	if (KEYDOWN('X'))
	{
		D3DXMatrixRotationY(&rot, speed*(float)dTime);
	}

	if (KEYDOWN('Z'))
	{
		D3DXMatrixRotationY(&rot, -speed*(float)dTime);
	}
}

float R = 0;
void TestScene::OnBeginFrame()
{
	D3DXMATRIX cW = mainCamera.GetWorldTransform();
	D3DXMATRIX moveMat;

	D3DXVECTOR3 move(0, 0, 0);
	ComputeMove(move);

	D3DXMATRIX rot;
	D3DXMatrixIdentity(&rot);
	ComputeRotate(rot);

	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	cW = moveMat *cW *  rot;
	mainCamera.SetWorldTransform(cW);
	
	move = D3DXVECTOR3(0, -5, -5);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationY(&rotMat, (180+R) / 180.0f * D3DX_PI);
	R+=1.0f;
	rotMat *= moveMat;
	krisEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(0, -5, 5);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	rotMat;
	D3DXMatrixIdentity(&rotMat);
	D3DXMatrixRotationY(&rotMat, -0.5f * D3DX_PI);
	rotMat *= moveMat;
	shevaEntity->SetWorldTransform(rotMat);
}

void TestScene::OnFrame()
{
	mainCamera.OnFrame();
}

void TestScene::OnEndFrame()
{

}
