#pragma once

class CGameSource;
class CScene;
class CPlayer;
class CGameTimer;
class CBoundingBoxObjects;

class ProcessCompute {
protected:
	const CGameSource& m_GameSource;
	const CGameTimer& m_GameTimer;

public:
	ProcessCompute(const CGameTimer& GameTimer, const CGameSource& GameSource);
	virtual ~ProcessCompute();

	virtual void Animate() const = 0;
	virtual void Collide() const = 0;
};

//-------------------------------------------------------------------------------
//	DefaultCompute_1
//-------------------------------------------------------------------------------
class DefaultCompute_1 : public ProcessCompute {
protected:
	CScene& m_Scene;
	CPlayer& m_Player;
	CBoundingBoxObjects& m_BBObjects;

public:
	DefaultCompute_1(const CGameTimer& GameTimer, const CGameSource& GameSource);

	void Animate() const override;
	void Collide() const override;
};
