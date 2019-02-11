#include "Camera.h"
#include "D3D9Device.h"
#include "Camera/CameraParam.h"

Camera::Camera(void)
{
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWorld);

	mPosW = CameraParam::posW;
	mRightW = CameraParam::rightW;
	mUpW = CameraParam::upW;
	mLookAtW = CameraParam::lookAtW;

	mPosW = D3DXVECTOR3(3.0f, 5, 33); //A-12, 0, 32.5f//B3.0f, 5, 33
	mRightW  = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	mUpW = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	mLookAtW = D3DXVECTOR3(-1.0f, 5.0f, 33.0f);//A-13.0f, 0.4f, 33.0f//B-1.0f, 5.0f, 33.0f

	mFOV = CameraParam::FOV;
}
Camera::~Camera(void)
{

}

void Camera::Init()
{
	D3DXMatrixTranslation(&mWorld, mPosW.x, mPosW.y, mPosW.z);
	BuildViewMtx();
	BuildProjMtx();
}

void Camera::OnFrame()
{


	RENDERDEVICE::Instance().LastViewMatrix = RENDERDEVICE::Instance().ViewMatrix;

	D3DXMatrixInverse(&mView, NULL, &mWorldTransform);
	RENDERDEVICE::Instance().ViewMatrix = mView;

	RENDERDEVICE::Instance().InvViewMatrix = mWorldTransform;

	RENDERDEVICE::Instance().ViewPosition = D3DXVECTOR3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43);
}

void Camera::BuildViewMtx()
{
// 	D3DXVECTOR3 mLookDirW = GetLookDir();
// 
// 	// Keep camera's axes orthogonal to each other and of unit length.
// 	D3DXVec3Normalize(&mLookDirW, &mLookDirW);
// 
// 	D3DXVec3Cross(&mUpW, &mLookDirW, &mRightW);
// 	D3DXVec3Normalize(&mUpW, &mUpW);
// 
// 	D3DXVec3Cross(&mRightW, &mUpW, &mLookDirW);
// 	D3DXVec3Normalize(&mRightW, &mRightW);
// 
// 	// Fill in the view matrix entries.
// 
// 	float x = -D3DXVec3Dot(&mPosW, &mRightW);
// 	float y = -D3DXVec3Dot(&mPosW, &mUpW);
// 	float z = -D3DXVec3Dot(&mPosW, &mLookDirW);
// 
// 	mView(0, 0) = mRightW.x;
// 	mView(1, 0) = mRightW.y;
// 	mView(2, 0) = mRightW.z;
// 	mView(3, 0) = x;
// 
// 	mView(0, 1) = mUpW.x;
// 	mView(1, 1) = mUpW.y;
// 	mView(2, 1) = mUpW.z;
// 	mView(3, 1) = y;
// 
// 	mView(0, 2) = mLookDirW.x;
// 	mView(1, 2) = mLookDirW.y;
// 	mView(2, 2) = mLookDirW.z;
// 	mView(3, 2) = z;
// 
// 	mView(0, 3) = 0.0f;
// 	mView(1, 3) = 0.0f;
// 	mView(2, 3) = 0.0f;
// 	mView(3, 3) = 1.0f;

	// 	D3DXMATRIX   matView;

	RENDERDEVICE::Instance().LastViewMatrix = RENDERDEVICE::Instance().ViewMatrix;

	D3DXMatrixLookAtLH(&mView, &mPosW,
		&mLookAtW,
		&mUpW);
	RENDERDEVICE::Instance().ViewMatrix = mView;
	D3DXMatrixInverse(&mWorldTransform, NULL, &mView);

	D3DXMATRIX invView;
	D3DXMatrixInverse(&invView, NULL, &mView);
	RENDERDEVICE::Instance().InvViewMatrix = invView;
}

void Camera::BuildProjMtx()
{

	float w = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	float h = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;

	D3DXMatrixPerspectiveFovLH(&mProj, mFOV, w / h, CameraParam::zNear, CameraParam::zFar);

	RENDERDEVICE::Instance().ProjMatrix = mProj;
	D3DXMatrixInverse(&RENDERDEVICE::Instance().InvProjMatrix, NULL, &mProj);

	D3DXMATRIX orthoWVP;
	D3DXMatrixIdentity(&orthoWVP);
	D3DXMatrixTranslation(&orthoWVP, 0, 0, 0);
	RENDERDEVICE::Instance().OrthoWVPMatrix = orthoWVP;
}
