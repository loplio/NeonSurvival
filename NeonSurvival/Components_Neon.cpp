#include "stdafx.h"
#include "Components_Neon.h"
#include "GameObject.h"
#include "ShaderObjects.h"
#include "GameSource.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
Player_Neon::Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext) : CPlayer()
{
	m_pCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fHeight = pTerrain->GetHeight(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);
	SetPosition(XMFLOAT3(pTerrain->GetWidth() * 0.5f, fHeight, pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(19)));
	SetOffset(XMFLOAT3(0.0f, METER_PER_PIXEL(1.5), 0.0f));	// fire offset.

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/NeonHuman/NeonHuman.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);
	UpdateMobility(Moveable);

	m_pMesh = new CMesh(pd3dDevice, pd3dCommandList);
	m_pMesh->SetBoundinBoxCenter(XMFLOAT3(0.0f, 9.0f, 0.0f));
	m_pMesh->SetBoundinBoxExtents(XMFLOAT3(2.5f, 9.0f, 1.5f));
	SetWorldTransformBoundingBox();
	m_xmf3BoundingScale = XMFLOAT3(1.0f, 1.0f, 1.0f);

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
	m_xmf3Displacement = timeElapsedDistance;

	//std::cout << "B : " << m_xmf3Position.x << ", " << m_xmf3Position.y << ", " << m_xmf3Position.z << std::endl;
	//std::cout << "Displacement : " << m_xmf3Displacement.x << ", " << m_xmf3Displacement.y << ", " << m_xmf3Displacement.z << std::endl;
	CPlayer::Move(timeElapsedDistance, false);
	//std::cout << "A : " << m_xmf3Position.x << ", " << m_xmf3Position.y << ", " << m_xmf3Position.z << std::endl;

	// Keep out of the ground and align the player and the camera.
	if (!m_vGroundObjects->empty()) OnGroundUpdateCallback(fTimeElapsed);
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

	//XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();

	//xmf4x4Rotate._11 = m_xmf3Right.x; xmf4x4Rotate._21 = m_xmf3Up.x; xmf4x4Rotate._31 = m_xmf3Look.x;
	//xmf4x4Rotate._12 = m_xmf3Right.y; xmf4x4Rotate._22 = m_xmf3Up.y; xmf4x4Rotate._32 = m_xmf3Look.y;
	//xmf4x4Rotate._13 = m_xmf3Right.z; xmf4x4Rotate._23 = m_xmf3Up.z; xmf4x4Rotate._33 = m_xmf3Look.z;

	//m_xmf3Offset = Vector3::TransformCoord(m_xmf3BaseOffset, xmf4x4Rotate);
	//std::cout << "Offset : " << m_xmf3Offset.x << ", " << m_xmf3Offset.y << ", " << m_xmf3Offset.z << std::endl;

	//서버로 위치 전송
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

