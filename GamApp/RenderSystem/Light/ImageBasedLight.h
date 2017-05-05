#pragma once
#include "Light/BaseLight.h"
 
class ImageBasedLight :
	public BaseLight
{
public:
	ImageBasedLight();
	~ImageBasedLight();

	//virtual void OnFrame();

	virtual void RenderLightVolume();
	virtual void BuildLightVolume();

	virtual void RebuildProjMatrix();
	virtual void RebuildViewMatrix();

	virtual D3DXMATRIX GetLightVolumeTransform();
	//获取用来计算相机到灯光体顶点的向量的矩阵
	virtual D3DXMATRIX GetToViewDirMatrix();

	void SetLightProbe(const std::string probeTex, bool useCube = false);
	virtual void SetLightEffect(ID3DXEffect* pEffect);

private:
	bool m_useCubemap;
	D3DXVECTOR2 m_shadowAreaSize;

	LPDIRECT3DTEXTURE9	m_pLightProbeTex; 
	LPDIRECT3DCUBETEXTURE9	m_pLightProbeCube;

};
