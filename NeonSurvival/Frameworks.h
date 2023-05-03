#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "Timer.h"

#include "GameSource.h"

class UILayer;
class KeyboardInput;
class MouseInput;
class DisplayOutput;
class ProcessCompute;

class InterfaceFramework {
public:
	_TCHAR						m_pszFrameRate[50];
	int							m_nWndClientWidth;
	int							m_nWndClientHeight;

protected:
	CGameTimer					m_GameTimer;

	HINSTANCE					m_hInstance;
	HWND						m_hWnd;

	IDXGIFactory4*				m_pdxgiFactory;							//DXGI ���丮 �������̽��� ���� �������̴�.
	IDXGISwapChain3*			m_pdxgiSwapChain;						//���� ü�� �������̽��� ���� �������̴�. �ַ� ���÷��̸� �����ϱ� ���Ͽ� �ʿ��ϴ�.
	ID3D12Device*				m_pd3dDevice;							//Direct3D ����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���Ͽ� �ʿ��ϴ�.

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;				//MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�.

	static const UINT			m_nSwapChainBuffers = 2;				//���� ü���� �ĸ� ������ �����̴�
	UINT						m_nSwapChainBufferIndex;				//���� ���� ü���� �ĸ� ���� �ε����̴�.

	ID3D12Resource*				m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];	//���� Ÿ�� ����
	ID3D12DescriptorHeap*		m_pd3dRtvDescriptorHeap;							//������ �� �������̽� ������
	UINT						m_nRtvDescriptorIncrementSize;						//���� Ÿ�� ������ ������ ũ���̴�.
	D3D12_CPU_DESCRIPTOR_HANDLE	m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];

	ID3D12Resource*				m_pd3dDepthStencilBuffer;				//����-���ٽ� ����
	ID3D12DescriptorHeap*		m_pd3dDsvDescriptorHeap;				//������ �� �������̽� ������
	UINT						m_nDsvDescriptorIncrementSize;			//����-���ٽ� ������ ������ ũ���̴�.
	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dDsvDescriptorCPUHandle;

	ID3D12CommandQueue*			m_pd3dCommandQueue;						//��� ť
	ID3D12CommandAllocator*		m_pd3dCommandAllocator;					//��� �Ҵ���
	ID3D12GraphicsCommandList*	m_pd3dCommandList;						//��� ����Ʈ �������̽� �������̴�.

	ID3D12PipelineState*		m_pd3dPipelineState;					//�׷��Ƚ� ���������� ���� ��ü�� ���� �������̽� �������̴�.

	ID3D12Fence*				m_pd3dFence;							//�潺 �������̽� ������
	UINT64						m_nFenceValues[m_nSwapChainBuffers];	//�潺�� ��
	HANDLE						m_hFenceEvent;							//�̺�Ʈ �ڵ��̴�.

public:
	InterfaceFramework();
	virtual ~InterfaceFramework();

	virtual void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	virtual void OnDestroy();

	void ChangeSwapChainState();
	void ClearDisplay(XMFLOAT4 xmfloat4 = XMFLOAT4(0.f, 0.f, 0.f, 0.f));
	void ResetCommand();
	void ExecuteCommand();
	void SynchronizeResourceTransition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void WaitForGpuComplete();	
	void MoveToNextFrame();

	CGameTimer& GetGameTimer() { return m_GameTimer; }
	HWND& GethWnd() { return m_hWnd; }
	UINT& GetSwapChainBufferIndex() { return m_nSwapChainBufferIndex; }
	UINT GetSwapChainBuffersNum() const { return m_nSwapChainBuffers; }
	ID3D12Device& GetDevice() const { return *m_pd3dDevice; }
	IDXGISwapChain3& GetSwapChain() const { return *m_pdxgiSwapChain; }
	ID3D12CommandQueue& GetCommandQueue() const { return *m_pd3dCommandQueue; }
	ID3D12CommandAllocator& GetCommandAllocator() const { return *m_pd3dCommandAllocator; }
	ID3D12GraphicsCommandList& GetGraphicsCommandList() const { return *m_pd3dCommandList; }
	ID3D12Resource** GetRenderTargetBuffers() { return m_ppd3dRenderTargetBuffers; }

