#include "Launcher.h"

Launcher::Launcher(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const CPlayer& pPlayer) : m_pPlayer(pPlayer)
{
	m_pShader = new CStandardShader();
	m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}

Launcher::~Launcher()
{
	Release();
	if (m_pShader) m_pShader->Release();
}

void Launcher::AddMissile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

	if (m_missiles.size() < missile_num)
		m_missiles.push_back(new Missile(pd3dDevice, pd3dCommandList, m_pPlayer));
}

std::list<Missile*>::iterator Launcher::DelMissile(const std::list<Missile*>::iterator& msl)
{
	(*msl)->Release();
	return m_missiles.erase(msl);
}

void Launcher::UpdateState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (state & add_missile) AddMissile(pd3dDevice, pd3dCommandList);
	for (std::list<Missile*>::iterator msl = m_missiles.begin(); msl != m_missiles.end(); ++msl)
	{
		if ((*msl)->moving_dist > range_of_shots)
		{
			std::list<Missile*>::iterator iter = DelMissile(msl);
			if (iter != m_missiles.end())
			{
				msl = iter;
				continue;
			}
			else
				break;
		}
	}

	state = zero;
}

void Launcher::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	m_pShader->Render(pd3dCommandList, pCamera);
	for (std::list<Missile*>::iterator msl = m_missiles.begin(); msl != m_missiles.end(); ++msl)
	{
		(*msl)->UpdateTransform(NULL);
		(*msl)->Render(pd3dCommandList, pCamera);
	}
}

void Launcher::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	for (std::list<Missile*>::iterator msl = m_missiles.begin(); msl != m_missiles.end(); ++msl)
	{
		(*msl)->Animate(fTimeElapsed, pxmf4x4Parent);
	}
}

void Launcher::Release()
{
	for (std::list<Missile*>::iterator msl = m_missiles.begin(); msl != m_missiles.end(); ++msl)
	{
		(*msl)->Release();
	}
	m_missiles.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
Missile::Missile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const CPlayer& pPlayer) : CGameObject(1, 1)
{
	SetObjectType(ObjectType::MISSILE_OBJ);

	SetTransform(pPlayer.GetRightVector(), pPlayer.GetUpVector(), pPlayer.GetLookVector(), pPlayer.GetPosition());

	CCubeMeshDiffused* pCubeMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 15.0f);
	SetMesh(pCubeMesh);

	CMaterial* pMaterial = new CMaterial();
	SetMaterial(0, pMaterial);
}

Missile::~Missile()
{
	ReleaseUploadBuffers();
}

void Missile::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	MoveForward(m_msSpeed * fTimeElapsed);
	moving_dist += m_msSpeed * fTimeElapsed;
}
