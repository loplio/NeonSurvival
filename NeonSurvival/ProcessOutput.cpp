#include "stdafx.h"
#include "ProcessOutput.h"

//-------------------------------------------------------------------------------
//	DisplayInput
//-------------------------------------------------------------------------------
DisplayOutput::DisplayOutput(BaseFramework& Framework) :
	m_GameSource(Framework.GetGameSource()), m_pd3dCommandList(m_InterfaceFramework.GetGraphicsCommandList()),
	m_pd3dDevice(m_InterfaceFramework.GetDevice()), m_GameTimer(m_InterfaceFramework.GetGameTimer()),
	ProcessOutput(Framework)
{
}

DisplayOutput::~DisplayOutput()
{
}

void DisplayOutput::Render()
{
}