#pragma once
#include "CommonUtil\BoundingVolume\BoundingVolume.h"

class BoundingBox :
	public BoundingVolume
{
public:
	BoundingBox();
	~BoundingBox();

	virtual bool IsPointInside(D3DXVECTOR3 pos);
};

