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

	LightType m_LightType;

	void SetLightType(LightType lightType);
	LightType GetLightType();

	D3DXVECTOR3 GetLightWorldDir();
	D3DXVECTOR3 GetLightWorldUp();
	D3DXVECTOR3 GetLightWorldPos();

	D3DXVECTOR3 GetLightViewDir();
	D3DXVECTOR3 GetLightViewUp();
	D3DXVECTOR3 GetLightViewPos();

	D3DXCOLOR GetLightColor();
	void SetLightColor(D3DXCOLOR color);

	void RebuildProjMatrix();
	void RebuildViewMatrix();

	D3DXMATRIX GetLightViewMatrix();
	D3DXMATRIX GetPointLightViewMatrix(int index);
	D3DXMATRIX GetLightProjMatrix();
	D3DXMATRIX GetLightInvProjMatrix();

	void SetUseShadow(bool useShadow);
	bool GetUseShadow();

	void	BuildShadowMap();
	void	SetShadowTarget();
	LPDIRECT3DTEXTURE9	GetShadowTarget();

	void	BuildPointShadowMap();
	void SetPointShadowTarget(int index);
	LPDIRECT3DCUBETEXTURE9 GetPointShadowTarget();

	float GetLightRange();
	void SetLightRange(float range);

	D3DXVECTOR2 GetLightCosHalfAngle();
	D3DXVECTOR2 GetLightAngle(); //Íâ½Ç¡¢ÄÚ½Ç
	void SetLightAngle(D3DXVECTOR2 angle);

	D3DXVECTOR4 GetLightAttenuation();
	void SetLightAttenuation(D3DXVECTOR4 lightAttenuation);

	virtual void OnFrame();

private:
	LPDIRECT3DTEXTURE9 m_pShadowTarget;
	LPDIRECT3DSURFACE9	m_pShadowSurface;
	LPDIRECT3DSURFACE9	m_pDepthStencilShadowSurface;

	LPDIRECT3DCUBETEXTURE9 m_pPointShadowTarget;

	bool		m_bUseShadow;
	D3DXVECTOR3 m_LightPos;
	D3DXVECTOR3 m_LightDir;
	D3DXVECTOR3 m_LightUp;

	D3DXMATRIX m_PointlightViewMat[6];

	D3DXMATRIX m_lightViewMat;
	D3DXMATRIX m_lightProjMat;
	D3DXMATRIX m_lightInvProjMat;

	float		mLightIntensity;
	D3DXCOLOR	m_LightColor;

	float		m_LightRange;
	D3DXVECTOR2		m_LightAngle;

	D3DXVECTOR4	m_LightAttenuation;

	void Init();
};

inline void BaseLight::SetLightType(LightType lightType)
{
	m_LightType = lightType;
}

inline LightType BaseLight::GetLightType()
{
	return m_LightType;
}


inline bool BaseLight::GetUseShadow()
{
	return m_bUseShadow;
}