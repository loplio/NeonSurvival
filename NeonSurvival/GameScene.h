#pragma once
#include "Frameworks.h"
class GameScene;

class GameState {
protected:
	GameScene* gGameScene;

public:
	virtual ~GameState();
	
	virtual void Enter() const = 0;
	virtual void Execute() const = 0;
	virtual void Exit() const = 0;

	void SetGameScene(GameScene* pScene);
};

class GameScene {
	FrameworkController* gFrameworkController;
	GameState* gGameState;

public:
	GameScene();
	virtual ~GameScene();

	void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void FrameAdvance() const;
	void OnDestroy();
	void OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ChangeState(GameState* state);

	FrameworkController* GetFrameworkController();
	GameState* GetGameState();
};

class GameLobby : public GameState {
public:
	GameLobby();
	virtual ~GameLobby();

	void Enter() const override;
	void Execute() const override;
	void Exit() const override;
};

class GamePlay : public GameState {
public:
	GamePlay();
	virtual ~GamePlay();

	void Enter() const override;
	void Execute() const override;
	void Exit() const override;
};