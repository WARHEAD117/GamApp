#pragma once
#include "CommonUtil\\D3D9Header.h"

class BoundingVolume
{
public:
	BoundingVolume();
	~BoundingVolume();
	virtual bool IsPointInside(D3DXVECTOR3 pos) = 0;
};

