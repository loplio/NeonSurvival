#include "stdafx.h"
#include "Components_Neon.h"
#include "GameObject.h"
#include "ShaderObjects.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
Player_Neon::Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes) : CPlayer()
{
	m_pCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fHeight = pTerrain->GetHeight(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);
	SetPosition(XMFLOAT3(pTerrain->GetWidth() * 0.5f, fHeight + METER_PER_PIXEL(20), pTerrain->GetLength() * 0.5f));
	SetOffset(XMFLOAT3(0.0f, METER_PER_PIXEL(1.5), 0.0f));	// fire offset.

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/NeonHuman/NeonHuman.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);
	UpdateMobility(Moveable);

	const int nAnimation = 26;
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimation, pPlayerModel);
	for (int i = 0; i < nAnimation; ++i)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

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
	//std::cout << "PlayerPos: " << m_xmf3Position.x << ", " << m_xmf3Position.y << ", " << m_xmf3Position.z << std::endl;
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
	if (nCameraMode == FIRST_PERSON_CAMERA || nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == SHOULDER_HOLD_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);

	// Reset ViewMatrix
	m_pCamera->RegenerateViewMatrix();

	// Decelerated velocity operation
	XMFLOAT3 fDeceleration = XMFLOAT3(m_fFriction * fTimeElapsed, 0.0f, m_fFriction * fTimeElapsed);
	if (fDeceleration.x > fabs(m_xmf3Velocity.x)) fDeceleration.x = m_xmf3Velocity.x;
	if (fDeceleration.y > fabs(m_xmf3Velocity.y)) fDeceleration.y = m_xmf3Velocity.y;
	if (fDeceleration.z > fabs(m_xmf3Velocity.z)) fDeceleration.z = m_xmf3Velocity.z;
	fDeceleration.x *= -m_xmf3Velocity.x; fDeceleration.y *= -m_xmf3Velocity.y; fDeceleration.z *= -m_xmf3Velocity.z;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, fDeceleration);

	int ServerInnResultAnimBundle = -1;
	float ServerfLength = 0;
	if (m_pSkinnedAnimationController)
	{
		int nResultAnimBundle = -1;
		m_pSkinnedAnimationController->SetAnimationBundle(m_nGunType);

		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (::IsZero(fLength))
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->IDLE];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			//m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
		}
		else if (!IsDash && m_dwDirection == DIR_FORWARD)	// walking
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->WALK];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
			ServerfLength = fLength / m_fMaxVelocityXZ;
			//printf("Walk\n");
		}
		else if (m_dwDirection == DIR_BACKWARD)	// backward walking
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->BACKWARD_WALK];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else if (m_dwDirection == DIR_LEFT)	// left walking
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->LEFT_WALK];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else if (m_dwDirection == DIR_RIGHT)	// right walking
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->RIGHT_WALK];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else if (m_dwDirection & DIR_LEFT && m_dwDirection & DIR_BACKWARD)	// left backward
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->LEFT_BACKWARD];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else if (m_dwDirection & DIR_RIGHT && m_dwDirection & DIR_BACKWARD)	// right backward
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->RIGHT_BACKWARD];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else if (m_dwDirection & DIR_LEFT && m_dwDirection & DIR_FORWARD)	// left forward
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->LEFT_FORWARD];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else if (m_dwDirection & DIR_RIGHT && m_dwDirection & DIR_FORWARD)	// right forward
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->RIGHT_FORWARD];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
		}
		else				// slow runing
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->RUN];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
			ServerfLength = fLength / m_fMaxVelocityXZ;
			//printf("run\n");
		}
		if (ServerfLength != 0)
		{
			ServerfLength = fLength / m_fMaxVelocityXZ;
		}
		ServerInnResultAnimBundle = nResultAnimBundle;
	}

	//서버로 위치 전송
	//SERVER::getInstance().SendPosition(GetPosition());
	SERVER::getInstance().SendPlayerData(*this, m_nGunType, ServerfLength, ServerInnResultAnimBundle);
	
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
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA || m_pCamera->GetMode() == SHOULDER_HOLD_CAMERA)
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
		SetMaxVelocityY(PIXEL_KPH(200));
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, METER_PER_PIXEL(1.55), METER_PER_PIXEL(0.3)));
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
		m_pCamera->SetOffset(XMFLOAT3(0.0f, METER_PER_PIXEL(1.5), 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(0.0f);
		SetGravity(XMFLOAT3(0.0f, PIXEL_MPS(-9.8), 0.0f));
		SetMaxVelocityXZ(PIXEL_KPH(40));
		SetMaxVelocityY(PIXEL_KPH(200));
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.15f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, METER_PER_PIXEL(2), METER_PER_PIXEL(-5)));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SHOULDER_HOLD_CAMERA:
		SetFriction(0.0f);
		SetGravity(XMFLOAT3(0.0f, PIXEL_MPS(-9.8), 0.0f));
		SetMaxVelocityXZ(PIXEL_KPH(40));
		SetMaxVelocityY(PIXEL_KPH(200));
		m_pCamera = OnChangeCamera(SHOULDER_HOLD_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.05f);
		m_pCamera->SetOffset(XMFLOAT3(METER_PER_PIXEL(0.8), METER_PER_PIXEL(1.55), METER_PER_PIXEL(-1)));
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
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 10, 300, 10);

	// Terrain Build.
	XMFLOAT3 xmf3Scale(12.0f, 1.0f, 12.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.1f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("GameTexture/terrain.raw"),
		(wchar_t*)L"GameTexture/neon_tile4_1.dds",
		(wchar_t*)L"GameTexture/neon_tile4_1.dds", 
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
	if(m_pTerrain) m_pTerrain->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);

	CScene::CreateBoundingBox(pd3dDevice, pd3dCommandList, BBShader);
}
void Scene_Neon::RunTimeBuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_ppShaders.size(); ++i)
	{
		m_ppShaders[i]->RunTimeBuild(pd3dDevice, pd3dCommandList);
	}
	CScene::RunTimeBuildObjects(pd3dDevice, pd3dCommandList);
}
void Scene_Neon::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildLightsAndMaterials();

	// SkyBox Build.
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (wchar_t*)L"SkyBox/NeonCity.dds");

	// ShaderObjects Build.
	m_ppShaders.reserve(5);
	m_ppComputeShaders.reserve(10);

	/// UI ///
	m_UIShaders.push_back(new CShader);
	CTextureToScreenShader* pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/exp_line.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, FRAME_BUFFER_WIDTH, 10, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT - 5, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/HP_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 50, 50, 0, 40, 40, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/HP_line.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 200, 20, 0, 170, 40, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/Energy_line.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 200, 5, 0, 170, 30, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/map_frame.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 200, 200, 0, FRAME_BUFFER_WIDTH-95, 100, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;


	//m_UIShaders.push_back(new CShader);
	//pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/minimap.dds");
	//pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 185, 185, 0.5f, FRAME_BUFFER_WIDTH - 95, 100,0);
	//pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//m_UIShaders.back() = pUITexture;
	/// background ///
	//m_ppComputeShaders.push_back(new CComputeShader);
	//CBrightAreaComputeShader* pBrightAreaComputeShader = new CBrightAreaComputeShader((wchar_t*)L"Image/Light3.dds");
	//pBrightAreaComputeShader->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	//m_ppComputeShaders.back() = pBrightAreaComputeShader;

	//m_ppComputeShaders.push_back(new CComputeShader);
	//CGaussian2DBlurComputeShader* pBlurComputeShader = new CGaussian2DBlurComputeShader((wchar_t*)L"Image/Light3.dds");
	//pBlurComputeShader->SetSourceResource(pBrightAreaComputeShader->m_pTexture->GetTexture(1));
	//pBlurComputeShader->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	//m_ppComputeShaders.back() = pBlurComputeShader;

	//m_ppShaders.push_back(new CShader);
	//CTextureToFullScreenShader* pGraphicsShader = new CTextureToFullScreenShader(pBlurComputeShader->m_pTexture);
	//pGraphicsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//m_ppShaders.back() = pGraphicsShader;
	//---------------------------------------------------------------------------------------------------//

	/// Particle1 ///
	m_ppComputeShaders.push_back(new CComputeShader);
	CBrightAreaComputeShader* pBrightAreaComputeShader = new CBrightAreaComputeShader((wchar_t*)L"Image/Particle/RoundSoftParticle.dds");
	pBrightAreaComputeShader->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	m_ppComputeShaders.back() = pBrightAreaComputeShader;

	//m_ppComputeShaders.push_back(new CComputeShader);
	//CGaussian2DBlurComputeShader* pParticleBlurComputeShader1 = new CGaussian2DBlurComputeShader((wchar_t*)L"Image/Particle/RoundSoftParticle.dds");
	//pParticleBlurComputeShader1->SetSourceResource(pBrightAreaComputeShader->m_pTexture->GetTexture(1));
	//pParticleBlurComputeShader1->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	//m_ppComputeShaders.back() = pParticleBlurComputeShader1;

	/// Bullet1 ///
	m_ppComputeShaders.push_back(new CComputeShader);
	CGaussian2DBlurComputeShader* pBulletBlurComputeShader1 = new CGaussian2DBlurComputeShader((wchar_t*)L"Image/Particle/RoundSoftParticle.dds");
	pBulletBlurComputeShader1->SetSourceResource(pBrightAreaComputeShader->m_pTexture->GetTexture(1));
	pBulletBlurComputeShader1->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	m_ppComputeShaders.back() = pBulletBlurComputeShader1;

	/// Monster ///
	m_ppShaders.push_back(new CShader);
	MonsterMetalonObjects* pMetalonShader = new MonsterMetalonObjects();
	pMetalonShader->BuildComponents(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pMetalonShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	// Monster Object의 (MST_INSTANCE m_InstInfo)만 수정 가능.
	for (int i = 0; i < 30; ++i) pMetalonShader->AppendMonster(pd3dDevice, pd3dCommandList, XMFLOAT3(3000.f + 20.f * i, 260.f, 3000.f));
	m_ppShaders.back() = pMetalonShader;

	/// PistolBullet ///
	m_ppShaders.push_back(new CShader);
	PistolBulletTexturedObjects* pPistolBulletShader = new PistolBulletTexturedObjects();
	pPistolBulletShader->BuildComponents(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBulletBlurComputeShader1->m_pTexture);
	pPistolBulletShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders.back() = pPistolBulletShader;

	// Monster.
	//CLoadedModelInfo* pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Monster/Metalon/Green_Metalon.bin", NULL);
	//m_vHierarchicalGameObjects.push_back(new DynamicObject());
	//m_vHierarchicalGameObjects.back()->SetChild(pMonsterModel->m_pModelRootObject);
	//m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pMonsterModel);
	//m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	//m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackEnable(0, 0);
	//m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	//m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 200.f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	//if (pMonsterModel) delete pMonsterModel;
	//---------------------------------------------------------------------------------------------------//

	// Objects build.
	m_vParticleObjects.reserve(5);
	m_vGameObjects.reserve(5);
	m_vHierarchicalGameObjects.reserve(5);

	// ParticleObjects.
	//m_vParticleObjects.push_back(new CParticleObject_Neon(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pParticleBlurComputeShader1->m_pTexture, XMFLOAT3(3100.0f, 290.0f, 3100.0f), XMFLOAT3(0.0f, PIXEL_KPH(60), 0.0f), 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(8.0f, 8.0f), MAX_PARTICLES));

	// GameObjects.
	m_vGameObjects.push_back(new Crosshair(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0.0035f, 0.02f, 0.03f, 0.003f, true));

	// HierarchicalGameObjects.
	CLoadedModelInfo* pNexusModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Nexus/Nexus.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new NexusObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pNexusModel, 1));
	m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackEnable(0, 0);
	m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackSpeed(0, 4.0f);
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, 17.0f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_NexusModelPos = m_vHierarchicalGameObjects.back()->GetPosition();
	if (pNexusModel) delete pNexusModel;

	CLoadedModelInfo* pGroundModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Ground/Ground.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGroundModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	if (pGroundModel) delete pGroundModel;

	CLoadedModelInfo* pBaseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Mana/Base.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBaseModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	if (pBaseModel) delete pBaseModel;

	CLoadedModelInfo* pGroundBrickModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/BrickForGround/BricksForGround.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGroundBrickModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 14.0f, 11.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	if (pGroundBrickModel) delete pGroundBrickModel;

	CLoadedModelInfo* pStoneGardians1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StoneGardians/StoneGardians.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pStoneGardians1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 20.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 110.0f + m_pTerrain->GetLength() * 0.5f);
	if (pStoneGardians1Model) delete pStoneGardians1Model;

	CLoadedModelInfo* pStoneGardians2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StoneGardians/StoneGardians.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pStoneGardians2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 35.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 107.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 180.0f, 0.0f);
	if (pStoneGardians2Model) delete pStoneGardians2Model;

	CLoadedModelInfo* pTreeModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Tree/Tree.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTreeModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 55.0f, 15.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 95.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -90.0f, 0.0f);
	if (pTreeModel) delete pTreeModel;

	CLoadedModelInfo* pTree2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Tree/Tree.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTree2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 70.0f, 13.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -73.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);
	if (pTree2Model) delete pTree2Model;

	CLoadedModelInfo* pTree3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Tree/Tree.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTree3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 75.0f, 15.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -35.0f + m_pTerrain->GetLength() * 0.5f);
	if (pTree3Model) delete pTree3Model;

	CLoadedModelInfo* pLowerWallModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/LowerWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLowerWallModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 81.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 20.0f + m_pTerrain->GetLength() * 0.5f);
	if (pLowerWallModel) delete pLowerWallModel;

	CLoadedModelInfo* pLowerWall2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/LowerWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLowerWall2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 73.0f, 8.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 30.0f + m_pTerrain->GetLength() * 0.5f);
	if (pLowerWall2Model) delete pLowerWall2Model;

	CLoadedModelInfo* pUpperWallModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/UpperWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pUpperWallModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 73.0f, 8.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -32.0f + m_pTerrain->GetLength() * 0.5f);
	if (pUpperWallModel) delete pUpperWallModel;

	CLoadedModelInfo* pUpperWall2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/UpperWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pUpperWall2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 45.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 104.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);
	if (pUpperWall2Model) delete pUpperWall2Model;

	CLoadedModelInfo* pUpperWall3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/UpperWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pUpperWall3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 30.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 106.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -90.0f, 0.0f);
	if (pUpperWall3Model) delete pUpperWall3Model;

	CLoadedModelInfo* pCylinder1PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase1.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder1PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 33.0f, 12.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -210.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 0.0f, -15.0f);
	if (pCylinder1PillarModel) delete pCylinder1PillarModel;

	CLoadedModelInfo* pCylinder1Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase1.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder1Pillar2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 15.0f, 12.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -172.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(-19.0f, 0.0f, 10.0f);
	if (pCylinder1Pillar2Model) delete pCylinder1Pillar2Model;

	CLoadedModelInfo* pCylinder3PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase3.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder3PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 40.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -210.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder3PillarModel) delete pCylinder3PillarModel;

	CLoadedModelInfo* pCylinder4PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder4PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 73.5f, 8.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -15.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder4PillarModel) delete pCylinder4PillarModel;

	CLoadedModelInfo* pCylinder4Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder4Pillar2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 20.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -177.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder4Pillar2Model) delete pCylinder4Pillar2Model;

	CLoadedModelInfo* pCylinder5PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase5.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder5PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 25.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -177.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder5PillarModel) delete pCylinder5PillarModel;

	CLoadedModelInfo* pCylinder5Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase5.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder5Pillar2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 34.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -210.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder5Pillar2Model) delete pCylinder5Pillar2Model;

	CLoadedModelInfo* pCylinder6PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase6.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder6PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 20.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -243.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder6PillarModel) delete pCylinder6PillarModel;

	CLoadedModelInfo* pCylinder6Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase6.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder6Pillar2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 25.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -243.f + m_pTerrain->GetLength() * 0.5f);
	if (pCylinder6Pillar2Model) delete pCylinder6Pillar2Model;

	CLoadedModelInfo* pSpawnObjectModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Mana/Spawn.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpawnObjectModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 2.0f, 13.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -220.f + m_pTerrain->GetLength() * 0.5f);
	if (pSpawnObjectModel) delete pSpawnObjectModel;

	CLoadedModelInfo* pPortalModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortalModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(300), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -90.0f, 0.0f);
	m_Potal1_Pos = m_vHierarchicalGameObjects.back()->GetPosition();
	if (pPortalModel) delete pPortalModel;

	CLoadedModelInfo* pPortal2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortal2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(300), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);
	m_Potal2_Pos = m_vHierarchicalGameObjects.back()->GetPosition();
	if (pPortal2Model) delete pPortal2Model;

	CLoadedModelInfo* pPortal3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortal3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, METER_PER_PIXEL(300) + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 180.0f, 0.0f);
	m_Potal3_Pos = m_vHierarchicalGameObjects.back()->GetPosition();
	if (pPortal3Model) delete pPortal3Model;

	CLoadedModelInfo* pPortal4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortal4Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, METER_PER_PIXEL(-300) + m_pTerrain->GetLength() * 0.5f);
	m_Potal4_Pos = m_vHierarchicalGameObjects.back()->GetPosition();
	if (pPortal4Model) delete pPortal4Model;

	CLoadedModelInfo* pLevelUpTableModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/LevelUpTable/Stylized_Table2.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLevelUpTableModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 63.f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);
	if (pLevelUpTableModel) delete pLevelUpTableModel;

	for (int i = 0; i < MAX_PLAYER - 1; ++i)
	{
		CLoadedModelInfo* pOtherModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/NeonHuman/NeonHuman.bin", NULL);
		m_vOtherPlayer.push_back(new CPlayer());
		m_vOtherPlayer.back()->SetChild(pOtherModel->m_pModelRootObject, true);
		m_vOtherPlayer.back()->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 26, pOtherModel);
		for (int j = 0; j < 26; ++j)
		{
			m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(j, false);
		}
		if (pOtherModel) delete pOtherModel;
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// BoundingBoxObjects Build.
	m_pBoundingObjects->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}
