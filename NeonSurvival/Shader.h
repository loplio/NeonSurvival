#pragma once
#include "GameObject.h"
#include "Camera.h"

//struct VS_VB_INSTANCE
//{
//	XMFLOAT4X4 m_xmf4x4Transform;
//	XMFLOAT4 m_xmcColor;
//};

class CShader {
public:
	CShader();
	virtual ~CShader();

private:
	int									m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommadnList, XMFLOAT4X4* pxmf4x4World) { }
	virtual void ReleaseShaderVariables();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState = 0);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);

	virtual void CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }
	virtual void ReleaseUploadBuffers() { }
	virtual CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance) { return NULL; }

	void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	void CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

protected:
	ID3DBlob*							m_pd3dVertexShaderBlob = NULL;
	ID3DBlob*							m_pd3dPixelShaderBlob = NULL;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	ID3D12PipelineState					**m_ppd3dPipelineStates = NULL;
	int									m_nPipelineStates = 0;

	ID3D12DescriptorHeap				*m_pd3dCbvSrvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorNextHandle;

	LPVOID								m_pBBObjects;

	float								m_fElapsedTime = 0.0f;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CBoundingBoxShader : public CShader {
public:
	CBoundingBoxShader();
	virtual ~CBoundingBoxShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader {
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardShader : public CShader {
public:
	CStandardShader();
	virtual ~CStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};

class CTexturedShader : public CShader {
public:
	CTexturedShader();
	virtual ~CTexturedShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedAnimationStandardShader : public CStandardShader
{
public:
	CSkinnedAnimationStandardShader();
	virtual ~CSkinnedAnimationStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};

class CSkinnedAnimationObjectsShader : public CSkinnedAnimationStandardShader
{
public:
	CSkinnedAnimationObjectsShader();
	virtual ~CSkinnedAnimationObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

protected:
	CGameObject**					m_ppObjects = 0;
	int								m_nObjects = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CObjectsShader's Shader

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBillboardShader : public CTexturedShader {
public:
	CBillboardShader();
	virtual ~CBillboardShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
};

class CMultiSpriteShader : public CTexturedShader {
public:
	CMultiSpriteShader();
	virtual ~CMultiSpriteShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};

class CBlendTextureShader : public CTexturedShader {
public:
	CBlendTextureShader();
	virtual ~CBlendTextureShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CPlayerShader : public CShader {
public:
	CPlayerShader();
	virtual ~CPlayerShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	//virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//virtual void ReleaseShaderVariables();
	//virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	//virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);
};

//class CInstancingShader : public CModelObjects {
//public:
//	CInstancingShader();
//	virtual ~CInstancingShader();
//	
//	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
//	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
//	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
//
//	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature);
//	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
//	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
//	virtual void ReleaseShaderVariables();
//	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader = NULL);
//	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState = 0);
//
//protected:
//	ID3D12Resource* m_pd3dcbGameObjects = NULL;
//	VS_VB_INSTANCE* m_pcbMappedGameObjects = NULL;
//};

class CTerrainShader : public CShader {
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature);
};

