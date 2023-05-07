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

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/NeonHuman/GunAnimation.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 14, pPlayerModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
	m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
	m_pSkinnedAnimationController->SetTrackAnimationSet(7, 7);
	m_pSkinnedAnimationController->SetTrackAnimationSet(8, 8);
	m_pSkinnedAnimationController->SetTrackAnimationSet(9, 9);
	m_pSkinnedAnimationController->SetTrackAnimationSet(10, 10);
	m_pSkinnedAnimationController->SetTrackAnimationSet(11, 11);
	m_pSkinnedAnimationController->SetTrackAnimationSet(12, 12);
	m_pSkinnedAnimationController->SetTrackAnimationSet(13, 13);
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackEnable(11, false);
	m_pSkinnedAnimationController->SetTrackEnable(12, false);
	m_pSkinnedAnimationController->SetTrackEnable(13, false);

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

	// RayTracePosition
	m_pCamera->SetRayLength(m_fRayLength);

	// Keep out of the ground and align the player and the camera.
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	DWORD nCameraMode = m_pCamera->GetMode();
	if (nCameraMode == FIRST_PERSON_CAMERA || nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == SHOULDER_HOLD_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == SHOULDER_HOLD_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
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
			ServerfLength = 0;
			//m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
			//printf("idle\n");
		}
		else if (!IsDash)	// walking
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->WALK];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
			ServerfLength = fLength / m_fMaxVelocityXZ;
			//printf("Walk\n");
		}
		else				// slow runing
		{
			nResultAnimBundle = m_pSkinnedAnimationController->m_nAnimationBundle[m_pSkinnedAnimationController->RUN];
			m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
			m_pSkinnedAnimationController->SetTrackSpeed(nResultAnimBundle, fLength / m_fMaxVelocityXZ);
			ServerfLength = fLength / m_fMaxVelocityXZ;
			//printf("run\n");
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
		m_pCamera->SetTimeLag(0.25f);
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
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 10, 100, 10);

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
void Scene_Neon::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildLightsAndMaterials();

	// SkyBox Build.
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (wchar_t*)L"SkyBox/NeonCity.dds");

	// ShaderObjects Build.
	m_ppShaders.reserve(5);
	m_ppComputeShaders.reserve(5);

	/// UI ///
	m_UIShaders.push_back(new CShader);
	CTextureToScreenShader* pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/ex.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, FRAME_BUFFER_WIDTH, 10, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT - 5, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	//m_UIShaders.push_back(new CShader);
	//pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/hpbg.dds");
	//pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 220, 24, 0, 90, FRAME_BUFFER_HEIGHT - 50, 0);
	//pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//m_UIShaders.back() = pUITexture;

	//m_UIShaders.push_back(new CShader);
	//pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/hp.dds");
	//pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 200, 18, 0, 100, FRAME_BUFFER_HEIGHT - 50, 0);
	//pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//m_UIShaders.back() = pUITexture;

	/// background ///
	m_ppComputeShaders.push_back(new CComputeShader);
	CBrightAreaComputeShader* pBrightAreaComputeShader = new CBrightAreaComputeShader((wchar_t*)L"Image/Light3.dds");
	pBrightAreaComputeShader->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	m_ppComputeShaders.back() = pBrightAreaComputeShader;

	m_ppComputeShaders.push_back(new CComputeShader);
	CGaussian2DBlurComputeShader* pBlurComputeShader = new CGaussian2DBlurComputeShader((wchar_t*)L"Image/Light3.dds");
	pBlurComputeShader->SetSourceResource(pBrightAreaComputeShader->m_pTexture->GetTexture(1));
	pBlurComputeShader->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	m_ppComputeShaders.back() = pBlurComputeShader;

	//m_ppShaders.push_back(new CShader);
	//CTextureToFullScreenShader* pGraphicsShader = new CTextureToFullScreenShader(pBlurComputeShader->m_pTexture);
	//pGraphicsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//m_ppShaders.back() = pGraphicsShader;
	//---------------------------------------------------------------------------------------------------//

	/// Particle1 ///
	//m_ppComputeShaders.push_back(new CComputeShader);
	//pBrightAreaComputeShader = new CBrightAreaComputeShader((wchar_t*)L"Image/Particle/RoundSoftParticle.dds");
	//pBrightAreaComputeShader->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	//m_ppComputeShaders.back() = pBrightAreaComputeShader;
	//
	//m_ppComputeShaders.push_back(new CComputeShader);
	//CGaussian2DBlurComputeShader* pParticleBlurComputeShader1 = new CGaussian2DBlurComputeShader((wchar_t*)L"Image/Particle/RoundSoftParticle.dds");
	//pParticleBlurComputeShader1->SetSourceResource(pBrightAreaComputeShader->m_pTexture->GetTexture(1));
	//pParticleBlurComputeShader1->CreateComputePipelineState(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature);
	//m_ppComputeShaders.back() = pParticleBlurComputeShader1;

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
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	if (pNexusModel) delete pNexusModel;

	CLoadedModelInfo* pOtherModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/NeonHuman/GunAnimation.bin", NULL);
	m_vOtherPlayer.push_back(new CPlayer());
	m_vOtherPlayer.back()->SetChild(pOtherModel->m_pModelRootObject,true);
	//m_vOtherPlayer.back()->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pOtherModel);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 14, pOtherModel);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(7, 7);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(8, 8);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(9, 9);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(10, 10);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(11, 11);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(12, 12);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(13, 13);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(11, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(12, false);
	m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(13, false);

	//m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	//m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackEnable(0, 0);
	//m_vOtherPlayer.back()->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	if (pOtherModel) delete pOtherModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// BoundingBoxObjects Build.
	m_pBBObjects->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
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
		XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),
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

	for (int i = 0; i < m_vOtherPlayer.size(); ++i)
	{
		//if (m_vOtherPlayer[i]->m_pSkinnedAnimationController) m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		//m_vOtherPlayer[i]->SetPosition(m_pPlayer->GetPosition());
		for (int j = 0; j < 2; ++j)
		{
			int OtherId = m_pOtherPlayerData2[j].id;
			if (m_MyId == -1)
			{
				m_MyId = SERVER::getInstance().GetClientNumId();
				printf("m_MyId : %d\n", m_MyId);
			}
		
			if (m_MyId != OtherId && -1 != OtherId)
			{
				
				//m_vOtherPlayer[i]->m_xmf4x4World = m_pOtherPlayerData2[OtherId].xmf4x4World;
				//m_vOtherPlayer[i]->m_xmf4x4Transform = m_pOtherPlayerData2[OtherId].xmf4x4Transform;
				//float Pitch =	m_pOtherPlayerData2[OtherId].pitch;
				//float Yaw =		m_pOtherPlayerData2[OtherId].yaw;
				//float Roll =	m_pOtherPlayerData2[OtherId].roll;
				//m_vOtherPlayer[i]->Rotate(m_pOtherPlayerData2[OtherId].pitch, m_pOtherPlayerData2[OtherId].yaw, m_pOtherPlayerData2[OtherId].roll);
				//OnPrepareRenderTransform(m_vOtherPlayer[i]);
				//애니메이션
				if (m_vOtherPlayer[i]->m_pSkinnedAnimationController)
				{
					//m_vOtherPlayer[i]->m_pSkinnedAnimationController->m_pAnimationTracks[m_pOtherPlayerData2[OtherId].AnicurrentTrack].m_fPosition = m_pOtherPlayerData2[OtherId].AniKeyFrame;
					//int nResultAnimBundle = -1;
					//m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackPosition(m_pOtherPlayerData2[OtherId].AnicurrentTrack, m_pOtherPlayerData2[OtherId].AniKeyFrame);
					
					//if (m_vOtherPlayer[i]->m_pSkinnedAnimationController->m_nCurrentTrack !=
					//	m_pOtherPlayerData2[OtherId].AnicurrentTrack)
					//{
					//	m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetAnimationBundle(m_pOtherPlayerData2[OtherId].GunType);
					//	m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(m_pOtherPlayerData2[OtherId].AnicurrentTrack);
					//	m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackSpeed(m_pOtherPlayerData2[OtherId].InnResultAnimBundle, m_pOtherPlayerData2[OtherId].fLength);
					//}
					
					//XMFLOAT3 m_xmf3Velocity = m_pOtherPlayerData2[OtherId].velocity;

					//float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
					if (::IsZero(m_pOtherPlayerData2[OtherId].fLength))
					{
						//nResultAnimBundle = m_vOtherPlayer[i]->m_pSkinnedAnimationController->m_nAnimationBundle[m_vOtherPlayer[i]->m_pSkinnedAnimationController->IDLE];
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(m_pOtherPlayerData2[OtherId].InnResultAnimBundle);
						//m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(nResultAnimBundle);
						//m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackSpeed(m_pOtherPlayerData2[OtherId].InnResultAnimBundle, m_pOtherPlayerData2[OtherId].fLength);
						//m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
						//printf("other idle\n");
					}
					else if (!m_pOtherPlayerData2[OtherId].IsDash)	// walking
					{
						//nResultAnimBundle = m_vOtherPlayer[i]->m_pSkinnedAnimationController->m_nAnimationBundle[m_vOtherPlayer[i]->m_pSkinnedAnimationController->WALK];
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(m_pOtherPlayerData2[OtherId].InnResultAnimBundle);
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackSpeed(m_pOtherPlayerData2[OtherId].InnResultAnimBundle, m_pOtherPlayerData2[OtherId].fLength);
						//printf("other walk\n");
					}
					else				// slow runing
					{
						//m_vOtherPlayer[i]->m_pSkinnedAnimationController->m_nAnimationBundle[m_vOtherPlayer[i]->m_pSkinnedAnimationController->RUN];
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetOneOfTrackEnable(m_pOtherPlayerData2[OtherId].InnResultAnimBundle);
						m_vOtherPlayer[i]->m_pSkinnedAnimationController->SetTrackSpeed(m_pOtherPlayerData2[OtherId].InnResultAnimBundle, m_pOtherPlayerData2[OtherId].fLength);
						//printf("other run\n");
					}
				}
				m_vOtherPlayer[i]->SetPosition(m_pOtherPlayerData2[OtherId].position);
				m_vOtherPlayer[i]->SetUpVector(m_pOtherPlayerData2[OtherId].UpVector);
				m_vOtherPlayer[i]->SetRightVector(m_pOtherPlayerData2[OtherId].RightVector);
				m_vOtherPlayer[i]->SetLookVector(m_pOtherPlayerData2[OtherId].LookVector);
				m_vOtherPlayer[i]->SetVelocity(m_pOtherPlayerData2[OtherId].velocity);
			}
		}
		m_vOtherPlayer[i]->Animate(fTimeElapsed);
	}
}

