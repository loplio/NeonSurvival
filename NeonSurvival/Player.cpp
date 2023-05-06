#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "ShaderObjects.h"
#include "GameSource.h"

CPlayer::CPlayer() : CGameObject()
{
	m_pCamera = NULL;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}
CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CPlayer::ReleaseShaderVariables()
{
	//CGameObject::ReleaseShaderVariables();
	if (m_pShader) m_pShader->Release();
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}
void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

//--ProcessOutput : CPlayer------------------------------------------------------
void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCameraMode = m_pCamera->GetMode();
	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA) || (nCameraMode == SHOULDER_HOLD_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
				XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
				XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}
void CPlayer::Move(ULONG dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		int count = 0;
		ULONG RemainOp = dwDirection;

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
		if (count % 2 == 0) fDistance *= sqrt(count) / count;

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		Move(xmf3Shift, bUpdateVelocity);
	}
}
void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		XMFLOAT3 xmf3Direction = xmf3Shift;
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		float fLength2 = sqrtf(xmf3Shift.x * xmf3Shift.x + xmf3Shift.z * xmf3Shift.z);
		if (fLength < EPSILON)
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		else
		{
			if (fLength2 > EPSILON)
			{
				m_xmf3Velocity.x = xmf3Direction.x * (fLength / fLength2 + 1);
				m_xmf3Velocity.z = xmf3Direction.z * (fLength / fLength2 + 1);
			}
			else
			{
				m_xmf3Velocity.x = xmf3Direction.x;
				m_xmf3Velocity.z = xmf3Direction.z;
			}
		}
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}
}

//--ProcessOutput : CPlayer------------------------------------------------------
void CPlayer::Update(float fTimeElapsed)
{
	// Gravity operation
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	// Distance traveled in elapsed time
	XMFLOAT3 timeElapsedDistance = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(timeElapsedDistance, false);

	// RayTracePosition
	m_xmf3RayStartPosition = Vector3::Add(m_xmf3Position, XMFLOAT3(0.f, METER_PER_PIXEL(1.5), 0.f));
	m_xmf3RayEndPosition = m_xmf3RayStartPosition;	// Require Ray Algorithm.

	// Keep out of the ground and align the player and the camera.
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	DWORD nCameraMode = m_pCamera->GetMode();
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == SHOULDER_HOLD_CAMERA) m_pCamera->Update(m_xmf3RayEndPosition, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == SHOULDER_HOLD_CAMERA) m_pCamera->SetLookAt(m_xmf3RayEndPosition);
	m_pCamera->RegenerateViewMatrix();

	// Decelerated velocity operation
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}
void CPlayer::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;
}
void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//카메라 모드가 3인칭이면 플레이어 객체를 렌더링한다.
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == SHOULDER_HOLD_CAMERA)
	{
		if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
		CGameObject::Render(pd3dCommandList, pCamera);
		//m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
	}
}

//--Common : CPlayer-------------------------------------------------------------
CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	case SHOULDER_HOLD_CAMERA:
		pNewCamera = new CShoulderHoldCamera(m_pCamera);
		break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}
	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}
	if (m_pCamera) delete m_pCamera;
	return(pNewCamera);
}