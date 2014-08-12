#pragma once
#include "Light/BaseLight.h"

class PointLight : 
	public BaseLight
{
public:
	PointLight();
	~PointLight();

	//virtual void OnFrame();
	virtual void RenderShadow(const std::vector<RenderUtil*>& mRenderUtilList);

	virtual void RenderLightVolume();
	virtual void BuildLightVolume();

	virtual void RebuildProjMatrix();
	virtual void RebuildViewMatrix();

	virtual D3DXMATRIX GetLightVolumeTransform();
	//获取用来计算相机到灯光体顶点的向量的矩阵
	virtual D3DXMATRIX GetToViewDirMatrix();

	virtual void SetUseShadow(bool useShadow);

	virtual void BuildShadowMap();

	virtual LPDIRECT3DBASETEXTURE9	GetShadowTarget();

	void	SetPointShadowTarget(int index);
	D3DXMATRIX GetPointLightViewMatrix(int index);

protected:
	LPDIRECT3DCUBETEXTURE9 m_pPointShadowTarget;
	D3DXMATRIX m_PointlightViewMat[6];
};
