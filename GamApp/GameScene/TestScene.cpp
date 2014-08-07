#include "TestScene.h"
#include "EntityFeature/Entity.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Input/Input.h"

#include "Light/DirectionLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Light/LightManager.h"

TestScene::TestScene()
{
}


TestScene::~TestScene()
{
}


Entity* krisEntity;
Entity* shevaEntity;
Entity* horseEntity;
Material testMat1;
EffectLoader effectLoader;
DirectionLight* dirLight1;
DirectionLight* dirLight2;
PointLight* pointLight1;
PointLight* pointLight2;
PointLight* pointLight3;
PointLight* pointLight4;
PointLight* pointLight5;
SpotLight* spotLight1;
SpotLight* spotLight2;
SpotLight* spotLight3;
SpotLight* spotLight4;
void TestScene::OnLoad()
{
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
	horseEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\horse\\horse.X");

	Entity* roomEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\room\\room.x");
	roomEntity->SetTexture("Res\\Mesh\\room\\wallpaper.dds",0);
	roomEntity->SetTexture("Res\\Mesh\\room\\DoorDiff.dds", 1);
	roomEntity->SetTexture("Res\\Mesh\\room\\oldwood.dds", 2);
	roomEntity->SetTexture("Res\\Mesh\\room\\FireplaceDiff.dds", 3);

	horseEntity->SetTexture("Res\\Mesh\\horse\\HorseB_512.jpg");
	horseEntity->SetNormalMap("Res\\Mesh\\horse\\HorseB _NRM_512.jpg");

	D3DXMATRIX planeM;
	D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&planeM, move.x, move.y, move.z); 
	roomEntity->SetWorldTransform(planeM);

	D3DXMATRIX horseM;
	move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&horseM, move.x, move.y, move.z);
	move = D3DXVECTOR3(0.1f, 0.1f, 0.1f);
	D3DXMATRIX horseS;
	D3DXMatrixScaling(&horseS, move.x, move.y, move.z);
	horseEntity->SetWorldTransform(horseS * horseM);

	effectLoader.LoadFxEffect("System\\BankBRDFLight.fx");
	testMat1.effect = effectLoader.GetEffect();
	krisEntity->SetMaterial(&testMat1);

	//--------------------------------------------------------------------------
	D3DXMATRIX lightMoveMat;
	D3DXMATRIX lightRot1Mat;
	D3DXMATRIX lightRot2Mat;
	dirLight1 = LIGHTMANAGER::Instance().CreateLight<DirectionLight>(eDirectionLight);
	dirLight1->SetLightColor(D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f));
	dirLight1->SetUseShadow(true);

	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 0);
	dirLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	dirLight2 = LIGHTMANAGER::Instance().CreateLight<DirectionLight>(eDirectionLight);
	dirLight2->SetLightColor(D3DXCOLOR(0.0f, 0.0f, 0.3f, 1.0f));
	dirLight2->SetUseShadow(false);
	
	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25f * D3DX_PI);

	dirLight2->SetWorldTransform(lightRot1Mat*lightMoveMat);
	//--------------------------------------------------------------------------
	pointLight1 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight1->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight1->SetUseShadow(true);
	pointLight1->SetLightRange(20);
	
	D3DXMatrixTranslation(&lightMoveMat, -10, -4, 0);
	pointLight1->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight2 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight2->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight2->SetUseShadow(false);
	pointLight2->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 20.0001f, -4, 9);
	pointLight2->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight3 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight3->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight3->SetUseShadow(false);
	pointLight3->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 19.995f, -4, 10);
	pointLight3->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight4 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight4->SetLightColor(D3DXCOLOR(3.7f, 3.7f, 3.7f, 1.0f));
	pointLight4->SetUseShadow(false);
	pointLight4->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 10, -4, 19.998f);
	pointLight4->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight5 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight5->SetLightColor(D3DXCOLOR(4.7f, 0.7f, 4.7f, 1.0f));
	pointLight5->SetUseShadow(false);
	pointLight5->SetLightRange(0.3f);

	D3DXMatrixTranslation(&lightMoveMat, 10.0f, -4, -19.998f);
	pointLight5->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight1 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight1->SetLightRange(20);
	spotLight1->SetLightColor(D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 5, 5, 5);
	spotLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight2 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight2->SetLightAngle(D3DXVECTOR2(60, 30));
	spotLight2->SetLightRange(45);
	spotLight2->SetUseShadow(false);
	spotLight2->SetLightColor(D3DXCOLOR(0.0f, 0.1f, 0.0f, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 2, 10, -5);
	D3DXMatrixRotationX(&lightRot1Mat, -0.4f * D3DX_PI);
	spotLight2->SetWorldTransform(lightRot1Mat * lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight3 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight3->SetLightRange(2);
	spotLight3->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.0, 1.0f));
	spotLight3->SetUseShadow(false);
	spotLight3->SetLightRange(20);
	D3DXMatrixTranslation(&lightMoveMat, -10, 1, 15);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25f * D3DX_PI);
	spotLight3->SetWorldTransform(lightRot1Mat * lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight4 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight4->SetLightRange(20);
	spotLight4->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, -10, 1, -15);
	D3DXMatrixRotationX(&lightRot1Mat, -0.25f * D3DX_PI);
	spotLight4->SetWorldTransform(lightRot1Mat * lightMoveMat);
	
}

