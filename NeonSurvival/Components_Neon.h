#pragma once
#include "Player.h"
#include "Scene.h"
#include "UILayer.h"
#include "Server.h"
#include "ShaderObjects.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
class Player_Neon : public CPlayer {
public:
	Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext);
	virtual ~Player_Neon();

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false) override;
	void Update(float fTimeElapsed) override;

	void OnPrepareRender() override;

	void OnGroundUpdateCallback(float fTimeElapsed) override;
	void OnPlayerUpdateCallback(float fTimeElapsed) override;
	void OnCameraUpdateCallback(float fTimeElapsed) override;
	CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) override;

public:
	float HP = 100.0f;
	float MAXHP = 100.0f;
	float EXP = 0.0f;
	float Damge = 30.0f;

	enum GunType {
		Empty,
		Pistol,
		Rifle,
		Aim_Rifle
	};

	UINT m_nGunType = Empty;
	void SetTypeDefine(UINT nType) override { m_nGunType = nType; };
	
	void AddExp(float exp);
	void SetExp(float exp) {EXP = exp; }
	float GetExp() { return EXP; }

	float GetDmg() { return Damge; }

	void UpgradeDmg();
	void UpgradeSpeed();
	void RecoveryHP();
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
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12DescriptorHeap* d3dRtvCPUDescriptorHeap = NULL, ID3D12Resource* d3dDepthStencilBuffer = NULL) override;
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
	PACKET_MONSTERDATA* m_pMonsterData = SERVER::getInstance().GetMonsterData();
	int m_MyId = -1;
	bool m_OtherPlayerPrevFire[3] = { false,false,false };

	float prevangle = 0;
	XMFLOAT3 m_NexusModelPos;
	XMFLOAT3 m_SpawnPotal_Pos[4];
	MonsterMetalonObjects* pMetalonShader = new MonsterMetalonObjects();
	PistolBulletTexturedObjects* pBullets = NULL;		//ÃÑ¾Ë
	bool MonsterRotate[30] = { false, };

	enum UI_LIST {
		EXP_LINE,
		HP_R,
		HP_LINE,
		ENERGE_LINE,
		MAP_FRAME,
		Mini_Map,
		Pick_Frame,
		Pick_Frame_g,
		Pick_Frame_r,
	};
};

//-------------------------------------------------------------------------------
/*	Monster Object															   */
//-------------------------------------------------------------------------------
class CMonsterMetalon : public MonsterObject {
public:
	CMonsterMetalon();
	CMonsterMetalon(const CGameObject& pGameObject);
	virtual ~CMonsterMetalon();

	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void Conflicted(float damage);
	void Update(float fTimeElapsed) override;
	void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL) override;
	void ReleaseUploadBuffers();
public:
	float m_fLife = 100.0f;
	float m_fMaxVelocityXZ = PIXEL_KPH(12);
	XMFLOAT3 m_fDriection = XMFLOAT3(0.0f, 0.0f, 0.0f);
};


class CMonsterDragon : public MonsterObject {
public:
	CMonsterDragon();
	CMonsterDragon(const CGameObject& pGameObject);
	virtual ~CMonsterDragon();

	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void Update(float fTimeElapsed) override;
	void ReleaseUploadBuffers();
	void SetAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* model);
public:
	float m_fLife = 100.0f;
	float m_fMaxVelocityXZ = PIXEL_KPH(12);
	XMFLOAT3 m_fDriection = XMFLOAT3(0.0f, 0.0f, 0.0f);
	int randomAniTrack = 0;
};

//-------------------------------------------------------------------------------
/*	Other Object															   */
//-------------------------------------------------------------------------------
class CParticleObject_Neon : public CParticleObject {
public:
	CParticleObject_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~CParticleObject_Neon();
};

class CPistolBulletObject : public DynamicObject {
public:
	CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection,int type,bool ismine, float fDistanceAtObject, float dmg);
	CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection,int type, float fDistanceAtObject, float dmg);
	virtual ~CPistolBulletObject();

	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void Update(float fTimeElapsed) override;
	bool Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects, UINT& nConflicted) override;
	void ReleaseUploadBuffers();

public:
	float m_fDamege = 0.0f;
	const float m_fSpeed = PIXEL_MPS(40);
	float m_fLifeTime = 0.0f;
	float m_fMaxLifeTime = 2.0f;
	float m_fDistanceAtObject = 0.0f;
	XMFLOAT3 m_fRayDriection;
	int Type;
	bool IsMine;
};

class NexusObject : public DynamicObject {
public:
	NexusObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~NexusObject();

	void ReleaseUploadBuffers() override;

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;

	ReafObjectType GetReafObjectType() override { return Nexus; }

	float HP = 1000.0f;
	float MAXHP = 1000.0f;

	CGameObject* m_pHPObject = NULL;
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
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12DescriptorHeap* d3dRtvCPUDescriptorHeap = NULL, ID3D12Resource* d3dDepthStencilBuffer = NULL) override;
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

public:
	bool bGameStart = false;
	bool bSetting = false;
	bool bGameQuit = false;
	bool bLodding = false;

	enum {
		GAMESTART_R = 2,
		GAMESTART_RL = 3,
		GAMESTART_B = 4,
		GAMESTART_BL = 5,
		SETTING_R = 6,
		SETTING_RL = 7,
		SETTING_B = 8,
		SETTING_BL = 9,
		GAMEQUIT_R = 10,
		GAMEQUIT_RL = 11,
		GAMEQUIT_B = 12,
		GAMEQUIT_BL = 13,
		GAME_LODDING = 14
	};
};
