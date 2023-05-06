#include "stdafx.h"
#include "Framework_Neon.h"
#include "ProcessTool_Neon.h"

//-------------------------------------------------------------------------------
/*	Concrete LobbyFramework_Neon											   */
//-------------------------------------------------------------------------------
CLobbyFramework_Neon::CLobbyFramework_Neon(InterfaceFramework& Iframe) : CLobbyFramework(Iframe)
{
}
CLobbyFramework_Neon::~CLobbyFramework_Neon()
{
}

void CLobbyFramework_Neon::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	BuildObjects();
	BuildToolCreator();
}
void CLobbyFramework_Neon::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	m_KeyboardInput->DataProcessing();
	m_MouseInput->DataProcessing();

	m_DisplayOutput->Render();
	//m_pUILayer->Render(m_nSwapChainBufferIndex);

	m_pdxgiSwapChain.Present(0, 0);

	m_Iframe.MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_Iframe.m_pszFrameRate + 8, 37);
	::SetWindowText(m_hWnd, m_Iframe.m_pszFrameRate);
}
void CLobbyFramework_Neon::OnDestroy()
{
	m_Iframe.WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다.
}

void CLobbyFramework_Neon::BuildObjects()
{
	m_pd3dCommandList.Reset(&m_pd3dCommandAllocator, NULL);

	m_GameSource = new NeonLobbySource(&m_pd3dDevice, &m_pd3dCommandList);

	m_pScene = m_GameSource->GetSharedPtrScene();

	if (m_pScene) m_pScene->BuildObjects(&m_pd3dDevice, &m_pd3dCommandList);

	m_pd3dCommandList.Close();
	ID3D12CommandList* ppd3dCommandLists[] = { &m_pd3dCommandList };
	m_pd3dCommandQueue.ExecuteCommandLists(1, ppd3dCommandLists);

	m_Iframe.WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}
void CLobbyFramework_Neon::BuildToolCreator()
{
	m_pUILayer = new UILayerLobby_Neon(m_Iframe, 1);

	m_KeyboardInput = new LobbyKeyInput_Neon(*this);
	m_MouseInput = new LobbyMouseInput_Neon(m_KeyboardInput->GetKeyBuffer(), *this);
	m_DisplayOutput = new LobbyRenderDisplay_Neon(*this);
}
void CLobbyFramework_Neon::ReleaseObjects()
{
}

void CLobbyFramework_Neon::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		GetCursorPos(&m_MouseInput->GetOldCursorPos());
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
void CLobbyFramework_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_F1:
		case VK_F2:
		case VK_F3:
			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			m_Iframe.ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------------------
/*	Concrete GameFramework_Neon												   */
//-------------------------------------------------------------------------------
CGameFramework_Neon::CGameFramework_Neon(InterfaceFramework& Iframe) : CGameFramework(Iframe)
{
}
CGameFramework_Neon::~CGameFramework_Neon()
{
}

void CGameFramework_Neon::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	BuildObjects();
	BuildToolCreator();
}
void CGameFramework_Neon::FrameAdvance()
{
	m_GameTimer.Tick(60.0f);

	m_KeyboardInput->DataProcessing();
	m_MouseInput->DataProcessing();

	m_ProcessCompute->Collide();
	m_ProcessCompute->Update();
	m_ProcessCompute->Animate();

	m_DisplayOutput->Render();
	//m_pUILayer->Render(m_nSwapChainBufferIndex);

	m_pdxgiSwapChain.Present(0, 0);

	m_Iframe.MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_Iframe.m_pszFrameRate + 8, 37);
	::SetWindowText(m_hWnd, m_Iframe.m_pszFrameRate);
}
void CGameFramework_Neon::OnDestroy()
{
	m_Iframe.WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다.
}

void CGameFramework_Neon::BuildObjects()
{
	m_pd3dCommandList.Reset(&m_pd3dCommandAllocator, NULL);

	m_GameSource = new NeonGameSource(&m_pd3dDevice, &m_pd3dCommandList);

	m_pScene = m_GameSource->GetSharedPtrScene();
	m_pPlayer = m_GameSource->GetSharedPtrPlayer();
	m_pCamera = m_pPlayer->GetCamera();

	if (m_pScene) m_pScene->BuildObjects(&m_pd3dDevice, &m_pd3dCommandList);

	m_pd3dCommandList.Close();
	ID3D12CommandList* ppd3dCommandLists[] = { &m_pd3dCommandList };
	m_pd3dCommandQueue.ExecuteCommandLists(1, ppd3dCommandLists);

	m_Iframe.WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}
void CGameFramework_Neon::BuildToolCreator()
{
	m_pUILayer = new UILayerGame_Neon(m_Iframe, 1);

	m_KeyboardInput = new GameKeyInput_Neon(*this);
	m_MouseInput = new GameMouseInput_Neon(m_KeyboardInput->GetKeyBuffer(), *this);
	m_DisplayOutput = new GameRenderDisplay_Neon(*this);
	m_ProcessCompute = new GameCompute_Neon(m_GameTimer, *m_GameSource);
}
void CGameFramework_Neon::ReleaseObjects()
{
}

void CGameFramework_Neon::UpdateUI() const
{
}

void CGameFramework_Neon::ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta)
{
	if (dwDirection != 0)
	{
		if (dwDirection & DIR_FORWARD) m_pSelectedObject->MoveForward(+1.0f);
		if (dwDirection & DIR_BACKWARD) m_pSelectedObject->MoveForward(-1.0f);
		if (dwDirection & DIR_LEFT) m_pSelectedObject->MoveStrafe(+1.0f);
		if (dwDirection & DIR_RIGHT) m_pSelectedObject->MoveStrafe(-1.0f);
		if (dwDirection & DIR_UP) m_pSelectedObject->MoveUp(+1.0f);
		if (dwDirection & DIR_DOWN) m_pSelectedObject->MoveUp(-1.0f);
	}
	else if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		m_pSelectedObject->Rotate(cyDelta, cxDelta, 0.0f);
	}
}

void CGameFramework_Neon::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		if (m_pPlayer && m_pCamera->GetMode() != SHOULDER_HOLD_CAMERA)
		{
			m_pCamera = m_pPlayer->ChangeCamera((SHOULDER_HOLD_CAMERA), m_GameTimer.GetTimeElapsed());
			m_pPlayer->SetTypeDefine(1);	// 1. GunType - Pistol, 2. GunType - Rifle.
		}
		//m_pSelectedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		//m_bReleaseCapture = false;
		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONUP:
		if (m_pPlayer)
		{
			m_pCamera = m_pPlayer->ChangeCamera((THIRD_PERSON_CAMERA), m_GameTimer.GetTimeElapsed());
			m_pPlayer->SetTypeDefine(0);	// Empty.
		}
		//m_pSelectedObject = NULL;
		break;
	case WM_MOUSEMOVE:
		if (!m_bReleaseCapture)
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			SetCapture(hWnd);
			m_MouseInput->SetOldCursorPos(POINT((rect.right + rect.left) / 2, (rect.bottom + rect.top) / 2));
		}
		break;
	default:
		break;
	}
}
void CGameFramework_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_pPlayer) m_pCamera = m_pPlayer->ChangeCamera((wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			m_Iframe.ChangeSwapChainState();
			break;
		case VK_F11:
			m_bReleaseCapture = true;
			ReleaseCapture();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}