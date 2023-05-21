#pragma once
#include "GameObject.h"
#include "Camera.h"
#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

class CPlayer : public DynamicObject {
protected:
	float m_fRayLength;
	XMFLOAT3 m_xmf3RayDirection;
	XMFLOAT4X4 m_xmf4x4View;

	XMFLOAT3 m_xmf3Offset;
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
	std::vector<CGroundObject*>* m_vGroundObjects;

	CCamera* m_pCamera = NULL;
	CShader* m_pShader = NULL;
	
	bool IsFire = false;


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

	void SetViewMatrix();
	XMFLOAT4X4 GetViewMatrix(XMFLOAT3 xmfLook, XMFLOAT3 xmfRight);
	XMFLOAT4X4& GetViewMatrix() { return m_xmf4x4View; }

	XMFLOAT3 GetOffset() const { return m_xmf3Offset; }
	XMFLOAT3 GetPosition() const { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() const { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() const { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() const { return(m_xmf3Right); }

	void SetLookVector(XMFLOAT3& LookVector) { m_xmf3Look = LookVector; }
	void SetUpVector(XMFLOAT3& UpVector) { m_xmf3Up = UpVector; }
	void SetRightVector(XMFLOAT3& RightVector) { m_xmf3Right = RightVector; }

	float GetCurrentVelToMaxVel() { return  sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z) / m_fMaxVelocityXZ; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3&& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetRayLength(float fLength) { m_fRayLength = fLength; }
	void SetRayDirection(XMFLOAT3 xmf3RayDirection) { m_xmf3RayDirection = xmf3RayDirection; }

	void SetOffset(XMFLOAT3&& xmf3Offset) { m_xmf3Offset = xmf3Offset; }
	void SetPosition(XMFLOAT3&& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetPosition(XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }
	float GetRayLength() { return m_fRayLength; }
	XMFLOAT3 GetRayDirection() { return m_xmf3RayDirection; }
	bool GetFire() const { return IsFire; }

	void SetYaw(float Yaw) { m_fYaw = Yaw; }
	void SetPitch(float Pitch) { m_fPitch = Pitch; }
	void SetRoll(float Roll) { m_fRoll = Roll; }
	void SetFire(bool fire) { IsFire = fire; }

	CCamera* GetCamera() const { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return NULL; }

	virtual void OnGroundUpdateCallback(float fTimeElapsed) {};
	void SetGroundUpdatedContext(std::vector<CGroundObject*>* vGroundObjects) { m_vGroundObjects = vGroundObjects; }
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {};
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	virtual void OnCameraUpdateCallback(float fTimeElapsed) {};
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	float GetCameraPitch();
public:
	const float fPitchThickness = 0.2f;
	bool IsDash = false;
	DWORD m_dwDirection = 0;
};