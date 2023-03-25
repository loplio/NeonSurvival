#pragma once

class CGameSource;
class CGameTimer;

class ProcessCompute {
protected:
	const CGameSource& m_GameSource;
	const CGameTimer& m_GameTimer;

public:
	ProcessCompute(const CGameTimer& GameTimer, const CGameSource& GameSource);
	virtual ~ProcessCompute();

	virtual void Update() const = 0;
	virtual void Animate() const = 0;
	virtual void Collide() const = 0;
};