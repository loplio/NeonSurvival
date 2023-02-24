#pragma once
#include "Frameworks.h"

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