#include "stdafx.h"
#include "ProcessInput.h"

//-------------------------------------------------------------------------------
//	KeyboardInput & MouseInput
//-------------------------------------------------------------------------------
void KeyboardInput::DataProcessing()
{
}

void KeyboardInput::SetKeyBuffer()
{
	bInput = GetKeyboardState(PBYTE(&m_KeyBuffer));
}

std::array<UCHAR, 256>& KeyboardInput::GetKeyBuffer()
{
	return m_KeyBuffer;
}

void MouseInput::DataProcessing()
{
}

POINT& MouseInput::GetOldCursorPos()
{
	return m_ptOldCursorPos;
}

//-------------------------------------------------------------------------------
//	PlayerKeyInput_Game1 : public KeyboardInput
//-------------------------------------------------------------------------------
PlayerKeyInput_Game1::PlayerKeyInput_Game1(CGameFramework& GameFramework) :
	m_Player(GameFramework.GetGameSource().GetRefPlayer()), m_GameTimer(m_InterfaceFramework.GetGameTimer()),
	KeyboardInput(GameFramework)
{
}

PlayerKeyInput_Game1::~PlayerKeyInput_Game1()
{
}

void PlayerKeyInput_Game1::DataProcessing()
{
	SetKeyBuffer();

	UpdateKeyboardState();

	UpdatePlayer();
}

void PlayerKeyInput_Game1::UpdateKeyboardState()
{
	dwDirection = 0;

	if (bInput)
	{
		if (m_KeyBuffer[FORWARD_W] & 0xF0) dwDirection |= DIR_FORWARD;
		if (m_KeyBuffer[BACKWARD_S] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (m_KeyBuffer[LEFT_A] & 0xF0) dwDirection |= DIR_LEFT;
		if (m_KeyBuffer[RIGHT_D] & 0xF0) dwDirection |= DIR_RIGHT;
		if (m_KeyBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
		if (m_KeyBuffer[VK_CONTROL] & 0xF0) dwDirection |= DIR_DOWN;
	}
}

void PlayerKeyInput_Game1::UpdatePlayer()
{
	// 수정필요. (이유 - 임의로 정한 상수값 / 해당 값은 이동거리와 관련이 있음)
	if (dwDirection) m_Player.Move(dwDirection, 350.0f * m_GameTimer.GetTimeElapsed(), true);
}
//-------------------------------------------------------------------------------
PlayerKeyInput_Lobby1::PlayerKeyInput_Lobby1(CLobbyFramework& LobbyFramework) : KeyboardInput(LobbyFramework)
{
}

PlayerKeyInput_Lobby1::~PlayerKeyInput_Lobby1()
{
}

void PlayerKeyInput_Lobby1::DataProcessing()
{
	SetKeyBuffer();

	UpdateKeyboardState();
}

void PlayerKeyInput_Lobby1::UpdateKeyboardState()
{
}

//-------------------------------------------------------------------------------
//	PlayerMouseInput_Game1 : public MouseInput
//-------------------------------------------------------------------------------
PlayerMouseInput_Game1::PlayerMouseInput_Game1(std::array<UCHAR, 256>& KeyBuffer, CGameFramework& GameFramework) : 
	m_hWnd(m_InterfaceFramework.GethWnd()), m_KeyBuffer(KeyBuffer), m_Player(GameFramework.GetGameSource().GetRefPlayer()),
	MouseInput(GameFramework)
{
}

PlayerMouseInput_Game1::~PlayerMouseInput_Game1()
{
}

void PlayerMouseInput_Game1::DataProcessing()
{
	UpdateDelta();

	UpdatePlayer();
}

void PlayerMouseInput_Game1::UpdateDelta()
{
	POINT ptCursorPos;
	cxDelta = 0.0f, cyDelta = 0.0f;

	// 수정필요. (이유 - 임의로 정한 상수값 / 해당 값은 감도와 관련이 있음)
	if (GetCapture() == m_hWnd)
	{
		SetCursor(NULL);
		GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}
}

void PlayerMouseInput_Game1::UpdatePlayer()
{
	if (cxDelta || cyDelta)
	{
		if (m_KeyBuffer[VK_RBUTTON] & 0xF0)
			m_Player.Rotate(cyDelta, 0.0f, -cxDelta);
		else
			m_Player.Rotate(cyDelta, cxDelta, 0.0f);
	}
}
//-------------------------------------------------------------------------------
PlayerMouseInput_Lobby1::PlayerMouseInput_Lobby1(std::array<UCHAR, 256>& KeyBuffer, CLobbyFramework& LobbyFramework) :
	m_KeyBuffer(KeyBuffer),
	MouseInput(LobbyFramework)
{
}

PlayerMouseInput_Lobby1::~PlayerMouseInput_Lobby1()
{
}

void PlayerMouseInput_Lobby1::DataProcessing()
{
}