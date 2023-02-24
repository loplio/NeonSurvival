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
	LIGHT m_pLights[MAX_LIGHTS];
	XMFLOAT4 m_xmf4GlobalAmbient;
};

//struct MATERIALS
//{
//	MATERIAL m_pReflections[MAX_MATERIALS];
//};

class CScene {
public:
	CScene(ID3D12Device* pd3dDevice);
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
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;

	LIGHTS* m_pLights = NULL;
	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	CHeightMapTerrain* m_pTerrain = NULL;

	//CInstancingShader* m_pShaders = NULL;
	std::vector<CShader*> m_ppShaders;

public:
	CSkyBox* m_pSkyBox = NULL;
	std::shared_ptr<CPlayer> m_pPlayer = NULL;
	std::shared_ptr<CBoundingBoxObjects> m_pBBObjects = NULL;

	CHeightMapTerrain* GetTerrain() { return m_pTerrain; }
	std::vector<CShader*>& GetShader() { return m_ppShaders; }
};