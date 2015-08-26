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
Entity* horse2Entity;
Entity* bunnyEntity;
Entity* deerEntity;
Entity* deerEntity2;

Entity* planeEntity;

Entity* sponzaEntity;
Material testMat1;
EffectLoader effectLoader;
DirectionLight* dirLight1;
DirectionLight* dirLight2;
PointLight* pointLight0;
PointLight* pointLight1;
PointLight* pointLight2;
PointLight* pointLight3;
PointLight* pointLight4;
PointLight* pointLight5;
SpotLight* spotLight1;
SpotLight* spotLight2;
SpotLight* spotLight3;
SpotLight* spotLight4;

SpotLight* spotLight5;
void TestScene::OnLoad()
{
	//===================================================================================================
	//BuildCamera
	mainCamera.Init();

	//Create Entity
	//krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Cube.X");
	krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\kris.X");
	{
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\378DAAED_SPEC.png", 0);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\3D9BBDC1_SPEC.png", 1);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\4240D889_SPEC.png", 2);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\292B0322_SPEC.png", 7);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\47732EA1_SPEC.png", 11);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\F0F64CA9_SPEC.png", 13);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\ED3E42C6_SPEC.png", 14);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\566EC93F_SPEC.png", 16);
	}
	//shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\teapot.X");
	shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\sheva.X");
	{
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\0A370D7F_SPEC.png", 0);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\174D3E25_SPEC.png", 1);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\E2813884_SPEC.png", 2);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\D10453BE_SPEC.png", 3);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\5120C7F9_SPEC.png", 4);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\BF40F1B9_SPEC.png", 6);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\6F98F51D_SPEC.png", 10);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\168C52A9_SPEC.png", 12);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\6D714412_SPEC.png", 14);
	}
	//testEntity.SetMeshFileName("Res\\Mesh\\car\\car25.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");

	horseEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\horse\\horse.X");
	horseEntity->SetTexture("Res\\Mesh\\horse\\HorseB_512.jpg");
	//horseEntity->SetNormalMap("Res\\Mesh\\horse\\HorseB _NRM_512.jpg");

	deerEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\deer\\deer.X");
	deerEntity2 = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\deer\\deer.X");

	planeEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\plane2_diff2.X");

	bunnyEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\bunny.X");

	horse2Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\horse2.X");

	if (false)
	{
		sponzaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\sponza\\sponza.X");

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_floor_a_spec.tga", 0);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\spnza_bricks_a_ddn.tga", 1);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\spnza_bricks_a_spec.tga", 1);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\lion_ddn.tga", 3);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\background_ddn.tga", 4);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\vase_ddn.tga", 5);

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\vase_plant_spec.tga", 6);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\vase_round_ddn.tga", 7);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\vase_round_spec.tga", 7);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\vase_round_ddn.tga", 8);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\vase_round_spec.tga", 8);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\chain_texture_ddn.tga", 9);

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_curtain_spec.tga", 10);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_curtain_spec.tga", 11);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_curtain_spec.tga", 12);

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_flagpole_spec.tga", 13);

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_fabric_spec.tga", 14);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_fabric_spec.tga", 15);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_fabric_spec.tga", 16);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_thorn_ddn.tga", 17);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_thorn_spec.tga", 17);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_column_b_ddn.tga", 18);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_column_b_spec.tga", 18);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_arch_ddn.tga", 19);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_arch_spec.tga", 19);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_column_a_ddn.tga", 20);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_column_a_spec.tga", 20);

		sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_column_c_ddn.tga", 21);
		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_column_c_spec.tga", 21);

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_details_spec.tga", 22);

		sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_ceiling_a_spec.tga", 23);
	}

	//Entity* roomEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\room\\room.x");
	//roomEntity->SetTexture("Res\\Mesh\\room\\wallpaper.dds",0);
	//roomEntity->SetTexture("Res\\Mesh\\room\\DoorDiff.dds", 1);
	//roomEntity->SetTexture("Res\\Mesh\\room\\oldwood.dds", 2);
	//roomEntity->SetTexture("Res\\Mesh\\room\\FireplaceDiff.dds", 3);

	

	D3DXMATRIX roomM;
	D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&roomM, move.x, move.y, move.z);
	//roomEntity->SetWorldTransform(planeM);

	D3DXMATRIX horseM;
	move = D3DXVECTOR3(-12, 0, 1.5f);
	D3DXMatrixTranslation(&horseM, move.x, move.y, move.z);
	move = D3DXVECTOR3(0.1f, 0.1f, 0.1f);
	D3DXMATRIX horseS;
	D3DXMatrixScaling(&horseS, move.x, move.y, move.z);

	D3DXMATRIX horseRotMat;
	D3DXMatrixRotationY(&horseRotMat, -0.225f * D3DX_PI);
	horseEntity->SetWorldTransform(horseRotMat * horseS * horseM);

	D3DXMATRIX bunnyM;
	move = D3DXVECTOR3(-12, 0, 22.5f);
	D3DXMatrixTranslation(&bunnyM, move.x, move.y, move.z);
	move = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	D3DXMATRIX bunnyS;
	D3DXMatrixScaling(&bunnyS, move.x, move.y, move.z);

	D3DXMATRIX bunnyRotMat;
	D3DXMatrixRotationY(&bunnyRotMat, -0.5f * D3DX_PI);
	bunnyEntity->SetWorldTransform(bunnyRotMat * bunnyS * bunnyM);

	D3DXMATRIX deerM;
	D3DXVECTOR3 deerMove = D3DXVECTOR3(-12, 0, 32.5f);
	D3DXMatrixTranslation(&deerM, deerMove.x, deerMove.y, deerMove.z);
	D3DXMATRIX deerS;
	D3DXVECTOR3 deerSV = D3DXVECTOR3(0.1, 0.1, 0.1);
	D3DXMatrixScaling(&deerS, deerSV.x, deerSV.y, deerSV.z);
	deerEntity->SetWorldTransform(deerS * deerM);

	D3DXMATRIX deer2M;
	D3DXVECTOR3 deer2Move = D3DXVECTOR3(-12, 0, 42.5f);
	D3DXMatrixTranslation(&deer2M, deer2Move.x, deer2Move.y, deer2Move.z);
	D3DXMATRIX deer2S;
	D3DXVECTOR3 deer2SV = D3DXVECTOR3(0.1, 0.1, 0.1);
	D3DXMatrixScaling(&deer2S, deer2SV.x, deer2SV.y, deer2SV.z);
	deerEntity2->SetWorldTransform(deer2S * deer2M);

	D3DXMATRIX planeM;
	D3DXVECTOR3 planeMove = D3DXVECTOR3(-30, -0.5f, 0);
	D3DXMatrixTranslation(&planeM, planeMove.x, planeMove.y, planeMove.z);
	D3DXMATRIX planeS;
	D3DXVECTOR3 planeSV = D3DXVECTOR3(1, 1, 1);
	D3DXMatrixScaling(&planeS, planeSV.x, planeSV.y, planeSV.z);
	D3DXMATRIX planeRotMat;
	D3DXMatrixRotationY(&planeRotMat, -0.5f * D3DX_PI);
	planeEntity->SetWorldTransform(planeRotMat * planeS * planeM);

	D3DXMATRIX horse2M;
	move = D3DXVECTOR3(-12, 25, -20.5f);
	D3DXMatrixTranslation(&horse2M, move.x, move.y, move.z);
	move = D3DXVECTOR3(0.01f, 0.01f, 0.01f);
	D3DXMATRIX horse2S;
	D3DXMatrixScaling(&horse2S, move.x, move.y, move.z);

	D3DXMATRIX horse2RotMat;
	D3DXMatrixRotationY(&horse2RotMat, -0.225f * D3DX_PI);
	horse2Entity->SetWorldTransform(horse2RotMat * horse2S * horse2M);

	effectLoader.LoadFxEffect("System\\BankBRDFLight.fx");
	testMat1.effect = effectLoader.GetEffect();
	krisEntity->SetMaterial(&testMat1);

	//--------------------------------------------------------------------------
	D3DXMATRIX lightMoveMat;
	D3DXMATRIX lightRot1Mat;
	D3DXMATRIX lightRot2Mat;
	dirLight1 = LIGHTMANAGER::Instance().CreateLight<DirectionLight>(eDirectionLight);
	dirLight1->SetLightColor(D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f));
	dirLight1->SetShadowAreaSize(100, 100);
	dirLight1->SetUseShadow(false);
	D3DXMatrixTranslation(&lightMoveMat, 0, 50, 0);
	dirLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	dirLight2 = LIGHTMANAGER::Instance().CreateLight<DirectionLight>(eDirectionLight);
	//dirLight2->SetLightColor(D3DXCOLOR(1.0f, 1.0f, 1.3f, 1.0f));
	dirLight2->SetLightColor(D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f));
	dirLight2->SetUseShadow(false);
	dirLight2->SetShadowAreaSize(150,150);
	D3DXMatrixTranslation(&lightMoveMat, 0, 60, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.125f * D3DX_PI);
	D3DXMatrixRotationY(&lightRot2Mat, 0.125f * D3DX_PI);
	dirLight2->SetWorldTransform(lightRot1Mat*lightRot2Mat*lightMoveMat);
	//--------------------------------------------------------------------------
	pointLight0 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight0->SetLightColor(D3DXCOLOR(0.01f, 0.01f, 0.01f, 1.0f));
	pointLight0->SetUseShadow(false);
	pointLight0->SetLightRange(100);
	//dirLight3->SetLightAttenuation(D3DXVECTOR4(0,0,0,0));
	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 0);
	D3DXMatrixRotationX(&lightRot1Mat, 0.0f * D3DX_PI);

	pointLight0->SetWorldTransform(lightRot1Mat*lightMoveMat);
	//--------------------------------------------------------------------------
	pointLight1 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	//pointLight1->SetLightColor(D3DXCOLOR(01.7f, 01.7f, 01.7f, 1.0f));
	pointLight1->SetLightColor(D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f));
	pointLight1->SetUseShadow(false);
	pointLight1->SetLightRange(40);
	
	D3DXMatrixTranslation(&lightMoveMat, -5, 3, 0);
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
	pointLight4->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight4->SetUseShadow(false);
	pointLight4->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 10, -4, 19.998f);
	pointLight4->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight5 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight5->SetLightColor(D3DXCOLOR(4.7f, 0.7f, 4.7f, 1.0f));
	pointLight5->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight5->SetUseShadow(false);
	pointLight5->SetLightRange(0.3f);

	D3DXMatrixTranslation(&lightMoveMat, 10.0f, -4, -19.998f);
	pointLight5->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight1 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight1->SetLightRange(20);
	spotLight1->SetLightColor(D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
	spotLight1->SetLightColor(D3DXCOLOR(0.1f, 0.0f, 0.0f, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 5, 5, 5);
	//spotLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight2 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight2->SetLightAngle(D3DXVECTOR2(60, 30));
	spotLight2->SetLightRange(45);
	spotLight2->SetUseShadow(false);
	spotLight2->SetLightColor(D3DXCOLOR(0.0f, 0.1f, 0.0f, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 2, 10, -5);
	D3DXMatrixRotationX(&lightRot1Mat, -0.4f * D3DX_PI);
	//spotLight2->SetWorldTransform(lightRot1Mat * lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight3 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight3->SetLightRange(2);
	spotLight3->SetLightColor(D3DXCOLOR(1.5, 1.5, 0.0, 1.0f));
	spotLight3->SetLightColor(D3DXCOLOR(0.5, 0.5, 0.0, 1.0f));
	spotLight3->SetUseShadow(false);
	spotLight3->SetLightRange(20);
	D3DXMatrixTranslation(&lightMoveMat, -10, 4, 5);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25f * D3DX_PI);
	//spotLight3->SetWorldTransform(lightRot1Mat * lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight4 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight4->SetLightRange(20);
	spotLight4->SetLightColor(D3DXCOLOR(0.0, 1.5, 1.5, 1.0f));
	spotLight4->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.5, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, -10, 4, -5);
	D3DXMatrixRotationX(&lightRot1Mat, -0.25f * D3DX_PI);
	//spotLight4->SetWorldTransform(lightRot1Mat * lightMoveMat);

	spotLight5 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight5->SetLightRange(100);
	spotLight5->SetUseShadow(false);
	spotLight5->SetLightColor(D3DXCOLOR(1.0, 0.2, 0.0, 1.0f));
	spotLight5->SetLightColor(D3DXCOLOR(0.1, 0.1, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 0, 50, 0);
	D3DXMatrixRotationX(&lightRot1Mat, -0.0f * D3DX_PI);
	//spotLight5->SetWorldTransform(lightRot1Mat * lightMoveMat);

	
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
float R2 = 0;
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
	
	move = D3DXVECTOR3(10, -5, -5);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationY(&rotMat, (180+R) / 180.0f * D3DX_PI);
	R+=1.0f;
	rotMat *= moveMat;
	krisEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(10, -4.5, 7);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	rotMat;
	D3DXMatrixIdentity(&rotMat);
	D3DXMatrixRotationY(&rotMat, -0.5f * D3DX_PI);
	rotMat *= moveMat;
	shevaEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(-12, 0, 42.5f);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMatrixRotationY(&rotMat, (180 + R) / 180.0f * D3DX_PI);
	D3DXMATRIX deer2S;
	D3DXVECTOR3 deer2SV = D3DXVECTOR3(0.1, 0.1, 0.1);
	D3DXMatrixScaling(&deer2S, deer2SV.x, deer2SV.y, deer2SV.z);
	R2 += 1.0f;
	rotMat *= moveMat;
	deerEntity2->SetWorldTransform(deer2S * rotMat);
}

void TestScene::OnFrame()
{
	mainCamera.OnFrame();
}

void TestScene::OnEndFrame()
{

}
