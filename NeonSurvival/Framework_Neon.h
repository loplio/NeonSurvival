#pragma once
#include "Frameworks.h"

//-------------------------------------------------------------------------------
/*	Concrete LobbyFramework_Neon											   */
//-------------------------------------------------------------------------------
class CLobbyFramework_Neon : public CLobbyFramework {
public:
	CLobbyFramework_Neon(InterfaceFramework& Iframe);
	virtual ~CLobbyFramework_Neon();

	void UpdateUI() const;

	std::string IPBuffer;

	bool bLodding = false;

private:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd) override;
	void EnterFrameAdvance() override;
	void ExitFrameAdvance() override;
	void FrameAdvance() override;
	void OnDestroy() override;

	void BuildObjects() override;
	void BuildToolCreator() override;
	void ReleaseObjects() override;

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
};

//-------------------------------------------------------------------------------
/*	Concrete GameFramework_Neon												   */
//-------------------------------------------------------------------------------
class CGameFramework_Neon : public CGameFramework {
public:
	CGameFramework_Neon(InterfaceFramework& Iframe);
	virtual ~CGameFramework_Neon();

	void UpdateUI() const;

private:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd) override;
	void EnterFrameAdvance() override;
	void ExitFrameAdvance() override;
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
	bool m_bReleaseCapture = false;
};