void Scene_Neon::BuildLightsAndMaterials()
{
	CScene::BuildLightsAndMaterials();

	m_nLights = 3;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	float terrainCenterX = m_pTerrain->GetWidth() * 0.5f;
	float terrainCenterZ = m_pTerrain->GetLength() * 0.5f;
	SetLight(m_pLights[0], XMFLOAT4(0.5f, 0.0f, 0.5f, 1.0f), XMFLOAT4(0.8f, 0.0f, 0.4f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX, m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(3), terrainCenterZ), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.001f, 0.0001f),
		0, 0, 0, true, POINT_LIGHT, 200.0f, 0);

	SetLight(m_pLights[1], XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(-50.0f, 20.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
		6.0f, (float)cos(XMConvertToRadians(20.0f)), (float)cos(XMConvertToRadians(40.0f)), true, SPOT_LIGHT, 150.0f, 0);

	SetLight(m_pLights[2], XMFLOAT4(0.8f, 0.6f, 0.6f, 1.0f), XMFLOAT4(0.035f, 0.05f, 0.13f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),
		0, 0, 0, true, DIRECTIONAL_LIGHT, 0.0f, 0);
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
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
		m_pPlayer->SetFire(true);
		for (int i = 0; i < m_ppShaders.size(); ++i)
		{
			if (m_ppShaders[i]->GetReafShaderType() == CShader::ReafShaderType::PistolBulletShader)
			{
				PistolBulletTexturedObjects* pObjectsShader = (PistolBulletTexturedObjects*)m_ppShaders[i];
				XMFLOAT3 startLocation = Vector3::Add(m_pPlayer.get()->GetPosition(), m_pPlayer.get()->GetOffset());
				XMFLOAT3 rayDirection = m_pPlayer.get()->GetRayDirection();
				pObjectsShader->AppendBullet(startLocation, rayDirection);
			}
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		m_pPlayer->SetFire(false);
		break;
	case WM_RBUTTONUP:
		break;
	default:
		break;
	}
	return(false);
}
bool Scene_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'C':
			if (m_pBoundingObjects && !m_pBoundingObjects->m_bCollisionBoxWireFrame) m_pBoundingObjects->m_bCollisionBoxWireFrame = true;
			else if (m_pBoundingObjects && m_pBoundingObjects->m_bCollisionBoxWireFrame) m_pBoundingObjects->m_bCollisionBoxWireFrame = false;
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
void Scene_Neon::Update(float fTimeElapsed)
{
	for (int i = 0; i < m_ppShaders.size(); ++i)
	{
		m_ppShaders[i]->Update(fTimeElapsed);
	}
	CScene::Update(fTimeElapsed);
}
void Scene_Neon::AnimateObjects(float fTimeElapsed)
{
	CScene::AnimateObjects(fTimeElapsed);

		//if (m_vOtherPlayer[i]->m_pSkinnedAnimationController) m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		//m_vOtherPlayer[i]->SetPosition(m_pPlayer->GetPosition());
	if (m_MyId == -1)
	{
		m_MyId = SERVER::getInstance().GetClientNumId();
		//printf("%d\n", m_MyId);
	}
	for (int i = 0; i < m_vOtherPlayer.size();++i)
	{
		for (int j = 0; j < MAX_PLAYER;++j)
		{
			int OtherId = m_pOtherPlayerData2[j].id;
			if (m_MyId != OtherId && -1 != OtherId)
			{
				//애니메이션
				if (m_vOtherPlayer[i]->m_pSkinnedAnimationController)
				{
					if (m_pOtherPlayerData2[OtherId].fLength == 0)
					{
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(m_pOtherPlayerData2[OtherId].InnResultAnimBundle);
					}
					else
					{
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(m_pOtherPlayerData2[OtherId].InnResultAnimBundle);
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackSpeed(m_pOtherPlayerData2[OtherId].InnResultAnimBundle, m_pOtherPlayerData2[OtherId].fLength);
					}
				}
				//방향 및 이동
				m_vOtherPlayer[i]->SetPosition(m_pOtherPlayerData2[OtherId].position);
				m_vOtherPlayer[i]->SetUpVector(m_pOtherPlayerData2[OtherId].UpVector);
				m_vOtherPlayer[i]->SetRightVector(m_pOtherPlayerData2[OtherId].RightVector);
				m_vOtherPlayer[i]->SetLookVector(m_pOtherPlayerData2[OtherId].LookVector);
				m_vOtherPlayer[i]->SetVelocity(m_pOtherPlayerData2[OtherId].velocity);
				m_vOtherPlayer[i]->Animate(fTimeElapsed);
				++i;

				//총알 발사
				bool currfire = m_pOtherPlayerData2[OtherId].Fire;
				if (currfire && m_OtherPlayerPrevFire[OtherId] == false)
				{
					printf("id : %d Fire\n", m_pOtherPlayerData2[OtherId].Fire);
					for (int k = 0; k < m_ppShaders.size(); ++k)
					{
						if (m_ppShaders[k]->GetReafShaderType() == CShader::ReafShaderType::PistolBulletShader)
						{
							PistolBulletTexturedObjects* pObjectsShader = (PistolBulletTexturedObjects*)m_ppShaders[k];
							XMFLOAT3 startLocation = m_pOtherPlayerData2[OtherId].position;
							XMFLOAT3 rayDirection = m_pOtherPlayerData2[OtherId].RayDirection;
							startLocation.y += METER_PER_PIXEL(1.5);
							pObjectsShader->AppendBullet(startLocation, rayDirection);
						}
					}
				}
				m_OtherPlayerPrevFire[OtherId] = currfire;
				
			}
		}
	}
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
void Scene_Neon::DrawUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::DrawUI(pd3dCommandList, pCamera);
}

