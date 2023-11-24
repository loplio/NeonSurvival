#include "stdafx.h"
#include "GameScene.h"

//-------------------------------------------------------------------------------
//	GameScene 
//-------------------------------------------------------------------------------
GameScene::GameScene()
{
	gFrameworkController = new FrameworkController();
}

GameScene::~GameScene()
{
	if (gGameState) delete gGameState;
	if (gFrameworkController) delete gFrameworkController;
#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

void GameScene::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	gFrameworkController->OnCreate(hInstance, hMainWnd);

	// Determines the initial state.
	ChangeState(new GameLobby());
}

void GameScene::FrameAdvance() const
{
	gGameState->Execute();
}

void GameScene::OnDestroy()
{
	gFrameworkController->OnDestroy();
}

void GameScene::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (gFrameworkController->OnProcessingWindowMessage(hWnd, nMessageID, wParam, lParam)) {
		GetGameState()->Exit();
	}
}

void GameScene::ChangeState(GameState* state)
{
	if (gGameState != nullptr)
		delete gGameState;
	gGameState = state;
	gGameState->SetGameScene(this);
	gGameState->Enter();
}

FrameworkController* GameScene::GetFrameworkController()
{
	return gFrameworkController;
}

GameState* GameScene::GetGameState()
{
	return gGameState;
}

//-------------------------------------------------------------------------------
//	GameState 
//-------------------------------------------------------------------------------
GameState::~GameState()
{
}

void GameState::SetGameScene(GameScene* pScene)
{
	gGameScene = pScene;
}

//-------------------------------------------------------------------------------
//	GameLobby : public GameState 
//-------------------------------------------------------------------------------
GameLobby::GameLobby()
{
}

GameLobby::~GameLobby()
{
}

void GameLobby::Enter() const
{
	gGameScene->GetFrameworkController()->SetStateLobbyFramework();
	gGameScene->GetFrameworkController()->EnterFrameAdvance();
}

void GameLobby::Execute() const
{
	gGameScene->GetFrameworkController()->FrameAdvance();
}

void GameLobby::Exit() const
{
	gGameScene->GetFrameworkController()->ExitFrameAdvance();
	gGameScene->ChangeState(new GamePlay());
}

//-------------------------------------------------------------------------------
//	GamePlay : public GameState
//-------------------------------------------------------------------------------
GamePlay::GamePlay()
{
}

GamePlay::~GamePlay()
{
}

void GamePlay::Enter() const
{
	gGameScene->GetFrameworkController()->SetStateGameFramework();
	gGameScene->GetFrameworkController()->EnterFrameAdvance();
}

void GamePlay::Execute() const
{
	gGameScene->GetFrameworkController()->FrameAdvance();
}

void GamePlay::Exit() const
{
	gGameScene->GetFrameworkController()->ExitFrameAdvance();
	gGameScene->ChangeState(new GameLobby());
}

