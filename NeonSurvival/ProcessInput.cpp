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

void MouseInput::SetOldCursorPos(POINT pos)
{
	m_ptOldCursorPos.x = pos.x;
	m_ptOldCursorPos.y = pos.y;
}

POINT& MouseInput::GetOldCursorPos()
{
	return m_ptOldCursorPos;
}