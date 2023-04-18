#pragma once
#include "Shader.h"

class CBoundingBoxObjects;
class CGameObject;
class CCamera;

struct LIGHT
{
	XMFLOAT4 m_xmf4Ambient;
	XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT4 m_xmf4Specular;
	XMFLOAT3 m_xmf3Position;
	float m_fFalloff;
	XMFLOAT3 m_xmf3Direction;
	float m_fTheta;
	XMFLOAT3 m_xmf3Attenuation;
	float m_fPhi;
	bool m_bEnable;
	int m_nType;
	float m_fRange;
	float padding;
};

struct LIGHTS
{
	LIGHT m_pLightss[MAX_LIGHTS];
	XMFLOAT4 m_xmf4GlobalAmbient;
	int gnLights;
};

struct FRAMEWORK_INFO
{
	float					m_fCurrentTime;
	float					m_fElapsedTime;
	float					m_fSecondsPerFirework = 1.0f;
	int						m_nFlareParticlesToEmit = 30;
	XMFLOAT3				m_xmf3Gravity = XMFLOAT3(0.0f, PIXEL_KPH(-9.8), 0.0f);
	int						m_nMaxFlareType2Particles = 15;
};

class CScene {
public:
	CScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CScene();

	ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetComputeRootSignature();
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();
	ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags, UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters, UINT nStaticSamplerDescs, D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs);

	// ShaderVariable.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// Build.
	virtual void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void BuildLightsAndMaterials();
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseObjects();

	// ProcessInput.
	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);

	// ProcessCompute.
	void Update(float fTimeElapsed, float fTotalTime);
	virtual void AnimateObjects(float fTimeElapsed);

	// ProcessOutput.
	void RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void OnPostRenderParticle();
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera *pCamera);

protected:
	ID3D12RootSignature*				m_pd3dComputeRootSignature = NULL;
	ID3D12RootSignature*				m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap*		m_pd3dCbvSrvUavDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dUavCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dUavGPUDescriptorStartHandle;
	
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dUavCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dUavGPUDescriptorNextHandle;

public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateSRVUAVs(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement, bool IsGraphics = true, bool IsSrv = true, UINT startIndex = 0, UINT nViews = 0);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	void SetLight(LIGHT& light, XMFLOAT4 xmf4Ambient, XMFLOAT4 xmf4Diffuse, XMFLOAT4 xmf4Specular,
		XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, XMFLOAT3 xmf3Attenuation,
		float fFalloff, float fTheta, float fPhi, bool bEnable, int nType, float fRange, float padding);

	float									m_fElapsedTime = 0.0f;
	float									m_fCurrentTime = 0.0f;

	// Lighting.
	int										m_nLights = 0;
	LIGHT*									m_pLights = NULL;
	ID3D12Resource*							m_pd3dcbLights = NULL;
	LIGHTS*									m_pcbMappedLights = NULL;

	// Framwork_info.
	ID3D12Resource*							m_pd3dcbFrameworkInfo = NULL;
	FRAMEWORK_INFO*							m_pcbMappedFrameworkInfo = NULL;

	XMFLOAT4								m_xmf4GlobalAmbient;

	// Shader(Include Object).
	std::vector<CShader*>					m_ppShaders;
	std::vector<CComputeShader*>			m_ppComputeShaders;
	//CInstancingShader*					m_pShaders = NULL;

	// Objects.
	CSkyBox*								m_pSkyBox = NULL;
	CHeightMapTerrain*						m_pTerrain = NULL;
	std::shared_ptr<CPlayer>				m_pPlayer = NULL;
	std::shared_ptr<CBoundingBoxObjects>	m_pBBObjects = NULL;
	std::vector<CParticleObject*>			m_vParticleObjects;
	std::vector<CGameObject*>				m_vGameObjects;
	std::vector<CGameObject*>				m_vHierarchicalGameObjects;
};