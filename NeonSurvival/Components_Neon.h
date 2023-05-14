#pragma once
#include "Player.h"
#include "Scene.h"
#include "UILayer.h"
#include "Server.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
class Player_Neon : public CPlayer {
public:
	Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes = 1);
	virtual ~Player_Neon();

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false) override;
	void Update(float fTimeElapsed) override;

	void OnPrepareRender() override;

	void OnPlayerUpdateCallback(float fTimeElapsed) override;
	void OnCameraUpdateCallback(float fTimeElapsed) override;
	CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) override;

public:
	enum GunType {
		Empty,
		Pistol,
		Rifle,
		Aim_Rifle
	};

	UINT m_nGunType = Empty;
	void SetTypeDefine(UINT nType) override { m_nGunType = nType; };
};

//-------------------------------------------------------------------------------
/*	Scene																	   */
//-------------------------------------------------------------------------------
class Scene_Neon : public CScene {
public:
	Scene_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~Scene_Neon();

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	void ReleaseShaderVariables() override;

	// Build..
	void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader) override;
	void RunTimeBuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void BuildLightsAndMaterials() override;
	void ReleaseUploadBuffers() override;
	void ReleaseObjects() override;

	// ProcessInput..
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	// ProcessAnimaiton..
	void Update(float fTimeElapsed) override;
	void AnimateObjects(float fTimeElapsed) override;

	// ProcessOutput..
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	void DrawUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;

public:
	PACKET_INGAME2* m_pOtherPlayerData2 = SERVER::getInstance().GetPlayersPosition2();
	int m_MyId = -1;
	bool m_OtherPlayerPrevFire[3] = { false,false,false };
};



//-------------------------------------------------------------------------------
/*	Other Object															   */
//-------------------------------------------------------------------------------
class CParticleObject_Neon : public CParticleObject {
public:
	CParticleObject_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~CParticleObject_Neon();
};

class CPistolBulletObject : public CGameObject {
public:
	CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection);
	virtual ~CPistolBulletObject();

	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void Update(float fTimeElapsed) override;
	void ReleaseUploadBuffers();

public:
	const float m_fSpeed = PIXEL_MPS(35);
	float m_fLifeTime = 0.0f;
	XMFLOAT3 m_fRayDriection;
	//CTexture* m_pRandowmValueTexture = NULL;
};

class NexusObject : public CGameObject {
public:
	NexusObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~NexusObject();
};

class StaticObject : public CGameObject {
public:
	StaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~StaticObject();
};

class MapObject : public CGameObject {
public:
	MapObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~MapObject();
};

class LevelUpTableObject : public CGameObject {
public:
	LevelUpTableObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~LevelUpTableObject();
};

class Crosshair : public CGameObject {
public:
	Crosshair(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float thickness = 0.005f, float length = 0.0125f, float interval = 0.025f, float radDot = 0.0025f, bool bDot = true);
	virtual ~Crosshair();
};

//-------------------------------------------------------------------------------
/*	SceneLobby																   */
//-------------------------------------------------------------------------------
class SceneLobby_Neon : public CScene {
public:
	SceneLobby_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~SceneLobby_Neon() {};

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	void ReleaseShaderVariables() override;

	// Build..
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void BuildLightsAndMaterials() override;
	void ReleaseUploadBuffers() override;
	void ReleaseObjects() override;

	// ProcessInput..
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	// ProcessAnimaiton..
	void AnimateObjects(float fTimeElapsed) override;

	// ProcessOutput..
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	void DrawUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
};