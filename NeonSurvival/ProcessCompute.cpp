#include "stdafx.h"
#include "ProcessCompute.h"
#include "GameSource.h"
#include "Timer.h"

ProcessCompute::ProcessCompute(const CGameTimer& GameTimer, const CGameSource& GameSource) : m_GameTimer(GameTimer), m_GameSource(GameSource)
{
}

ProcessCompute::~ProcessCompute()
{
}

//-------------------------------------------------------------------------------
//	DefaultCompute_1
//-------------------------------------------------------------------------------
DefaultCompute_1::DefaultCompute_1(const CGameTimer& GameTimer, const CGameSource& GameSource) :
	m_Scene(GameSource.GetRefScene()), m_Player(GameSource.GetRefPlayer()), m_BBObjects(GameSource.GetRefBBShader()),
	ProcessCompute(GameTimer, GameSource)
{
}

void DefaultCompute_1::Animate() const
{
	// Scene Animate
	m_Scene.AnimateObjects(m_GameTimer.GetTimeElapsed());

	// Player Animate
	m_Player.Animate(m_GameTimer.GetTimeElapsed(), NULL);
}

void DefaultCompute_1::Collide() const
{
	m_Player.Collide(m_GameSource, m_BBObjects, NULL);
}
