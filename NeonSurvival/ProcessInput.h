#pragma once
#include "Frameworks.h"

class CPlayer;
class CGameTimer;

class ProcessInput {
protected:
	const BaseFramework& gGameFramework;
	InterfaceFramework& m_InterfaceFramework;

public:
	ProcessInput(const BaseFramework& Framework) : gGameFramework(Framework), m_InterfaceFramework(Framework.GetInterfaceFramework()) {}
	virtual void DataProcessing() = 0;
};

//-------------------------------------------------------------------------------
//	KeyboardInput & MouseInput 
//-------------------------------------------------------------------------------
class KeyboardInput : public ProcessInput {
protected:
	std::array<UCHAR, 256> m_KeyBuffer;
	bool bInput;
	DWORD dwDirection = 0;

public:
	KeyboardInput(const BaseFramework& Framework) : m_KeyBuffer(), bInput(), ProcessInput(Framework) {}
	virtual ~KeyboardInput() {};

	void DataProcessing() override;

	void SetKeyBuffer();
	std::array<UCHAR, 256>& GetKeyBuffer();

protected:
	virtual void UpdateKeyboardState() = 0;
};

class MouseInput : public ProcessInput {
protected:
	POINT m_ptOldCursorPos = { 0, 0 };

public:
	MouseInput(const BaseFramework& Framework) : ProcessInput(Framework) {}
	virtual ~MouseInput() {};

	void DataProcessing() override;

	POINT& GetOldCursorPos();
};

//-------------------------------------------------------------------------------
//	Concrete KeyboardInput
//-------------------------------------------------------------------------------
class PlayerKeyInput_Game1 : public KeyboardInput {
	const CGameTimer& m_GameTimer;
	CPlayer& m_Player;

public:
	PlayerKeyInput_Game1(CGameFramework& GameFramework);
	virtual ~PlayerKeyInput_Game1();

	void DataProcessing() override;

private:
	void UpdateKeyboardState() override;
	void UpdatePlayer();
};

class PlayerKeyInput_Lobby1 : public KeyboardInput {
public:
	PlayerKeyInput_Lobby1(CLobbyFramework& LobbyFramework);
	virtual ~PlayerKeyInput_Lobby1();

	void DataProcessing() override;

private:
	void UpdateKeyboardState() override;
};

//-------------------------------------------------------------------------------
//	Concrete MouseInput 
//-------------------------------------------------------------------------------
class PlayerMouseInput_Game1 : public MouseInput {
	const HWND& m_hWnd;
	const std::array<UCHAR, 256>& m_KeyBuffer;
	CPlayer& m_Player;

	float cxDelta = 0.0f, cyDelta = 0.0f;

public:
	PlayerMouseInput_Game1(std::array<UCHAR, 256>& KeyBuffer, CGameFramework& GameFramework);
	virtual ~PlayerMouseInput_Game1();

	void DataProcessing() override;

private:
	void UpdateDelta();
	void UpdatePlayer();
};

class PlayerMouseInput_Lobby1 : public MouseInput {
	const std::array<UCHAR, 256>& m_KeyBuffer;

public:
	PlayerMouseInput_Lobby1(std::array<UCHAR, 256>& KeyBuffer, CLobbyFramework& LobbyFramework);
	virtual ~PlayerMouseInput_Lobby1();

	void DataProcessing() override;
};