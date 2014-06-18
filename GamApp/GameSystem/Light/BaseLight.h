#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"

#include "Transform/Transform.h"

enum LightType
{
	eDirectionLight,
	ePointLight,
	eSpotLight
};
class BaseLight :
	public Transform
{
public:
	BaseLight();
	~BaseLight();

	LightType mLightType;

	void SetLightType(LightType lightType);
	LightType GetLightType();

	D3DXVECTOR3 GetLightDir();
	D3DXVECTOR3 GetLightUp();
	D3DXVECTOR3 GetLightPos();

	D3DXCOLOR GetLightColor();
	void SetLightColor(D3DXCOLOR color);

	D3DXMATRIX GetLightViewMatrix();
	D3DXMATRIX GetLightProjMatrix();
	D3DXMATRIX GetLightInvProjMatrix();

	void SetUseShadow(bool useShadow);
	bool GetUseShadow();

	void	BuildShadowMap();
	void	SetShadowTarget();
	LPDIRECT3DTEXTURE9	GetShadowTarget();
	LPDIRECT3DTEXTURE9 m_pShadowTarget;
	LPDIRECT3DSURFACE9	m_pShadowSurface;
	LPDIRECT3DSURFACE9	m_pDepthStencilShadowSurface;

	bool		mUseShadow;
	D3DXVECTOR3 m_LightPos;
	D3DXVECTOR3 m_LightDir;
	D3DXVECTOR3 m_LightUp;

	D3DXMATRIX m_lightViewMat;
	D3DXMATRIX m_lightProjMat;
	D3DXMATRIX m_lightInvProjMat;

	float		mLightIntensity;
	D3DXCOLOR	m_LightColor;

	virtual void OnFrame();
};

inline void BaseLight::SetLightType(LightType lightType)
{
	mLightType = lightType;
}

inline LightType BaseLight::GetLightType()
{
	return mLightType;
}


inline bool BaseLight::GetUseShadow()
{
	return mUseShadow;
}