#include "stdafx.h"
#include "ProcessTool_Test.h"
#include "Framework_Test.h"

//-------------------------------------------------------------------------------
/*	GameKeyInput_Test : public KeyboardInput								   */
//-------------------------------------------------------------------------------
GameKeyInput_Test::GameKeyInput_Test(CGameFramework& GameFramework) :
	m_Player(GameFramework.GetGameSource().GetRefPlayer()), m_GameTimer(m_InterfaceFramework.GetGameTimer()),
	KeyboardInput(GameFramework)
{
}
GameKeyInput_Test::~GameKeyInput_Test()
{
}

void GameKeyInput_Test::DataProcessing()
{
	SetKeyBuffer();

	UpdateKeyboardState();

	UpdatePlayer();
}

void GameKeyInput_Test::UpdateKeyboardState()
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

void GameKeyInput_Test::UpdatePlayer()
{
	// 수정필요. (이유 - 임의로 정한 상수값 / 해당 값은 이동거리와 관련이 있음)
	if (dwDirection) m_Player.Move(dwDirection, 350.0f * m_GameTimer.GetTimeElapsed(), true);
}
//-------------------------------------------------------------------------------
LobbyKeyInput_Test::LobbyKeyInput_Test(CLobbyFramework& LobbyFramework) : KeyboardInput(LobbyFramework)
{
}
LobbyKeyInput_Test::~LobbyKeyInput_Test()
{
}

void LobbyKeyInput_Test::DataProcessing()
{
	SetKeyBuffer();

	UpdateKeyboardState();
}

void LobbyKeyInput_Test::UpdateKeyboardState()
{
}

//-------------------------------------------------------------------------------
/*	GameMouseInput_Test : public MouseInput								   */
//-------------------------------------------------------------------------------
GameMouseInput_Test::GameMouseInput_Test(std::array<UCHAR, 256>& KeyBuffer, CGameFramework& GameFramework) :
	m_hWnd(m_InterfaceFramework.GethWnd()), m_KeyBuffer(KeyBuffer), m_Player(GameFramework.GetGameSource().GetRefPlayer()),
	MouseInput(GameFramework)
{
}
GameMouseInput_Test::~GameMouseInput_Test()
{
}

void GameMouseInput_Test::DataProcessing()
{
	UpdateDelta();

	UpdatePlayer();
}

void GameMouseInput_Test::UpdateDelta()
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

void GameMouseInput_Test::UpdatePlayer()
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
LobbyMouseInput_Test::LobbyMouseInput_Test(std::array<UCHAR, 256>& KeyBuffer, CLobbyFramework& LobbyFramework) :
	m_KeyBuffer(KeyBuffer),
	MouseInput(LobbyFramework)
{
}
LobbyMouseInput_Test::~LobbyMouseInput_Test()
{
}

void LobbyMouseInput_Test::DataProcessing()
{
}

//-------------------------------------------------------------------------------
/*	GameCompute_Test														   */
//-------------------------------------------------------------------------------
GameCompute_Test::GameCompute_Test(const CGameTimer& GameTimer, const CGameSource& GameSource) :
	m_Scene(GameSource.GetRefScene()), m_Player(GameSource.GetRefPlayer()), m_BoundingObjects(GameSource.GetRefBBShader()),
	ProcessCompute(GameTimer, GameSource)
{
}

void GameCompute_Test::Update() const
{
	// Player Update
	m_Player.Update(m_GameTimer.GetTimeElapsed());
}

void GameCompute_Test::Animate() const
{
	// Scene Animate
	m_Scene.AnimateObjects(m_GameTimer.GetTimeElapsed());

	// Player Animate
	m_Player.Animate(m_GameTimer.GetTimeElapsed(), NULL);
}

void GameCompute_Test::Collide() const
{
	m_Player.Collide(m_GameSource, m_BoundingObjects, NULL);
}

void GameCompute_Test::RayTrace() const
{
}

