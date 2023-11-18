#pragma once
#include "Mesh.h"

class CGameSource;
class CBoundingBoxObjects;
class CShader;
class CCamera;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct CALLBACKKEY
{
	float  							m_fTime = 0.0f;
	void*							m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition) { }
};

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
	~CAnimationSet();

public:
	char							m_pstrAnimationSetName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int								m_nKeyFrames = 0;
	float*							m_pfKeyFrameTimes = NULL;
	XMFLOAT4X4**					m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float* m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float* m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float* m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif

	float 							m_fPosition = 0.0f;
	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY*					m_pCallbackKeys = NULL;

	CAnimationCallbackHandler*		m_pAnimationCallbackHandler = NULL;

public:
	void SetPosition(float fTrackPosition);

	XMFLOAT4X4 GetSRT(int nBone);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler);

	void HandleCallback();
};

class CAnimationSets
{
private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CAnimationSets(int nAnimationSets);
	~CAnimationSets();

public:
	int								m_nAnimationSets = 0;
	CAnimationSet**					m_pAnimationSets = NULL;

	int								m_nAnimatedBoneFrames = 0;
	CGameObject**					m_ppAnimatedBoneFrameCaches = NULL; //[m_nAnimatedBoneFrames]

public:
	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler* pCallbackHandler);
};

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack() { }

public:
	BOOL 							m_bEnable = true;
	BOOL							m_bHandOverPosition = false;
	float 							m_fSpeed = 1.0f;
	float 							m_fPosition = 0.0f;
	float 							m_fWeight = 1.0f;

	int 							m_nAnimationSet = 0;

public:
	void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }
	void SetPosition(float fPosition) { m_fPosition = fPosition; }
	void SetHandOverPosition(bool bEnable) { m_bHandOverPosition = bEnable; }
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

	CGameObject* m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationSets* m_pAnimationSets = NULL;

public:
	void PrepareSkinning();
};

class CAnimationController
{
public:
	CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CAnimationController();

public:
	int								m_nCurrentTrack = 0;
	float 							m_fTime = 0.0f;

	int 							m_nAnimationTracks = 0;
	CAnimationTrack*				m_pAnimationTracks = NULL;

	CAnimationSets*					m_pAnimationSets = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh**					m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	ID3D12Resource**				m_ppd3dcbSkinningBoneTransforms = NULL; //[SkinnedMeshes]
	XMFLOAT4X4**					m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

public:
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetOneOfTrackEnable(int nAnimationTrack);
	void SetHandOverPosition(int nAnimationTrack, bool bEnable);
	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler* pCallbackHandler);

	void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);

	enum {
		IDLE,
		WALK,
		BACKWARD_WALK,
		RUN,
		FIRE,
		LEFT_BACKWARD,
		RIGHT_BACKWARD,
		LEFT_FORWARD,
		RIGHT_FORWARD,
		LEFT_WALK,
		RIGHT_WALK
	};
	int m_nAnimationBundle[11]{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	void SetAnimationBundle(UINT n);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RESOURCE_TEXTURE1D			0x01
#define RESOURCE_TEXTURE2D			0x02
#define RESOURCE_TEXTURE2D_ARRAY	0x03	//[]
#define RESOURCE_TEXTURE2DARRAY		0x04
#define RESOURCE_TEXTURE_CUBE		0x05
#define RESOURCE_BUFFER				0x06
#define RESOURCE_STRUCTURED_BUFFER	0x07

class CGameObject;

struct GRAPHICS_SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

struct COMPUTE_SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

struct COMPUTE_UAVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGpuDescriptorHandle;
};

class CTexture {
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters, int nGraphicsSrvGpuHandles = 0, int nComputeSrvGpuHandles = 0, int nComputeUavGpuHandles = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;
	DXGI_FORMAT						m_dxgiBufferFormat = DXGI_FORMAT_UNKNOWN;
	int								m_nBufferElement = 0;
	int								m_nBufferStride = 0;

	int								m_nTextures = 0;
	ID3D12Resource**				m_ppd3dTextures = NULL;
	ID3D12Resource**				m_ppd3dTextureUploadBuffers;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE*	m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	GRAPHICS_SRVROOTARGUMENTINFO*			m_pGraphicsSrvRootArgumentInfos = NULL;
	COMPUTE_SRVROOTARGUMENTINFO*			m_pComputeSrvRootArgumentInfos = NULL;
	COMPUTE_UAVROOTARGUMENTINFO*			m_pComputeUavRootArgumentInfos = NULL;
	int										m_nGraphicsSrvGpuHandles = 0;
	int										m_nComputeSrvGpuHandles = 0;
	int										m_nComputeUavGpuHandles = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetGraphicsSrvRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetComputeSrvRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetComputeUavRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3duavGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateGraphicsSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex);
	void UpdateComputeSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex);
	void UpdateComputeUavShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex);
	void UpdateGraphicsShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateComputeShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex, bool bIsDDSFile = true, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	void CreateBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT dxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, UINT nIndex);
	void CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nDepthOrArraySize, UINT nMipLevels, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, DXGI_FORMAT dxgiFormat, UINT nIndex);

	int GetTextures() const { return(m_nTextures); }
	int GetBufferElement() const { return m_nBufferElement; }
	int GetBufferStride() const { return m_nBufferStride; }
	ID3D12Resource* GetTexture(int nIndex) { return m_ppd3dTextures[nIndex]; }
	UINT GetTextureType() const { return m_nTextureType; }
	DXGI_FORMAT GetBufferFormat() const { return m_dxgiBufferFormat; }

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

