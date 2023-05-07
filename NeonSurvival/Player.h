#pragma once
#include "GameObject.h"
#include "Camera.h"
#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

class CPlayer : public CGameObject {
protected:
	float m_fRayLength;
	XMFLOAT4X4 m_xmf4x4View;

	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmf3Gravity;
	float m_fMaxVelocityXZ;
	float m_fMaxVelocityY;
	float m_fFriction;

	LPVOID m_pPlayerUpdatedContext;
	LPVOID m_pCameraUpdatedContext;

	CCamera* m_pCamera = NULL;
	CShader* m_pShader = NULL;

public:
	CPlayer();
	virtual ~CPlayer();

	// ProcessCompute.
	virtual void Rotate(float x, float y, float z);
	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	virtual void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	virtual void Update(float fTimeElapsed);

	// ShaderVariable.
	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override final;
	void ReleaseShaderVariables() override final;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override final;

	// ProcessOutput..
	void OnPrepareRender() override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;

	// Others
	virtual void SetTypeDefine(UINT nType) {};

	void SetViewMatrix() {
		m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
		m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

		m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
		m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
		m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
		m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
		m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
		m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
	}
	XMFLOAT4X4& GetViewMatrix() { return m_xmf4x4View; }

	XMFLOAT3 GetPosition() const { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() const { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() const { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() const { return(m_xmf3Right); }

	float GetCurrentVelToMaxVel() { return  sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z) / m_fMaxVelocityXZ; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3&& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetRayLength(float length) { m_fRayLength = length; }

	void SetPosition(XMFLOAT3&& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetPosition(XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	void SetYaw(float Yaw) { m_fYaw = Yaw; }
	void SetPitch(float Pitch) { m_fPitch = Pitch; }
	void SetRoll(float Roll) { m_fRoll = Roll; }

	CCamera* GetCamera() const { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return NULL; }

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {};
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	virtual void OnCameraUpdateCallback(float fTimeElapsed) {};
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

public:
	bool IsDash = false;
};