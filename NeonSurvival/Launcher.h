#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Player.h"

class Missile;

// 미사일을 관리하는 발사 장치 Class
// Launch Device Class to Manage Missiles
const unsigned short missile_num = 4;
const unsigned int range_of_shots = 2700;

enum {
	zero = 0x00,
	add_missile = 0x01
};

class Launcher {
public:
	Launcher(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const CPlayer& pPlayer);
	~Launcher();

	unsigned short state;
	const CPlayer& m_pPlayer;
	CStandardShader* m_pShader;
	std::list<Missile*> m_missiles;
	std::list<Missile*>::iterator DelMissile(const std::list<Missile*>::iterator& msl);
	void AddMissile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	void Release();
};

class Missile : public CGameObject {
public:
	Missile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const CPlayer& pPlayer);
	virtual ~Missile();

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	int moving_dist = 0;
	float m_msSpeed = 1200.f;
};