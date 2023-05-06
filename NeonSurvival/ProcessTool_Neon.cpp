#include "stdafx.h"
#include "ProcessTool_Neon.h"
#include "Framework_Neon.h"

#define SPECIAL_SHIFT 0x01
//-------------------------------------------------------------------------------
/*	GameKeyInput_Neon : public KeyboardInput							       */
//-------------------------------------------------------------------------------
GameKeyInput_Neon::GameKeyInput_Neon(CGameFramework& GameFramework) :
	m_Player(GameFramework.GetGameSource().GetRefPlayer()), m_GameTimer(m_InterfaceFramework.GetGameTimer()),
	KeyboardInput(GameFramework)
{
}
GameKeyInput_Neon::~GameKeyInput_Neon()
{
}

void GameKeyInput_Neon::DataProcessing()
{
	SetKeyBuffer();

	UpdateKeyboardState();

	UpdatePlayer();
}

void GameKeyInput_Neon::UpdateKeyboardState()
{
	dwDirection = 0;
	dwSpecialKey = 0;

	if (bInput)
	{
		if (m_KeyBuffer[FORWARD_W] & 0xF0) dwDirection |= DIR_FORWARD;
		if (m_KeyBuffer[BACKWARD_S] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (m_KeyBuffer[LEFT_A] & 0xF0) dwDirection |= DIR_LEFT;
		if (m_KeyBuffer[RIGHT_D] & 0xF0) dwDirection |= DIR_RIGHT;
		if (m_KeyBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
		if (m_KeyBuffer[VK_CONTROL] & 0xF0) dwDirection |= DIR_DOWN;

		if (m_KeyBuffer[VK_SHIFT] & 0xF0) dwSpecialKey |= SPECIAL_SHIFT;
	}
}

void GameKeyInput_Neon::UpdatePlayer()
{
	// �����ʿ�. (���� - ���Ƿ� ���� ����� / �ش� ���� �̵��Ÿ��� ������ ����)
	if (dwDirection) {
		if (!dwSpecialKey) {
			m_Player.SetMaxVelocityXZ(PIXEL_KPH(20));
			m_Player.Move(dwDirection, PIXEL_KPH(15) * m_GameTimer.GetTimeElapsed(), true);
			m_Player.IsDash = false;
		}
		else {
			m_Player.SetMaxVelocityXZ(PIXEL_KPH(40));
			m_Player.Move(dwDirection, PIXEL_KPH(30) * m_GameTimer.GetTimeElapsed(), true);
			m_Player.IsDash = true;
		}
	}
	else {
		m_Player.SetMaxVelocityXZ(PIXEL_KPH(0));
	}
}
//-------------------------------------------------------------------------------
LobbyKeyInput_Neon::LobbyKeyInput_Neon(CLobbyFramework& LobbyFramework) : KeyboardInput(LobbyFramework)
{
}
LobbyKeyInput_Neon::~LobbyKeyInput_Neon()
{
}

void LobbyKeyInput_Neon::DataProcessing()
{
	SetKeyBuffer();

	UpdateKeyboardState();
}

void LobbyKeyInput_Neon::UpdateKeyboardState()
{
}

//-------------------------------------------------------------------------------
/*	GameMouseInput_Neon : public MouseInput								   */
//-------------------------------------------------------------------------------
GameMouseInput_Neon::GameMouseInput_Neon(std::array<UCHAR, 256>& KeyBuffer, CGameFramework& GameFramework) :
	m_hWnd(m_InterfaceFramework.GethWnd()), m_KeyBuffer(KeyBuffer), m_Player(GameFramework.GetGameSource().GetRefPlayer()),
	MouseInput(GameFramework)
{
}
GameMouseInput_Neon::~GameMouseInput_Neon()
{
}

void GameMouseInput_Neon::DataProcessing()
{
	UpdateDelta();

	UpdatePlayer();
}

void GameMouseInput_Neon::UpdateDelta()
{
	POINT ptCursorPos;
	cxDelta = 0.0f, cyDelta = 0.0f;

	// �����ʿ�. (���� - ���Ƿ� ���� ����� / �ش� ���� ������ ������ ����)
	if (GetCapture() == m_hWnd)
	{
		SetCursor(NULL);
		GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}
}

void GameMouseInput_Neon::UpdatePlayer()
{
	if (cxDelta || cyDelta)
	{
		m_Player.Rotate(cyDelta, cxDelta, 0.0f);
	}
}
//-------------------------------------------------------------------------------
LobbyMouseInput_Neon::LobbyMouseInput_Neon(std::array<UCHAR, 256>& KeyBuffer, CLobbyFramework& LobbyFramework) :
	m_KeyBuffer(KeyBuffer),
	MouseInput(LobbyFramework)
{
}
LobbyMouseInput_Neon::~LobbyMouseInput_Neon()
{
}

void LobbyMouseInput_Neon::DataProcessing()
{
}

//-------------------------------------------------------------------------------
/*	GameCompute_Neon														   */
//-------------------------------------------------------------------------------
GameCompute_Neon::GameCompute_Neon(const CGameTimer& GameTimer, const CGameSource& GameSource) :
	m_Scene(GameSource.GetRefScene()), m_Player(GameSource.GetRefPlayer()), m_BBObjects(GameSource.GetRefBBShader()),
	ProcessCompute(GameTimer, GameSource)
{
}

void GameCompute_Neon::Update() const
{
	// Scene Update
	m_Scene.Update(m_GameTimer.GetTotalTime(), m_GameTimer.GetTimeElapsed());

	// Player Update
	m_Player.Update(m_GameTimer.GetTimeElapsed());
}

void GameCompute_Neon::Animate() const
{
	// Scene Animate
	m_Scene.AnimateObjects(m_GameTimer.GetTimeElapsed());

	// Player Animate
	m_Player.Animate(m_GameTimer.GetTimeElapsed(), NULL);
}

void GameCompute_Neon::Collide() const
{
	// Ray Trace
	m_Player.SetViewMatrix();
	if (m_Player.GetCamera()->GetMode() == FIRST_PERSON_CAMERA)
	{
		std::vector<CGameObject*>& BoundingObjects = m_BBObjects.GetBBObject();
		int nIntersected = 0;
		float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
		CGameObject* pSelectedObject = NULL;
		XMFLOAT3 ClientPosition = XMFLOAT3(0.0f, 0.0f, 1.0f);
		for (int i = 0; i < BoundingObjects.size(); ++i)
		{
			XMFLOAT4X4 xmfViewMatrix = m_Player.GetCamera()->GetViewMatrix();

			//std::cout << "Position: " << m_Player.GetPosition().x << ", " << m_Player.GetPosition().y << ", " << m_Player.GetPosition().z << std::endl;
			//std::cout << "Right: " << m_Player.GetRightVector().x << ", " << m_Player.GetRightVector().y << ", " << m_Player.GetRightVector().z << std::endl;
			//std::cout << "Look: " << m_Player.GetLookVector().x << ", " << m_Player.GetLookVector().y << ", " << m_Player.GetLookVector().z << std::endl;
			//std::cout << "Up: " << m_Player.GetUpVector().x << ", " << m_Player.GetUpVector().y << ", " << m_Player.GetUpVector().z << std::endl;
			nIntersected = BoundingObjects[i]->PickObjectByRayIntersection(ClientPosition, xmfViewMatrix, &fHitDistance);
			if ((nIntersected > 0) && (fHitDistance < fNearestHitDistance))
			{
				fNearestHitDistance = fHitDistance;
				pSelectedObject = BoundingObjects[i];

				//std::cout << "Intersect Num: " << nIntersected << ", " << "Index - " << i << std::endl;
			}
		}

	}

	// Collide
	m_Player.Collide(m_GameSource, m_BBObjects, NULL);
}

//-------------------------------------------------------------------------------
/*	GameRenderDisplay_Neon : public DisplayOutput							   */
//-------------------------------------------------------------------------------
GameRenderDisplay_Neon::GameRenderDisplay_Neon(CGameFramework& GameFramework) :
	m_Scene(m_GameSource.GetRefScene()),
	m_Player(m_GameSource.GetRefPlayer()),
	m_BoundingBox(m_GameSource.GetRefBBShader()),
	DisplayOutput(GameFramework)
{
}
GameRenderDisplay_Neon::~GameRenderDisplay_Neon()
{
}

void GameRenderDisplay_Neon::Render()
{
	CCamera* Camera = m_Player.GetCamera();

	m_InterfaceFramework.ClearDisplay();

	// Update
	((CGameFramework_Neon*)&gBaseFramework)->UpdateUI();

	// Scene Render
	m_Scene.OnPrepareRender(&m_pd3dCommandList, Camera);
	m_Scene.Render(&m_pd3dCommandList, Camera);

	// Player Render
	m_Player.Render(&m_pd3dCommandList, Camera);

	// UI Draw
	m_Scene.DrawUI(&m_pd3dCommandList, Camera);

	// BoundingBox Render
	m_BoundingBox.Render(&m_pd3dCommandList, Camera);

	m_InterfaceFramework.SynchronizeResourceTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_InterfaceFramework.ExecuteCommand();

	m_Scene.OnPostRenderParticle();
}
//-------------------------------------------------------------------------------
LobbyRenderDisplay_Neon::LobbyRenderDisplay_Neon(CLobbyFramework& LobbyFramework) : 
	m_Scene(m_GameSource.GetRefScene()), m_Camera(m_GameSource.GetRefCamera()),
	DisplayOutput(LobbyFramework)
{
}
LobbyRenderDisplay_Neon::~LobbyRenderDisplay_Neon()
{
}

void LobbyRenderDisplay_Neon::Render()
{
	m_InterfaceFramework.ClearDisplay();

	// Update

	// Render
	m_Scene.OnPrepareRender(&m_pd3dCommandList, &m_Camera);
	m_Scene.DrawUI(&m_pd3dCommandList, &m_Camera);

	m_InterfaceFramework.SynchronizeResourceTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_InterfaceFramework.ExecuteCommand();
}

//-------------------------------------------------------------------------------
/*	UILayerGame_Neon : UILayer	        									   */
//-------------------------------------------------------------------------------
UILayerLobby_Neon::UILayerLobby_Neon(InterfaceFramework& Iframe, UINT nTextBlocks) : UILayer(Iframe, nTextBlocks)
{
	BuildUI();
}
UILayerLobby_Neon::~UILayerLobby_Neon()
{
}

void UILayerLobby_Neon::Render(UINT nFrame)
{
	UILayer::Render(nFrame);
}
void UILayerLobby_Neon::BuildUI()
{
	ID2D1SolidColorBrush* pd2dBrush;
	IDWriteTextFormat* pdwTextFormat;
	D2D1_RECT_F d2dRect;
	WCHAR pstrOutputText[256];

	wcscpy_s(pstrOutputText, 256, L"NeonSign Survival Lobby\n");
	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect = D2D1::RectF(0.0f, m_fHeight - 90.0f, m_fWidth, m_fHeight);

	UpdateTextOutputs(0, pstrOutputText, &d2dRect, pdwTextFormat, pd2dBrush);
}
//-------------------------------------------------------------------------------
UILayerGame_Neon::UILayerGame_Neon(InterfaceFramework& Iframe, UINT nTextBlocks) : UILayer(Iframe, nTextBlocks)
{
	BuildUI();
}
UILayerGame_Neon::~UILayerGame_Neon()
{
}

void UILayerGame_Neon::Render(UINT nFrame)
{
	UILayer::Render(nFrame);
}
void UILayerGame_Neon::BuildUI()
{
	ID2D1SolidColorBrush* pd2dBrush;
	IDWriteTextFormat* pdwTextFormat;
	D2D1_RECT_F d2dRect;
	WCHAR pstrOutputText[256];

	wcscpy_s(pstrOutputText, 256, L" \n");
	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect = D2D1::RectF(0.0f, 0.0f, m_fWidth, m_fHeight);

	UpdateTextOutputs(0, pstrOutputText, &d2dRect, pdwTextFormat, pd2dBrush);
}
