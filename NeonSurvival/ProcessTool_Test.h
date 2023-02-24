#pragma once
#include "ProcessCompute.h"
#include "ProcessOutput.h"
#include "ProcessInput.h"
#include "UILayer.h"

//-------------------------------------------------------------------------------
/*	Concrete KeyboardInput													   */
//-------------------------------------------------------------------------------
class GameKeyInput_Test : public KeyboardInput {
	const CGameTimer& m_GameTimer;
	CPlayer& m_Player;

public:
	GameKeyInput_Test(CGameFramework& GameFramework);
	virtual ~GameKeyInput_Test();

	void DataProcessing() override;

private:
	void UpdateKeyboardState() override;
	void UpdatePlayer();
};

class LobbyKeyInput_Test : public KeyboardInput {
public:
	LobbyKeyInput_Test(CLobbyFramework& LobbyFramework);
	virtual ~LobbyKeyInput_Test();

	void DataProcessing() override;

private:
	void UpdateKeyboardState() override;
};

//-------------------------------------------------------------------------------
/*	Concrete MouseInput														   */
//-------------------------------------------------------------------------------
class GameMouseInput_Test : public MouseInput {
	const HWND& m_hWnd;
	const std::array<UCHAR, 256>& m_KeyBuffer;
	CPlayer& m_Player;

	float cxDelta = 0.0f, cyDelta = 0.0f;

public:
	GameMouseInput_Test(std::array<UCHAR, 256>& KeyBuffer, CGameFramework& GameFramework);
	virtual ~GameMouseInput_Test();

	void DataProcessing() override;

private:
	void UpdateDelta();
	void UpdatePlayer();
};

class LobbyMouseInput_Test : public MouseInput {
	const std::array<UCHAR, 256>& m_KeyBuffer;

public:
	LobbyMouseInput_Test(std::array<UCHAR, 256>& KeyBuffer, CLobbyFramework& LobbyFramework);
	virtual ~LobbyMouseInput_Test();

	void DataProcessing() override;
};

//-------------------------------------------------------------------------------
/*	Concrete Compute													       */
//-------------------------------------------------------------------------------
class GameCompute_Test : public ProcessCompute {
protected:
	CScene& m_Scene;
	CPlayer& m_Player;
	CBoundingBoxObjects& m_BBObjects;

public:
	GameCompute_Test(const CGameTimer& GameTimer, const CGameSource& GameSource);

	void Animate() const override;
	void Collide() const override;
};

//-------------------------------------------------------------------------------
/*	Concrete DisplayOutput													   */
//-------------------------------------------------------------------------------
class GameRenderDisplay_Test : public DisplayOutput {
	CScene& m_Scene;
	CPlayer& m_Player;
	CBoundingBoxShader& m_BoundingBox;

public:
	GameRenderDisplay_Test(CGameFramework& GameFramework);
	virtual ~GameRenderDisplay_Test();

	void Render() override;
};

class LobbyRenderDisplay_Test : public DisplayOutput {
public:
	LobbyRenderDisplay_Test(CLobbyFramework& LobbyFramework);
	virtual ~LobbyRenderDisplay_Test();

	void Render() override;
};

//-------------------------------------------------------------------------------
/*	Concrete UILayer_Test		        									   */
//-------------------------------------------------------------------------------
class UILayerLobby_Test : public UILayer {
public:
	UILayerLobby_Test(InterfaceFramework& Iframe, UINT nTextBlocks);
	virtual ~UILayerLobby_Test();

	void Render(UINT nFrame) override;
	void BuildUI() override;
};

class UILayerGame_Test : public UILayer {
public:
	UILayerGame_Test(InterfaceFramework& Iframe, UINT nTextBlocks);
	virtual ~UILayerGame_Test();

	void Render(UINT nFrame) override;
	void BuildUI() override;
};
