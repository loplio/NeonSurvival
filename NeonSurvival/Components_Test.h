#pragma once
#include "Player.h"
#include "Scene.h"

//-------------------------------------------------------------------------------
/*	Player																	   */
//-------------------------------------------------------------------------------
class Player_Test : public CPlayer {
public:
	Player_Test(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes = 1);
	virtual ~Player_Test();

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false) override;
	void Update(float fTimeElapsed) override;

	void OnPrepareRender() override;

	void OnPlayerUpdateCallback(float fTimeElapsed) override;
	void OnCameraUpdateCallback(float fTimeElapsed) override;
	CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) override;
};

//-------------------------------------------------------------------------------
/*	Scene																	   */
//-------------------------------------------------------------------------------
class Scene_Test : public CScene {
public:
	Scene_Test(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~Scene_Test();

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

//-------------------------------------------------------------------------------
/*	Objects																	   */
//-------------------------------------------------------------------------------

//--CRotatingObject_1--------------------------------------------------------------
class CRotatingObject : public CGameObject {
public:
	CRotatingObject();
	virtual ~CRotatingObject();

private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

//--COceanObject_1-----------------------------------------------------------------
class COceanObject_1 : public CGameObject {
public:
	COceanObject_1();
	virtual ~COceanObject_1();

	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	void WaterFlows(XMFLOAT4X4* textureUV, float fTime);
};

//--CMultiSpriteObject_1-----------------------------------------------------------
class CMultiSpriteObject_1 : public CGameObject {
public:
	CMultiSpriteObject_1(bool bAct, int m_nCols, int m_nRows);
	virtual ~CMultiSpriteObject_1();

	bool m_bOwner = true;

	float cooltime = 0.5f;

	bool bActive;
	bool temporary;
	float m_fTime = 0.0f;

	int m_nCols = 0;
	int m_nRows = 0;

	XMFLOAT4X4	m_xmf4x4Texture;

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

//--CBillboardObject_1-------------------------------------------------------------
class CBillboardObject_1 : public CGameObject {
public:
	CBillboardObject_1();
	virtual ~CBillboardObject_1();

	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	void SwayInTheWind(XMFLOAT4X4* textureUV, float fTime);

	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;
};