//-------------------------------------------------------------------------------
/*	GameRenderDisplay_Test : public DisplayOutput								   */
//-------------------------------------------------------------------------------
GameRenderDisplay_Test::GameRenderDisplay_Test(CGameFramework& GameFramework) :
	m_Scene(m_GameSource.GetRefScene()),
	m_Player(m_GameSource.GetRefPlayer()),
	m_BoundingBox(m_GameSource.GetRefBBShader()),
	DisplayOutput(GameFramework)
{
}
GameRenderDisplay_Test::~GameRenderDisplay_Test()
{
}

void GameRenderDisplay_Test::Render()
{
	CCamera* Camera = m_Player.GetCamera();

	m_InterfaceFramework.ClearDisplay();

	// Update
	//m_Player.Update(m_GameTimer.GetTimeElapsed());
	((CGameFramework_Test*)&gBaseFramework)->UpdateUI();

	// Scene Render
	m_Scene.OnPrepareRender(&m_pd3dCommandList, Camera);
	m_Scene.Render(&m_pd3dCommandList, Camera);

	// Player Render
	m_Player.Render(&m_pd3dCommandList, Camera);

	// BoundingBox Render
	m_BoundingBox.Render(&m_pd3dCommandList, Camera);

	m_InterfaceFramework.ExecuteCommand();
}
//-------------------------------------------------------------------------------
LobbyRenderDisplay_Test::LobbyRenderDisplay_Test(CLobbyFramework& LobbyFramework) : DisplayOutput(LobbyFramework)
{
}
LobbyRenderDisplay_Test::~LobbyRenderDisplay_Test()
{
}

void LobbyRenderDisplay_Test::Render()
{
	m_InterfaceFramework.ClearDisplay(XMFLOAT4(0.0f, 0.125f, 0.3f, 1.0f));

	// Update

	// Render

	//m_InterfaceFramework.SynchronizeResourceTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_InterfaceFramework.ExecuteCommand();
}

//-------------------------------------------------------------------------------
/*	UILayerGame_Test : UILayer	        									   */
//-------------------------------------------------------------------------------
UILayerLobby_Test::UILayerLobby_Test(InterfaceFramework& Iframe, UINT nTextBlocks) : UILayer(Iframe, nTextBlocks)
{
	BuildUI();
}
UILayerLobby_Test::~UILayerLobby_Test()
{
}

void UILayerLobby_Test::Render(UINT nFrame)
{
	UILayer::Render(nFrame);
}
void UILayerLobby_Test::BuildUI()
{
	ID2D1SolidColorBrush* pd2dBrush;
	IDWriteTextFormat* pdwTextFormat;
	D2D1_RECT_F d2dRect;
	WCHAR pstrOutputText[256];

	wcscpy_s(pstrOutputText, 256, L"Test Game Airplane & Missile\n");
	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect = D2D1::RectF(0.0f, m_fHeight - 90.0f, m_fWidth, m_fHeight);

	UpdateTextOutputs(0, pstrOutputText, &d2dRect, pdwTextFormat, pd2dBrush);
}
//-------------------------------------------------------------------------------
UILayerGame_Test::UILayerGame_Test(InterfaceFramework& Iframe, UINT nTextBlocks) : UILayer(Iframe, nTextBlocks)
{
	BuildUI();
}
UILayerGame_Test::~UILayerGame_Test()
{
}

void UILayerGame_Test::Render(UINT nFrame)
{
	UILayer::Render(nFrame);
}
void UILayerGame_Test::BuildUI()
{
	ID2D1SolidColorBrush* pd2dBrush;
	IDWriteTextFormat* pdwTextFormat;
	D2D1_RECT_F d2dRect;
	WCHAR pstrOutputText[256];

	wcscpy_s(pstrOutputText, 256, L"미사일 1개수\n");
	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect = D2D1::RectF(0.0f, m_fHeight - 90.0f, m_fWidth, m_fHeight);

	UpdateTextOutputs(0, pstrOutputText, &d2dRect, pdwTextFormat, pd2dBrush);


	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 15.0f);
	d2dRect = D2D1::RectF(0.0f, m_fHeight - 60.0f, m_fWidth, m_fHeight);

	UpdateTextOutputs(1, NULL, &d2dRect, pdwTextFormat, pd2dBrush);
}
