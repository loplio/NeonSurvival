#pragma once
#include "Player.h"
#include "Scene.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
class Player_Neon : public CPlayer {
public:
	Player_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes = 1);
	virtual ~Player_Neon();

	void OnPlayerUpdateCallback(float fTimeElapsed) override;
	void OnCameraUpdateCallback(float fTimeElapsed) override;
	CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) override;
};

//-------------------------------------------------------------------------------
/*	Scene																	   */
//-------------------------------------------------------------------------------
class Scene_Neon : public CScene {
public:
	Scene_Neon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~Scene_Neon();

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	void ReleaseShaderVariables() override;

	// Build..
	void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader) override;
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void BuildLightsAndMaterials() override;
	void ReleaseUploadBuffers() override;
	void ReleaseObjects() override;

	// ProcessInput..
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	// ProcessAnimaiton..
	void AnimateObjects(float fTimeElapsed) override;

	// ProcessOutput..
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
};

