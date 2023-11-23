#pragma once
#include "Frameworks.h"

//-------------------------------------------------------------------------------
/*	Concrete LobbyFramework_Test											   */
//-------------------------------------------------------------------------------
class CLobbyFramework_Test : public CLobbyFramework {
public:
	CLobbyFramework_Test(InterfaceFramework& Iframe);
	virtual ~CLobbyFramework_Test();

private:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd) override;
	void FrameAdvance() override;
	void OnDestroy() override;

	void BuildObjects() override;
	void BuildToolCreator() override;
	void ReleaseObjects() override;

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
};

//-------------------------------------------------------------------------------
/*	Concrete GameFramework_Test(Airplane/missile/terrain/skybox etc.)		   */
//-------------------------------------------------------------------------------
class CGameFramework_Test : public CGameFramework {
public:
	CGameFramework_Test(InterfaceFramework& Iframe);
	virtual ~CGameFramework_Test();

	void UpdateUI() const;

private:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd) override;
	void FrameAdvance() override;
	void OnDestroy() override;

	void BuildObjects() override;
	void BuildToolCreator() override;
	void ReleaseObjects() override;

	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

public:
	CGameObject* m_pSelectedObject = NULL;
};