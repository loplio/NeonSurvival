#pragma once

#include "MyMath.h"
#include <memory>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

class Corner {
public:
	XMFLOAT3 LT;
	XMFLOAT3 RT;
	XMFLOAT3 LB;
	XMFLOAT3 RB;

	XMFLOAT3 max;
	XMFLOAT3 min;
};

class Obstacle {
	XMFLOAT4X4 m_xmf4x4World = Matrix4x4::Identity();
	XMFLOAT3 m_xmf3Extents;
	XMFLOAT3 m_xmf3Center;
	Corner corner;
	float Scale = 0.0f;
	float diagonalLength = 0.0f;

public:
	void SetPosition(float x, float y, float z);
	void SetScale(float width, float height, float depth);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	bool IsIntersectingL(XMFLOAT3 start, XMFLOAT3 end);
	bool IsIntersectingV(const XMFLOAT3 start, const XMFLOAT3 direction);
	void SetCorner(XMFLOAT3& Extents, XMFLOAT3& Center, float scale);

	Corner& GetCorner() { return corner; }
};

class TerrainInfo {
	XMFLOAT3 m_xmf3Size;
	XMFLOAT3 m_xmf3Scale;
public:
	TerrainInfo(XMFLOAT3 size, XMFLOAT3 scale) : m_xmf3Size{ size }, m_xmf3Scale{ scale } {}

	XMFLOAT3 GetSize() const { return m_xmf3Size; }
	XMFLOAT3 GetScale() const { return m_xmf3Scale; }
	XMFLOAT3 GetPosition(float x, float y, float z) const {
		XMFLOAT3 xmf3Result;
		xmf3Result.x = m_xmf3Size.x * m_xmf3Scale.x * x;
		xmf3Result.y = m_xmf3Size.y * m_xmf3Scale.y * y;
		xmf3Result.z = m_xmf3Size.z * m_xmf3Scale.z * z;
		return xmf3Result;
	}
	float GetWidth(float x) const { return m_xmf3Size.x * m_xmf3Scale.x * x; }
	float GetLength(float z) const { return m_xmf3Size.z * m_xmf3Scale.z * z; }
};

class Node {
public:
	XMFLOAT3 pos;
	double f, g, h;
	std::shared_ptr<Node> parent;  // Use shared_ptr for ownership

public:
	Node(XMFLOAT3& pos) : pos{ pos }, f(0), g(0), h(0), parent(nullptr) {}

	double transportCost(const Node& other) {
		return std::sqrt(std::pow(pos.x - other.pos.x, 2) + std::pow(pos.z - other.pos.z, 2));
	}

	bool isGoal(Node& other) {
		return Vector3::Length(Vector3::Subtract(other.pos, pos)) < EPSILON;
	}
};

struct Compare {
	bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) {
		return lhs->f > rhs->f;
	}
};

class AStar {
private:
	Obstacle* obstacle;
	int		nObstacle;

public:
	AStar() { obstacle = NULL; nObstacle = 0; }
	AStar(Obstacle* obstacle, int n) :obstacle{ obstacle }, nObstacle{ n } {}
	~AStar() { if (obstacle) delete obstacle; }

	void SetObstacle(Obstacle* other, int n) {
		if (obstacle) delete obstacle;
		nObstacle = n;
		obstacle = other;
	}