void Player_Neon::OnGroundUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	float fHeight = m_vGroundObjects->back()->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z);

	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}

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
		m_nGunType = Empty;
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
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 10, 500, 10);

	// Terrain Build.
	XMFLOAT3 xmf3Scale(12.0f, 1.0f, 12.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.1f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("GameTexture/terrain.raw"),
		(wchar_t*)L"GameTexture/neon_tile4_1.dds",
		(wchar_t*)L"GameTexture/neon_tile4_1.dds", 
		512, 512, xmf3Scale, xmf4Color);
	m_pTerrain->SetIsExistBoundingBox(false);
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
	//pUITexture->SetGauge(0.5f);
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
	GeneralMonsterObjects* pMonsterShader = new GeneralMonsterObjects();
	pMonsterShader->BuildComponents(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pMonsterShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders.back() = pMonsterShader;

	/// PistolBullet ///
	m_ppShaders.push_back(new CShader);
	PistolBulletTexturedObjects* pPistolBulletShader = new PistolBulletTexturedObjects();
	pPistolBulletShader->BuildComponents(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBulletBlurComputeShader1->m_pTexture);
	pPistolBulletShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pBullets = pPistolBulletShader;
	m_ppShaders.back() = pPistolBulletShader;
	//---------------------------------------------------------------------------------------------------//

	// Objects build.
	m_vParticleObjects.reserve(5);
	m_vGameObjects.reserve(5);
	m_vHierarchicalGameObjects.reserve(5);
	m_vGroundObjects.reserve(5);

	// ParticleObjects.
	//m_vParticleObjects.push_back(new CParticleObject_Neon(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pParticleBlurComputeShader1->m_pTexture, XMFLOAT3(3100.0f, 290.0f, 3100.0f), XMFLOAT3(0.0f, PIXEL_KPH(60), 0.0f), 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(8.0f, 8.0f), MAX_PARTICLES));

	// GameObjects.
	m_vGameObjects.push_back(new Crosshair(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0.0035f, 0.02f, 0.03f, 0.003f, true));

	// GroundObjects.
	CLoadedModelInfo* pGroundModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Ground/Ground2.bin", NULL);
	m_vGroundObjects.push_back(new CGroundObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGroundModel, 80, 80));
	m_vGroundObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, 11.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vGroundObjects.back()->SetDefaultHeight(m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f));
	m_vGroundObjects.back()->SetHeightBuffer();
	if (pGroundModel) delete pGroundModel;

	m_pPlayer.get()->SetGroundUpdatedContext(&m_vGroundObjects);

	// HierarchicalGameObjects.
	CLoadedModelInfo* pNexusModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Nexus/Nexus.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new NexusObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pNexusModel, 1));
	m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackEnable(0, 0);
	m_vHierarchicalGameObjects.back()->m_pSkinnedAnimationController->SetTrackSpeed(0, 4.0f);
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, 17.0f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetOneBoundingBox(true, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.15f, 0.8f, 0.15f));
	//m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	m_NexusModelPos = m_vHierarchicalGameObjects.back()->GetPosition();
	if (pNexusModel) delete pNexusModel;

	CLoadedModelInfo* pBaseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Mana/Base.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBaseModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsBoundingCylinder(true);
	//m_vHierarchicalGameObjects.back()->Rotate(0.0f, 45.0f, 0.0f);
	if (pBaseModel) delete pBaseModel;

	CLoadedModelInfo* pGroundBrickModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/BrickForGround/BricksForGround.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGroundBrickModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 14.0f, 11.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pGroundBrickModel) delete pGroundBrickModel;

	CLoadedModelInfo* pStoneGardians1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StoneGardians/StoneGardians.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pStoneGardians1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 20.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 110.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pStoneGardians1Model) delete pStoneGardians1Model;

	CLoadedModelInfo* pStoneGardians2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StoneGardians/StoneGardians.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pStoneGardians2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 35.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 107.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 180.0f, 0.0f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pStoneGardians2Model) delete pStoneGardians2Model;

	CLoadedModelInfo* pTreeModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Tree/Tree.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTreeModel));
	//m_vHierarchicalGameObjects.back()->SetBoundingScale(XMFLOAT3(0.14f, 1.0f, 0.1f));
	//m_vHierarchicalGameObjects.back()->SetBoundingLocation(XMFLOAT3(-8.0f, 0.0f, 12.0f));
	m_vHierarchicalGameObjects.back()->SetOneBoundingBox(true, XMFLOAT3(-0.3f, 0.0f, 0.25f), XMFLOAT3(0.35f, 4.0f, 0.35f));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 55.0f, 15.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 95.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->SetIsBoundingCylinder(true);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 0.0f, 0.0f);
	if (pTreeModel) delete pTreeModel;

	CLoadedModelInfo* pTree2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Tree/Tree.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTree2Model));
	//m_vHierarchicalGameObjects.back()->SetBoundingScale(XMFLOAT3(0.14f, 1.0f, 0.1f));
	//m_vHierarchicalGameObjects.back()->SetBoundingLocation(XMFLOAT3(-8.0f, 0.0f, 12.0f));
	m_vHierarchicalGameObjects.back()->SetOneBoundingBox(true, XMFLOAT3(-0.3f, 0.0f, 0.25f), XMFLOAT3(0.35f, 4.0f, 0.35f));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 70.0f, 13.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -73.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->SetIsBoundingCylinder(true);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 0.0f, 0.0f);
	if (pTree2Model) delete pTree2Model;

	CLoadedModelInfo* pTree3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Tree/Tree.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTree3Model));
	//m_vHierarchicalGameObjects.back()->SetBoundingScale(XMFLOAT3(0.14f, 1.0f, 0.1f));
	//m_vHierarchicalGameObjects.back()->SetBoundingLocation(XMFLOAT3(-8.0f, 0.0f, 12.0f));
	m_vHierarchicalGameObjects.back()->SetOneBoundingBox(true, XMFLOAT3(-0.3f, 0.0f, 0.25f), XMFLOAT3(0.35f, 4.0f, 0.35f));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 75.0f, 15.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -35.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsBoundingCylinder(true);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	if (pTree3Model) delete pTree3Model;

	CLoadedModelInfo* pLowerWallModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/LowerWall2.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLowerWallModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 81.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 20.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 180.0f, 0.0f);
	if (pLowerWallModel) delete pLowerWallModel;

	CLoadedModelInfo* pLowerWall2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/LowerWall2.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLowerWall2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 73.0f, 8.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 30.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 180.0f, 0.0f);
	if (pLowerWall2Model) delete pLowerWall2Model;

	CLoadedModelInfo* pUpperWallModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/UpperWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pUpperWallModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 73.0f, 8.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -29.0f + m_pTerrain->GetLength() * 0.5f);
	if (pUpperWallModel) delete pUpperWallModel;

	CLoadedModelInfo* pUpperWall2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/UpperWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pUpperWall2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 45.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 104.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);
	if (pUpperWall2Model) delete pUpperWall2Model;

	CLoadedModelInfo* pUpperWall3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Wall/UpperWall.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pUpperWall3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 30.0f, 9.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, 106.0f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -90.0f, 0.0f);
	if (pUpperWall3Model) delete pUpperWall3Model;

	CLoadedModelInfo* pCylinder1PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase1.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder1PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 33.0f, 12.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -210.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 0.0f, -15.0f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pCylinder1PillarModel) delete pCylinder1PillarModel;

	CLoadedModelInfo* pCylinder1Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase1.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder1Pillar2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 15.0f, 12.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -172.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(-19.0f, 0.0f, 10.0f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pCylinder1Pillar2Model) delete pCylinder1Pillar2Model;

	CLoadedModelInfo* pCylinder3PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase3.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder3PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 40.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -210.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pCylinder3PillarModel) delete pCylinder3PillarModel;

	CLoadedModelInfo* pCylinder4PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder4PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 73.5f, 8.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -15.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pCylinder4PillarModel) delete pCylinder4PillarModel;

	//CLoadedModelInfo* pCylinder4Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase4.bin", NULL);
	//m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder4Pillar2Model));
	//m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 20.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -177.f + m_pTerrain->GetLength() * 0.5f);
	//m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	//if (pCylinder4Pillar2Model) delete pCylinder4Pillar2Model;

	CLoadedModelInfo* pCylinder5PillarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase5.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder5PillarModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 25.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -177.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pCylinder5PillarModel) delete pCylinder5PillarModel;

	CLoadedModelInfo* pCylinder5Pillar2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/CylinderPillar/CylinderBase5.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCylinder5Pillar2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - 34.0f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, -210.f + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
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
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(250), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -90.0f, 0.0f);

	m_SpawnPotal_Pos[0] = m_vHierarchicalGameObjects.back()->GetPosition();

	if (pPortalModel) delete pPortalModel;

	CLoadedModelInfo* pPortal2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortal2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(250), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	m_SpawnPotal_Pos[1] = m_vHierarchicalGameObjects.back()->GetPosition();

	if (pPortal2Model) delete pPortal2Model;

	CLoadedModelInfo* pPortal3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortal3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, METER_PER_PIXEL(250) + m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 180.0f, 0.0f);

	m_SpawnPotal_Pos[2] = m_vHierarchicalGameObjects.back()->GetPosition();

	if (pPortal3Model) delete pPortal3Model;

	//CLoadedModelInfo* pPortal4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Portal/Portal.bin", NULL);
	//m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPortal4Model));
	//m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, METER_PER_PIXEL(-300) + m_pTerrain->GetLength() * 0.5f);
	//m_SpawnPotal_Pos[3] = m_vHierarchicalGameObjects.back()->GetPosition();

	//if (pPortal4Model) delete pPortal4Model;

	CLoadedModelInfo* pRightLamp1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Lamp/Lamp.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRightLamp1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(15));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pRightLamp1Model) delete pRightLamp1Model;

	CLoadedModelInfo* pRightLamp2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Lamp/Lamp.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRightLamp2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(15));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pRightLamp2Model) delete pRightLamp2Model;

	CLoadedModelInfo* pLeftLamp3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Lamp/Lamp.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLeftLamp3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(15));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLeftLamp3Model) delete pLeftLamp3Model;

	CLoadedModelInfo* pLeftLamp4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Lamp/Lamp.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLeftLamp4Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(15));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLeftLamp4Model) delete pLeftLamp4Model;

	CLoadedModelInfo* pFrontLamp5Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Lamp/Lamp.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pFrontLamp5Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(15), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pFrontLamp5Model) delete pFrontLamp5Model;

	CLoadedModelInfo* pFrontLamp6Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/Lamp/Lamp.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pFrontLamp6Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(15), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pFrontLamp6Model) delete pFrontLamp6Model;

	////////////////////////////////////
	// Street Object
	////////////////////////////////////

	/// ConcreteBarrier //////////////////////////////////////////
	// right
	CLoadedModelInfo* pConcreteBarrier1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(14));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 60.0f, 0.0f);

	if (pConcreteBarrier1Model) delete pConcreteBarrier1Model;

	CLoadedModelInfo* pConcreteBarrier3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(51.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(12.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 60.0f, 0.0f);

	if (pConcreteBarrier3Model) delete pConcreteBarrier3Model;

	CLoadedModelInfo* pConcreteBarrier2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(14));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -60.0f, 0.0f);

	if (pConcreteBarrier2Model) delete pConcreteBarrier2Model;

	CLoadedModelInfo* pConcreteBarrier4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier4Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(51.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(12.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -60.0f, 0.0f);

	if (pConcreteBarrier4Model) delete pConcreteBarrier4Model;

	CLoadedModelInfo* pConcreteBarrier13Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier13Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(49.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-40));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -67.5f, 0.0f);

	if (pConcreteBarrier13Model) delete pConcreteBarrier13Model;

	CLoadedModelInfo* pConcreteBarrier14Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier14Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(40), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-49.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -22.5f, 0.0f);

	if (pConcreteBarrier14Model) delete pConcreteBarrier14Model;

	CLoadedModelInfo* pConcreteBarrier21Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier21Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(38), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-50.3));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -22.5f, 0.0f);

	if (pConcreteBarrier21Model) delete pConcreteBarrier21Model;

	CLoadedModelInfo* pConcreteBarrier15Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier15Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(49.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(40));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 67.5f, 0.0f);

	if (pConcreteBarrier15Model) delete pConcreteBarrier15Model;

	CLoadedModelInfo* pConcreteBarrier16Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier16Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(40), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(49.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 22.5f, 0.0f);

	if (pConcreteBarrier16Model) delete pConcreteBarrier16Model;
	/////////////////////////////////////////////////////////////
	// left
	CLoadedModelInfo* pConcreteBarrier5Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier5Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(14));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -60.0f, 0.0f);

	if (pConcreteBarrier5Model) delete pConcreteBarrier5Model;

	CLoadedModelInfo* pConcreteBarrier6Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier6Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(51.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(12.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -60.0f, 0.0f);

	if (pConcreteBarrier6Model) delete pConcreteBarrier6Model;

	CLoadedModelInfo* pConcreteBarrier7Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier7Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(14));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 60.0f, 0.0f);

	if (pConcreteBarrier7Model) delete pConcreteBarrier7Model;

	CLoadedModelInfo* pConcreteBarrier8Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier8Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(51.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(12.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 60.0f, 0.0f);

	if (pConcreteBarrier8Model) delete pConcreteBarrier8Model;

	CLoadedModelInfo* pConcreteBarrier17Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier17Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(49.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-40));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 67.5f, 0.0f);

	if (pConcreteBarrier17Model) delete pConcreteBarrier17Model;

	CLoadedModelInfo* pConcreteBarrier18Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier18Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(40), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-49.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 22.5f, 0.0f);

	if (pConcreteBarrier18Model) delete pConcreteBarrier18Model;

	CLoadedModelInfo* pConcreteBarrier22Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier22Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(38), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-50.3));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 22.5f, 0.0f);

	if (pConcreteBarrier22Model) delete pConcreteBarrier22Model;

	CLoadedModelInfo* pConcreteBarrier19Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier19Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(49.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(40));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -67.5f, 0.0f);

	if (pConcreteBarrier19Model) delete pConcreteBarrier19Model;

	CLoadedModelInfo* pConcreteBarrier20Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier20Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(40), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(49.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -22.5f, 0.0f);

	if (pConcreteBarrier20Model) delete pConcreteBarrier20Model;
	/////////////////////////////////////////////////////////////
	// front
	CLoadedModelInfo* pConcreteBarrier9Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier9Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(14), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -150.0f, 0.0f);

	if (pConcreteBarrier9Model) delete pConcreteBarrier9Model;

	CLoadedModelInfo* pConcreteBarrier10Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier10Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(12.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(51.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -150.0f, 0.0f);

	if (pConcreteBarrier10Model) delete pConcreteBarrier10Model;

	CLoadedModelInfo* pConcreteBarrier11Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier11Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(14), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 150.0f, 0.0f);

	if (pConcreteBarrier11Model) delete pConcreteBarrier11Model;

	CLoadedModelInfo* pConcreteBarrier12Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/ConcreteBarrier/ConcreteBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pConcreteBarrier12Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(12.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(51.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 150.0f, 0.0f);

	if (pConcreteBarrier12Model) delete pConcreteBarrier12Model;


	/// RedBarrier //////////////////////////////////////////
	// right
	CLoadedModelInfo* pRedBarrier1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/RedBarrier/RedBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRedBarrier1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(7.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pRedBarrier1Model) delete pRedBarrier1Model;

	CLoadedModelInfo* pRedBarrier2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/RedBarrier/RedBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRedBarrier2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(7.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pRedBarrier2Model) delete pRedBarrier2Model;
	/////////////////////////////////////////////////////////////
	// left
	CLoadedModelInfo* pRedBarrier3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/RedBarrier/RedBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRedBarrier3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(7.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pRedBarrier3Model) delete pRedBarrier3Model;

	CLoadedModelInfo* pRedBarrier4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/RedBarrier/RedBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRedBarrier4Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(7.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pRedBarrier4Model) delete pRedBarrier4Model;
	/////////////////////////////////////////////////////////////
	// front
	CLoadedModelInfo* pRedBarrier5Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/RedBarrier/RedBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRedBarrier5Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(7.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pRedBarrier5Model) delete pRedBarrier5Model;

	CLoadedModelInfo* pRedBarrier6Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/RedBarrier/RedBarrier.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRedBarrier6Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(7.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pRedBarrier6Model) delete pRedBarrier6Model;


	/// LongFence //////////////////////////////////////////
	// right
	CLoadedModelInfo* pLongFence1_4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(21));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model) delete pLongFence1_4Model;

	CLoadedModelInfo* pLongFence1_4Model2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model2));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-21));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model2) delete pLongFence1_4Model2;

	CLoadedModelInfo* pLongFence1_4Model5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model5));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(33.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model5) delete pLongFence1_4Model5;

	CLoadedModelInfo* pLongFence1_4Model6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model6));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-33.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model6) delete pLongFence1_4Model6;


	CLoadedModelInfo* pLongFence1_4Model13 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model13));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(45), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(45));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 45.0f, 0.0f);

	if (pLongFence1_4Model13) delete pLongFence1_4Model13;

	CLoadedModelInfo* pLongFence1_4Model14 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model14));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(45), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-45));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -45.0f, 0.0f);

	if (pLongFence1_4Model14) delete pLongFence1_4Model14;

	/////////////////////////////////////////////////////////////
	// left
	CLoadedModelInfo* pLongFence1_4Model3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model3));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(21));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model3) delete pLongFence1_4Model3;

	CLoadedModelInfo* pLongFence1_4Model4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model4));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-21));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model4) delete pLongFence1_4Model4;

	CLoadedModelInfo* pLongFence1_4Model11 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model11));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(21));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model11) delete pLongFence1_4Model11;

	CLoadedModelInfo* pLongFence1_4Model12 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model12));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-21));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model12) delete pLongFence1_4Model12;

	CLoadedModelInfo* pLongFence1_4Model15 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model15));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(45), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(45));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -45.0f, 0.0f);

	if (pLongFence1_4Model15) delete pLongFence1_4Model15;

	CLoadedModelInfo* pLongFence1_4Model16 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model16));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(45), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-45));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 45.0f, 0.0f);

	if (pLongFence1_4Model16) delete pLongFence1_4Model16;

	CLoadedModelInfo* pLongFence1_4Model17 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model17));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(33.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model17) delete pLongFence1_4Model17;

	CLoadedModelInfo* pLongFence1_4Model18 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model18));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(-33.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);

	if (pLongFence1_4Model18) delete pLongFence1_4Model18;

	/////////////////////////////////////////////////////////////
	// front
	CLoadedModelInfo* pLongFence1_4Model7 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model7));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(-21), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLongFence1_4Model7) delete pLongFence1_4Model7;

	CLoadedModelInfo* pLongFence1_4Model8 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model8));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(21), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLongFence1_4Model8) delete pLongFence1_4Model8;

	CLoadedModelInfo* pLongFence1_4Model9 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model9));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(-33.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLongFence1_4Model9) delete pLongFence1_4Model9;

	CLoadedModelInfo* pLongFence1_4Model10 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_4.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_4Model10));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(33.5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLongFence1_4Model10) delete pLongFence1_4Model10;

	/////////////////////////////////////////////////////////////
	// back
	CLoadedModelInfo* pLongFence1_24Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence1/Fence1_24.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLongFence1_24Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(50.6));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pLongFence1_24Model) delete pLongFence1_24Model;


	//ShortFence
	//CLoadedModelInfo* pShortFence1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Fence2/Fence2.bin", NULL);
	//m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pShortFence1Model));
	//m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(30), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(0));
	//m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	//if (pShortFence1Model) delete pShortFence1Model;

	/// Barrel //////////////////////////////////////////
	// right
	CLoadedModelInfo* pBarrel1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(0));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel1Model) delete pBarrel1Model;

	CLoadedModelInfo* pBarrel2Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel2Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel2Model) delete pBarrel2Model;

	CLoadedModelInfo* pBarrel3Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel3Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(10));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel3Model) delete pBarrel3Model;

	CLoadedModelInfo* pBarrel4Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel4Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel4Model) delete pBarrel4Model;

	CLoadedModelInfo* pBarrel5Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel5Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(10));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel5Model) delete pBarrel5Model;


	CLoadedModelInfo* pBarrel16Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel16Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(49.7), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(40.2));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel16Model) delete pBarrel16Model;

	CLoadedModelInfo* pBarrel17Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel17Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(40.2), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(49.7));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel17Model) delete pBarrel17Model;

	/////////////////////////////////////////////////////////////
	// left
	CLoadedModelInfo* pBarrel6Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel6Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(0));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel6Model) delete pBarrel6Model;

	CLoadedModelInfo* pBarrel7Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel7Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel7Model) delete pBarrel7Model;

	CLoadedModelInfo* pBarrel8Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel8Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(10));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel8Model) delete pBarrel8Model;

	CLoadedModelInfo* pBarrel9Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel9Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel9Model) delete pBarrel9Model;

	CLoadedModelInfo* pBarrel10Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel10Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f - METER_PER_PIXEL(10));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel10Model) delete pBarrel10Model;

	/////////////////////////////////////////////////////////////
	// front
	CLoadedModelInfo* pBarrel11Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel11Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(10), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel11Model) delete pBarrel11Model;

	CLoadedModelInfo* pBarrel12Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel12Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel12Model) delete pBarrel12Model;

	CLoadedModelInfo* pBarrel13Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel13Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(0), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel13Model) delete pBarrel13Model;

	CLoadedModelInfo* pBarrel14Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel14Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(5), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel14Model) delete pBarrel14Model;

	CLoadedModelInfo* pBarrel15Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Barrel/Barrel.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBarrel15Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f - METER_PER_PIXEL(10), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(50));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();

	if (pBarrel15Model) delete pBarrel15Model;


	/////////////////////////////////////////////////////////////

	//Stop
	CLoadedModelInfo* pStopSign1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Stop/Stop.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pStopSign1Model));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(53), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(12.5));
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, -120.0f, 0.0f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);

	if (pStopSign1Model) delete pStopSign1Model;

	////Ban
	//CLoadedModelInfo* pBanSign1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Ban/Ban.bin", NULL);
	//m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBanSign1Model));
	//m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(45), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(0));
	//m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	//m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);

	//if (pBanSign1Model) delete pBanSign1Model;

	////!
	//CLoadedModelInfo* pExclamationSign1Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/StreetAsset/Exclamation/Exclamation.bin", NULL);
	//m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pExclamationSign1Model));
	//m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + METER_PER_PIXEL(50), m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) + METER_PER_PIXEL(0.2), m_pTerrain->GetLength() * 0.5f + METER_PER_PIXEL(0));
	//m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	//m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);

	//if (pExclamationSign1Model) delete pExclamationSign1Model;

	CLoadedModelInfo* pLevelUpTableModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, (char*)"Model/LevelUpTable/Stylized_Table2.bin", NULL);
	m_vHierarchicalGameObjects.push_back(new StaticObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLevelUpTableModel));
	m_vHierarchicalGameObjects.back()->SetPosition(m_pTerrain->GetWidth() * 0.5f + 63.f, 10.f + m_pTerrain->GetHeight(m_pTerrain->GetWidth() * 0.5f, m_pTerrain->GetLength() * 0.5f) - 1, m_pTerrain->GetLength() * 0.5f);
	m_vHierarchicalGameObjects.back()->SetWorldTransformBoundingBox();
	m_vHierarchicalGameObjects.back()->Rotate(0.0f, 90.0f, 0.0f);
	m_vHierarchicalGameObjects.back()->SetIsExistBoundingBox(false);
	if (pLevelUpTableModel) delete pLevelUpTableModel;

	// 다른 플레이어
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
	for (int i = 0; i < m_vOtherPlayer.size(); ++i)
	{
		m_vOtherPlayer[i]->m_pMesh = new CMesh(pd3dDevice, pd3dCommandList);
		m_vOtherPlayer[i]->m_pMesh->SetBoundinBoxCenter(XMFLOAT3(0.0f, 9.0f, 0.0f));
		m_vOtherPlayer[i]->m_pMesh->SetBoundinBoxExtents(XMFLOAT3(2.5f, 9.0f, 1.5f));
		m_vOtherPlayer[i]->SetWorldTransformBoundingBox();
		m_vOtherPlayer[i]->SetBoundingScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
		m_vOtherPlayer[i]->CreateBoundingBoxObjectSet(pd3dDevice, pd3dCommandList, m_pBoundingObjects.get());
	}
}
void Scene_Neon::BuildLightsAndMaterials()
{
	CScene::BuildLightsAndMaterials();

	m_nLights = 9;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.0015f, 0.0015f, 0.0015f, 1.0f);

	float terrainCenterX = m_pTerrain->GetWidth() * 0.5f;
	float terrainCenterZ = m_pTerrain->GetLength() * 0.5f;
	SetLight(m_pLights[0], XMFLOAT4(0.002f, 0.001f, 0.0015f, 1.0f), XMFLOAT4(0.9f, 0.4f, 0.6f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX, m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(4), terrainCenterZ), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.001f, 0.0001f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(20), 0);

	SetLight(m_pLights[1], XMFLOAT4(0.0005f, 0.0005f, 0.0005f, 1.0f), XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
			5.0f, (float)cos(XMConvertToRadians(15.0f)), (float)cos(XMConvertToRadians(30.0f)), true, SPOT_LIGHT, METER_PER_PIXEL(15), 0);

	SetLight(m_pLights[2], XMFLOAT4(0.001f, 0.001f, 0.001f, 1.0f), /*XMFLOAT4(0.054f, 0.003f, 0.052f, 1.0f)*/XMFLOAT4(0.055f, 0.05f, 0.13f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),
		0, 0, 0, true, DIRECTIONAL_LIGHT, 0.0f, 0);

	SetLight(m_pLights[3], XMFLOAT4(0.008f, 0.004f, 0.006f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX - METER_PER_PIXEL(50), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ + METER_PER_PIXEL(15)), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.065f, 0.00004f, 0.00002f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(30), 0);

	SetLight(m_pLights[4], XMFLOAT4(0.008f, 0.004f, 0.006f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX - METER_PER_PIXEL(50), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ - METER_PER_PIXEL(15)), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.065f, 0.00004f, 0.00002f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(30), 0);

	SetLight(m_pLights[5], XMFLOAT4(0.008f, 0.004f, 0.006f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX + METER_PER_PIXEL(50), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ + METER_PER_PIXEL(15)), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.065f, 0.00004f, 0.00002f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(30), 0);

	SetLight(m_pLights[6], XMFLOAT4(0.008f, 0.004f, 0.006f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX + METER_PER_PIXEL(50), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ - METER_PER_PIXEL(15)), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.065f, 0.00004f, 0.00002f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(30), 0);

	SetLight(m_pLights[7], XMFLOAT4(0.008f, 0.004f, 0.006f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX - METER_PER_PIXEL(15), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ + METER_PER_PIXEL(50)), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.065f, 0.00004f, 0.00002f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(30), 0);

	SetLight(m_pLights[8], XMFLOAT4(0.008f, 0.004f, 0.006f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
		XMFLOAT3(terrainCenterX + METER_PER_PIXEL(15), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ + METER_PER_PIXEL(50)), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.065f, 0.00004f, 0.00002f),
		0, 0, 0, true, POINT_LIGHT, METER_PER_PIXEL(30), 0);

	//SetLight(m_pLights[3], XMFLOAT4(0.0005f, 0.0005f, 0.0005f, 1.0f), XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
	//	XMFLOAT3(terrainCenterX - METER_PER_PIXEL(50), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ + METER_PER_PIXEL(15)), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
	//	3.0f, (float)cos(XMConvertToRadians(100.0f)), (float)cos(XMConvertToRadians(180.0f)), true, SPOT_LIGHT, METER_PER_PIXEL(25), 0);

	//SetLight(m_pLights[4], XMFLOAT4(0.0005f, 0.0005f, 0.0005f, 1.0f), XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f),
	//	XMFLOAT3(terrainCenterX - METER_PER_PIXEL(100), m_pTerrain->GetHeight(terrainCenterX, terrainCenterZ) + METER_PER_PIXEL(5), terrainCenterZ - METER_PER_PIXEL(15)), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.01f, 0.0001f),
	//	3.0f, (float)cos(XMConvertToRadians(100.0f)), (float)cos(XMConvertToRadians(180.0f)), true, SPOT_LIGHT, METER_PER_PIXEL(25), 0);
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
		if (m_pPlayer->GetCamera()->GetMode() == THIRD_PERSON_CAMERA) break;

		// Change the animation while shooting
		if (((Player_Neon*)m_pPlayer.get())->GetReafObjectType() == CGameObject::Player)
		{
			if (m_pPlayer->m_pSkinnedAnimationController->m_nCurrentTrack == m_pPlayer->m_pSkinnedAnimationController->m_nAnimationBundle[CAnimationController::RUN])
				break;

			if (((Player_Neon*)m_pPlayer.get())->m_nGunType == Player_Neon::Empty)
			{
				((Player_Neon*)m_pPlayer.get())->m_nGunType = Player_Neon::Pistol;
			}
		}

		m_pPlayer->SetReadyFire(true);
		for (int i = 0; i < m_ppShaders.size(); ++i)
		{
			if (m_ppShaders[i]->GetReafShaderType() == CShader::ReafShaderType::PistolBulletShader)
			{
				if (((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fLastTime < ((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fMaxCoolTime) break;

				((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fCoolTime = 0.0f;
			}
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		m_pPlayer->SetReadyFire(false);
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

		if (m_ppShaders[i]->GetReafShaderType() == CShader::ReafShaderType::PistolBulletShader)
		{
			((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fLastTime += m_fElapsedTime;

			if (m_pPlayer->GetReadyFire())
			{
				if (((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fCoolTime <= 0.0f)
				{
					PistolBulletTexturedObjects* pObjectsShader = (PistolBulletTexturedObjects*)m_ppShaders[i];
					XMFLOAT3 rayDirection = m_pPlayer.get()->GetRayDirection();
					XMFLOAT3 startLocation = Vector3::Add(Vector3::Add(m_pPlayer.get()->GetPosition(), m_pPlayer.get()->GetOffset()), Vector3::ScalarProduct(Vector3::Normalize(rayDirection), ((PistolBulletTexturedObjects*)m_ppShaders[i])->OffsetLength, false));
					pObjectsShader->AppendBullet(startLocation, rayDirection, 0); //총알
					((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fCoolTime = ((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fMaxCoolTime;
					((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fLastTime = 0.0f;
					
					m_pPlayer->SetFire(true);
					//std::cout << "Fire!!!" << std::endl;

					//총알 발사 패킷 서버 전송
					SERVER::getInstance().SendShot();
				}
				else
				{
					((PistolBulletTexturedObjects*)m_ppShaders[i])->m_fCoolTime -= m_fElapsedTime;

					m_pPlayer->SetFire(false);
				}
			}
		}
	}

	for (int i = 0; i < m_UIShaders.size(); ++i)
	{
		if (m_UIShaders[i]->GetReafShaderType() != CShader::TextureToScreenShader) break;

		switch (i)
		{
		case HP_LINE:
			{
				((CTextureToScreenShader*)m_UIShaders[i])->SetGauge(((Player_Neon*)m_pPlayer.get())->HP / ((Player_Neon*)m_pPlayer.get())->MAXHP);
			}
			break;
		}
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
		m_pPlayer.get()->Player_ID = m_MyId;
		//printf("%d\n", m_MyId);
	}
	//다른 플레이어 위치 이동 및 애니메이션
	for (int i = 0; i < m_vOtherPlayer.size();++i)
	{
		for (int j = 0; j < MAX_PLAYER;++j)
		{
			int OtherId = m_pOtherPlayerData2[j].id;
			if (m_MyId != OtherId && -1 != OtherId)
			{
				m_vOtherPlayer[i]->Player_ID = OtherId;

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
				//방향 및 이동 ###
				m_vOtherPlayer[i]->SetPosition(m_pOtherPlayerData2[OtherId].position);
				m_vOtherPlayer[i]->SetUpVector(m_pOtherPlayerData2[OtherId].UpVector);
				m_vOtherPlayer[i]->SetRightVector(m_pOtherPlayerData2[OtherId].RightVector);
				m_vOtherPlayer[i]->SetLookVector(m_pOtherPlayerData2[OtherId].LookVector);
				m_vOtherPlayer[i]->SetVelocity(m_pOtherPlayerData2[OtherId].velocity);
				m_vOtherPlayer[i]->Animate(fTimeElapsed);
				++i;

				//총알 발사
				//bool currfire = m_pOtherPlayerData2[OtherId].Fire;
				//if(currfire) std::cout << "OtherPlayer ID: " << OtherId << ", FireState: " << currfire  << ", PrevState: " << m_OtherPlayerPrevFire[OtherId] << std::endl;
				//if(!currfire && m_OtherPlayerPrevFire[OtherId]) std::cout << "PrevOtherPlayer ID: " << OtherId << ", FireState: " << currfire  << ", PrevState: " << m_OtherPlayerPrevFire[OtherId] << std::endl;
				int shoterId = SERVER::getInstance().GetShotClinetId();
				if (shoterId != -1)
				{
					for (int k = 0; k < m_ppShaders.size(); ++k)
					{
						if (m_ppShaders[k]->GetReafShaderType() == CShader::ReafShaderType::PistolBulletShader)
						{
							PistolBulletTexturedObjects* pObjectsShader = (PistolBulletTexturedObjects*)m_ppShaders[k];
							XMFLOAT3 rayDirection = m_pOtherPlayerData2[shoterId].RayDirection;
							XMFLOAT3 startLocation = Vector3::Add(Vector3::Add(m_pOtherPlayerData2[shoterId].position, m_pPlayer.get()->GetOffset()), Vector3::ScalarProduct(Vector3::Normalize(rayDirection), ((PistolBulletTexturedObjects*)m_ppShaders[k])->OffsetLength, false));
							pObjectsShader->AppendBullet(startLocation, rayDirection,1); //총알
							SERVER::getInstance().SetShotClinetId(-1);
							break;
						}
					}
				}
				//m_OtherPlayerPrevFire[OtherId] = currfire;
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
	m_xmf3BoundingScale = pGameObject.GetBoundingScale();
	m_xmf3BoundingLocation = pGameObject.GetBoundingLocation();
	m_IsExistBoundingBox = pGameObject.GetIsExistBoundingBox();
	m_Mass = pGameObject.m_Mass;

	m_nMaterials = 0;
	m_ppMaterials = NULL;

	if (pGameObject.m_pMesh)
	{
		m_pMesh = new CMesh();
		BoundingOrientedBox BoundingBox = pGameObject.m_pMesh->GetBoundingBox();
		m_pMesh->SetBoundinBoxCenter(BoundingBox.Center);
		m_pMesh->SetBoundinBoxExtents(BoundingBox.Extents);
	}
	else
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
void CMonsterMetalon::Conflicted(float damage)
{
	m_fLife -= damage;
	std::cout << "Monster Life: " << m_fLife << std::endl;
}
void CMonsterMetalon::Update(float fTimeElapsed)
{
	// 애니메이션 처리 해야함
	//if (m_pSkinnedAnimationController)
	//{
	//	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	//	Animate(fTimeElapsed);
	//}
	// m_fDriection = Vector3::Normalize(m_fDriection);
	//SetPosition(Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_fDriection, m_fMaxVelocityXZ * fTimeElapsed, false)));
}

void CMonsterMetalon::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	// 애니메이션 처리 해야함
	if (m_pSkinnedAnimationController)
	{
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Animate(fTimeElapsed);
	}
}

void CMonsterMetalon::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();
}

//Dragon
CMonsterDragon::CMonsterDragon()
{
}
CMonsterDragon::CMonsterDragon(const CGameObject& pGameObject)
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

	randomAniTrack = rand() % 4;

	if (pGameObject.m_pChild) m_pChild = new CGameObject(*pGameObject.m_pChild);
	if (pGameObject.m_pSibling) m_pSibling = new CGameObject(*pGameObject.m_pSibling);
}
CMonsterDragon::~CMonsterDragon()
{
}

void CMonsterDragon::SetAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* model)
{
	//m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 5, model);
	//for (int i = 0; i < 5; ++i)
	//{
	//	m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	//	m_pSkinnedAnimationController->SetTrackEnable(i, 0);
	//	m_pSkinnedAnimationController->SetTrackSpeed(i, 5.0f);
	//}
}

void CMonsterDragon::RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CMonsterDragon::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController)
	{
		//printf("Update Dragon\n");
		m_pSkinnedAnimationController->SetTrackEnable(0, 0);
		m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
		Animate(fTimeElapsed);
	}
	// m_fDriection = Vector3::Normalize(m_fDriection);
	//SetPosition(Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_fDriection, m_fMaxVelocityXZ * fTimeElapsed, false)));
}

void CMonsterDragon::ReleaseUploadBuffers()
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
CPistolBulletObject::CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection, int type,bool ismine)
{
	Type = type;
	SetMaterial(0, pMaterial);
	SetPosition(startLocation);
	m_fRayDriection = Vector3::Normalize(rayDirection);
	IsMine = ismine;
}
CPistolBulletObject::CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection, int type)
{
	Type = type;
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
bool CPistolBulletObject::Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects, UINT& nConflicted)
{
	float distance = 0.0f;
	XMFLOAT3 RayOrigin = GetPosition();
	std::vector<CGameObject*>& BoundingObjects = BoundingBoxObjects.GetBoundingObjects();
	for (int i = 0; i < BoundingObjects.size(); ++i)
	{
		if (BoundingObjects[i]->m_Mobility == Moveable && BoundingObjects[i]->GetReafObjectType() != CGameObject::Player)
		{
			BoundingObjects[i]->UpdateWorldTransformBoundingBox();
			if (BoundingObjects[i]->Collide(XMLoadFloat3(&RayOrigin), XMLoadFloat3(&m_fRayDriection), distance))
			{
				nConflicted = i;
				return true;
			}
		}
	}

	return false;
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

	//m_pHPMaterial = new CMaterial();
	//m_pHPMaterial->AddRef();

	CTexture* pHPBarTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pHPBarTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"UI/hpds.dds", 0);
	//m_pHPMaterial->SetTexture(pHPBarTexture);
	CScene::CreateSRVUAVs(pd3dDevice, pHPBarTexture, ROOT_PARAMETER_TEXTURE, true);

	SetMaterial(0, new CMaterial());
	m_ppMaterials[0]->SetShader(new CHPBarShader());
	m_ppMaterials[0]->m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_ppMaterials[0]->m_pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	m_ppMaterials[0]->SetTexture(pHPBarTexture);

	m_pHPObject = new CHPBarTextureObject(pd3dDevice, pd3dCommandList, m_ppMaterials[0], 1000.0f, 1000.0f);

	((CHPBarShader*)m_ppMaterials[0]->m_pShader)->SetInstancingObject(this);
}
NexusObject::~NexusObject()
{
	if (m_pHPObject) m_pHPObject->Release();
}

void NexusObject::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();

	if (m_pHPObject) m_pHPObject->ReleaseUploadBuffers();
}

void NexusObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);

	m_ppMaterials[0]->m_pShader->Render(pd3dCommandList, pCamera);
	m_pHPObject->Render(pd3dCommandList, pCamera, 1, ((CHPBarShader*)m_ppMaterials[0]->m_pShader)->GetInstancingBufferView());
}

//-------------------------------------------------------------------------------
Crosshair::Crosshair(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float fthickness, float flength, float interval, float radDot, bool bDot)
{
	SetMesh(new CCrosshairMesh(pd3dDevice, pd3dCommandList, fthickness, flength, interval, radDot, bDot));
	SetMaterial(0, new CMaterial());
	m_ppMaterials[0]->SetShader(new CCrosshairShader());
	m_ppMaterials[0]->m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_ppMaterials[0]->m_pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	m_IsCrosshair = true;
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

	// GameStart
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/gamestart_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.6, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.6, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/gamestart_b.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.6, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.6, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	// Configuration Settings
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/setting_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.7, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.7, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/setting_b.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.7, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.7, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	// Game Exit
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/quit_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.8, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line_r.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.8, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/quit_b.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 40, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.8, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;
	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/line.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.8, 0);
	pUITexture->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_UIShaders.back() = pUITexture;

	m_UIShaders.push_back(new CShader);
	pUITexture = new CTextureToScreenShader((wchar_t*)L"UI/chip_b.dds");
	pUITexture->CreateRectTexture(pd3dDevice, pd3dCommandList, 300, 35, 0, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT * 0.525, 0);
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
	int mouseX = LOWORD(lParam);
	int mouseY = HIWORD(lParam);

	switch (nMessageID) {
	case WM_LBUTTONDOWN:
		if (bGameStart)
		{
			return true;
		}
		else if (bSetting)
		{

		}
		else if(bGameQuit)
		{
			::PostQuitMessage(0);
		}
		break;
	case WM_MOUSEMOVE:
		if (FRAME_BUFFER_WIDTH / 2 - 150 <= mouseX && mouseX <= FRAME_BUFFER_WIDTH / 2 + 150 && FRAME_BUFFER_HEIGHT * 0.6 - 20 <= mouseY && mouseY <= FRAME_BUFFER_HEIGHT * 0.6 + 20)
		{
			bGameStart = true;
		}
		else
		{
			bGameStart = false;
		}

		if (FRAME_BUFFER_WIDTH / 2 - 150 <= mouseX && mouseX <= FRAME_BUFFER_WIDTH / 2 + 150 && FRAME_BUFFER_HEIGHT * 0.7 - 20 <= mouseY && mouseY <= FRAME_BUFFER_HEIGHT * 0.7 + 20)
		{
			bSetting = true;
		}
		else
		{
			bSetting = false;
		}

		if (FRAME_BUFFER_WIDTH / 2 - 150 <= mouseX && mouseX <= FRAME_BUFFER_WIDTH / 2 + 150 && FRAME_BUFFER_HEIGHT * 0.8 - 20 <= mouseY && mouseY <= FRAME_BUFFER_HEIGHT * 0.8 + 20)
		{
			bGameQuit = true;
		}
		else
		{
			bGameQuit = false;
		}
		break;
	default:
		break;
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
	for (int i = 0; i < m_UIShaders.size(); ++i)
	{
		switch (i)
		{
		case GAMESTART_R:
		case GAMESTART_RL:
			if(!bGameStart)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		case GAMESTART_B:
		case GAMESTART_BL:
			if(bGameStart)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		case SETTING_R:
		case SETTING_RL:
			if (!bSetting)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		case SETTING_B:
		case SETTING_BL:
			if (bSetting)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		case GAMEQUIT_R:
		case GAMEQUIT_RL:
			if (!bGameQuit)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		case GAMEQUIT_B:
		case GAMEQUIT_BL:
			if (bGameQuit)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		case GAME_LODDING:
			if(bLodding)
				m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		default:
			m_UIShaders[i]->Render(pd3dCommandList, pCamera);
			break;
		}
	}
}