void ComputeMove(D3DXVECTOR3& move)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	
	float speed = CameraParam::speed;
	if (GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		speed *= 5;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_W))
	{
		move = D3DXVECTOR3(0, 0, 1) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_S))
	{
		move = D3DXVECTOR3(0, 0, -1) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_A))
	{
		move = D3DXVECTOR3(-1, 0, 0) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_D))
	{
		move = D3DXVECTOR3(1, 0, 0) * speed*(float)dTime;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Q))
	{
		move = D3DXVECTOR3(0, 1, 0) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_E))
	{
		move = D3DXVECTOR3(0, -1, 0) * speed*(float)dTime;
	}
}

void BuildRot(D3DXMATRIX& world)
{
	if (!GAMEINPUT::Instance().m_CurMState.right)
		return;

	D3DXVECTOR3 pos;
	pos.x = world(3, 0);
	pos.y = world(3, 1);
	pos.z = world(3, 2);

	world(3, 0) = 0;
	world(3, 1) = 0;
	world(3, 2) = 0;

	D3DXVECTOR3 right;
	right.x = world(0, 0);
	right.y = world(0, 1);
	right.z = world(0, 2);

	D3DXVECTOR3 up;
	up.x = world(1, 0);
	up.y = world(1, 1);
	up.z = world(1, 2);

	D3DXVECTOR3 front;
	front.x = world(2, 0);
	front.y = world(2, 1);
	front.z = world(2, 2);

	D3DXVec3Normalize(&right, &right);
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	D3DXVECTOR2 mouseMove = GAMEINPUT::Instance().GetMouseMove();
	float speed = 0.2f;
	if (GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		speed *= 5;
	}

	D3DXMATRIX rot1;
	D3DXMATRIX rot2;

	D3DXMatrixRotationAxis(&rot1, &right, mouseMove.y*speed*(float)dTime);
	D3DXMatrixRotationY(&rot2, mouseMove.x*speed*(float)dTime);

	world = world * rot1 * rot2;

	world(3, 0) = pos.x;
	world(3, 1) = pos.y;
	world(3, 2) = pos.z;
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

	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	BuildRot(cW);
	cW = moveMat *cW;
	mainCamera.SetWorldTransform(cW);
	
	move = D3DXVECTOR3(0, -5, -5);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationY(&rotMat, (180+R) / 180.0f * D3DX_PI);
	R+=1.0f;
	rotMat *= moveMat;
	krisEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(0, -4.5, 7);
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
