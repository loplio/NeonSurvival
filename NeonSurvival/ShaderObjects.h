#pragma once
#include "Shader.h"
#include "Player.h"
#include "Scene.h"
#include "Server.h"

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
	virtual void Update(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);
	void AppendBoundingObject(CGameObject* obj);
	std::vector<CGameObject*>& GetBoundingObjects() { return m_BoundingObjects; }
	int IsCollide(CGameObject* obj/*, ObjectType excludetype*/);

	bool m_bCollisionBoxWireFrame = false;
	bool bCreate = false;

protected:
	std::vector<CGameObject*> m_BoundingObjects;

public:
	std::vector<CGameObject*> m_ParentObjects;
	std::vector<UINT> m_nObjects;
	std::vector<UINT> m_StartIndex;
};

//--Concrete_1-------------------------------------------------------------------
class BoundingBoxObjects_1 : public CBoundingBoxObjects {
public:
	BoundingBoxObjects_1(CScene& Scene, CPlayer& Player) : m_Scene(Scene), m_Player(Player) {
	}
	virtual ~BoundingBoxObjects_1() {}

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override {
		CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

		m_Player.CreateBoundingBoxObjectSet(pd3dDevice, pd3dCommandList, this);
		//m_Player.CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, this);
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
/*	CMonsterObjects															   */
//-------------------------------------------------------------------------------
class CMonsterObjects : public CSkinnedAnimationObjectsShader {
public:
	CMonsterObjects();
	virtual ~CMonsterObjects();
	
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual void BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture = NULL) {};
	void Update(float fTimeElapsed) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;
	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandLis) override;

	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void ReleaseShaderVariables() override;

protected:
	ID3D12Resource* m_pd3dcbObjects = NULL;
	HP_INSTANCE* m_pcbMappedGameObjects = NULL;

	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;

public:
	int m_nBuildIndex = 0;
	int m_nMaxObjects = 0;
	int m_nMonsterLoop = 0;
	std::vector<CGameObject*> m_ppObjects;

	CLoadedModelInfo* m_pMonsterModel = NULL;

	CGameObject* m_pHPObject = NULL;
	CMaterial* m_pHPMaterial = NULL;
};

class GeneralMonsterObjects : public CMonsterObjects {
public:
	GeneralMonsterObjects();
	virtual ~GeneralMonsterObjects();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

	void InitShader(CGameObject* pChild);
	void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader) override;
	void BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture = NULL) override;
	
	void CreateMonsters(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nLoop);
	void CreateMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* model, float maxHP, int x, int z, bool bModifyBouondingBox = false, XMFLOAT3 Center = XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3 Extent = XMFLOAT3(0.0f,0.0f,0.0f));

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0) override;
	void Update(float fTimeElapsed) override;
	void Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects) override;
	void AnimateObjects(float fTimeElapsed) override;
	void AppendMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, MonsterObject::MonsterType Type);
	void EventRemove();
	void OnPostReleaseUploadBuffers() override;

	void SetGroundUpdatedContext(std::vector<CGroundObject*>* vGroundObjects) { m_vGroundObjects = vGroundObjects; }
	void OnGroundUpdateCallback(float fTimeElapsed);

public:
	std::vector<CGroundObject*>* m_vGroundObjects;
	PACKET_MONSTERDATA* m_pMonsterData = SERVER::getInstance().GetMonsterData();
	const int nMaxMonster = 30;

	enum {
		IDLE,
		ATTACK,
		MOVE,
		DIE,
		DAMAGED
	};
};

class MonsterMetalonObjects : public CMonsterObjects {
public:
	MonsterMetalonObjects();
	virtual ~MonsterMetalonObjects();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

	void InitShader(CGameObject* pChild);
	void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader) override;
	void BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture = NULL) override;
	void Update(float fTimeElapsed) override;
	void Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects) override;
	void AnimateObjects(float fTimeElapsed) override;
	void AppendMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3&& StartPosition);
	void EventRemove();
	void OnPostReleaseUploadBuffers() override;
	void SetPosition(XMFLOAT3& xmf3Position, int index);
	void SetTransform(int index, XMFLOAT4X4& transform);

	void SetPosition(XMFLOAT3 xmf3Position, int index);

public:
	const int nMaxMetalon = 20;
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

public:
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
	void Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects) override;
	void AppendBullet(XMFLOAT3& startLocation, XMFLOAT3& rayDirection, int type,bool ismine, float fDistanceAtObject = 0.0f);
	void AppendBullet(XMFLOAT3& startLocation, XMFLOAT3& rayDirection, int type, float fDistanceAtObject = 0.0f);
	void EventRemove();
	void ReleaseUploadBuffers() override;
	void OnPostReleaseUploadBuffers() override;
	
	ReafShaderType GetReafShaderType() override { return PistolBulletShader; }

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGPUDescriptorStartHandle;

public:
	const int nMaxBullet = 100;
	const float OffsetLength = METER_PER_PIXEL(0.8);

	float m_fCoolTime = 0.0f;
	float m_fLastTime = 0.2f;
	float m_fMaxCoolTime = 0.2f;

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
struct SCREEN_TEXTURE_INSTANCE {
	float m_fGauge;
	XMFLOAT2 m_fTemp;
};

class CTextureToScreenShader : public CTexturedShader {
public:
	CTextureToScreenShader(wchar_t* texturePath);
	virtual ~CTextureToScreenShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
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

	ReafShaderType GetReafShaderType() override { return TextureToScreenShader; }

protected:
	ID3D12Resource* m_pd3dInstScreenTexture = NULL;
	SCREEN_TEXTURE_INSTANCE* m_pcbMappedInstScreenTexture = NULL;

	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;

	float m_fGauge = 1.0f;

public:
	CTexturedRectMesh* m_RectMesh = NULL;
	CTexture* m_pTexture = NULL;
	wchar_t* pszFileName = NULL;

	void SetGauge(float fGauge) { m_fGauge = fGauge; }
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

	void ChangeTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pTexture);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGPUDescriptorStartHandle;

public:
	CTexture* m_pTexture = NULL;
	bool bFirst = true;
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

	virtual void ChangeTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pTexture, bool bRtvTexture = true, wchar_t* texturePath = NULL);

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dUavGPUDescriptorStartHandle;

	ReafShaderType GetReafShaderType() override { return AddTextureComputeShader; }

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

	virtual void ChangeTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pTexture, bool bRtvTexture = true, wchar_t* texturePath = NULL);

	virtual void ReleaseUploadBuffers();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dUavGPUDescriptorStartHandle;

	ReafShaderType GetReafShaderType() override { return BrightAreaComputeShader; }

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

	virtual void ChangeTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pTexture, bool bRtvTexture = true, wchar_t* texturePath = NULL);

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

	void SetSourceResource(ID3D12Resource* pSourceResource) { m_pSourceResource = pSourceResource; }

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dUavGPUDescriptorStartHandle;

	ReafShaderType GetReafShaderType() override { return Gaussian2DBlurComputeShader; }

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