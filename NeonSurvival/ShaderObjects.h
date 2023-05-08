#pragma once
#include "Shader.h"
#include "Player.h"
#include "Scene.h"

class CGameObject;
class CCamera;

//-------------------------------------------------------------------------------
/*	CBoundingBoxObjects														   */
//-------------------------------------------------------------------------------
class CBoundingBoxObjects : public CBoundingBoxShader {
public:
	CBoundingBoxObjects();
	virtual ~CBoundingBoxObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) = 0;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);
	void AddBBObject(CGameObject* obj) { m_BBObjects.push_back(obj); }
	std::vector<CGameObject*>& GetBBObject() { return m_BBObjects; }
	int IsCollide(CGameObject* obj/*, ObjectType excludetype*/);

	bool m_bCollisionBoxWireFrame = false;

protected:
	std::vector<CGameObject*> m_BBObjects;
};

//--Concrete_1-------------------------------------------------------------------
class BoundingBoxObjects_1 : public CBoundingBoxObjects {
public:
	BoundingBoxObjects_1(CScene& Scene, CPlayer& Player) : m_Scene(Scene), m_Player(Player) {
	}
	virtual ~BoundingBoxObjects_1() {}

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override {
		CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

		m_Player.CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, this);
		m_Scene.CreateBoundingBox(pd3dDevice, pd3dCommandList, this);		// 종료 시 느려지는 현상.
	}

protected:
	CScene& m_Scene;
	CPlayer& m_Player;
};

//-------------------------------------------------------------------------------
/*	CModelObjects															   */
//-------------------------------------------------------------------------------
class CModelObjects : public CStandardShader {
public:
	CModelObjects();
	virtual ~CModelObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) = 0;

	void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader) override;
	void Collide(CBoundingBoxObjects& BoundingBoxObjects);
	void AnimateObjects(float fTimeElapsed) override;
	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;

	CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance) override final;

protected:
	std::vector<CGameObject*> m_ppObjects;
};

//--Concrete_1-------------------------------------------------------------------
class ModelObjects_1 : public CModelObjects {
public:
	ModelObjects_1() {}
	virtual ~ModelObjects_1() {}

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;
};

//-------------------------------------------------------------------------------
/*	CTexturedObjects														   */
//-------------------------------------------------------------------------------
class CTexturedObjects : public CTexturedShader {
public:
	CTexturedObjects();
	virtual ~CTexturedObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);

	void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader) override;
	void AnimateObjects(float fTimeElapsed) override;
	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;

	CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance) override final;

protected:
	std::vector<CGameObject*> m_ppObjects;
};

//--Concrete_1-------------------------------------------------------------------
class TexturedObjects_1 : public CTexturedObjects {
public:
	TexturedObjects_1() {}
	virtual ~TexturedObjects_1() {}

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL) override;
};

//-------------------------------------------------------------------------------
/*	CBulletObjects															   */
//-------------------------------------------------------------------------------
class CBulletObjects : public CBulletShader {
public:
	CBulletObjects();
	virtual ~CBulletObjects();
	virtual void BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture = NULL) {};

	void Update(float fTimeElapsed) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;
	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandLis) override;

	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;

protected:
	int m_nBuildIndex = 0;
	std::list<CGameObject*> m_ppObjects;
	//std::vector<CGameObject*> m_ppObjects;
};

class PistolBulletTexturedObjects : public CBulletObjects {
public:
	PistolBulletTexturedObjects();
	virtual ~PistolBulletTexturedObjects();

	void BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture = NULL) override;
	void Update(float fTimeElapsed) override;
	void AppendBullet(XMFLOAT3& startLocation, XMFLOAT3& rayDirection);
	void EventRemove();
	void ReleaseUploadBuffers() override;
	void OnPostReleaseUploadBuffers() override;
	
	ReafShaderType GetReafShaderType() override { return PistolBulletShader; }

public:
	const int nMaxBullet = 100;

	CMaterial* m_pMaterial = NULL;
};

//-------------------------------------------------------------------------------
/*	CBillboardObjects														   */
//-------------------------------------------------------------------------------
class CBillboardObjects : public CBillboardShader {
public:
	CBillboardObjects();
	virtual ~CBillboardObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL) = 0;

	void AnimateObjects(float fTimeElapsed) override;
	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;

	CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance) override final;

