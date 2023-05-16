#pragma once
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define VERTEXT_POSITION				0x01
#define VERTEXT_COLOR					0x02
#define VERTEXT_NORMAL					0x04
#define VERTEXT_TANGENT					0x08
#define VERTEXT_TEXTURE_COORD0			0x10
#define VERTEXT_TEXTURE_COORD1			0x20

#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

#define VERTEXT_TEXTURE					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_DETAIL					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TEXTURE			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_TANGENT_TEXTURE	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_DETAIL			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TANGENT__DETAIL	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)

class CVertex {
public:
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {}
};

class CDiffusedVertex : public CVertex {
protected:
	XMFLOAT4 m_xmf4Diffuse;
public:
	CDiffusedVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf4Diffuse = xmf4Diffuse;
	}
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position = xmf3Position;
		m_xmf4Diffuse = xmf4Diffuse;
	}
	~CDiffusedVertex() {}

	XMFLOAT3 GetPosition() { return m_xmf3Position; }
};

class CParticleVertex : public CVertex {
public:
	XMFLOAT3						m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float							m_fLifetime = 0.0f;
	UINT							m_nType = 0;

public:
	CParticleVertex() { }
	~CParticleVertex() { }
};
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

class CMesh {
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	virtual void ReleaseUploadBuffers();

public:
	char							m_pstrMeshName[256] = { 0 };

protected:
	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3*						m_pxmf3Positions = NULL;

	ID3D12Resource*					m_pd3dPositionBuffer = NULL;
	ID3D12Resource*					m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dIndexBufferView;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;

	int								m_nSubMeshes = 0;
	int*							m_pnSubSetIndices = NULL;
	UINT**							m_ppnSubSetIndices = NULL;

	ID3D12Resource**				m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource**				m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW*		m_pd3dSubSetIndexBufferViews = NULL;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	BoundingOrientedBox				m_xmBoundingBox;

	UINT m_nIndices = 0;
	UINT m_nStartIndex = 0;
	int m_nBaseVertex = 0;