	std::vector<std::shared_ptr<Node>> findPath(std::shared_ptr<Node> start, std::shared_ptr<Node> goal)
	{
		std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, Compare> open;
		std::vector<std::shared_ptr<Node>> closed;

		open.push(start);

		while (!open.empty())
		{
			std::shared_ptr<Node> current = open.top();
			open.pop();

			if (current->isGoal(*goal)) {
				std::vector<std::shared_ptr<Node>> path;
				std::shared_ptr<Node> parent = current->parent;

				path.push_back(current);
				while (parent) {
					path.push_back(parent);
					parent = parent->parent;
				}
				std::reverse(path.begin(), path.end());

				return path;
			}

			closed.push_back(current);


			// 이웃 노드 찾기
			std::vector<std::shared_ptr<Node>> neighbors;
			XMFLOAT3& position = current->pos;
			for (int i = 0; i < nObstacle; ++i)
			{
				Corner& corner = obstacle[i].GetCorner();

				bool bLB = true, bRB = true, bLT = true, bRT = true;
				for (int j = 0; j < nObstacle; ++j)
				{
					if (obstacle[j].IsIntersectingL(position, corner.LB))
						bLB = false;
					if (obstacle[j].IsIntersectingL(position, corner.LT))
						bLT = false;
					if (obstacle[j].IsIntersectingL(position, corner.RB))
						bRB = false;
					if (obstacle[j].IsIntersectingL(position, corner.RT))
						bRT = false;
				}

				if (bLB) neighbors.push_back(std::make_shared<Node>(corner.LB));
				if (bLT) neighbors.push_back(std::make_shared<Node>(corner.LT));
				if (bRB) neighbors.push_back(std::make_shared<Node>(corner.RB));
				if (bRT) neighbors.push_back(std::make_shared<Node>(corner.RT));
			}

			bool bEnd = true;
			for (int j = 0; j < nObstacle; ++j)
			{
				if (obstacle[j].IsIntersectingL(position, goal->pos))
					bEnd = false;
			}
			if (bEnd) neighbors.push_back(std::make_shared<Node>(goal->pos));


			for (std::shared_ptr<Node> neighbor : neighbors) {
				if (std::find_if(closed.begin(), closed.end(), [neighbor](const std::shared_ptr<Node>& node) {
					return Vector3::Length(Vector3::Subtract(node->pos, neighbor->pos)) < EPSILON;
					}) != closed.end())
					continue;

					double tentative_g = current->g + current->transportCost(*neighbor);

					if (tentative_g < neighbor->g || std::find(closed.cbegin(), closed.cend(), neighbor) == closed.cend()) {
						neighbor->g = tentative_g;
						neighbor->h = neighbor->transportCost(*goal);
						neighbor->f = neighbor->g + neighbor->h;
						neighbor->parent = current;

						open.push(neighbor);
					}
			}
		}

		// No path found
		return std::vector<std::shared_ptr<Node>>();
	}

	std::vector<XMFLOAT3> GetPath(XMFLOAT3& s, XMFLOAT3& g)
	{
		std::shared_ptr<Node> start = std::make_shared<Node>(s);
		std::shared_ptr<Node> goal = std::make_shared<Node>(g);
		std::vector<std::shared_ptr<Node>> pathNode = findPath(start, goal);
		std::vector<XMFLOAT3> path;

		for (int i = 1; i < pathNode.size(); ++i)
			path.push_back(pathNode[i].get()->pos);

		return path;
	}
	std::vector<XMFLOAT3> GetPath(XMFLOAT3&& s, XMFLOAT3& g)
	{
		std::shared_ptr<Node> start = std::make_shared<Node>(s);
		std::shared_ptr<Node> goal = std::make_shared<Node>(g);
		std::vector<std::shared_ptr<Node>> pathNode = findPath(start, goal);
		std::vector<XMFLOAT3> path;

		for (int i = 1; i < pathNode.size(); ++i)
			path.push_back(pathNode[i].get()->pos);

		return path;
	}

	std::vector<XMFLOAT3> GetStartPath(int n) {
		switch (n)
		{
		case 0:
			return path0;
			break;
		case 1:
			return path1;
			break;
		case 2:
			return path2;
			break;
		}

		return std::vector<XMFLOAT3>();
	}

	std::vector<XMFLOAT3> path0;
	std::vector<XMFLOAT3> path1;
	std::vector<XMFLOAT3> path2;
};

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
	float						m_AnimPosition;
	int							m_PrevState;
	float						m_Speed;
	double						m_SpawnToMoveDelay;
	double						m_SpawnCoolTime;
	XMFLOAT3					m_TargetPos;
	int							m_TargetId;
	int							m_TargetType;
	int							m_SpawnPotalNum;
	int							m_Type;
	bool						m_IsStop;
	float						m_ResetPathTime;

	float						m_AttackCoolTime;
	CGameObject*				m_pParent = NULL;
	CGameObject*				m_pChild = NULL;
	CGameObject*				m_pSibling = NULL;

	std::vector<XMFLOAT3>		m_path;

	int							m_nMaterials;

	enum						{ IDLE, ATTACK, MOVE, DIE, TAKEDAMAGE,NONE };
	enum                        {Nexus,Player};
	enum						{ Dragon, Giant_Bee, Golem, KingCobra, TreasureChest, Spider, Bat, Magama, Treant, Wolf};
	int MonsterHPs[10] =		{   1000,	    150,   500,	      400,		     250,    300, 100,    400,    500,	300};
	float MonsterSpeed[10] =    {   1.0f,      2.0f,  1.2f,      1.5f,          1.4f,   2.2f, 1.5f,  0.7f,   1.0f, 2.3f};
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