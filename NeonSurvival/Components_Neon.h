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

	void CreateMonsters(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,int x);
	void CreateMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,char* model,int x,int z);

public:
	PACKET_INGAME2* m_pOtherPlayerData2 = SERVER::getInstance().GetPlayersPosition2();
	PACKET_MONSTERDATA* m_pMonsterData = SERVER::getInstance().GetMonsterData();
	int m_MyId = -1;
	bool m_OtherPlayerPrevFire[3] = { false,false,false };

	float prevangle = 0;
	XMFLOAT3 m_NexusModelPos;
	XMFLOAT3 m_SpawnPotal_Pos[4];
	MonsterMetalonObjects* pMetalonShader = new MonsterMetalonObjects(); //¸ó½ºÅÍ
	PistolBulletTexturedObjects* pBullets = NULL;		//ÃÑ¾Ë
	bool MonsterRotate[30] = { false, };
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
	CPistolBulletObject(CMaterial* pMaterial, XMFLOAT3& startLocation, XMFLOAT3& rayDirection,int type);
	virtual ~CPistolBulletObject();

	void RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void Update(float fTimeElapsed) override;
	void ReleaseUploadBuffers();

public:
	const float m_fSpeed = PIXEL_MPS(35);
	float m_fLifeTime = 0.0f;
	XMFLOAT3 m_fRayDriection;
	int Type;
	//CTexture* m_pRandowmValueTexture = NULL;
};

class CRectTextureObject : public StaticObject {
public:
	CRectTextureObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);
	virtual ~CRectTextureObject();

	void Update(float fTimeElapsed) override;
};

class NexusObject : public DynamicObject {
public:
	NexusObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~NexusObject();
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

public:
	bool on1 = false;
	bool on2 = false;
	bool on3 = false;
};
