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

//-------------------------------------------------------------------------------
//	RenderDisplay_Game1 : public DisplayOutput
//-------------------------------------------------------------------------------
RenderDisplay_Game1::RenderDisplay_Game1(CGameFramework& GameFramework) :
	m_Scene(m_GameSource.GetRefScene()),
	m_Player(m_GameSource.GetRefPlayer()),
	m_BoundingBox(m_GameSource.GetRefBBShader()),
	DisplayOutput(GameFramework)
{
}

RenderDisplay_Game1::~RenderDisplay_Game1()
{
}

void RenderDisplay_Game1::Render()
{
	CCamera* Camera = m_Player.GetCamera();

	m_InterfaceFramework.ClearDisplay();

	// Update
	m_Player.Update(m_GameTimer.GetTimeElapsed());
	((CGameFramework_1*)&gBaseFramework)->UpdateUI();

	// Scene Render
	m_Scene.OnPrepareRender(&m_pd3dCommandList, Camera);
	m_Scene.Render(&m_pd3dCommandList, Camera);

	// Player Render
	m_Player.Render(&m_pd3dDevice, &m_pd3dCommandList, Camera);

	// BoundingBox Render
	m_BoundingBox.Render(&m_pd3dCommandList, Camera);

	m_InterfaceFramework.ExecuteCommand();
}
//-------------------------------------------------------------------------------
RenderDisplay_Lobby1::RenderDisplay_Lobby1(CLobbyFramework& LobbyFramework) : DisplayOutput(LobbyFramework)
{
}

RenderDisplay_Lobby1::~RenderDisplay_Lobby1()
{
}

void RenderDisplay_Lobby1::Render()
{
	m_InterfaceFramework.ClearDisplay();

	// Update

	// Render

	m_InterfaceFramework.SynchronizeResourceTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_InterfaceFramework.ExecuteCommand();
}