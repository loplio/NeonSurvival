#include "stdafx.h"
#include "Components_Neon.h"
#include "GameObject.h"
#include "ShaderObjects.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
Player_Neon::Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes) : CPlayer(nMeshes)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fHeight = pTerrain->GetHeight(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);
	SetPosition(XMFLOAT3(pTerrain->GetWidth() * 0.5f, fHeight + 1500.0f, pTerrain->GetLength() * 0.5f));
	Rotate(0.0f, 90.0f, 0.0f);

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	m_pShader = new CStandardShader();
	m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 9);

	CGameObject* Robot_Soldier_Blue = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Robot_Soldier_Blue.bin", m_pShader);

	SetChild(Robot_Soldier_Blue);
	Robot_Soldier_Blue->AddRef();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
Player_Neon::~Player_Neon()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();
}

void Player_Neon::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 6.0f;

	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}
void Player_Neon::OnCameraUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pCameraUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z) +
		5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera* p3rdPersonCamera = (CThirdPersonCamera*)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}
CCamera* Player_Neon::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return m_pCamera;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, -500.0f, 0.0f));
		SetMaxVelocityXZ(500.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(500.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(PIXEL_KPH(5));
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(500.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	return m_pCamera;
}

//-------------------------------------------------------------------------------
/*	Scene																	   */
//-------------------------------------------------------------------------------
Scene_Neon::Scene_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CScene(pd3dDevice)
{
	// Terrain Build.
	XMFLOAT3 xmf3Scale(12.0f, 1.0f, 12.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.1f, 0.0f, 0.0f);
#ifdef _WITH_TERRAIN_PARTITION
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("../Assets/Image/Terrain/HeightMap.raw"), 257, 257, 17, 17, xmf3Scale, xmf4Color);
#else
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
		m_pd3dGraphicsRootSignature, _T("GameTexture/terrain.raw"), 512, 512, 512, 512, xmf3Scale, xmf4Color);
#endif
}
Scene_Neon::~Scene_Neon()
{
}

void Scene_Neon::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void Scene_Neon::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::UpdateShaderVariables(pd3dCommandList);
}
void Scene_Neon::ReleaseShaderVariables()
{
	CScene::ReleaseShaderVariables();
}

//--Build : Scene_Neon---------------------------------------------------------------
void Scene_Neon::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader)
{
	CScene::CreateBoundingBox(pd3dDevice, pd3dCommandList, BBShader);
}
void Scene_Neon::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildLightsAndMaterials();

	// SkyBox Build.
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	// ShaderObjects Build.
	m_ppShaders.reserve(5);

	CModelObjects* pModelObjectShader = new ModelObjects_1();
	pModelObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pModelObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders.push_back(pModelObjectShader);

	//CBillboardObject_1s* pBillboardObjectShader = new BillboardObjects_1();
	//pBillboardObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//pBillboardObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	//m_ppShaders.push_back(pBillboardObjectShader);

	CMultiSpriteObject_1s* pMultiSpriteObjectsShader = new MultiSpriteObjects_1();
	pMultiSpriteObjectsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pMultiSpriteObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders.push_back(pMultiSpriteObjectsShader);

	//CBlendTextureObjects* pBlendTextureObjectsShader = new BlendTextureObjects_1();
	//pBlendTextureObjectsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//pBlendTextureObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	//m_ppShaders.push_back(pBlendTextureObjectsShader);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// BoundingBoxObjects Build.
	m_pBBObjects->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}
void Scene_Neon::BuildLightsAndMaterials()
{
	CScene::BuildLightsAndMaterials();
}
void Scene_Neon::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();
}
void Scene_Neon::ReleaseObjects()
{
	CScene::ReleaseObjects();
}

//--ProcessInput : Scene_Neon--------------------------------------------------------
bool Scene_Neon::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}
bool Scene_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_CONTROL:
			break;
		case 'C':
			if (m_pBBObjects && !m_pBBObjects->m_bCollisionBoxWireFrame) m_pBBObjects->m_bCollisionBoxWireFrame = true;
			else if (m_pBBObjects && m_pBBObjects->m_bCollisionBoxWireFrame) m_pBBObjects->m_bCollisionBoxWireFrame = false;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

//--ProcessAnimation : Scene_Neon----------------------------------------------------
void Scene_Neon::AnimateObjects(float fTimeElapsed)
{
	CScene::AnimateObjects(fTimeElapsed);
}

//--ProcessOutput : Scene_Neon-------------------------------------------------------
void Scene_Neon::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::OnPrepareRender(pd3dCommandList, pCamera);
}
void Scene_Neon::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::Render(pd3dCommandList, pCamera);
}
