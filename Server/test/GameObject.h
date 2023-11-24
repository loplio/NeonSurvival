#pragma once

#include "MyMath.h"

class CGameObject {
public:
	CGameObject(int nMaterials = 1);
	CGameObject(const CGameObject& pChild);
	virtual ~CGameObject();

private:
	int m_nReferences = 0;

public:
	void AddRef();
	void Release();
	
public:
	XMFLOAT4X4					m_xmf4x4World;
	XMFLOAT4X4					m_xmf4x4Transform;
	XMFLOAT3					m_xmf3Scale;
	XMFLOAT3					m_xmf3PrevScale;
	float						m_Mass;
	int							m_Id;
	int							m_MAXHP;
	int							m_HP;
	int							m_State;
	int							m_PrevState;
	float						m_Speed;
	double						m_SpawnToMoveDelay;
	XMFLOAT3					m_TargetPos;
	int							m_TargetId;
	int							m_TargetType;
	int							m_SpawnPotalNum;
	int							m_Type;
	bool						m_IsStop;

	CGameObject*				m_pParent = NULL;
	CGameObject*				m_pChild = NULL;
	CGameObject*				m_pSibling = NULL;

	int							m_nMaterials;

	enum						{ IDLE, ATTACK, MOVE, DIE, TAKEDAMAGE,NONE };
	enum                        {Nexus,Player};
	enum						{ Dragon, Giant_Bee, Golem, KingCobra, TreasureChest, Spider, Bat, Magama, Treant, Wolf};
	int MonsterHPs[10] =		{   1000,	    150,   500,	      400,		     250,    300, 100,    400,    500,	300};
	enum Mobility				{ Static, Moveable };

public:
	// ShaderVariable.
	virtual void ReleaseShaderVariables();
	void ReleaseUploadBuffers();
	void UpdateShaderVariable(XMFLOAT4X4* pxmf4x4World);
	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	// processcompute..
	virtual void OnPrepareAnimate();
	virtual void Update(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	// processoutput..
	virtual void OnPrepareRender();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	CGameObject* GetRootParentObject();

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	void UpdateMobility(Mobility mobility);
	void SetPrevScale(XMFLOAT4X4* pxmf4x4Parent = NULL);
	
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	
	void Rotate(XMFLOAT4* pxmf4Quaternion);
	void Rotate(XMFLOAT3* pxmfAxis, float fAngle);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void SetScale(float width = 1.f, float height = 1.f, float depth = 1.f);
	void SetScale(const XMFLOAT3& scale);
	void SetMass(float mass);
	void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3&& xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetTransform(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& look, const XMFLOAT3& pos);

	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance);

public:
	CGameObject* GetParent() { return(m_pParent); }
};