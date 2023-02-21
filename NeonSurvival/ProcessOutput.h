#pragma once
#include "Frameworks.h"

class CGameSource;
class CCamera;
class CScene;
class CPlayer;
class CGameTimer;
class CBoundingBoxShader;

class ProcessOutput {
protected:
	BaseFramework& gBaseFramework;
	InterfaceFramework& m_InterfaceFramework;

public:
	ProcessOutput(BaseFramework& Framework) : gBaseFramework(Framework), m_InterfaceFramework(Framework.GetInterfaceFramework()) {}
	virtual void Render() = 0;
};

//-------------------------------------------------------------------------------
//	DisplayOutput
//-------------------------------------------------------------------------------
class DisplayOutput : public ProcessOutput {
protected:
	const CGameTimer& m_GameTimer;
	const CGameSource& m_GameSource;
	ID3D12Device& m_pd3dDevice;
	ID3D12GraphicsCommandList& m_pd3dCommandList;

public:
	DisplayOutput(BaseFramework& Framework);
	virtual ~DisplayOutput();

	void Render() override;
};

//-------------------------------------------------------------------------------
//	Concrete DisplayOutput
//-------------------------------------------------------------------------------
class RenderDisplay_Game1 : public DisplayOutput {
	CScene& m_Scene;
	CPlayer& m_Player;
	CBoundingBoxShader& m_BoundingBox;

public:
	RenderDisplay_Game1(CGameFramework& GameFramework);
	virtual ~RenderDisplay_Game1();

	void Render() override;
};

class RenderDisplay_Lobby1 : public DisplayOutput {
public:
	RenderDisplay_Lobby1(CLobbyFramework& LobbyFramework);
	virtual ~RenderDisplay_Lobby1();

	void Render() override;
};