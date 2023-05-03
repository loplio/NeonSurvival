#pragma once
#include "Camera.h"
#include "Components_Neon.h"
#include "Components_Test.h"
#include "UILayer.h"
#include "Shader.h"
#include "ShaderObjects.h"

class CGameSource {
protected:
	std::shared_ptr<CScene> m_pScene = NULL;
	std::shared_ptr<CPlayer> m_pPlayer = NULL;
	std::shared_ptr<CBoundingBoxObjects> m_pBBObjects = NULL;
	CCamera* m_pCamera = NULL;

public:
	CGameSource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	~CGameSource() { if (m_pCamera) delete m_pCamera; }

	std::shared_ptr<CScene> GetSharedPtrScene() const { return m_pScene; }
	CScene& GetRefScene() const { return *m_pScene; }

	std::shared_ptr<CPlayer> GetSharedPtrPlayer() const { return m_pPlayer; }
	CPlayer& GetRefPlayer() const { return *m_pPlayer; }

	std::shared_ptr<CBoundingBoxObjects> GetSharedPtrBBShader() const { return m_pBBObjects; }
	CBoundingBoxObjects& GetRefBBShader() const { return *m_pBBObjects; }

	CCamera& GetRefCamera() const { return *m_pCamera; }
};

class TestGameSource : public CGameSource {
public:
	TestGameSource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CGameSource(pd3dDevice, pd3dCommandList) {
		m_pScene = std::make_shared<Scene_Test>(pd3dDevice, pd3dCommandList);
		m_pPlayer = m_pScene->m_pPlayer = std::make_shared<Player_Test>(pd3dDevice, pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain, 1);
		m_pBBObjects = m_pScene->m_pBBObjects = std::make_shared<BoundingBoxObjects_1>(*m_pScene, *m_pPlayer);
	}
};

class NeonGameSource : public CGameSource {
public:
	NeonGameSource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CGameSource(pd3dDevice, pd3dCommandList) {
		m_pScene = std::make_shared<Scene_Neon>(pd3dDevice, pd3dCommandList);
		m_pPlayer = m_pScene->m_pPlayer = std::make_shared<Player_Neon>(pd3dDevice, pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain, 1);
		m_pBBObjects = m_pScene->m_pBBObjects = std::make_shared<BoundingBoxObjects_1>(*m_pScene, *m_pPlayer);
	}
};

class NeonLobbySource : public CGameSource {
public:
	NeonLobbySource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CGameSource(pd3dDevice, pd3dCommandList) {
		m_pScene = std::make_shared<SceneLobby_Neon>(pd3dDevice, pd3dCommandList);
		
		m_pCamera = new CFirstPersonCamera(m_pCamera);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, MERTER_PER_PIXEL(1.6), 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	}
};