//-------------------------------------------------------------------------------
/*	Monster Object															   */
//-------------------------------------------------------------------------------
CMonsterMetalon::CMonsterMetalon()
{
}
CMonsterMetalon::CMonsterMetalon(const CGameObject& pGameObject)
{
	strcpy(m_pstrFrameName, pGameObject.m_pstrFrameName);

	m_xmf4x4World = pGameObject.m_xmf4x4World;
	m_xmf4x4Transform = pGameObject.m_xmf4x4Transform;
	m_xmf3Scale = pGameObject.m_xmf3Scale;
	m_xmf3PrevScale = pGameObject.m_xmf3PrevScale;
	m_Mass = pGameObject.m_Mass;

	m_nMaterials = 0;
	m_ppMaterials = NULL;
	m_pMesh = NULL;

	m_Mobility = pGameObject.m_Mobility;

	if (pGameObject.m_pChild) m_pChild = new CGameObject(*pGameObject.m_pChild);
	if (pGameObject.m_pSibling) m_pSibling = new CGameObject(*pGameObject.m_pSibling);
}
CMonsterMetalon::~CMonsterMetalon()
{
}

void CMonsterMetalon::RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}
void CMonsterMetalon::Update(float fTimeElapsed)
{
	m_fDriection = Vector3::Normalize(m_fDriection);
	SetPosition(Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_fDriection, m_fMaxVelocityXZ * fTimeElapsed, false)));
}
void CMonsterMetalon::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();
}

