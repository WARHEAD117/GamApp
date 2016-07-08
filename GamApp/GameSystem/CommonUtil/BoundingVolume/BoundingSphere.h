#pragma once 
#include "CommonUtil\BoundingVolume\BoundingVolume.h"
class BoundingSphere :
	public BoundingVolume
{
public:
	BoundingSphere();
	~BoundingSphere();

	virtual bool IsPointInside(D3DXVECTOR3 pos);
	void SetCenter(D3DXVECTOR3 center);
	void SetRadius(float radius);
	D3DXVECTOR3 GetCenter();
	float GetRadius();
private:
	D3DXVECTOR3	m_Center;
	float		m_Radius;
};

inline D3DXVECTOR3 BoundingSphere::GetCenter()
{
	return m_Center;
}

inline float BoundingSphere::GetRadius()
{
	return m_Radius;
}

inline void BoundingSphere::SetCenter(D3DXVECTOR3 Center)
{
	m_Center = m_Center;
}

inline void BoundingSphere::SetRadius(float radius)
{
	m_Radius = m_Radius;
}