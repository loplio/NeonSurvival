#pragma once
#include "Frameworks.h"

class CGameSource;
class CGameTimer;

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