	UINT m_nSlot = 0;
	UINT m_nVertices = 0;
	UINT m_nOffset = 0;
	UINT m_nStride = sizeof(XMFLOAT3);

public:
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void PreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
	virtual void PostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) { };
	virtual void OnPostRender(int nPipelineState) { };

	int CheckRayIntersection(XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance, XMFLOAT4X4& xmf4x4World, float ReduceScale);

	UINT GetType() { return(m_nType); }
	XMFLOAT3& GetAABBExtents() { return m_xmf3AABBExtents; }
	XMFLOAT3& GetAABBCenter() { return m_xmf3AABBCenter; }
	BoundingOrientedBox GetBoundingBox() { return m_xmBoundingBox; }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStandardMesh : public CMesh
{
public:
	CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CStandardMesh();
	virtual void ReleaseUploadBuffers();

protected:
	XMFLOAT4*						m_pxmf4Colors = NULL;
	XMFLOAT3*						m_pxmf3Normals = NULL;
	XMFLOAT3*						m_pxmf3Tangents = NULL;
	XMFLOAT3*						m_pxmf3BiTangents = NULL;
	XMFLOAT2*						m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2*						m_pxmf2TextureCoords1 = NULL;

	ID3D12Resource*					m_pd3dColorBuffer = NULL;
	ID3D12Resource*					m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

	ID3D12Resource*					m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource*					m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource*					m_pd3dTextureCoord1Buffer = NULL;
	ID3D12Resource*					m_pd3dTextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

	ID3D12Resource*					m_pd3dNormalBuffer = NULL;
	ID3D12Resource*					m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	ID3D12Resource*					m_pd3dTangentBuffer = NULL;
	ID3D12Resource*					m_pd3dTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTangentBufferView;

	ID3D12Resource*					m_pd3dBiTangentBuffer = NULL;
	ID3D12Resource*					m_pd3dBiTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBiTangentBufferView;

public:
	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBoundingBoxMesh : public CMesh {
public:
	XMFLOAT4X4 CenterTransform;

public:
	CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const XMFLOAT3& Extents, const XMFLOAT3& Center, XMFLOAT4X4& WorldTransform, CMesh* pMesh);
	virtual ~CBoundingBoxMesh();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkyBoxMesh : public CMesh {
public:
	CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMeshDiffused : public CStandardMesh {
public:
	CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshDiffused();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMeshTextured : public CStandardMesh {
public:
	CCubeMeshTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTextured();

public:
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTexturedRectMesh : public CStandardMesh {
public:
	CTexturedRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f, float fxPosition = 0.0f, float fyPosition = 0.0f, float fzPosition = 0.0f);
	virtual ~CTexturedRectMesh();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMeshIlluminated : public CMesh {
public:
	CMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMeshIlluminated();

public:
	void CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices);
	void CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices);
	void CalculateTriangleStripVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices);
	void CalculateVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices, UINT* pnIndices, int nIndices);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCrosshairMesh : public CMesh {
public:
	CCrosshairMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float thickness, float length, float interval, float radDot, bool bDot);
	virtual ~CCrosshairMesh();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRawFormatImage {
protected:
	BYTE*			m_pRawImagePixels = NULL;

	int				m_nWidth;
	int				m_nLength;

public:
	CRawFormatImage(LPCTSTR pFileName, int nWidth, int nLength, bool bFlipY = false);
	~CRawFormatImage();

	BYTE GetRawImagePixel(int x, int z) { return(m_pRawImagePixels[x + (z * m_nWidth)]); }
	void SetRawImagePixel(int x, int z, BYTE nPixel) { m_pRawImagePixels[x + (z * m_nWidth)] = nPixel; }

	BYTE* GetRawImagePixels() { return(m_pRawImagePixels); }

	int GetRawImageWidth() { return(m_nWidth); }
	int GetRawImageLength() { return(m_nLength); }
};

class CHeightMapImage : public CRawFormatImage {
private:
	XMFLOAT3 m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeightMapTerrain;

class CHeightMapGridMesh : public CMeshIlluminated {
protected:
	int								m_nWidth;
	int								m_nLength;
	XMFLOAT3						m_xmf3Scale;

	XMFLOAT4*						m_pxmf4Colors = NULL;
	XMFLOAT3*						m_pxmf3Normals = NULL;
	XMFLOAT2*						m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2*						m_pxmf2TextureCoords1 = NULL;

	ID3D12Resource*					m_pd3dColorBuffer = NULL;
	ID3D12Resource*					m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

	ID3D12Resource*					m_pd3dNormalBuffer = NULL;
	ID3D12Resource*					m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	ID3D12Resource*					m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource*					m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource*					m_pd3dTextureCoord1Buffer = NULL;
	ID3D12Resource*					m_pd3dTextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CHeightMapTerrain* heightMapTerrain, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void* pContext = NULL);
	virtual ~CHeightMapGridMesh();

	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	int GetWidth() { return m_nWidth; }
	int GetLength() { return m_nLength; }

	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SKINNED_ANIMATION_BONES		128

class CGameObject;

class CSkinnedMesh : public CStandardMesh
{
public:
	CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CSkinnedMesh();

protected:
	int								m_nBonesPerVertex = 4;
	int								m_RootBoneIndex = 0;

	XMINT4*							m_pxmn4BoneIndices = NULL;
	XMFLOAT4*						m_pxmf4BoneWeights = NULL;

	ID3D12Resource*					m_pd3dBoneIndexBuffer = NULL;
	ID3D12Resource*					m_pd3dBoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneIndexBufferView;

	ID3D12Resource*					m_pd3dBoneWeightBuffer = NULL;
	ID3D12Resource*					m_pd3dBoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneWeightBufferView;

public:
	int								m_nSkinningBones = 0;

	char							(*m_ppstrSkinningBoneNames)[64];
	CGameObject**					m_ppSkinningBoneFrameCaches = NULL; //[m_nSkinningBones]

	XMFLOAT4X4*						m_pxmf4x4BindPoseBoneOffsets = NULL; //Transposed

	ID3D12Resource*					m_pd3dcbBindPoseBoneOffsets = NULL;
	XMFLOAT4X4*						m_pcbxmf4x4MappedBindPoseBoneOffsets = NULL;

	ID3D12Resource*					m_pd3dcbSkinningBoneTransforms = NULL; //Pointer Only
	XMFLOAT4X4*						m_pcbxmf4x4MappedSkinningBoneTransforms = NULL;

public:
	void PrepareSkinning(CGameObject* pModelRootObject);
	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	CGameObject* GetSkinningBoneFrameCache() { return m_ppSkinningBoneFrameCaches[m_RootBoneIndex]; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PARTICLE_TYPE_EMITTER		0
#define PARTICLE_TYPE_SHELL			1
#define PARTICLE_TYPE_FLARE01		2
#define PARTICLE_TYPE_FLARE02		3
#define PARTICLE_TYPE_FLARE03		4

#define MAX_PARTICLES				300000

//#define _WITH_QUERY_DATA_SO_STATISTICS

class CParticleMesh : public CMesh
{
public:
	CParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~CParticleMesh();
	virtual void ReleaseUploadBuffers();

	bool								m_bStart = true;

	UINT								m_nMaxParticles = MAX_PARTICLES;

	ID3D12Resource*						m_pd3dStreamOutputBuffer = NULL;
	ID3D12Resource*						m_pd3dDrawBuffer = NULL;

	ID3D12Resource*						m_pd3dDefaultBufferFilledSize = NULL;
	ID3D12Resource*						m_pd3dUploadBufferFilledSize = NULL;
	UINT64*								m_pnUploadBufferFilledSize = NULL;
#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	ID3D12QueryHeap* m_pd3dSOQueryHeap = NULL;
	ID3D12Resource* m_pd3dSOQueryBuffer = NULL;
	D3D12_QUERY_DATA_SO_STATISTICS* m_pd3dSOQueryDataStatistics = NULL;
#else
	ID3D12Resource*						m_pd3dReadBackBufferFilledSize = NULL;
#endif

	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_d3dStreamOutputBufferView;

	ID3D12Resource*						m_pd3dVertexBuffer = NULL;
	ID3D12Resource*						m_pd3dVertexUploadBuffer = NULL;

	ID3D12Resource*						m_pd3dIndexBuffer = NULL;
	ID3D12Resource*						m_pd3dIndexUploadBuffer = NULL;

	D3D12_VERTEX_BUFFER_VIEW			m_d3dVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW				m_d3dIndexBufferView;

	virtual void CreateVertexBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size);
	virtual void CreateStreamOutputBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaxParticles);

	void PreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) override;
	void PostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) override;

	void OnPostRender(int nPipelineState) override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPistolBulletMesh : public CStandardMesh {
public:
	CPistolBulletMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT2 xmf2Size);
	virtual ~CPistolBulletMesh();

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) override;
};