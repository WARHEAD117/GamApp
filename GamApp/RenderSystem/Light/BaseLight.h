#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"



#include "Transform/Transform.h"

class RenderUtil;

enum LightType
{
	eDirectionLight,
	ePointLight,
	eSpotLight
};
class BaseLight :
	public Transform
{
#define LIGHTVOLUME_FVF (D3DFVF_XYZ)
protected:
	struct LIGHTVOLUMEVERTEX
	{
		D3DXVECTOR3		position;
	};
	LPDIRECT3DVERTEXBUFFER9		m_pBufferVex;
	LPDIRECT3DINDEXBUFFER9		m_pBufferIndex;

	int m_SegmentCount;

public:
	BaseLight();
	~BaseLight();

	LightType m_LightType;

	virtual void RenderShadow(const std::vector<RenderUtil*>& mRenderUtilList);

	virtual void RenderLightVolume();
	virtual void BuildLightVolume();

	virtual void RebuildProjMatrix();
	virtual void RebuildViewMatrix();

	virtual D3DXMATRIX GetLightVolumeTransform();
	//获取用来计算相机到灯光体顶点的向量的矩阵
	virtual D3DXMATRIX GetToViewDirMatrix();

	virtual void SetUseShadow(bool useShadow);
	bool GetUseShadow();

	virtual void BuildShadowMap();
	void	SetShadowTarget();

	virtual LPDIRECT3DBASETEXTURE9	GetShadowTarget();

	D3DXMATRIX GetLightViewMatrix();
	
	D3DXMATRIX GetLightProjMatrix();
	D3DXMATRIX GetLightInvProjMatrix();

	

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


	float GetLightRange();
	void SetLightRange(float range);

	D3DXVECTOR2 GetLightCosHalfAngle();
	D3DXVECTOR2 GetLightAngle(); //外角、内角
	//两个分量分别是用角度表示的外角和内角
	void SetLightAngle(D3DXVECTOR2 angle);

	D3DXVECTOR4 GetLightAttenuation();
	void SetLightAttenuation(D3DXVECTOR4 lightAttenuation);

	int GetShadowMapSize();
	void SetShadowMapSize(int size);

	virtual void OnFrame();

protected:
	
	LPD3DXMESH			m_lightVolume;
	

	bool		m_bUseShadow;
	int			m_ShadowMapSize;

	LPDIRECT3DTEXTURE9 m_pShadowTarget;
	LPDIRECT3DSURFACE9	m_pShadowSurface;
	LPDIRECT3DSURFACE9	m_pDepthStencilShadowSurface;
	
	D3DXVECTOR3 m_LightPos;
	D3DXVECTOR3 m_LightDir;
	D3DXVECTOR3 m_LightUp;
	

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