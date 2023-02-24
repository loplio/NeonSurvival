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