void Scene_Neon::OnPrepareRenderTransform(CPlayer* player)
{
	player->m_xmf4x4Transform._11 = player->GetRightVector().x;		player->m_xmf4x4Transform._12 = player->GetRightVector().y;		player->m_xmf4x4Transform._13 = player->GetRightVector().z;
	player->m_xmf4x4Transform._21 = player->GetUpVector().x;			player->m_xmf4x4Transform._22 = player->GetUpVector().y;			player->m_xmf4x4Transform._23 = player->GetUpVector().z;
	player->m_xmf4x4Transform._31 = player->GetLookVector().x;		player->m_xmf4x4Transform._32 = player->GetLookVector().y;		player->m_xmf4x4Transform._33 = player->GetLookVector().z;
	player->m_xmf4x4Transform._41 = player->GetPosition().x;	player->m_xmf4x4Transform._42 = player->GetPosition().y;	player->m_xmf4x4Transform._43 = player->GetPosition().z;

	player->SetScale(XMFLOAT3(1.f, 1.f, 1.f));
	player->m_xmf4x4Transform = Matrix4x4::Multiply(XMMatrixScaling(player->m_xmf3Scale.x, player->m_xmf3Scale.y, player->m_xmf3Scale.z), player->m_xmf4x4Transform);
}

//--ProcessOutput : Scene_Neon-------------------------------------------------------
void Scene_Neon::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::OnPrepareRender(pd3dCommandList, pCamera);
}
void Scene_Neon::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::Render(pd3dCommandList, pCamera);

	//for (int i = 0; i < m_vOtherPlayer.size(); i++)
	//{
	//	m_vOtherPlayer[i]->Render(pd3dCommandList, pCamera);
	//}
}
void Scene_Neon::DrawUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::DrawUI(pd3dCommandList, pCamera);
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
	CTextureToScreenShader* pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/Lobby.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/title.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 400, 400, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.7 / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/bar1on.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.2 / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/bar2on.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.4 / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/bar3on.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.6 / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/bar4on.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 1.8 / 2, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

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