private:
	void CreateSwapChain();	
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	void CreateRenderTargetViews();
	void CreateDepthstencilView();
};

class BaseFramework {
protected:
	CGameSource*				m_GameSource;

	UILayer*					m_pUILayer;
	KeyboardInput*				m_KeyboardInput;
	MouseInput*					m_MouseInput;
	DisplayOutput*				m_DisplayOutput;
	ProcessCompute*				m_ProcessCompute;

	InterfaceFramework&			m_Iframe;
	CGameTimer&					m_GameTimer;
	HWND&						m_hWnd;
	UINT&						m_nSwapChainBufferIndex;

	ID3D12Device&				m_pd3dDevice;
	IDXGISwapChain3&			m_pdxgiSwapChain;
	ID3D12CommandQueue&			m_pd3dCommandQueue;
	ID3D12CommandAllocator&		m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList&	m_pd3dCommandList;

public:
	BaseFramework(InterfaceFramework& Iframe);
	virtual ~BaseFramework();

	virtual void OnCreate(HINSTANCE hInstance, HWND hMainWnd) = 0;
	virtual void FrameAdvance() = 0;
	virtual void OnDestroy() = 0;

	virtual void BuildObjects() = 0;
	virtual void BuildToolCreator() = 0;
	virtual void ReleaseObjects() = 0;

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) = 0;

	InterfaceFramework& GetInterfaceFramework() const { return m_Iframe; }
	CGameSource& GetGameSource() const { return *m_GameSource; }
};

//-------------------------------------------------------------------------------
/*	CLobbyFramework : BaseFramework											   */
//-------------------------------------------------------------------------------
class CLobbyFramework : public BaseFramework {
protected:
	std::shared_ptr<CScene>		m_pScene = NULL;

public:
	CLobbyFramework(InterfaceFramework& Iframe);
	virtual ~CLobbyFramework();

private:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd) override {};
	void FrameAdvance() override {};
	void OnDestroy() override {};

	void BuildObjects() override {};
	void BuildToolCreator() override {};
	void ReleaseObjects() override {};

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override {};
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override {};
};

//-------------------------------------------------------------------------------
/*	CGameFramework : BaseFramework											   */
//-------------------------------------------------------------------------------
class CGameFramework : public BaseFramework {
protected:
	CCamera*					m_pCamera = NULL;
	std::shared_ptr<CScene>		m_pScene = NULL;
	std::shared_ptr<CPlayer>	m_pPlayer = NULL;

public:
	CGameFramework(InterfaceFramework& Iframe);
	virtual ~CGameFramework();

private:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd) override {};
	void FrameAdvance() override {};
	void OnDestroy() override {};

	void BuildObjects() override {};
	void BuildToolCreator() override {};
	void ReleaseObjects() override {};

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override {};
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override {};
};

//-------------------------------------------------------------------------------
/*	FrameworkController														   */
//-------------------------------------------------------------------------------
class FrameworkController {
	InterfaceFramework* m_InterfaceFramework;
	CGameFramework*		m_GameFramework;
	CLobbyFramework*	m_LobbyFramework;
	BaseFramework*		m_state;

public:
	FrameworkController();
	virtual ~FrameworkController();

	void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void FrameAdvance() { m_state->FrameAdvance(); };
	void OnDestroy();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) { m_state->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam); }
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) { m_state->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam); }
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects() { m_state->BuildObjects(); }
	void BuildToolCreator() { m_state->BuildToolCreator(); }
	void ReleaseObjects() { m_state->ReleaseObjects(); }

	void SetStateGameFramework() { m_state = m_GameFramework; }
	void SetStateLobbyFramework() { m_state = m_LobbyFramework; }
};