//-------------------------------------------------------------------------------
/*	Other Object															   */
//-------------------------------------------------------------------------------
CParticleObject_Neon::CParticleObject_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles)
	: CParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, xmf3Position, xmf3Velocity, fLifetime, xmf3Acceleration, xmf3Color, xmf2Size, nMaxParticles)
{
	CParticleMesh* pMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, xmf3Position, xmf3Velocity, fLifetime, xmf3Acceleration, xmf3Color, xmf2Size, nMaxParticles);
	SetMesh(pMesh);

	CMaterial* pMaterial = new CMaterial();
	if (pTexture)
	{
		pMaterial->SetTexture(pTexture);
		pTexture->AddRef();

		CScene::CreateSRVUAVs(pd3dDevice, pMaterial->m_ppTextures[0], ROOT_PARAMETER_PARTICLE_TEXTURE, true, true, true, 0, 1);
		CScene::CreateSRVUAVs(pd3dDevice, pMaterial->m_ppTextures[0], ROOT_PARAMETER_OUTPUT, true, true, true, 2, 1, 1);
	}
	else
	{
		CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
		pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Particle/RoundSoftParticle.dds", 0);

		pMaterial->SetTexture(pParticleTexture);

		CScene::CreateSRVUAVs(pd3dDevice, pParticleTexture, ROOT_PARAMETER_PARTICLE_TEXTURE, true);
	}
	/*
		XMFLOAT4 *pxmf4RandomValues = new XMFLOAT4[1024];
		random_device rd;   // non-deterministic generator
		mt19937 gen(rd());  // to seed mersenne twister.
		uniform_real_distribution<> vdist(-1.0, +1.0);
		uniform_real_distribution<> cdist(0.0, +1.0);
		for (int i = 0; i < 1024; i++) pxmf4RandomValues[i] = XMFLOAT4(float(vdist(gen)), float(vdist(gen)), float(vdist(gen)), float(cdist(gen)));
	*/
	srand((unsigned)time(NULL));

	XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	for (int i = 0; i < 1024; i++) { pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f; }

	m_pRandowmValueTexture = new CTexture(1, RESOURCE_BUFFER, 0, 1);
	m_pRandowmValueTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	m_pRandowmValueOnSphereTexture = new CTexture(1, RESOURCE_TEXTURE1D, 0, 1);
	m_pRandowmValueOnSphereTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 256, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	CParticleShader* pShader = new CParticleShader();
	pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	CScene::CreateSRVUAVs(pd3dDevice, m_pRandowmValueTexture, ROOT_PARAMETER_RANDOM_TEXTURE, true);
	CScene::CreateSRVUAVs(pd3dDevice, m_pRandowmValueOnSphereTexture, ROOT_PARAMETER_RANDOM_ON_SPHERE_TEXTURE, true);

	pMaterial->SetShader(pShader);
	SetMaterial(0, pMaterial);
}
CParticleObject_Neon::~CParticleObject_Neon()
{
}
//-------------------------------------------------------------------------------
CPistolBulletObject::CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection)
{
	SetMaterial(0, pMaterial);
	SetPosition(startLocation);
	m_fRayDriection = Vector3::Normalize(rayDirection);
}
CPistolBulletObject::~CPistolBulletObject()
{
	//if (m_pRandowmValueTexture) m_pRandowmValueTexture->Release();
}
void CPistolBulletObject::RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pMesh = new CPistolBulletMesh(pd3dDevice, pd3dCommandList, XMFLOAT2(8.0f, 8.0f));
}
void CPistolBulletObject::Update(float fTimeElapsed)
{
	SetPosition(Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_fRayDriection, m_fSpeed * fTimeElapsed, false)));
	m_fLifeTime += fTimeElapsed;
}
void CPistolBulletObject::ReleaseUploadBuffers()
{
	//if (m_pRandowmValueTexture) m_pRandowmValueTexture->ReleaseUploadBuffers();

	CGameObject::ReleaseUploadBuffers();
}
//-------------------------------------------------------------------------------
NexusObject::NexusObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pNexusModel = pModel;
	if (!pNexusModel) pNexusModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Nexus/model.bin", NULL);

	SetChild(pNexusModel->m_pModelRootObject, true);
	if(pNexusModel->m_pAnimationSets) m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pNexusModel);
}
NexusObject::~NexusObject()
{
}
//-------------------------------------------------------------------------------
Crosshair::Crosshair(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float fthickness, float flength, float interval, float radDot, bool bDot)
{
	SetMesh(new CCrosshairMesh(pd3dDevice, pd3dCommandList, fthickness, flength, interval, radDot, bDot));
	SetMaterial(0, new CMaterial());
	m_ppMaterials[0]->SetShader(new CCrosshairShader());
	m_ppMaterials[0]->m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_ppMaterials[0]->m_pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
Crosshair::~Crosshair()
{
}

//-------------------------------------------------------------------------------
/*	SceneLobby																   */
//-------------------------------------------------------------------------------
SceneLobby_Neon::SceneLobby_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CScene(pd3dDevice, pd3dCommandList)
{
}

void SceneLobby_Neon::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void SceneLobby_Neon::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::UpdateShaderVariables(pd3dCommandList);
}
void SceneLobby_Neon::ReleaseShaderVariables()
{
	CScene::ReleaseShaderVariables();
}

