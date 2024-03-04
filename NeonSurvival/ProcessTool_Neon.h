#pragma once
#include "ProcessCompute.h"
#include "ProcessOutput.h"
#include "ProcessInput.h"
#include "UILayer.h"

//-------------------------------------------------------------------------------
/*	Concrete KeyboardInput													   */
//-------------------------------------------------------------------------------
class GameKeyInput_Neon : public KeyboardInput {
	const CGameTimer& m_GameTimer;
	CPlayer& m_Player;

public:
	GameKeyInput_Neon(CGameFramework& GameFramework);
	virtual ~GameKeyInput_Neon();

	void DataProcessing() override;

private:
	void UpdateKeyboardState() override;
	void UpdatePlayer();
};

class LobbyKeyInput_Neon : public KeyboardInput {
public:
	LobbyKeyInput_Neon(CLobbyFramework& LobbyFramework);
	virtual ~LobbyKeyInput_Neon();

	void DataProcessing() override;

private:
	void UpdateKeyboardState() override;
};

//-------------------------------------------------------------------------------
/*	Concrete MouseInput														   */
//-------------------------------------------------------------------------------
class GameMouseInput_Neon : public MouseInput {
	const HWND& m_hWnd;
	const std::array<UCHAR, 256>& m_KeyBuffer;
	CPlayer& m_Player;

	float cxDelta = 0.0f, cyDelta = 0.0f;

public:
	GameMouseInput_Neon(std::array<UCHAR, 256>& KeyBuffer, CGameFramework& GameFramework);
	virtual ~GameMouseInput_Neon();

	void DataProcessing() override;

private:
	void UpdateDelta();
	void UpdatePlayer();
};

class LobbyMouseInput_Neon : public MouseInput {
	const std::array<UCHAR, 256>& m_KeyBuffer;

public:
	LobbyMouseInput_Neon(std::array<UCHAR, 256>& KeyBuffer, CLobbyFramework& LobbyFramework);
	virtual ~LobbyMouseInput_Neon();

	void DataProcessing() override;
};

//-------------------------------------------------------------------------------
/*	Concrete Compute													       */
//-------------------------------------------------------------------------------
class GameCompute_Neon : public ProcessCompute {
protected:
	CScene& m_Scene;
	CPlayer& m_Player;
	CBoundingBoxObjects& m_BoundingObjects;

public:
	GameCompute_Neon(const CGameTimer& GameTimer, const CGameSource& GameSource);

	void Update() const override;
	void Animate() const override;
	void Collide() const override;
	void RayTrace() const override;
};

//-------------------------------------------------------------------------------
/*	Concrete DisplayOutput													   */
//-------------------------------------------------------------------------------
class GameRenderDisplay_Neon : public DisplayOutput {
	CScene& m_Scene;
	CPlayer& m_Player;
	CBoundingBoxShader& m_BoundingBox;

public:
	GameRenderDisplay_Neon(CGameFramework& GameFramework);
	virtual ~GameRenderDisplay_Neon();

	void Render() override;
};

class LobbyRenderDisplay_Neon : public DisplayOutput {
	CScene& m_Scene;
	CCamera& m_Camera;

public:
	LobbyRenderDisplay_Neon(CLobbyFramework& LobbyFramework);
	virtual ~LobbyRenderDisplay_Neon();

	void Render() override;
};

//-------------------------------------------------------------------------------
/*	Concrete UILayer_Neon		        									   */
//-------------------------------------------------------------------------------
class UILayerLobby_Neon : public UILayer {
public:
	UILayerLobby_Neon(InterfaceFramework& Iframe, UINT nTextBlocks);
	virtual ~UILayerLobby_Neon();

	void Render(UINT nFrame) override;
	void Render(UINT nFrame, CGameSource* pGameSource) override;
	void BuildUI() override;
};

class UILayerGame_Neon : public UILayer {
public:
	UILayerGame_Neon(InterfaceFramework& Iframe, UINT nTextBlocks);
	virtual ~UILayerGame_Neon();

	void Render(UINT nFrame) override;
	void Render(UINT nFrame, CGameSource* pGameSource) override;
	void BuildUI() override;

	enum UIType {
		ATTACK,
		SPEED,
		HP,
		DEFEAT,
		DEFEAT2,
		PLAYER_ATTACK,
		PLAYER_SPEED
	};
};
