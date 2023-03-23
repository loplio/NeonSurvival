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

	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler* pCallbackHandler);

	void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CGameObject;

struct SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters, int nRows = 1, int nCols = 1);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	int								m_nTextures = 0;
	ID3D12Resource**				m_ppd3dTextures = NULL;
	ID3D12Resource**				m_ppd3dTextureUploadBuffers;

	UINT*							m_pnResourceTypes = NULL;

	_TCHAR							(*m_ppstrTextureNames)[64] = NULL;

	DXGI_FORMAT*					m_pdxgiBufferFormats = NULL;
	int*							m_pnBufferElements = NULL;

	int								m_nRootParameters = 0;
	int*							m_pnRootParameterIndices = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE*	m_pd3dSrvGpuDescriptorHandles = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE*	m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	SRVROOTARGUMENTINFO*			m_pRootArgumentInfos = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	int LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader, UINT nIndex);
	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex, bool bIsDDSFile = true);
	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	_TCHAR* GetTextureName(int nIndex) { return(m_ppstrTextureNames[nIndex]); }
	ID3D12Resource* GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	ID3D12Resource* GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int nIndex) { return(m_pd3dSrvGpuDescriptorHandles[nIndex]); }
	int GetRootParameter(int nIndex) { return(m_pnRootParameterIndices[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();

	void Animate() { }
	void AnimateRowColumn(bool& bAct, float& fTime, float coolTime = 1.0f);		// 昏力 - test module code

public:
	int 							m_nRows = 1;	// 昏力 - test module code
	int 							m_nCols = 1;	// 昏力 - test module code

	XMFLOAT4X4						m_xmf4x4Texture;
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
	CTexture*						m_pTexture = NULL;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	UINT							m_nReflection = 0;
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

//protected:
//	ID3D12Resource*					m_pd3dcbTextureUV = NULL;
//	CB_TEXTURE_UV_INFO*				m_pcbMappedTextureUV = NULL;

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
	CGameObject(int nMeshes = 1, int nMaterials = 1);
	virtual ~CGameObject();

private:
	int m_nReferences = 0;

public:
	void AddRef();
	void Release();

	bool IsVisible(CCamera* pCamera = NULL);
	bool IsCollide(BoundingOrientedBox& box);

protected:
	std::vector<CBoundingBoxMesh*> m_ppBBMeshes;
	ObjectType objtype = ObjectType::NONE;

public:
	char						m_pstrFrameName[64];

	XMFLOAT4X4					m_xmf4x4World;
	XMFLOAT4X4					m_xmf4x4Transform;
	XMFLOAT3					m_xmf3Shift;
	XMFLOAT3					m_xmf3Scale;
	float						m_Mass;

	CGameObject*				m_pParent = NULL;
	CGameObject*				m_pChild = NULL;
	CGameObject*				m_pSibling = NULL;

	int							m_nMaterials;
	CMaterial**					m_ppMaterials;
	CMesh*						m_pMesh;

	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvGPUDescriptorHandle;

public:
	// hold off..
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void ReleaseShaderVariables();
	void ReleaseUploadBuffers();

	virtual void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);
	virtual void SetShader(int nMaterial, CShader* pShader);
	void SetMaterial(int nMaterial, CMaterial* pMaterial);
	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	// processcompute..
	virtual void Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	// processoutput..
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	void Rotate(XMFLOAT4* pxmf4Quaternion);
	void Rotate(XMFLOAT3* pxmfAxis, float fAngle);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Scale(float width = 1.f, float height = 1.f, float depth = 1.f);
	void SetScale(const XMFLOAT3& scale);
	void SetMass(float mass);
	void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3&& xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));
	void SetLookAt2(XMFLOAT3& xmf3Target, XMFLOAT3&& xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));	//昏力 - test module 内靛.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetTransform(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& look, const XMFLOAT3& pos);

	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance);

	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return m_d3dCbvGPUDescriptorHandle; }

public:
	ObjectType GetObjectType() { return objtype; }		// 昏力 - test moudule code
	void SetObjectType(ObjectType type);		// 昏力 - test moudule code
	
	std::vector<CBoundingBoxMesh*>& GetBBMesh() { return m_ppBBMeshes; }
	void CreateBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader);
	void AddBoundingBoxMesh(CBoundingBoxMesh* pBBMesh) { m_ppBBMeshes.push_back(pBBMesh); }

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }
	CMaterial* GetMaterial(int n) { return m_ppMaterials[n]; }

	CGameObject* GetParent() { return(m_pParent); }
	CGameObject* FindFrame(const char* pstrFrameName);

	int FindReplicatedTexture(_TCHAR* pstrTextureName, D3D12_GPU_DESCRIPTOR_HANDLE* pd3dSrvGpuDescriptorHandle);
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

	void PrintObjectInfo() {		// 昏力 - test debug
		if (m_pMesh)
			std::cout << "MeshName: " << m_pstrFrameName << ", max height: " << m_pMesh->GetAABBCenter().y + m_pMesh->GetAABBExtents().y << ", min height: " << m_pMesh->GetAABBCenter().y - m_pMesh->GetAABBExtents().y
			<< ", left: " << m_pMesh->GetAABBCenter().x - m_pMesh->GetAABBExtents().x << "right: " << m_pMesh->GetAABBCenter().x + m_pMesh->GetAABBExtents().x
			<< ", back: " << m_pMesh->GetAABBCenter().y - m_pMesh->GetAABBExtents().y << "front: " << m_pMesh->GetAABBCenter().y + m_pMesh->GetAABBExtents().y << std::endl;

		if (m_pSibling) m_pSibling->PrintObjectInfo();
		if (m_pChild) m_pChild->PrintObjectInfo();
	}
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
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeightMapTerrain : public CGameObject {
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
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
class CSuperCobraObject : public CGameObject
{
public:
	CSuperCobraObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CSuperCobraObject();

private:
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;

public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
