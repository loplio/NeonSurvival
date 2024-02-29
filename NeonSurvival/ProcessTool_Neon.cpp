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
	if (dwDirection & DIR_FORWARD && dwDirection & DIR_BACKWARD)
	{
		dwDirection -= DIR_FORWARD;
		dwDirection -= DIR_BACKWARD;
	}
	if (dwDirection & DIR_LEFT && dwDirection & DIR_RIGHT)
	{
		dwDirection -= DIR_LEFT;
		dwDirection -= DIR_RIGHT;
	}
	
	m_Player.m_dwDirection = dwDirection;
	if (dwDirection) {
		m_Player.IsDash = false;

		if (dwDirection == DIR_FORWARD)	// foward walk
		{
			if (!dwSpecialKey) {
				m_Player.SetMaxVelocityXZ(PIXEL_KPH(18) * m_Player.MaxSpeed);
				m_Player.Move(dwDirection, PIXEL_KPH(15) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
			}
			else {
				m_Player.SetMaxVelocityXZ(PIXEL_KPH(40) * m_Player.MaxSpeed);
				m_Player.Move(dwDirection, PIXEL_KPH(30) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
				m_Player.IsDash = true;
			}
		}
		else if (dwDirection == DIR_BACKWARD)	// backward walk
		{
			m_Player.SetMaxVelocityXZ(PIXEL_KPH(7) * m_Player.MaxSpeed);
			m_Player.Move(dwDirection, PIXEL_KPH(7) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
		}
		else if (dwDirection == DIR_LEFT || dwDirection == DIR_RIGHT)	// left/right walk
		{
			m_Player.SetMaxVelocityXZ(PIXEL_KPH(10) * m_Player.MaxSpeed);
			m_Player.Move(dwDirection, PIXEL_KPH(10) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
		}
		else if ((dwDirection & DIR_LEFT && dwDirection & DIR_BACKWARD) || (dwDirection & DIR_RIGHT && dwDirection & DIR_BACKWARD))	// left/right backward
		{
			m_Player.SetMaxVelocityXZ(PIXEL_KPH(10) * m_Player.MaxSpeed);
			m_Player.Move(dwDirection, PIXEL_KPH(10) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
		}
		else if ((dwDirection & DIR_LEFT && dwDirection & DIR_FORWARD) || (dwDirection & DIR_RIGHT && dwDirection & DIR_FORWARD))	// left/right forward
		{
			if (!dwSpecialKey) {
				m_Player.SetMaxVelocityXZ(PIXEL_KPH(15) * m_Player.MaxSpeed);
				m_Player.Move(dwDirection, PIXEL_KPH(15) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
			}
			else {
				m_Player.SetMaxVelocityXZ(PIXEL_KPH(30) * m_Player.MaxSpeed);
				m_Player.Move(dwDirection, PIXEL_KPH(20) * m_Player.MaxSpeed * m_GameTimer.GetTimeElapsed(), true);
				m_Player.IsDash = true;
			}
		}
		else {
			m_Player.SetMaxVelocityXZ(PIXEL_KPH(0));
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
	m_Scene(GameSource.GetRefScene()), m_Player(GameSource.GetRefPlayer()), m_BoundingObjects(GameSource.GetRefBBShader()),
	ProcessCompute(GameTimer, GameSource)
{
}

void GameCompute_Neon::Update() const
{
	// BoundingBox Update
	m_BoundingObjects.Update(m_GameTimer.GetTimeElapsed());

	// Scene Update
	m_Scene.Update(m_GameTimer.GetTotalTime(), m_GameTimer.GetTimeElapsed());
	m_Scene.Update(m_GameTimer.GetTimeElapsed());

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

void GameCompute_Neon::RayTrace() const
{
	m_Player.SetViewMatrix();
	CCamera* pCamera = m_Player.GetCamera();
	XMFLOAT3 FirePosition = Vector3::Add(m_Player.GetPosition(), m_Player.GetOffset());

	// Crosshair's Ray Trace.
	int nIntersected = 0, accumulate = 0, nSelected = -1;
	float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX, fNearestStaticObjectDistance = METER_PER_PIXEL(80), fNearestMuzzleToObject = FLT_MAX;
	CGameObject* pSelectedObject = NULL;
	XMFLOAT3 ClientPosition = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT4X4 xmfCameraViewMatrix = pCamera->GetViewMatrix();
	std::vector<CGameObject*>& BoundingObjects = m_BoundingObjects.GetBoundingObjects();

	for (int i = 0; i < BoundingObjects.size(); ++i)
	{
		if (BoundingObjects[i]->GetReafObjectType() == CGameObject::Player) continue;

		nIntersected = BoundingObjects[i]->PickObjectByRayIntersection(ClientPosition, xmfCameraViewMatrix, &fHitDistance);
		accumulate += nIntersected;
		if (nIntersected > 0)
		{
			pSelectedObject = BoundingObjects[i];
			if (pCamera->GetMode() == FIRST_PERSON_CAMERA || pCamera->GetMode() == SHOULDER_HOLD_CAMERA)
			{
				float distance = FLT_MAX;
				XMFLOAT3 xmf3PickRayOrigin, xmf3PickRayDirection;
				pSelectedObject->GenerateRayForPicking(ClientPosition, xmfCameraViewMatrix, &xmf3PickRayOrigin, &xmf3PickRayDirection);
				if (pSelectedObject->RayIntersectsTriangle(xmf3PickRayOrigin, xmf3PickRayDirection, distance))
				{
					if (fNearestHitDistance > distance)
					{
						fNearestHitDistance = distance;
						nSelected = i;
						if (BoundingObjects[i]->m_Mobility == CGameObject::Mobility::Static)
							fNearestStaticObjectDistance = distance;
					}
				}
			}
		}
	}

	XMFLOAT3 TargetPosition = Vector3::Add(Vector3::ScalarProduct(pCamera->GetLookVector(), fNearestHitDistance), pCamera->GetPosition());
	if (nSelected >= 0)
	{
		if (BoundingObjects[nSelected]->m_Mobility == CGameObject::Mobility::Static)
		{
			m_Player.bSelectedObject = true;
			m_Player.fDistanceAtObject = Vector3::Length(Vector3::Subtract(TargetPosition, FirePosition));
		}
		else
		{
			XMFLOAT3 NearestStaticObjectPosition = Vector3::Add(Vector3::ScalarProduct(pCamera->GetLookVector(), fNearestStaticObjectDistance), pCamera->GetPosition());
			m_Player.bSelectedObject = true;
			m_Player.fDistanceAtObject = Vector3::Length(Vector3::Subtract(NearestStaticObjectPosition, FirePosition));
		}
		m_Player.SetRayDirection(Vector3::Subtract(TargetPosition, Vector3::Add(m_Player.GetPosition(), m_Player.GetOffset())));
		pCamera->SetRayDirection(Vector3::ScalarProduct(pCamera->GetLookVector(), fNearestHitDistance));
		pCamera->SetRayLength(fNearestHitDistance);
		//std::cout << "" << "Index - " << nSelected << ", Length: " << float(fNearestHitDistance) << std::endl;
	}

	bool bGunRayIntersection = false;
	XMFLOAT3 DefualtTargetPosition = Vector3::Add(pCamera->GetPosition(), Vector3::ScalarProduct(pCamera->GetLookVector(), METER_PER_PIXEL(80)));
	XMFLOAT3 DefualtDirection = Vector3::Normalize(Vector3::Subtract(DefualtTargetPosition, FirePosition));
	for (int i = 0; i < BoundingObjects.size(); ++i)
	{
		if (BoundingObjects[i]->GetReafObjectType() == CGameObject::Player ||
			BoundingObjects[i]->m_Mobility == CGameObject::Mobility::Moveable) continue;

		if (BoundingObjects[i]->m_pMesh && pCamera->GetMode() == SHOULDER_HOLD_CAMERA)
		{
			XMFLOAT4X4 inverseWorldMatrix = Matrix4x4::Inverse(BoundingObjects[i]->m_xmf4x4World);
			XMFLOAT3 vLocalPosition = Vector3::TransformCoord(FirePosition, inverseWorldMatrix);
			inverseWorldMatrix._41 = 0.0f; inverseWorldMatrix._42 = 0.0f; inverseWorldMatrix._43 = 0.0f;
			XMFLOAT3 vLocalDirection = (nSelected > 0) ?
				Vector3::Normalize(Vector3::TransformCoord(Vector3::Subtract(TargetPosition, FirePosition), inverseWorldMatrix)) :
				Vector3::Normalize(Vector3::TransformCoord(DefualtDirection, inverseWorldMatrix));

			nIntersected = BoundingObjects[i]->m_pMesh->CheckRayIntersection(vLocalPosition, vLocalDirection, &fHitDistance, BoundingObjects[i]->m_xmf4x4World);
			if (nIntersected > 0)
			{
				pSelectedObject = BoundingObjects[i];
				if (pCamera->GetMode() == FIRST_PERSON_CAMERA || pCamera->GetMode() == SHOULDER_HOLD_CAMERA)
				{
					float distance = FLT_MAX;
					if (pSelectedObject->RayIntersectsTriangle(vLocalPosition, vLocalDirection, distance))
					{
						if (fNearestMuzzleToObject > distance)
						{
							fNearestMuzzleToObject = distance;
							//std::cout << "index - " << i << ", distance - " << fNearestMuzzleToObject << std::endl;
							bGunRayIntersection = true;
						}
					}
				}
			}
		}
	}
	if(nSelected < 0)
	{
		if (bGunRayIntersection)
		{
			m_Player.bSelectedObject = true;
			m_Player.fDistanceAtObject = fNearestMuzzleToObject;
		}
		m_Player.SetRayDirection(DefualtDirection);
		pCamera->SetRayDirection(pCamera->GetLookVector());
	}

}


void GameCompute_Neon::Collide() const
{
	//XMFLOAT3 displacement = *m_Player.GetDisplacement();

	for (int i = 0; i < m_Scene.m_ppShaders.size(); ++i)
	{
		m_Scene.m_ppShaders[i]->Collide(m_GameSource, m_BoundingObjects);
	}

	// Prepare Collide
	m_Player.UpdateWorldTransformBoundingBox();

	// Collide
	if (m_Player.Collide(m_GameSource, m_BoundingObjects))
	{
		// Apply Sliding.
		//std::cout << "Apply before Pos: " << m_Player.GetPosition().x << ", " << m_Player.GetPosition().y << ", " << m_Player.GetPosition().z << std::endl;
		m_Player.SetOnlyPlayerPosition(*m_Player.GetDisplacement());
		m_Player.SetViewMatrix();
		//std::cout << "Apply after Pos: " << m_Player.GetPosition().x << ", " << m_Player.GetPosition().y << ", " << m_Player.GetPosition().z << std::endl;
		//*m_Player.GetDisplacement() = displacement;/* Vector3::ScalarProduct(displacement, 0.1, false);*/
		//std::cout << "PlayerPosition: " << m_Player.GetPosition().x << ", " << m_Player.GetPosition().y << ", " << m_Player.GetPosition().z << std::endl;
	}
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

	m_InterfaceFramework.ClearDisplay(m_Scene);

	// Runtime Build.
	m_Scene.RunTimeBuildObjects(&m_pd3dDevice, &m_pd3dCommandList);

	// Update
	((CGameFramework_Neon*)&gBaseFramework)->UpdateUI();

	// Scene Render
	m_Scene.OnPrepareRender(&m_pd3dCommandList, Camera);
	m_Scene.Render(&m_pd3dCommandList, Camera);
	//static float time = 0.0f;
	//if (time > 0.5f)
	//{
	//	time = 0.0f;
	//	std::cout << m_Player.GetPosition().x << ", " << m_Player.GetPosition().y << ", " << m_Player.GetPosition().z << std::endl;
	//}
	//time += m_Scene.m_fElapsedTime;
	// Player Render
	m_Player.Render(&m_pd3dCommandList, Camera);

	// UI Draw
	m_Scene.DrawUI(&m_pd3dCommandList, Camera);

	// PostProcessing
	m_Scene.CopyRenderScene(&m_pd3dDevice, &m_pd3dCommandList, m_InterfaceFramework.GetRenderTargetBuffers()[m_InterfaceFramework.GetSwapChainBufferIndex()]);
	m_Scene.OnPreparePostProcessing(&m_pd3dDevice, &m_pd3dCommandList);
	m_Scene.m_pPostProcessingShader->Render(&m_pd3dCommandList, Camera, &m_Scene.m_nDrawOptions);

	// BoundingBox Render
	m_BoundingBox.Render(&m_pd3dCommandList, Camera);
	
	m_Scene.PostRenderParticle(&m_pd3dCommandList);

	m_InterfaceFramework.SynchronizeResourceTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, m_Scene.m_pPostProcessingShader->m_pTexture);
	m_InterfaceFramework.ExecuteCommand();

	m_Scene.OnPostRenderParticle();
	m_Scene.OnPostReleaseUploadBuffers();
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
	m_InterfaceFramework.ClearDisplay(m_Scene);

	// Update
	((CLobbyFramework_Neon*)&gBaseFramework)->UpdateUI();

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
void UILayerLobby_Neon::Render(UINT nFrame, CGameSource* pGameSource)
{
	UILayer::Render(nFrame);
}
void UILayerLobby_Neon::BuildUI()
{
	ID2D1SolidColorBrush* pd2dBrush;
	IDWriteTextFormat* pdwTextFormat;
	D2D1_RECT_F d2dRect;

	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect = D2D1::RectF(0.0f, m_fHeight - 300.0f, m_fWidth, m_fHeight);

	UpdateTextOutputs(0, NULL, &d2dRect, pdwTextFormat, pd2dBrush);
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
void UILayerGame_Neon::Render(UINT nFrame, CGameSource* pGameSource)
{
	if (((CTextureToScreenShader*)pGameSource->GetRefScene().m_UIShaders[Scene_Neon::Pick_Frame])->GetIsRender())
		UILayer::Render(nFrame);
}
void UILayerGame_Neon::BuildUI()
{
	ID2D1SolidColorBrush* pd2dBrush, *pd2dBrush2, *pd2dBrush3;
	IDWriteTextFormat* pdwTextFormat, *pdwTextFormat2, *pdwTextFormat3;
	D2D1_RECT_F d2dRect, d2dRect2, d2dRect3;
	WCHAR pstrOutputText[256];

	wcscpy_s(pstrOutputText, 256, L" \n");
	pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
	pdwTextFormat = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect = D2D1::RectF(-480.0, m_fHeight - 220.0f, m_fWidth, m_fHeight);
	UpdateTextOutputs(0, pstrOutputText, &d2dRect, pdwTextFormat, pd2dBrush);

	pd2dBrush2 = CreateBrush(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
	pdwTextFormat2 = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect2 = D2D1::RectF(0.0f, m_fHeight - 220.0f, m_fWidth, m_fHeight);
	UpdateTextOutputs(1, pstrOutputText, &d2dRect2, pdwTextFormat2, pd2dBrush2);

	pd2dBrush3 = CreateBrush(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));
	pdwTextFormat3 = CreateTextFormat((wchar_t*)L"Arial", m_fHeight / 20.0f);
	d2dRect3 = D2D1::RectF(480.0f, m_fHeight - 220.0f, m_fWidth, m_fHeight);
	UpdateTextOutputs(2, pstrOutputText, &d2dRect3, pdwTextFormat3, pd2dBrush3);
}
