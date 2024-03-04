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

void CLobbyFramework_Neon::InitFramework()
{
	IPBuffer = "";
	bLodding = false;
	
	m_pScene->InitScene(&m_pd3dDevice, &m_pd3dCommandList);
}

void CLobbyFramework_Neon::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	BuildObjects();
	BuildToolCreator();
}
void CLobbyFramework_Neon::EnterFrameAdvance()
{
	m_pSoundManager->PlayBg("play ", "Sound/bg.wav");
}
void CLobbyFramework_Neon::ExitFrameAdvance()
{
	m_pSoundManager->PlayBg("stop ", "Sound/bg.wav");
}
void CLobbyFramework_Neon::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	m_KeyboardInput->DataProcessing();
	m_MouseInput->DataProcessing();

	m_DisplayOutput->Render();
	if(bLodding) m_pUILayer->Render(m_nSwapChainBufferIndex);

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

void CLobbyFramework_Neon::UpdateUI() const
{
	if (bLodding)
	{
		wchar_t text[128];
		for (int i = 0; i < IPBuffer.size(); ++i)
		{
			text[i] = IPBuffer[i];
		}
		memset(text + IPBuffer.size(), 0, sizeof(char) * (128 - IPBuffer.size()));
		m_pUILayer->UpdateTextOutputs(0, (_TCHAR*)text, NULL, NULL, NULL);
	}
}

bool CLobbyFramework_Neon::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int retval = 0;
	if (m_pScene) retval = m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	if (retval) {
		bLodding = true;
		((SceneLobby_Neon*)m_pScene.get())->bLodding = true;
		retval = 0;
	}
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

	return retval;
}
bool CLobbyFramework_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int retval = 0;
	if (m_pScene) retval = m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case  0x30:
		case  0x31:
		case  0x32:
		case  0x33:
		case  0x34:
		case  0x35:
		case  0x36:
		case  0x37:
		case  0x38:
		case  0x39:
			if (bLodding)
			{
				char text[2];
				_itoa_s(wParam - 0x30, text, 10);
				IPBuffer = IPBuffer + text[0];
			}
			break;
		case 0xBE:
			if (bLodding)
			{
				IPBuffer.append(".");
			}
			break;
		case 0x08:
			if (bLodding && !IPBuffer.empty())
			{
				IPBuffer.pop_back();
			}
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			if (IPBuffer.size())
			{
				retval = 1;
				SERVER::getInstance().init(hWnd, (char*)IPBuffer.c_str());

				InitFramework();

				Sleep(1);
			}
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

	return retval;
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

void CGameFramework_Neon::InitFramework()
{
	Player_Neon* pPlayer = (Player_Neon*)m_pPlayer.get();
	m_bReleaseCapture = false;
	m_pScene->InitScene(&m_pd3dDevice, &m_pd3dCommandList);
	pPlayer->InitObject(&m_pd3dCommandList, m_pScene->m_pTerrain);
}

void CGameFramework_Neon::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	BuildObjects();
	BuildToolCreator();
}
void CGameFramework_Neon::EnterFrameAdvance()
{
	//m_pSoundManager->PlayBg("play ", "Sound/slowstep.wav",100);
}
void CGameFramework_Neon::ExitFrameAdvance()
{
	//m_pSoundManager->PlayBg("stop ", "Sound/bg.wav");
}
void CGameFramework_Neon::FrameAdvance()
{
	m_GameTimer.Tick(60.0f);

	m_KeyboardInput->DataProcessing();
	m_MouseInput->DataProcessing();

	m_ProcessCompute->RayTrace();
	m_ProcessCompute->Update();
	m_ProcessCompute->Animate();
	m_ProcessCompute->Collide();

	m_pSoundManager->SoundUpdate(m_GameSource);

	m_DisplayOutput->Render();
	m_pUILayer->Render(m_nSwapChainBufferIndex, m_GameSource);

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
	
	if (m_pScene) m_pScene->BuildObjects(&m_pd3dDevice, &m_pd3dCommandList, m_Iframe.GetRenderTargetDescriptorHeap(), m_Iframe.GetDepthStencilBuffer());

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
	m_pSoundManager = new SoundManager();
	m_pUILayer = new UILayerGame_Neon(m_Iframe, 5);

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
	wchar_t text[128] = L"공격력 10%";
	m_pUILayer->UpdateTextOutputs(0, (_TCHAR*)text, NULL, NULL, NULL);

	wchar_t text2[128] = L"스피드 10%";
	m_pUILayer->UpdateTextOutputs(1, (_TCHAR*)text2, NULL, NULL, NULL);

	wchar_t text3[128] = L"체력회복 30";
	m_pUILayer->UpdateTextOutputs(2, (_TCHAR*)text3, NULL, NULL, NULL);

	wchar_t text4[128] = L"패 배";
	m_pUILayer->UpdateTextOutputs(3, (_TCHAR*)text4, NULL, NULL, NULL);	
	
	wchar_t text5[128] = L"아무 키나 누르세요...";
	m_pUILayer->UpdateTextOutputs(4, (_TCHAR*)text5, NULL, NULL, NULL);
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

bool CGameFramework_Neon::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int retval = 0;
	if (m_pScene) retval = m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		if (m_pPlayer && m_pCamera->GetMode() != SHOULDER_HOLD_CAMERA && !(*m_pPlayer).GetDead())
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
		if (m_pPlayer && !(*m_pPlayer).GetDead())
		{
			m_pCamera = m_pPlayer->ChangeCamera((m_pCamera->GetPrevMode()), m_GameTimer.GetTimeElapsed());
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

	return retval;
}
bool CGameFramework_Neon::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int retval = 0;
	if (m_pScene) retval = m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case 0x56:
			if (m_pPlayer && !(*m_pPlayer).GetDead())
			{
				CCamera* pCamera = m_pPlayer.get()->GetCamera();
				if(pCamera->GetMode() == FIRST_PERSON_CAMERA)
					pCamera = m_pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());
				else if(pCamera->GetMode() == THIRD_PERSON_CAMERA)
					pCamera = m_pPlayer->ChangeCamera(FIRST_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());
			}
			break;
		case VK_ESCAPE:
		{
			SERVER::getInstance().SendExit();
			::PostQuitMessage(0);
		}
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		{
			SERVER::getInstance().SendHit(0,50);
			break;
		}
		case VK_F2:
		case VK_F3:
		case VK_F4:
			m_pPlayer->SetTypeDefine(wParam - VK_F1);	// 1. GunType - Pistol, 2. GunType - Rifle.
			break;
		case VK_F8:
			break;
		case VK_F9:
			m_Iframe.ChangeSwapChainState();
			break;
		case VK_F11:
			if (!m_bReleaseCapture)
			{
				m_bReleaseCapture = true;
				ReleaseCapture();
			}
			else
				m_bReleaseCapture = false;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	if (retval)
	{
		SERVER::getInstance().SendExit();
		InitFramework();
	}

	return retval;
}