struct CB_TEXTURE_UV_INFO {
	XMFLOAT4X4 m_xmf4x4Texture;
};

class CMaterial {
public:
	CMaterial(int nTextures = 1);
	virtual ~CMaterial();

private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetReflection(UINT nReflection) { m_nReflection = nReflection; }
	void SetShader(CShader* pShader);
	void SetTexture(CTexture* pTexture, UINT nTexture = 0);
	void SetMaterialType(UINT nType) { m_nType |= nType; }

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();
	void ReleaseUploadBuffers();

public:
	CShader*						m_pShader = NULL;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	UINT							m_nReflection = 0;
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

public:
	int 							m_nTextures = 0;
	_TCHAR							(*m_ppstrTextureNames)[64] = NULL;
	CTexture**						m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader);

public:
	static CShader*					m_pStandardShader;
	static CShader*					m_pSkinnedAnimationShader;

	static void PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CGameObject {
public:
	CGameObject(int nMaterials = 1);
	CGameObject(const CGameObject& pChild);
	virtual ~CGameObject();

private:
	int m_nReferences = 0;

public:
	void AddRef();
	void Release();
	
	bool IsVisible(CCamera* pCamera = NULL);
	bool IsCollide(BoundingOrientedBox& box);
	bool IsCrosshair();
	void SetUseTransform(bool bUse) { m_NotUseTransform = !bUse; }
	bool bNotUseTransform() const { return m_NotUseTransform; }

protected:
	std::vector<CBoundingBoxMesh*> m_ppBoundingMeshes;
	XMFLOAT3					m_xmf3BoundingScale;
	XMFLOAT3					m_xmf3BoundingLocation;
	float						m_nBoundingCylinderRadius;
	bool						m_IsBoundingCylinder;
	bool						m_IsExistBoundingBox;
	bool						m_IsCrosshair;
	bool						m_NotUseTransform;

public:
	char						m_pstrFrameName[64];

	XMFLOAT4X4					m_xmf4x4World;
	XMFLOAT4X4					m_xmf4x4Transform;
	XMFLOAT3					m_xmf3Scale;
	XMFLOAT3					m_xmf3PrevScale;
	float						m_Mass;

	CGameObject*				m_pParent = NULL;
	CGameObject*				m_pChild = NULL;
	CGameObject*				m_pSibling = NULL;

	int							m_nMaterials;
	CMaterial**					m_ppMaterials;
	CMesh*						m_pMesh;
	CBoundingBoxMesh*			m_pTopBoundingMesh = NULL;

	enum Mobility				{ Static, Moveable };
	UINT						m_Mobility = Static;
public:
	// ShaderVariable.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);
	void SetShader(int nMaterial, CShader* pShader);
	void SetMaterial(int nMaterial, CMaterial* pMaterial);
	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	// processcompute..
	virtual bool Collide(FXMVECTOR Origin, FXMVECTOR Direction, float& Dist);
	virtual bool Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects);
	virtual bool Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects, UINT& nConflicted) { return false; };
	virtual void OnPrepareAnimate();
	virtual void Update(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	// processoutput..
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView);

	virtual XMFLOAT3* GetDisplacement();

	enum ReafObjectType {
		Object,
		Player,
		SkyBox
	};
	virtual ReafObjectType GetReafObjectType() { return Object; }

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	CGameObject* GetRootParentObject();

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	void UpdateMobility(Mobility mobility);
	void SetPrevScale(XMFLOAT4X4* pxmf4x4Parent = NULL);
	
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	
	void Rotate(XMFLOAT4* pxmf4Quaternion);
	void Rotate(XMFLOAT3* pxmfAxis, float fAngle);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void SetScale(float width = 1.f, float height = 1.f, float depth = 1.f);
	void SetScale(const XMFLOAT3& scale);
	void SetMass(float mass);
	void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3&& xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetTransform(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& look, const XMFLOAT3& pos);

	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance);

