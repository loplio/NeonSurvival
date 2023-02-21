#pragma once
#include "Camera.h"
#include "Scene.h"
#include "Player.h"
#include "UILayer.h"
#include "Shader.h"
#include "ShaderObjects.h"

class CGameSource
{
	std::shared_ptr<CScene> m_pScene = NULL;
	std::shared_ptr<CPlayer> m_pPlayer = NULL;
	std::shared_ptr<CBoundingBoxObjects> m_pBBObjects = NULL;

public:
	CGameSource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
		m_pScene = std::make_shared<CScene>(pd3dDevice);
		m_pPlayer = m_pScene->m_pPlayer = std::make_shared<CAirplanePlayer>(pd3dDevice, pd3dCommandList, m_pScene->GetGraphicsRootSignature(), 1);
		m_pBBObjects = m_pScene->m_pBBObjects = std::make_shared<BoundingBoxObjects_1>(*m_pScene, *m_pPlayer);
	}

	std::shared_ptr<CScene> GetSharedPtrScene() const { return m_pScene; }
	CScene& GetRefScene() const { return *m_pScene; }

	std::shared_ptr<CPlayer> GetSharedPtrPlayer() const { return m_pPlayer; }
	CPlayer& GetRefPlayer() const { return *m_pPlayer; }

	std::shared_ptr<CBoundingBoxObjects> GetSharedPtrBBShader() const { return m_pBBObjects; }
	CBoundingBoxObjects& GetRefBBShader() const { return *m_pBBObjects; }
};

