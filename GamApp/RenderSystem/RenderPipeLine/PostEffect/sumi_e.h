#pragma once
#include "PostEffectBase.h"
class SumiE :
	public PostEffectBase
{
public:
	SumiE();
	~SumiE();

	void RenderPost(LPDIRECT3DTEXTURE9 mainBuffer);
	void CreatePostEffect();
	
	void SetEdgeImage(LPDIRECT3DTEXTURE9 edgeImage);
	float ComputeGaussianWeight(float n);
	void SetGaussian(LPD3DXEFFECT effect, float deltaX, float deltaY, std::string weightArrayName, std::string offsetArrayName);

	bool SaveViewToFile(std::string strFileName, LPDIRECT3DTEXTURE9 target);
private:
	LPDIRECT3DVERTEXDECLARATION9	mParticleDecl;
	LPDIRECT3DVERTEXBUFFER9			mParticleBuffer;

	LPDIRECT3DVERTEXBUFFER9			mParticleBuffer2;

	LPDIRECT3DTEXTURE9	m_ColorCoCBuffer;
	LPDIRECT3DTEXTURE9	m_ColorHorizontal;
	LPDIRECT3DTEXTURE9	m_ColorStep1;
	LPDIRECT3DTEXTURE9	m_ColorStep2;

	LPDIRECT3DTEXTURE9	m_pEdgeBlur;
	LPDIRECT3DTEXTURE9	m_pEdgeBlur2;
	LPDIRECT3DTEXTURE9	m_pEdgeImage;

	LPDIRECT3DTEXTURE9	m_pJudgeImage;

	LPDIRECT3DTEXTURE9	m_pInkTex1;
	LPDIRECT3DTEXTURE9	m_pInkTex2;
	LPDIRECT3DTEXTURE9	m_pInkTex3;

	LPDIRECT3DTEXTURE9	m_pInkMask;

	LPDIRECT3DTEXTURE9	m_StrokesArea;
	LPDIRECT3DTEXTURE9	m_StrokesArea2;

	LPDIRECT3DTEXTURE9 m_pInsideTarget;
	LPDIRECT3DTEXTURE9 m_pInsideTarget2;


	LPD3DXEFFECT		m_SynthesisEffect;

	LPDIRECT3DTEXTURE9 m_pDarkPart;
	LPDIRECT3DTEXTURE9 m_pHorizontalBlur;
	LPDIRECT3DTEXTURE9 m_pVerticalBlur;

	LPDIRECT3DTEXTURE9 m_pBluredInside;
	LPDIRECT3DTEXTURE9 m_pBackground;
	LPDIRECT3DTEXTURE9 m_pRandomTex;

	int texCount = 5;
	LPDIRECT3DTEXTURE9 m_pTexList[5];
	PDIRECT3DSURFACE9 m_pTexSurfList[5];

	LPDIRECT3DTEXTURE9 m_pFootprintTarget;
	LPDIRECT3DTEXTURE9 m_pFootprintTarget_LF;
	LPDIRECT3DTEXTURE9 m_pDiffusionTarget;
	LPDIRECT3DTEXTURE9 m_pOUTTarget;

};

