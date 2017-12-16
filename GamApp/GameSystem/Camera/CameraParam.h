#include "CommonUtil/D3D9Header.h"

namespace CameraParam
{
	// camera data, not defined yet
	const float speed = 5.0f;
	const D3DXVECTOR3 posW = D3DXVECTOR3(-12, 10, 32.5f); //-12, 0, 32.5f//3.0f, 5, 33
	const D3DXVECTOR3 rightW = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	const D3DXVECTOR3 upW = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	const D3DXVECTOR3 lookAtW = D3DXVECTOR3(-13.0f, 10.0f, 33.0f);

	const float FOV = D3DX_PI*0.25f;

	const float zNear = 0.1f;
	const float zFar = 1000;
};