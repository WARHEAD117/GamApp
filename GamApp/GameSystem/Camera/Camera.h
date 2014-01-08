#pragma once
#include "CommonUtil\\D3D9Header.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Init(); //初始化
	void OnFrame(D3DXVECTOR3 pos, D3DXVECTOR3 lookAt, D3DXVECTOR3  right, D3DXVECTOR3 up);

	void BuildViewMtx();
	void BuildProjMtx();

	D3DXMATRIX GetView(){ return mView; };
	D3DXMATRIX GetProj(){ return mProj; };
	D3DXMATRIX GetWorld(){ return mWorld; };

	D3DXVECTOR4 GetPos4() { return D3DXVECTOR4(mPosW, 1.0f); };
	D3DXVECTOR3 GetPos(){ return mPosW; };
	D3DXVECTOR3 GetRight(){ return mRightW; };
	D3DXVECTOR3 GetUp(){ return mUpW; };
	D3DXVECTOR3 GetLookAt(){ return mLookAtW; };
	D3DXVECTOR3 GetLookDir(){ return mLookAtW - mPosW; };

	D3DXMATRIX mView;   //观察矩阵
	D3DXMATRIX mProj;     //投影矩阵
	D3DXMATRIX mWorld;  //世界矩阵

private:
	D3DXVECTOR3 mPosW;
	D3DXVECTOR3 mRightW;
	D3DXVECTOR3 mUpW;
	D3DXVECTOR3 mLookAtW;
	//	D3DXVECTOR3 mLookDirW;
	float mFOV;
};

namespace ns_camera
{
	// camera data, not defined yet
	const float speed = 10.0f;
	const D3DXVECTOR3 posW = D3DXVECTOR3(500.0f, 500.0f, -500.0f);
	const D3DXVECTOR3 rightW = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	const D3DXVECTOR3 upW = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	const D3DXVECTOR3 lookAtW = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	const float FOV = D3DX_PI*0.25f;
};