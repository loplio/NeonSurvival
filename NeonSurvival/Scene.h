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

class CScene {
public:
	CScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CScene();

	// hold..
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// Build..
	virtual void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void BuildLightsAndMaterials();
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseObjects();

	// ProcessInput..
	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);

	// ProcessAnimaiton..
	virtual void AnimateObjects(float fTimeElapsed);

	// ProcessOutput..
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera *pCamera);

protected:
	ID3D12RootSignature*				m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap*		m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;

public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	float									m_fElapsedTime = 0.0f;

	//int										m_nGameObjects = 0;
	//CGameObject**							m_ppGameObjects = NULL;

	int										m_nHierarchicalGameObjects = 0;
	CGameObject**							m_ppHierarchicalGameObjects = NULL;

	int										m_nLights = 0;
	LIGHT*									m_pLights = NULL;
	ID3D12Resource*							m_pd3dcbLights = NULL;
	LIGHTS*									m_pcbMappedLights = NULL;

	XMFLOAT4								m_xmf4GlobalAmbient;

	CHeightMapTerrain*						m_pTerrain = NULL;

	//CInstancingShader*					m_pShaders = NULL;
	std::vector<CShader*>					m_ppShaders;

public:
	CSkyBox*								m_pSkyBox = NULL;
	std::shared_ptr<CPlayer>				m_pPlayer = NULL;
	std::shared_ptr<CBoundingBoxObjects>	m_pBBObjects = NULL;

	CHeightMapTerrain* GetTerrain() { return m_pTerrain; }
	std::vector<CShader*>& GetShader() { return m_ppShaders; }

	void SetLight(LIGHT& light , XMFLOAT4 xmf4Ambient, XMFLOAT4 xmf4Diffuse, XMFLOAT4 xmf4Specular,
		XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, XMFLOAT3 xmf3Attenuation,
		float fFalloff, float fTheta, float fPhi, bool bEnable, int nType, float fRange, float padding);
};