public:
	std::vector<CBoundingBoxMesh*>& GetMesh() { return m_ppBoundingMeshes; }
	void CreateBoundingBoxMeshSet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader);
	void CreateBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader);
	void CreateBoundingBoxInstSet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pGameObject, LPVOID BBShader);
	void CreateBoundingBoxInst(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pGameObject, LPVOID BBShader);
	void CreateBoundingBoxObjectSet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader);
	void CreateBoundingBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader);
	void SetWorldTransformBoundingBox();
	void UpdateWorldTransformBoundingBox();
	void SetBoundingBox(XMFLOAT3& Center, XMFLOAT3& Extent);
	void SetBoundingScale(XMFLOAT3& BoundingScale);
	void SetBoundingScale(XMFLOAT3&& BoundingScale);
	void SetBoundingLocation(XMFLOAT3& BoundingLocation);
	void SetBoundingLocation(XMFLOAT3&& BoundingLocation);
	XMFLOAT3& GetBoundingScale() { return m_xmf3BoundingScale; }
	XMFLOAT3 GetBoundingScale() const { return m_xmf3BoundingScale; }
	XMFLOAT3& GetBoundingLocation() { return m_xmf3BoundingLocation; }
	XMFLOAT3 GetBoundingLocation() const { return m_xmf3BoundingLocation; }
	bool GetIsExistBoundingBox() const { return m_IsExistBoundingBox; }
	void SetIsExistBoundingBox(bool bIsExist) { m_IsExistBoundingBox = bIsExist; }
	void SetIsBoundingCylinder(bool bIsCylinder, float fRadius = 0.0f);
	bool BeginOverlapBoundingBox(const BoundingOrientedBox& OtherOBB, XMFLOAT3* displacement);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }
	CGameObject* GetTopParent();
	CGameObject* GetParent() { return(m_pParent); }
	CGameObject* FindFrame(const char* pstrFrameName);

	CTexture* FindReplicatedTexture(_TCHAR* pstrTextureName);

public:
	CAnimationController* m_pSkinnedAnimationController = NULL;

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	CSkinnedMesh* FindSkinnedMesh(char* pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);
	static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);

	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);
	static std::string m_pstrTextureFilePath;
};
//-------------------------------------------------------------------------------
/*	Object Type																   */
//-------------------------------------------------------------------------------
class StaticObject : public CGameObject {
public:
	StaticObject();
	StaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~StaticObject();
};

class DynamicObject : public CGameObject {
public:
	DynamicObject();
	DynamicObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~DynamicObject();
};

class MonsterObject : public CGameObject {
public:
	MonsterObject();
	MonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~MonsterObject();
	void UpdaetHP();
	void Conflicted(float damage);

public:
	enum MonsterState{IDLE,ATTACK,MOVE,DIE,TAKEDAMAGE};
	enum MonsterType{Dragon, Giant_Bee, Golem, KingCobra, TreasureChest, Spider, Bat, Magma, Treant, Wolf};
	int State = IDLE;
	float HP = 100.0f;
	float MAXHP = 100.0f;
	int Type;
	bool bActivate = true;
	
	CGameObject* m_pHPObject = NULL;
	CMaterial* m_pHPMaterial = NULL;
};

class CRectTextureObject : public StaticObject {
public:
	CRectTextureObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);
	virtual ~CRectTextureObject();

	void Update(float fTimeElapsed) override;
};

class CHPBarTextureObject : public CRectTextureObject {
public:
	CHPBarTextureObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial, float HP, float MAXHP);
	virtual ~CHPBarTextureObject();

	float HP;
	float MAXHP;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CParticleObject : public DynamicObject {
public:
	CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~CParticleObject();

	CTexture*					m_pRandowmValueTexture = NULL;
	CTexture*					m_pRandowmValueOnSphereTexture = NULL;

	void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void OnPostRender();
	virtual void PostRender(ID3D12GraphicsCommandList* pd3dCommandList);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBoundingBox : public CGameObject {
public:
	CBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBoundingBox();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pszFileName);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	ReafObjectType GetReafObjectType() override { return SkyBox; }
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeightMapTerrain : public CGameObject {
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, wchar_t* baseTexture, wchar_t* detailTexture, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage* m_pHeightMapImage;

	int m_nWidth;
	int m_nLength;

public:
	float GetHeight(float x, float z) { return m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y; }
	XMFLOAT3 GetIntervalNormal(float x, float z) { return m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)); }
	XMFLOAT3 GetNormal(float x, float z) { return m_pHeightMapImage->GetHeightMapNormal(int(x), int(z)); }

	int GetHeightMapWidth() { return m_pHeightMapImage->GetRawImageWidth(); }
	int GetHeightMapLength() { return m_pHeightMapImage->GetRawImageLength(); }

	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	float GetWidth() { return m_nWidth * m_xmf3Scale.x; }
	float GetLength() { return m_nLength * m_xmf3Scale.z; }
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct fRect {
	float left;
	float top;
	float right;
	float bottom;
};
class CGroundObject : public StaticObject {
public:
	CGroundObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nWidth, int nHeight);
	virtual ~CGroundObject();

	void SetHeightBuffer();
	void SetDefaultHeight(float fHeight);
	float GetHeight(float fx, float fz);

public:
	std::vector<XMFLOAT3>* m_pHeightBuffer = NULL;
	int m_nWidth = 0;
	int m_nLength = 0;
	float m_fHeight = 0.0f;
	float m_xInterval = 0.0f;
	float m_zInterval = 0.0f;
	fRect m_Rect;
};