#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Launcher.h"
#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

class CBoundingBoxObjects;
class CGameSource;

class CPlayer : public CGameObject {
protected:
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
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();

	// ProcessInput..
	virtual void Rotate(float x, float y, float z);
	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	virtual void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);

	// hold off..
	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override final;
	void ReleaseShaderVariables() override final;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override final;

	// ProcessOutput..
	virtual void Update(float fTimeElapsed);
	void OnPrepareRender() override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;
	void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;

	// Common
	XMFLOAT3 GetPosition() const { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() const { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() const { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() const { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3&& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	void SetPosition(XMFLOAT3&& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetPosition(XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera* GetCamera() const { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return NULL; }

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {};
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	virtual void OnCameraUpdateCallback(float fTimeElapsed) {};
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }
};

class Launcher;
class CAirplanePlayer : public CPlayer {
public:
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;
	Launcher* m_launcher;

public:
	CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nMeshes = 1);
	virtual ~CAirplanePlayer();

	void AddObject(std::vector<CShader*>& Shaders);
	void Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects, XMFLOAT4X4* pxmf4x4Parent = NULL) override;
	void PrepareAnimate() override;
	void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL) override;
	void Update(float fTimeElapsed) override;
	void OnPrepareRender() override;
	void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;
	CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) override;
};

class CTerrainPlayer : public CPlayer {
public:
	CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes = 1);
	virtual ~CTerrainPlayer();

	void OnPlayerUpdateCallback(float fTimeElapsed) override;
	void OnCameraUpdateCallback(float fTimeElapsed) override;
	CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) override;
};