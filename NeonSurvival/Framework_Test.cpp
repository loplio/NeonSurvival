#include "stdafx.h"
#include "Framework_Test.h"
#include "ProcessTool_Test.h"

//-------------------------------------------------------------------------------
/*	Concrete LobbyFramework_1												   */
//-------------------------------------------------------------------------------
CLobbyFramework_Test::CLobbyFramework_Test(InterfaceFramework& Iframe) : CLobbyFramework(Iframe)
{
}
CLobbyFramework_Test::~CLobbyFramework_Test()
{
}

void CLobbyFramework_Test::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	BuildObjects();
	BuildToolCreator();
}
void CLobbyFramework_Test::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	m_KeyboardInput->DataProcessing();
	m_MouseInput->DataProcessing();

	m_DisplayOutput->Render();
	m_pUILayer->Render(m_nSwapChainBufferIndex);

	m_pdxgiSwapChain.Present(0, 0);

	m_Iframe.MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_Iframe.m_pszFrameRate + 8, 37);
	::SetWindowText(m_hWnd, m_Iframe.m_pszFrameRate);
}
void CLobbyFramework_Test::OnDestroy()
{
	m_Iframe.WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다.
}

void CLobbyFramework_Test::BuildObjects()
{
	m_pd3dCommandList.Reset(&m_pd3dCommandAllocator, NULL);

	m_pd3dCommandList.Close();
	ID3D12CommandList* ppd3dCommandLists[] = { &m_pd3dCommandList };
	m_pd3dCommandQueue.ExecuteCommandLists(1, ppd3dCommandLists);

	m_Iframe.WaitForGpuComplete();

	m_GameTimer.Reset();
}
void CLobbyFramework_Test::BuildToolCreator()
{
	m_pUILayer = new UILayerLobby_Test(m_Iframe, 1);

	m_KeyboardInput = new LobbyKeyInput_Test(*this);
	m_MouseInput = new LobbyMouseInput_Test(m_KeyboardInput->GetKeyBuffer(), *this);
	m_DisplayOutput = new LobbyRenderDisplay_Test(*this);
}
void CLobbyFramework_Test::ReleaseObjects()
{
}

void CLobbyFramework_Test::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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
void CLobbyFramework_Test::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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
/*	Concrete GameFramework_1(Airplane/missile/terrain/skybox etc.)			   */
//-------------------------------------------------------------------------------
CGameFramework_Test::CGameFramework_Test(InterfaceFramework& Iframe) : CGameFramework(Iframe)
{
}
CGameFramework_Test::~CGameFramework_Test()
{
}

void CGameFramework_Test::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	BuildObjects();
	BuildToolCreator();
}
void CGameFramework_Test::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	m_KeyboardInput->DataProcessing();
	m_MouseInput->DataProcessing();

	m_ProcessCompute->Update();
	m_ProcessCompute->Animate();
	m_ProcessCompute->Collide();

	m_DisplayOutput->Render();
	m_pUILayer->Render(m_nSwapChainBufferIndex);

	m_pdxgiSwapChain.Present(0, 0);

	m_Iframe.MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_Iframe.m_pszFrameRate + 8, 37);
	::SetWindowText(m_hWnd, m_Iframe.m_pszFrameRate);
}
void CGameFramework_Test::OnDestroy()
{
	m_Iframe.WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다.
}

void CGameFramework_Test::BuildObjects()
{
	m_pd3dCommandList.Reset(&m_pd3dCommandAllocator, NULL);

	m_GameSource = new TestGameSource(&m_pd3dDevice, &m_pd3dCommandList);
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
void CGameFramework_Test::BuildToolCreator()
{
	m_pUILayer = new UILayerGame_Test(m_Iframe, 2);

	m_KeyboardInput = new GameKeyInput_Test(*this);
	m_MouseInput = new GameMouseInput_Test(m_KeyboardInput->GetKeyBuffer(), *this);
	m_DisplayOutput = new GameRenderDisplay_Test(*this);
	m_ProcessCompute = new GameCompute_Test(m_GameTimer, *m_GameSource);
}
void CGameFramework_Test::ReleaseObjects()
{
}

void CGameFramework_Test::UpdateUI() const
{
	if (typeid(CAirplanePlayer) == typeid(*m_pPlayer))
	{
		char text[128];
		_itoa_s(missile_num - ((CAirplanePlayer*)m_pPlayer.get())->m_launcher->m_missiles.size(), text, 10);
		memset(text + 1, 0, sizeof(char) * 127);
		m_pUILayer->UpdateTextOutputs(1, (_TCHAR*)text, NULL, NULL, NULL);
	}
}

void CGameFramework_Test::ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta)
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

void CGameFramework_Test::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		m_pSelectedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		SetCapture(hWnd);
		GetCursorPos(&m_MouseInput->GetOldCursorPos());
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		m_pSelectedObject = NULL;
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
void CGameFramework_Test::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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
		case 0x46:
			if (typeid(CAirplanePlayer) == typeid(*m_pPlayer)) {
				((CAirplanePlayer*)m_pPlayer.get())->m_launcher->state += add_missile;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

