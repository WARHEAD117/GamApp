#include "BoundingSphere.h"


BoundingSphere::BoundingSphere() : 
m_Center(ZEROVECTOR3),
m_Radius(0.0f)
{
}


BoundingSphere::~BoundingSphere()
{
}

bool BoundingSphere::IsPointInside(D3DXVECTOR3 pos)
{
	D3DXVECTOR3 distance = this->m_Center - pos;
	return pow(distance.x, 2) + pow(distance.y, 2) + pow(distance.z, 2) <= pow(this->m_Radius, 2);
}

