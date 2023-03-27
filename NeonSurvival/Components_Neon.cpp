#include "stdafx.h"
#include "Components_Neon.h"
#include "GameObject.h"
#include "ShaderObjects.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
Player_Neon::Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes) : CPlayer()
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fHeight = pTerrain->GetHeight(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);
	SetPosition(XMFLOAT3(pTerrain->GetWidth() * 0.5f, fHeight + MERTER_PER_PIXEL(20), pTerrain->GetLength() * 0.5f));
	SetMass(60);

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/DefaultHuman/Walking.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pPlayerModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackEnable(0, false);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pPlayerModel) delete pPlayerModel;
}
Player_Neon::~Player_Neon()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();
}

void Player_Neon::Move(ULONG dwDirection, float fDistance, bool bUpdateVelocity)
{
	int count = 0;
	ULONG RemainOp = dwDirection;

	if (dwDirection)
	{
		//m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//m_pSkinnedAnimationController->SetTrackEnable(1, true);
	}

	// 여러 방향키를 눌렀을 때, 각 방향마다 적용되므로 각 방향에 적용되는 총 수치를 fDistance로 맞춤.
	// When multiple direction keys are pressed, the total number applied to each direction is aligned with fDistance.
	for (int val = 0x10; val != 0; val /= 4)
	{
		if (RemainOp % val != RemainOp)
		{
			RemainOp = RemainOp % val;
			count++;
		}
	}
	if (count) fDistance *= sqrt(count) / count;

	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
	if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
	if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
	if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
	if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
	if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

	CPlayer::Move(xmf3Shift, bUpdateVelocity);
}
void Player_Neon::Update(float fTimeElapsed)
{
	// Gravity operation
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	// Limit xz-speed
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	if (fLength < 0.1) m_xmf3Velocity.x = 0.0f, m_xmf3Velocity.z = 0.0f;
	//std::cout << "Length: " << fLength  << ", km/h: " << PIXEL_TO_KPH(fLength) << std::endl;
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (m_fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (m_fMaxVelocityXZ / fLength);
	}

	// Limit y-speed
	//float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	//fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	//if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (m_fMaxVelocityY / fLength);

	// Distance traveled in elapsed time
	XMFLOAT3 timeElapsedDistance = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	CPlayer::Move(timeElapsedDistance, false);

	// Keep out of the ground and align the player and the camera.
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	DWORD nCameraMode = m_pCamera->GetMode();
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	// Decelerated velocity operation
	XMFLOAT3 fDeceleration = XMFLOAT3(m_fFriction * fTimeElapsed, 0.0f, m_fFriction * fTimeElapsed);
	if (fDeceleration.x > fabs(m_xmf3Velocity.x)) fDeceleration.x = m_xmf3Velocity.x;
	if (fDeceleration.y > fabs(m_xmf3Velocity.y)) fDeceleration.y = m_xmf3Velocity.y;
	if (fDeceleration.z > fabs(m_xmf3Velocity.z)) fDeceleration.z = m_xmf3Velocity.z;
	fDeceleration.x *= -m_xmf3Velocity.x; fDeceleration.y *= -m_xmf3Velocity.y; fDeceleration.z *= -m_xmf3Velocity.z;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, fDeceleration);

	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (::IsZero(fLength))
		{
			m_pSkinnedAnimationController->SetTrackEnable(0, true);
			//m_pSkinnedAnimationController->SetTrackEnable(1, false);
			//m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
		}
	}
}

void Player_Neon::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	SetScale(XMFLOAT3(1.f, 1.f, 1.f));
	m_xmf4x4Transform = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4Transform);
}

void Player_Neon::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z);

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
		SetFriction(0.0f);
		SetGravity(XMFLOAT3(0.0f, PIXEL_MPS(-9.8), 0.0f));
		SetMaxVelocityXZ(PIXEL_KPH(40));
		SetMaxVelocityY(PIXEL_MPS(20));
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(0.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(5000.0f);
		SetMaxVelocityY(4000.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(0.0f);
		SetGravity(XMFLOAT3(0.0f, PIXEL_MPS(-9.8), 0.0f));
		SetMaxVelocityXZ(PIXEL_KPH(40));
		SetMaxVelocityY(PIXEL_MPS(20));
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
Scene_Neon::Scene_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CScene(pd3dDevice, pd3dCommandList)
{
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 10, 100);

	// Terrain Build.
	XMFLOAT3 xmf3Scale(12.0f, 1.0f, 12.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.1f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("GameTexture/terrain.raw"),
		(wchar_t*)L"GameTexture/neon_tile4_1.dds",
		(wchar_t*)L"GameTexture/neon_tile3_1.dds", 
		512, 512, xmf3Scale, xmf4Color);
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
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (wchar_t*)L"SkyBox/NeonCity.dds");

	// ShaderObjects Build.
	m_ppShaders.reserve(5);


	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// BoundingBoxObjects Build.
	m_pBBObjects->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}
void Scene_Neon::BuildLightsAndMaterials()
{
	CScene::BuildLightsAndMaterials();

	m_nLights = 4;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	SetLight(m_pLights[0], XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(130.0f, 30.0f, 30.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.001f, 0.0001f),
		0, 0, 0, true, POINT_LIGHT, 100.0f, 0);

	SetLight(m_pLights[1], XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(-50.0f, 20.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
		8.0f, (float)cos(XMConvertToRadians(20.0f)), (float)cos(XMConvertToRadians(40.0f)), true, SPOT_LIGHT, 50.0f, 0);

	SetLight(m_pLights[2], XMFLOAT4(0.8f, 0.6f, 0.6f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),
		0, 0, 0, true, DIRECTIONAL_LIGHT, 0.0f, 0);

	SetLight(m_pLights[3], XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(-150.0f, 30.0f, 30.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
		8.0f, (float)cos(XMConvertToRadians(30.0f)), (float)cos(XMConvertToRadians(90.0f)), true, SPOT_LIGHT, 60.0f, 0);
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