protected:
	std::vector<CGameObject*> m_ppObjects;
};

//--Concrete_1-------------------------------------------------------------------

//-------------------------------------------------------------------------------
/*	CMultiSpriteObjects														   */
//-------------------------------------------------------------------------------
class CMultiSpriteObjects : public CMultiSpriteShader {
public:
	CMultiSpriteObjects();
	virtual ~CMultiSpriteObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL) = 0;

	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void AnimateObjects(float fTimeElapsed) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;

	CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance) override final;

protected:
	std::vector<CGameObject*> m_ppObjects;
	std::vector<CTexture*> m_ppSpriteTextures;
	std::vector<CMaterial*> m_ppSpriteMaterials;
	CTexturedRectMesh* m_pSpriteMesh = NULL;
};

//--Concrete_1-------------------------------------------------------------------

//-------------------------------------------------------------------------------
/*	CBlendTextureObjects													   */
//-------------------------------------------------------------------------------
class CBlendTextureObjects : public CBlendTextureShader {
public:
	CBlendTextureObjects();
	virtual ~CBlendTextureObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL) = 0;

	void AnimateObjects(float fTimeElapsed) override;
	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState) override;

	CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance) override final;

protected:
	std::vector<CGameObject*> m_ppObjects;
};

//--Concrete_1-------------------------------------------------------------------
class BlendTextureObjects_1 : public CBlendTextureObjects {
public:
	BlendTextureObjects_1() {}
	virtual ~BlendTextureObjects_1() {}

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL) override;
	void AnimateObjects(float fTimeElapsed) override;
};

//-------------------------------------------------------------------------------
/*	CTextureToScreenShader												   */
//-------------------------------------------------------------------------------
class CTextureToScreenShader : public CTexturedShader {
public:
	CTextureToScreenShader(wchar_t* texturePath);
	virtual ~CTextureToScreenShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);

	void CreateRectTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition);

public:
	CTexturedRectMesh* m_RectMesh = NULL;
	CTexture* m_pTexture = NULL;
	wchar_t* pszFileName = NULL;
};

//-------------------------------------------------------------------------------
/*	CTextureToFullScreenShader												   */
//-------------------------------------------------------------------------------
class CTextureToFullScreenShader : public CShader {
public:
	CTextureToFullScreenShader(CTexture* pTexture);
	virtual ~CTextureToFullScreenShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);

public:
	CTexture* m_pTexture = NULL;
};

//-------------------------------------------------------------------------------
/*	CAddTexturesComputeShader												   */
//-------------------------------------------------------------------------------
class CAddTexturesComputeShader : public CComputeShader {
public:
	CAddTexturesComputeShader();
	virtual ~CAddTexturesComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader();

	virtual void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

public:
	CTexture* m_pTexture = NULL;
};

//-------------------------------------------------------------------------------
/*	CBrightAreaComputeShader													   */
//-------------------------------------------------------------------------------
class CBrightAreaComputeShader : public CComputeShader {
public:
	CBrightAreaComputeShader();
	CBrightAreaComputeShader(wchar_t* pszFileName);
	virtual ~CBrightAreaComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader();

	virtual void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

public:
	CTexture* m_pTexture = NULL;
	wchar_t* pszFileName = NULL;
};

//-------------------------------------------------------------------------------
/*	CGaussian2DBlurComputeShader											   */
//-------------------------------------------------------------------------------
class CGaussian2DBlurComputeShader : public CComputeShader {
public:
	CGaussian2DBlurComputeShader();
	CGaussian2DBlurComputeShader(wchar_t* pszFileName);
	virtual ~CGaussian2DBlurComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader();

	virtual void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

	void SetSourceResource(ID3D12Resource* pSourceResource) { m_pSourceResource = pSourceResource; }

public:
	CTexture* m_pTexture = NULL;
	wchar_t* pszFileName = NULL;
	ID3D12Resource* m_pSourceResource = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGaussian1DBlurComputeShader : public CComputeShader {
public:
	CGaussian1DBlurComputeShader();
	virtual ~CGaussian1DBlurComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	virtual void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

public:
	CTexture* m_pTexture = NULL;
};