//--Build : SceneLobby_Neon---------------------------------------------------------------
void SceneLobby_Neon::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildLightsAndMaterials();

	/// UI ///
	m_UIShaders.push_back(new CShader);
	CTextureToScreenShader* pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/main_bg.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;



	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/title.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 400, 400, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.7 / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	if (!on1)
	{
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line_r.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.2 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/gamestart_r.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.2 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
	}
	else
	{
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.2 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/gamestart_b.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.2 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
	}
	if (!on2)
	{
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line_r.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.4 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/setting_r.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.4 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
	}
	else
	{
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.4 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/setting_b.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.4 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
	}
	if (!on3)
	{
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line_r.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.6 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/quit_r.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.6 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
	}
	else
	{
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.6 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
		m_UIShaders.push_back(new CShader);
		pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/quit_b.dds");
		pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.6 / 2, 0);
		pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_UIShaders.back() = pUITexture;
	}
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void SceneLobby_Neon::BuildLightsAndMaterials()
{
	CScene::BuildLightsAndMaterials();

	m_nLights = 2;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	SetLight(m_pLights[0], XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f), XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(-50.0f, 20.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
		6.0f, (float)cos(XMConvertToRadians(20.0f)), (float)cos(XMConvertToRadians(40.0f)), true, SPOT_LIGHT, 150.0f, 0);

	SetLight(m_pLights[1], XMFLOAT4(0.8f, 0.6f, 0.6f, 1.0f), XMFLOAT4(0.035f, 0.05f, 0.13f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),
		0, 0, 0, true, DIRECTIONAL_LIGHT, 0.0f, 0);
}
void SceneLobby_Neon::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();
}
void SceneLobby_Neon::ReleaseObjects()
{
	CScene::ReleaseObjects();
}

//--ProcessInput : SceneLobby_Neon--------------------------------------------------------
bool SceneLobby_Neon::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int mouseX = LOWORD(lParam);
	int mouseY = HIWORD(lParam);

	if (mouseX >= 250 && mouseX <= 550 && mouseY >= 380 && mouseY <= 420)
	{
		on1 = true;
	}
	else
	{
		on1 = false;
	}

	return false;
}
bool SceneLobby_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_CONTROL:
			on1 = true;
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

//--ProcessAnimation : SceneLobby_Neon----------------------------------------------------
void SceneLobby_Neon::AnimateObjects(float fTimeElapsed)
{
	CScene::AnimateObjects(fTimeElapsed);
}

//--ProcessOutput : SceneLobby_Neon-------------------------------------------------------
void SceneLobby_Neon::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::OnPrepareRender(pd3dCommandList, pCamera);
}
void SceneLobby_Neon::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::Render(pd3dCommandList, pCamera);
}
void SceneLobby_Neon::DrawUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::DrawUI(pd3dCommandList, pCamera);
}