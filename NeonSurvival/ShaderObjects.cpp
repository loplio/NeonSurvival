#include "stdafx.h"
#include "ShaderObjects.h"
#include "Frameworks.h"

//-------------------------------------------------------------------------------
/*	CBoundingBoxObjects : public CShader									   */
//-------------------------------------------------------------------------------
CBoundingBoxObjects::CBoundingBoxObjects()
{
	m_BoundingObjects.reserve(64);
}
CBoundingBoxObjects::~CBoundingBoxObjects()
{
}

void CBoundingBoxObjects::Update(float fTimeElapsed)
{
	for (int i = 0; i < m_BoundingObjects.size(); ++i)
	{
		if (m_BoundingObjects[i]->m_Mobility == CGameObject::Static) continue;

		std::vector<CBoundingBoxMesh*>& boundingMeshes = m_BoundingObjects[i]->GetMesh();
		for (int n = 0; n < boundingMeshes.size(); ++n)
		{
			XMFLOAT3 Scale = m_BoundingObjects[i]->GetBoundingScale();
			XMFLOAT4X4 CenterPosition = Matrix4x4::Identity();
			CenterPosition._41 = boundingMeshes[n]->GetAABBCenter().x; CenterPosition._42 = boundingMeshes[n]->GetAABBCenter().y;  CenterPosition._43 = boundingMeshes[n]->GetAABBCenter().z;
			CenterPosition._11 = Scale.x; CenterPosition._22 = Scale.y; CenterPosition._33 = Scale.z;
			if (m_BoundingObjects[i]->GetTopParent()->bNotUseTransform())
				boundingMeshes[n]->CenterTransform = Matrix4x4::Multiply(CenterPosition, m_BoundingObjects[i]->m_pParent->m_xmf4x4World);
			else
				boundingMeshes[n]->CenterTransform = Matrix4x4::Multiply(CenterPosition, m_BoundingObjects[i]->m_xmf4x4World);
		}
	}
}

void CBoundingBoxObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (!m_bCollisionBoxWireFrame) return;

	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_BoundingObjects.size(); ++i)
	{
		std::vector<CBoundingBoxMesh*>& boundingMeshes = m_BoundingObjects[i]->GetMesh();
		for (int n = 0; n < boundingMeshes.size(); ++n)
		{
			if (boundingMeshes[n])
			{
				m_BoundingObjects[i]->UpdateShaderVariable(pd3dCommandList, &boundingMeshes[n]->CenterTransform);
				boundingMeshes[n]->Render(pd3dCommandList, 0);
			}
		}
	}
}

void CBoundingBoxObjects::AppendBoundingObject(CGameObject* obj)
{
	m_BoundingObjects.push_back(obj);
	m_BoundingObjects.back()->m_Mobility = obj->m_Mobility;
}

int CBoundingBoxObjects::IsCollide(CGameObject* obj)
{
	return -1;
}

//-------------------------------------------------------------------------------
/*	CModelObjects : public CStandardShader									   */
//-------------------------------------------------------------------------------
CModelObjects::CModelObjects()
{
}
CModelObjects::~CModelObjects()
{
}

void CModelObjects::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
}

void CModelObjects::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
		m_ppObjects[i]->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);
}
void CModelObjects::Collide(CBoundingBoxObjects& BoundingBoxObjects)
{
}
void CModelObjects::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
		m_ppObjects[i]->Animate(fTimeElapsed);
}
void CModelObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CModelObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}

void CModelObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

CGameObject* CModelObjects::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
	int nIntersected = 0;
	*pfNearHitDistance = FLT_MAX;
	float fHitDistance = FLT_MAX;
	CGameObject* pSelectedObject = NULL;
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pSelectedObject = m_ppObjects[i];
		}
	}

	return pSelectedObject;
}

//--Concrete_1-------------------------------------------------------------------
void ModelObjects_1::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
}

void ModelObjects_1::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

//-------------------------------------------------------------------------------
/*	CTexturedObjects														   */
//-------------------------------------------------------------------------------
CTexturedObjects::CTexturedObjects()
{
}
CTexturedObjects::~CTexturedObjects()
{
}

void CTexturedObjects::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CTexturedObjects::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
		m_ppObjects[i]->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);
}
void CTexturedObjects::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}
void CTexturedObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CTexturedObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}

void CTexturedObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		if (m_ppObjects[i])
		{
			if (m_ppObjects[i]->m_pChild) m_ppObjects[i]->UpdateTransform(NULL);
			m_ppObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

CGameObject* CTexturedObjects::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
	int nIntersected = 0;
	*pfNearHitDistance = FLT_MAX;
	float fHitDistance = FLT_MAX;
	CGameObject* pSelectedObject = NULL;
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pSelectedObject = m_ppObjects[i];
		}
	}

	return pSelectedObject;
}

//--Concrete_1-------------------------------------------------------------------
void TexturedObjects_1::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fTerrainWidth = pTerrain->GetWidth(), fTerrainLength = pTerrain->GetLength();
	float fxPitch = 30.0f * 3.5f;
	float fyPitch = 30.0f * 3.5f;
	float fzPitch = 30.0f * 3.5f;
	//직육면체를 지형 표면에 그리고 지형보다 높은 위치에 일정한 간격으로 배치한다.
	int xObjects = int(fTerrainWidth / fxPitch), yObjects = 1, zObjects = int(fTerrainLength / fzPitch);

	m_ppObjects.resize(xObjects * yObjects * zObjects);

	CTexture* ppTextures[1];
	ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"GameTexture/neon_tile4_1.dds", 0, true);

	CScene::CreateSRVUAVs(pd3dDevice, ppTextures[0], 4, true);

	CMaterial* ppMaterials[1];
	ppMaterials[0] = new CMaterial();
	ppMaterials[0]->SetTexture(ppTextures[0]);

	CCubeMeshTextured* pCubeMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);

	XMFLOAT3 xmf3RotateAxis, xmf3SurfaceNormal;
	CRotatingObject* pRotatingObject = NULL;
	for (int i = 0, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				pRotatingObject = new CRotatingObject();
				pRotatingObject->SetMaterial(0, ppMaterials[0]);
				pRotatingObject->SetMesh(pCubeMesh);
				float xPosition = x * fxPitch;
				float zPosition = z * fzPitch;
				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
				pRotatingObject->SetPosition(xPosition, fHeight + (y * 10.0f * fyPitch) + 6.0f, zPosition);
				if (y == 0)
				{
					xmf3SurfaceNormal = pTerrain->GetIntervalNormal(xPosition, zPosition);
					xmf3RotateAxis = Vector3::CrossProduct(XMFLOAT3(0.0f, 1.0f, 0.0f),
						xmf3SurfaceNormal);
					if (Vector3::IsZero(xmf3RotateAxis)) xmf3RotateAxis = XMFLOAT3(0.0f, 1.0f,
						0.0f);
					float fAngle = acos(Vector3::DotProduct(XMFLOAT3(0.0f, 1.0f, 0.0f),
						xmf3SurfaceNormal));
					pRotatingObject->Rotate(&xmf3RotateAxis, XMConvertToDegrees(fAngle));
				}
				pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
				pRotatingObject->SetRotationSpeed(36.0f * (i % 10) + 36.0f);
				m_ppObjects[i++] = pRotatingObject;
			}
		}
	}
}

//-------------------------------------------------------------------------------
/*	CMonsterObjects															   */
//-------------------------------------------------------------------------------
CMonsterObjects::CMonsterObjects()
{
}
CMonsterObjects::~CMonsterObjects()
{
	if (m_pMonsterModel) {
		if (m_pMonsterModel->m_pModelRootObject)
		{
			m_pMonsterModel->m_pModelRootObject->ReleaseShaderVariables();
			m_pMonsterModel->m_pModelRootObject->Release();
		}
		delete m_pMonsterModel;
	}
	if (m_pHPMaterial) m_pHPMaterial->Release();
	if (m_pHPObject) m_pHPObject->Release();
}

D3D12_INPUT_LAYOUT_DESC CMonsterObjects::CreateInputLayout()
{
	UINT nInputElementDescs = 11;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[7] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[8] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[9] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[10] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

void CMonsterObjects::Update(float fTimeElapsed)
{
	for (CGameObject* monster : m_ppObjects)
	{
		if (monster) monster->Update(fTimeElapsed);
	}
}
void CMonsterObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera);

	UpdateShaderVariables(pd3dCommandList);

	if(!m_ppObjects.empty()) m_pMonsterModel->m_pModelRootObject->Render(pd3dCommandList, pCamera, m_ppObjects.size(), m_d3dInstancingBufferView);

	CShader::Render(pd3dCommandList, pCamera, 1);

	if (!m_ppObjects.empty()) m_pHPObject->Render(pd3dCommandList, pCamera, m_ppObjects.size(), m_d3dInstancingBufferView);
}
void CMonsterObjects::RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandLis)
{
	for (int i = 0; i < m_nBuildIndex; ++i)
	{
		auto monster = m_ppObjects.rbegin();
		std::advance(monster, i);
		if (*monster)
		{
			(*monster)->RunTimeBuild(pd3dDevice, pd3dCommandLis);
		}
	}

	m_nBuildIndex = 0;
}

void CMonsterObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (CGameObject* monster : m_ppObjects)
		{
			if (monster) monster->Release();
		}
	}
}
void CMonsterObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (CGameObject* monster : m_ppObjects)
		{
			if (monster) monster->ReleaseUploadBuffers();
		}
	}
	if (m_pMonsterModel)
	{
		if (m_pMonsterModel->m_pModelRootObject)
		{
			m_pMonsterModel->m_pModelRootObject->ReleaseUploadBuffers();
		}
	}
	if (m_pHPMaterial) m_pHPMaterial->ReleaseUploadBuffers();
	if (m_pHPObject)  m_pHPObject->ReleaseUploadBuffers();

}
void CMonsterObjects::ReleaseShaderVariables()
{
	if (m_pd3dcbObjects) m_pd3dcbObjects->Unmap(0, NULL);
	if (m_pd3dcbObjects) m_pd3dcbObjects->Release();
}
//--Concrete_1-------------------------------------------------------------------
GeneralMonsterObjects::GeneralMonsterObjects()
{
}
GeneralMonsterObjects::~GeneralMonsterObjects()
{
}

void GeneralMonsterObjects::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}

D3D12_INPUT_LAYOUT_DESC GeneralMonsterObjects::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	if (m_nCurrentPipelineState == 0)
	{
		UINT nInputElementDescs = 7;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	else
	{
		UINT nInputElementDescs = 8;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "HP", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[3] = { "MAXHP", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 4, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[4] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 8, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[5] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 24, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[6] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 40, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[7] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 56, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	return(d3dInputLayoutDesc);
}
D3D12_SHADER_BYTECODE GeneralMonsterObjects::CreateVertexShader()
{
	if (m_nCurrentPipelineState == 0)
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
	else
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "VSHPbar", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE GeneralMonsterObjects::CreatePixelShader()
{
	if (m_nCurrentPipelineState == 0)
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
	else
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "PSHPbar", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void GeneralMonsterObjects::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dcbObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(HP_INSTANCE) * m_nMaxObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);

	m_d3dInstancingBufferView.BufferLocation = m_pd3dcbObjects->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(HP_INSTANCE);
	m_d3dInstancingBufferView.SizeInBytes = sizeof(HP_INSTANCE) * m_nMaxObjects;
}
void GeneralMonsterObjects::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int n = 0;
	for (CGameObject* monster : m_ppObjects)
	{
		XMFLOAT4X4 InstInfo = monster->m_pTopBoundingMesh->CenterTransform;
		XMFLOAT4X4 IncreseY = Matrix4x4::Identity();
		
		float ExtentY = monster->m_pTopBoundingMesh->GetAABBExtents().y;
		float ScaleY = monster->m_pTopBoundingMesh->CenterTransform._22;	
		IncreseY._42 = ExtentY * ScaleY * 1.4f;
		InstInfo = Matrix4x4::Multiply(InstInfo, IncreseY);

		m_pcbMappedGameObjects[n].m_fHP = ((MonsterObject*)monster)->HP;
		m_pcbMappedGameObjects[n].m_fMaxHP = ((MonsterObject*)monster)->MAXHP;
		XMStoreFloat4x4(&m_pcbMappedGameObjects[n++].m_xmf4x4Transform, XMMatrixTranspose(XMLoadFloat4x4(&InstInfo)));
	}
}

void GeneralMonsterObjects::InitShader(CGameObject* pChild)
{
	for (int i = 0; i < pChild->m_nMaterials; ++i)
	{
		if (pChild->m_ppMaterials[i] && pChild->m_ppMaterials[i]->m_pShader) {
			pChild->m_ppMaterials[i]->m_pShader->Release();
			pChild->m_ppMaterials[i]->m_pShader = NULL;
		}
	}

	if (pChild->m_pChild) InitShader(pChild->m_pChild);
	if (pChild->m_pSibling) InitShader(pChild->m_pSibling);
}
void GeneralMonsterObjects::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	for (CGameObject* monster : m_ppObjects)
	{
		//monster->CreateBoundingBoxInstSet(pd3dDevice, pd3dCommandList, m_pMonsterModel->m_pModelRootObject, BBShader);
		monster->CreateBoundingBoxMeshSet(pd3dDevice, pd3dCommandList, BBShader);
		monster->SetWorldTransformBoundingBox();
	}
}
void GeneralMonsterObjects::BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture)
{
	CreateMonsters(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 3);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pHPMaterial = new CMaterial();
	m_pHPMaterial->AddRef();
	if (pTexture)	// blur
	{
		m_pHPMaterial->SetTexture(pTexture);
		pTexture->AddRef();

		CScene::CreateSRVUAVs(pd3dDevice, m_pHPMaterial->m_ppTextures[0], ROOT_PARAMETER_TEXTURE, true, true, true, 0, 1);
		CScene::CreateSRVUAVs(pd3dDevice, m_pHPMaterial->m_ppTextures[0], ROOT_PARAMETER_OUTPUT, true, true, true, 2, 1, 1);
	}
	else
	{
		CTexture* pBulletTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
		pBulletTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"UI/hpds.dds", 0);

		m_pHPMaterial->SetTexture(pBulletTexture);

		CScene::CreateSRVUAVs(pd3dDevice, pBulletTexture, ROOT_PARAMETER_TEXTURE, true);
	}
	m_pHPObject = new CRectTextureObject(pd3dDevice, pd3dCommandList, m_pHPMaterial);
}
void GeneralMonsterObjects::CreateMonsters(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nLoop)
{
	m_nMonsterLoop = nLoop;
	m_nMaxObjects = nLoop * 10;
	for (int i = 0; i < nLoop; ++i)
	{
		int Z = 0;

		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Dragon/Polygonal_Dragon.bin", 1000.0f, i, Z++, true, XMFLOAT3(0.0f,2.4f,0.2f), XMFLOAT3(0.6f,1.0f,2.0f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Giant_Bee/Polygonal_Giant_Bee.bin", 150.0f, i, Z++, true, XMFLOAT3(0.0f, 1.0f, 0.05f), XMFLOAT3(0.3f, 0.5f, 0.4f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Golem/Polygonal_Golem.bin", 500.0f, i, Z++, true, XMFLOAT3(0.0f, 1.0f, 0.05f), XMFLOAT3(1.f, 1.0f, 0.6f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/KingCobra/Polygonal_King_Cobra.bin", 400.0f, i, Z++, true, XMFLOAT3(-0.36f, 0.7f, -0.1f), XMFLOAT3(0.62f, 0.7f, 0.6f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/TreasureChest/Polygonal_Treasure_Chest_Monster.bin", 250.0f, i, Z++, true, XMFLOAT3(0.0f, 0.5f, 0.05f), XMFLOAT3(0.5f, 0.5f, 0.42f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Spider/Polygonal_Spiderling_Venom.bin", 300.0f, i, Z++, true, XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT3(0.7f, 0.5f, 0.7f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Bat/Polygonal_One_Eyed_Bat.bin", 100.0f, i, Z++, true, XMFLOAT3(0.0f, 1.1f, 0.0f), XMFLOAT3(0.44f, 0.25f, 0.15f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Magma/Polygonal_Magma.bin", 400.0f, i, Z++, true, XMFLOAT3(0.0f, 0.6f, 0.0f), XMFLOAT3(0.8f, 0.45f, 0.45f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Treant/Polygonal_Treant.bin", 500.0f, i, Z++, true, XMFLOAT3(0.0f, 1.0f, -0.1f), XMFLOAT3(1.f, 1.0f, 0.9f));
		CreateMonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Wolf/Polygonal_Wolf.bin", 300.0f, i, Z++, true, XMFLOAT3(0.0f, 0.4f, 0.05f), XMFLOAT3(0.3f, 0.4f, 1.0f));
	}
}
void GeneralMonsterObjects::CreateMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* model, float maxHP, int x, int z, bool bModifyBouondingBox, XMFLOAT3 Center, XMFLOAT3 Extent)
{
	CLoadedModelInfo* pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, model, NULL);
	//m_ppObjects.push_back(new MonsterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel));
	m_ppObjects.push_back(new MonsterObject());
	m_ppObjects.back()->SetChild(pModel->m_pModelRootObject);
	m_ppObjects.back()->UpdateMobility(CGameObject::Moveable);
	m_ppObjects.back()->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 5, pModel);
	for (int i = 0; i < 5; ++i)
	{
		m_ppObjects.back()->m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_ppObjects.back()->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_ppObjects.back()->m_pSkinnedAnimationController->SetTrackSpeed(i, 1.0f);
	}
	//m_ppObjects.back()->SetIsExistBoundingBox(false);
	m_ppObjects.back()->SetPosition(2800.f + 30.f * x, 265.0f, 3000.f - 30.f * z);
	m_ppObjects.back()->SetUseTransform(false);
	if (bModifyBouondingBox) m_ppObjects.back()->SetBoundingBox(Center, Extent);

	((MonsterObject*)m_ppObjects.back())->MAXHP = maxHP;
	((MonsterObject*)m_ppObjects.back())->HP = maxHP;
	if (pModel) delete pModel;
}
void GeneralMonsterObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera);

	UpdateShaderVariables(pd3dCommandList);

	if (!m_ppObjects.empty())
	{
		int i = 0;
		//for (int i = 0; i < m_ppObjects.size(); ++i)
		for (auto monster : m_ppObjects)
		{
			if (((MonsterObject*)(monster))->bActivate)
				monster->Render(pd3dCommandList, pCamera);
		}
		//m_pMonsterModel->m_pModelRootObject->Render(pd3dCommandList, pCamera, m_ppObjects.size(), m_d3dInstancingBufferView);
	}

	CShader::Render(pd3dCommandList, pCamera, 1);

	if (!m_ppObjects.empty()) m_pHPObject->Render(pd3dCommandList, pCamera, m_ppObjects.size(), m_d3dInstancingBufferView);
}
void GeneralMonsterObjects::Update(float fTimeElapsed)
{
	CMonsterObjects::Update(fTimeElapsed);

	////몬스터 위치값 적용(서버)
	int count = 0;
	for (auto monster : m_ppObjects)
	{
		XMFLOAT4X4 world = m_pMonsterData[count].m_xmf4x4World;
		XMFLOAT3 pos = m_pMonsterData[count].Pos;
		XMFLOAT3 right = XMFLOAT3(world._11, world._12, world._13);
		XMFLOAT3 up = XMFLOAT3(world._21, world._22, world._23);
		XMFLOAT3 look = XMFLOAT3(world._31, world._32, world._33);
		if (world._11 < EPSILON && world._22 < EPSILON && world._33 < EPSILON)
			monster->SetPosition(pos);
		else
			monster->SetTransform(right, up, look, pos);

		count++;
	}

	EventRemove();
}
void GeneralMonsterObjects::Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects)
{
	int n = 0;
	for (CGameObject* monster : m_ppObjects)
	{
		// Prepare Collision
		if (monster) monster->UpdateWorldTransformBoundingBox();
		if (m_pMonsterData[n].State == ATTACK && ((MonsterObject*)monster)->IsAttackAnimPosition())
		{
			if (m_pMonsterData[n].TargetType == CGameObject::TargetType::TagetNexus)
			{
				std::vector<CGameObject*> BoundingObjects = BoundingBoxObjects.GetBoundingObjects();
				for (int i = 0; i < BoundingObjects.size(); ++i)
				{
					if (BoundingObjects[i]->GetTopParent()->GetReafObjectType() == CGameObject::Nexus)
					{
						((NexusObject*)BoundingObjects[i]->GetTopParent())->HP -= ((MonsterObject*)monster)->Damage;
					}
				}
			}
			else
			{
				std::vector<CGameObject*> BoundingObjects = BoundingBoxObjects.GetBoundingObjects();
				for (int i = 0; i < BoundingObjects.size(); ++i)
				{
					if (BoundingObjects[i]->GetReafObjectType() == CGameObject::Player &&
						m_pMonsterData[n].TargetID == ((CPlayer*)BoundingObjects[i])->Player_ID)
					{
						((Player_Neon*)BoundingObjects[i])->HP -= ((MonsterObject*)monster)->Damage;
					}
				}
			}
		}
		n++;
	}
}
void GeneralMonsterObjects::AnimateObjects(float fTimeElapsed)
{
	int n = 0;
	for (auto monster : m_ppObjects)
	{
		int aniTrack = m_pMonsterData[n++].State;
		if (0 <= aniTrack && aniTrack < 5)
		{
			monster->m_pSkinnedAnimationController->SetOneOfTrackEnable(aniTrack);
			monster->m_pSkinnedAnimationController->SetTrackSpeed(aniTrack, 1.0f);
			monster->Animate(fTimeElapsed);
		}
	}
}
void GeneralMonsterObjects::AppendMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, MonsterObject::MonsterType Type)
{
	// append monster
	int MSTIndex = (int)Type * m_nMonsterLoop;

	for (int i = 0; i < m_nMonsterLoop; ++i)
	{
		if (((MonsterObject*)m_ppObjects[MSTIndex + i])->bActivate == false)
		{
			((MonsterObject*)m_ppObjects[MSTIndex + i])->HP = ((MonsterObject*)m_ppObjects[MSTIndex + i])->MAXHP;
			((MonsterObject*)m_ppObjects[MSTIndex + i])->bActivate = true;
			break;
		}
	}
}
void GeneralMonsterObjects::EventRemove()
{
	for (auto monster : m_ppObjects)
	{
		if (((MonsterObject*)(monster))->HP <= 0.0f)
		{
			((MonsterObject*)(monster))->bActivate = false;
			monster->SetPosition(0.0f, 0.0f, 0.0f);
		}
	}
}

void GeneralMonsterObjects::OnPostReleaseUploadBuffers()
{
	CMonsterObjects::ReleaseUploadBuffers();
}

//--Concrete_2-------------------------------------------------------------------
MonsterMetalonObjects::MonsterMetalonObjects()
{
}
MonsterMetalonObjects::~MonsterMetalonObjects()
{
}

void MonsterMetalonObjects::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}

D3D12_INPUT_LAYOUT_DESC MonsterMetalonObjects::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	if (m_nCurrentPipelineState == 0)
	{
		d3dInputLayoutDesc = CMonsterObjects::CreateInputLayout();
	}
	else
	{
		UINT nInputElementDescs = 6;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[3] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[4] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[5] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	return(d3dInputLayoutDesc);
}
D3D12_SHADER_BYTECODE MonsterMetalonObjects::CreateVertexShader()
{
	if (m_nCurrentPipelineState == 0)
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "VSSkinnedAnimationStandard_Inst", "vs_5_1", &m_pd3dVertexShaderBlob));
	else
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "VSBillboard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE MonsterMetalonObjects::CreatePixelShader()
{
	if (m_nCurrentPipelineState == 0)
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
	else
		return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "PSBillboard", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void MonsterMetalonObjects::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dcbObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(HP_INSTANCE) * m_nMaxObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);

	m_d3dInstancingBufferView.BufferLocation = m_pd3dcbObjects->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(HP_INSTANCE);
	m_d3dInstancingBufferView.SizeInBytes = sizeof(HP_INSTANCE) * m_nMaxObjects;
}
void MonsterMetalonObjects::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int n = 0;
	for (CGameObject* monster : m_ppObjects)
	{
		XMFLOAT4X4 InstInfo = monster->m_xmf4x4World;
		InstInfo._11 = 20.f * (n + 1);
		InstInfo._21 = 40.f * (n + 1);
		XMStoreFloat4x4(&m_pcbMappedGameObjects[n++].m_xmf4x4Transform, XMMatrixTranspose(XMLoadFloat4x4(&InstInfo)));
	}
}

void MonsterMetalonObjects::InitShader(CGameObject* pChild)
{
	for (int i = 0; i < pChild->m_nMaterials; ++i)
	{
		if (pChild->m_ppMaterials[i] && pChild->m_ppMaterials[i]->m_pShader) {
			pChild->m_ppMaterials[i]->m_pShader->Release();
			pChild->m_ppMaterials[i]->m_pShader = NULL;
		}
	}

	if (pChild->m_pChild) InitShader(pChild->m_pChild);
	if (pChild->m_pSibling) InitShader(pChild->m_pSibling);
}
void MonsterMetalonObjects::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	for (CGameObject* monster : m_ppObjects)
	{
		monster->CreateBoundingBoxInstSet(pd3dDevice, pd3dCommandList, m_pMonsterModel->m_pModelRootObject, BBShader);
		monster->SetWorldTransformBoundingBox();
	}
}
void MonsterMetalonObjects::BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture)
{
	CLoadedModelInfo* pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Metalon/Green_Metalon.bin", NULL);
	m_pMonsterModel = pMonsterModel;
	pMonsterModel->m_pModelRootObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, m_pMonsterModel);
	pMonsterModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	pMonsterModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackEnable(0, 0);
	pMonsterModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	pMonsterModel->m_pModelRootObject->UpdateMobility(CGameObject::Moveable);

	////드래곤
	//CLoadedModelInfo* pDragonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Dragon/Polygonal_Dragon.bin", NULL);
	//m_pDragononModel = pDragonModel;
	//pDragonModel->m_pModelRootObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 5, m_pDragononModel);
	//for (int i = 0; i < 5; ++i)
	//{
	//	pDragonModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	//	pDragonModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackEnable(i, 0);
	//}
	//	pDragonModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	//pDragonModel->m_pModelRootObject->UpdateMobility(CGameObject::Moveable);
	//pDragonModel->m_pModelRootObject->SetMonsterType(CGameObject::Dragon);
	//
	////골렘
	//CLoadedModelInfo* pGolemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/Monster/Golem/Polygonal_Golem.bin", NULL);
	//m_pGolemModel = pGolemModel;
	//pGolemModel->m_pModelRootObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 5, pGolemModel);
	//for (int i = 0; i < 5; ++i)
	//{
	//	pGolemModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	//	pGolemModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackEnable(i, 0);
	//}
	//	pGolemModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	//pGolemModel->m_pModelRootObject->UpdateMobility(CGameObject::Moveable);
	//pGolemModel->m_pModelRootObject->SetMonsterType(CGameObject::Golem);
	//
	//m_nMaxObjects = 30;
	//
	//InitShader(m_pMetalonModel->m_pModelRootObject);
	//InitShader(m_pDragononModel->m_pModelRootObject);
	//InitShader(m_pGolemModel->m_pModelRootObject);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pHPMaterial = new CMaterial();
	m_pHPMaterial->AddRef();
	if (pTexture)	// blur
	{
		m_pHPMaterial->SetTexture(pTexture);
		pTexture->AddRef();

		CScene::CreateSRVUAVs(pd3dDevice, m_pHPMaterial->m_ppTextures[0], ROOT_PARAMETER_TEXTURE, true, true, true, 0, 1);
		CScene::CreateSRVUAVs(pd3dDevice, m_pHPMaterial->m_ppTextures[0], ROOT_PARAMETER_OUTPUT, true, true, true, 2, 1, 1);
	}
	else
	{
		CTexture* pBulletTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
		pBulletTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"UI/hpds.dds", 0);

		m_pHPMaterial->SetTexture(pBulletTexture);

		CScene::CreateSRVUAVs(pd3dDevice, pBulletTexture, ROOT_PARAMETER_TEXTURE, true);
	}
	m_pHPObject = new CRectTextureObject(pd3dDevice, pd3dCommandList, m_pHPMaterial);
}
void MonsterMetalonObjects::Update(float fTimeElapsed)
{
	CMonsterObjects::Update(fTimeElapsed);

	EventRemove();
}
void MonsterMetalonObjects::Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects)
{
	for (CGameObject* bullet : m_ppObjects)
	{
		// Prepare Collision
		if (bullet) bullet->UpdateWorldTransformBoundingBox();
	}
}
void MonsterMetalonObjects::AnimateObjects(float fTimeElapsed)
{
	if (m_pMonsterModel->m_pModelRootObject->m_pSkinnedAnimationController)
	{
		m_pMonsterModel->m_pModelRootObject->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_pMonsterModel->m_pModelRootObject->Animate(fTimeElapsed);
	}
	//for (auto monster : m_ppObjects)
	//{
	//	//if (monster->m_pSkinnedAnimationController) monster->m_pSkinnedAnimationController->SetTrackEnable(0, true);
	//	monster->Animate(fTimeElapsed);
	//}
}
void MonsterMetalonObjects::AppendMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3&& StartPosition)
{
	//m_nBuildIndex++;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> vdist(-1.0, +1.0);
	srand((unsigned)time(NULL));

	// Instanced.
	if (m_ppObjects.size() < m_nMaxObjects)
	{
		CMonsterMetalon* metalon = new CMonsterMetalon(*m_pMonsterModel->m_pModelRootObject);
		metalon->SetPosition(StartPosition.x, StartPosition.y, StartPosition.z);
		metalon->SetWorldTransformBoundingBox();
		metalon->m_fDriection = XMFLOAT3(vdist(gen), 0.0f, vdist(gen));
		m_ppObjects.push_back(metalon);
	}
}
void MonsterMetalonObjects::EventRemove()
{
	//for (std::list<CGameObject*>::iterator monster = m_ppObjects.begin(); monster != m_ppObjects.end(); ++monster)
	//{
	//	if (((CMonsterMetalon*)(*monster))->m_fLife < 0.0f)
	//	{
	//		(*monster)->Release();
	//		std::list<CGameObject*>::iterator iter = m_ppObjects.erase(monster);
	//		if (iter != m_ppObjects.end())
	//		{
	//			monster = iter;
	//			continue;
	//		}
	//		else
	//			break;
	//	}
	//}
}

void MonsterMetalonObjects::OnPostReleaseUploadBuffers()
{
	CMonsterObjects::ReleaseUploadBuffers();
}

/////////////////////////////////// 몬스터 위치 입력 ///////////////////////////////////
void MonsterMetalonObjects::SetPosition(XMFLOAT3& xmf3Position, int index)
{
	auto monster = m_ppObjects.begin();
	std::advance(monster, index);
	(*monster)->SetPosition(xmf3Position);
}
void MonsterMetalonObjects::SetTransform(int index, XMFLOAT4X4& transform)
{
	auto monster = m_ppObjects.begin();
	std::advance(monster, index);
	XMFLOAT3 pos	=	XMFLOAT3(transform._41, transform._42, transform._43);
	XMFLOAT3 look	=	XMFLOAT3(transform._31, transform._32, transform._33);
	XMFLOAT3 up		=	XMFLOAT3(transform._21, transform._22, transform._23);
	XMFLOAT3 right	=	XMFLOAT3(transform._11, transform._12, transform._13);
	(*monster)->SetTransform(right, up, look, pos);
}
//-------------------------------------------------------------------------------
/*	CBulletObjects															   */
//-------------------------------------------------------------------------------
CBulletObjects::CBulletObjects()
{
}
CBulletObjects::~CBulletObjects()
{
}

void CBulletObjects::Update(float fTimeElapsed)
{
	for (CGameObject* bullet : m_ppObjects) 
	{
		if (bullet) bullet->Update(fTimeElapsed);
	}
}
void CBulletObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (CGameObject* bullet : m_ppObjects)
	{
		if (bullet) bullet->Render(pd3dCommandList, pCamera);
	}
}
void CBulletObjects::RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandLis)
{
	for (int i = 0; i < m_nBuildIndex; ++i)
	{
		auto bullet = m_ppObjects.rbegin();
		std::advance(bullet, i);
		if (*bullet)
		{
			(*bullet)->RunTimeBuild(pd3dDevice, pd3dCommandLis);
		}
	}

	m_nBuildIndex = 0;
}

void CBulletObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (CGameObject* bullet : m_ppObjects)
		{
			if (bullet) bullet->Release();
		}
	}
}
void CBulletObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (CGameObject* bullet : m_ppObjects)
		{
			if (bullet) bullet->ReleaseUploadBuffers();
		}
	}
}

//--Concrete_1-------------------------------------------------------------------
PistolBulletTexturedObjects::PistolBulletTexturedObjects()
{
}
PistolBulletTexturedObjects::~PistolBulletTexturedObjects()
{
	if (m_pMaterial) delete m_pMaterial;
}

void PistolBulletTexturedObjects::ReleaseUploadBuffers()
{
	if(m_pMaterial) m_pMaterial->ReleaseUploadBuffers();
}

void PistolBulletTexturedObjects::OnPostReleaseUploadBuffers()
{
	CBulletObjects::ReleaseUploadBuffers();
}

void PistolBulletTexturedObjects::BuildComponents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CTexture* pTexture)
{
	m_pMaterial = new CMaterial();
	m_pMaterial->AddRef();
	if (pTexture)
	{
		m_pMaterial->SetTexture(pTexture);
		pTexture->AddRef();

		CScene::CreateSRVUAVs(pd3dDevice, m_pMaterial->m_ppTextures[0], ROOT_PARAMETER_TEXTURE, true, true, true, 0, 1);
		CScene::CreateSRVUAVs(pd3dDevice, m_pMaterial->m_ppTextures[0], ROOT_PARAMETER_OUTPUT, true, true, true, 2, 1, 1);
	}
	else
	{
		CTexture* pBulletTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
		pBulletTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Particle/RoundSoftParticle.dds", 0);

		m_pMaterial->SetTexture(pBulletTexture);

		CScene::CreateSRVUAVs(pd3dDevice, pBulletTexture, ROOT_PARAMETER_TEXTURE, true);
	}

	//srand((unsigned)time(NULL));

	//XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	//for (int i = 0; i < 1024; i++) { pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f; }

	//m_pRandowmValueTexture = new CTexture(1, RESOURCE_BUFFER, 0, 1);
	//m_pRandowmValueTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	//CScene::CreateSRVUAVs(pd3dDevice, m_pRandowmValueTexture, ROOT_PARAMETER_RANDOM_TEXTURE, true);
}

void PistolBulletTexturedObjects::Update(float fTimeElapsed)
{
	CBulletObjects::Update(fTimeElapsed);

	EventRemove();
}

void PistolBulletTexturedObjects::Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects)
{
	UINT nConflicted;
	int count = 0;
	for (std::list<CGameObject*>::iterator bullet = m_ppObjects.begin(); bullet != m_ppObjects.end(); ++bullet, ++count)
	{
		if (*bullet && ((CPistolBulletObject*)(*bullet))->Collide(GameSource, BoundingBoxObjects, nConflicted))
		{
			for (int i = 0; i < BoundingBoxObjects.m_ParentObjects.size(); ++i)
			{
				if (BoundingBoxObjects.m_StartIndex[i] <= nConflicted && nConflicted < BoundingBoxObjects.m_StartIndex[i] + BoundingBoxObjects.m_nObjects[i])
				{
					float damage = ((CPistolBulletObject*)(*bullet))->m_fDamege;
					((MonsterObject*)BoundingBoxObjects.m_ParentObjects[i])->Conflicted(damage);
					break;
				}
			}
			(*bullet)->Release();
			std::list<CGameObject*>::iterator iter = m_ppObjects.erase(bullet);
			if (m_ppObjects.size() - count <= m_nBuildIndex) --m_nBuildIndex;
			if (iter != m_ppObjects.end())
			{
				bullet = iter;
				continue;
			}
			else
				break;
		}
	}
}

void PistolBulletTexturedObjects::AppendBullet(XMFLOAT3& startLocation, XMFLOAT3& rayDirection, int type)
{
	m_nBuildIndex++;
	m_ppObjects.push_back(new CPistolBulletObject(m_pMaterial, startLocation, rayDirection, type));
}
void PistolBulletTexturedObjects::EventRemove()
{
	for (std::list<CGameObject*>::iterator bullet = m_ppObjects.begin(); bullet != m_ppObjects.end(); ++bullet)
	{
		if (((CPistolBulletObject*)(*bullet))->m_fLifeTime > 5.0f)
		{
			(*bullet)->Release();
			std::list<CGameObject*>::iterator iter = m_ppObjects.erase(bullet);
			if (iter != m_ppObjects.end())
			{
				bullet = iter;
				continue;
			}
			else
				break;
		}
	}
}

//-------------------------------------------------------------------------------
/*	CBillboardObjects														   */
//-------------------------------------------------------------------------------
CBillboardObjects::CBillboardObjects()
{
}
CBillboardObjects::~CBillboardObjects()
{
}

void CBillboardObjects::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CBillboardObjects::AnimateObjects(float fTimeElapsed)
{
}
void CBillboardObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CBillboardObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}

void CBillboardObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

CGameObject* CBillboardObjects::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
	int nIntersected = 0;
	*pfNearHitDistance = FLT_MAX;
	float fHitDistance = FLT_MAX;
	CGameObject* pSelectedObject = NULL;
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pSelectedObject = m_ppObjects[i];
		}
	}

	return pSelectedObject;
}

//--Concrete_1-------------------------------------------------------------------

//-------------------------------------------------------------------------------
/*	CMultiSpriteObjects														   */
//-------------------------------------------------------------------------------
CMultiSpriteObjects::CMultiSpriteObjects()
{
}
CMultiSpriteObjects::~CMultiSpriteObjects()
{
}

void CMultiSpriteObjects::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CMultiSpriteObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CMultiSpriteObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}
void CMultiSpriteObjects::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}
void CMultiSpriteObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

CGameObject* CMultiSpriteObjects::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
	int nIntersected = 0;
	*pfNearHitDistance = FLT_MAX;
	float fHitDistance = FLT_MAX;
	CGameObject* pSelectedObject = NULL;
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pSelectedObject = m_ppObjects[i];
		}
	}

	return pSelectedObject;
}

//--Concrete_1-------------------------------------------------------------------

//-------------------------------------------------------------------------------
/*	CBlendTextureObjects													   */
//-------------------------------------------------------------------------------
CBlendTextureObjects::CBlendTextureObjects()
{
}
CBlendTextureObjects::~CBlendTextureObjects()
{
}

void CBlendTextureObjects::AnimateObjects(float fTimeElapsed)
{
}
void CBlendTextureObjects::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CBlendTextureObjects::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}
void CBlendTextureObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		if (m_ppObjects[i])
		{
			if (m_ppObjects[i]->m_pChild) m_ppObjects[i]->UpdateTransform(NULL);
			m_ppObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

CGameObject* CBlendTextureObjects::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
	int nIntersected = 0;
	*pfNearHitDistance = FLT_MAX;
	float fHitDistance = FLT_MAX;
	CGameObject* pSelectedObject = NULL;
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pSelectedObject = m_ppObjects[i];
		}
	}

	return pSelectedObject;
}

//--Concrete_1-------------------------------------------------------------------
void BlendTextureObjects_1::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	//CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	//float fTerrainWidth = pTerrain->GetWidth(), fTerrainLength = pTerrain->GetLength();
	//float fxPitch = 50.f;
	//float fyPitch = 50.f;
	//float fzPitch = 50.f;
	////사각형을 지형 표면에 그리고 지형보다 높은 위치에 일정한 간격으로 배치한다.
	//int xObjects = int(fTerrainWidth / fxPitch), yObjects = 1, zObjects = int(fTerrainLength / fzPitch);
	////m_nObjects = xObjects * yObjects * zObjects;
	////m_ppObjects = new CGameObject * [m_nObjects];
	//m_ppObjects.resize(xObjects * yObjects * zObjects);

	//CTexture* ppWaterTextures[1];
	//ppWaterTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//ppWaterTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Water-0341.dds", RESOURCE_TEXTURE2D, 0);

	//CMaterial* ppSpriteMaterials[1];
	//ppSpriteMaterials[0] = new CMaterial();
	//ppSpriteMaterials[0]->SetTexture(ppWaterTextures[0]);

	//CTexturedRectMesh* pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 0.0f, 50.0f, 0.0f, 0.0f, 0.0f);

	//CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	//CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//CreateShaderResourceViews(pd3dDevice, ppWaterTextures[0], 0, 4);

	//COceanObject_1* pGameObject = NULL;
	//for (int i = 0, x = 0; x < xObjects; x++)
	//{
	//	for (int z = 0; z < zObjects; z++)
	//	{
	//		for (int y = 0; y < yObjects; y++)
	//		{
	//			pGameObject = new COceanObject_1();

	//			pGameObject->SetMesh(pSpriteMesh);
	//			pGameObject->SetMaterial(0, ppSpriteMaterials[0]);

	//			float xPosition = (x + 0.5f) * fxPitch;
	//			float zPosition = (z + 0.5f) * fzPitch;
	//			pGameObject->SetPosition(XMFLOAT3(xPosition, 100.f, zPosition));

	//			m_ppObjects[i++] = pGameObject;
	//		}
	//	}
	//}
}
void BlendTextureObjects_1::AnimateObjects(float fTimeElapsed)
{
	//if (!m_ppObjects.empty()) m_ppObjects[0]->BatchAnimate(fTimeElapsed);
}

//-------------------------------------------------------------------------------
/*	CTextureToScreenShader												       */
//-------------------------------------------------------------------------------
CTextureToScreenShader::CTextureToScreenShader(wchar_t* texturePath)
{
	pszFileName = texturePath;
}

CTextureToScreenShader::~CTextureToScreenShader()
{
	if (m_RectMesh) delete m_RectMesh;
	if (m_pTexture) delete m_pTexture;
}

D3D12_INPUT_LAYOUT_DESC CTextureToScreenShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMALGAUGE", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[3] = { "TEMPORARY", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 4, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}
D3D12_RASTERIZER_DESC CTextureToScreenShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CTextureToScreenShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC CTextureToScreenShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CTextureToScreenShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "VSTextureToScreen", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToScreenShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "PSTextureToScreen", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CTextureToScreenShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)pszFileName, 0, true);

	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, ROOT_PARAMETER_TEXTURE, true);

	// Screen Texture's instancing info.
	m_pd3dInstScreenTexture = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(SCREEN_TEXTURE_INSTANCE), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dInstScreenTexture->Map(0, NULL, (void**)&m_pcbMappedInstScreenTexture);

	m_d3dInstancingBufferView.BufferLocation = m_pd3dInstScreenTexture->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(SCREEN_TEXTURE_INSTANCE);
	m_d3dInstancingBufferView.SizeInBytes = sizeof(SCREEN_TEXTURE_INSTANCE);
}

void CTextureToScreenShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateGraphicsSrvShaderVariable(pd3dCommandList, 0);

	m_pcbMappedInstScreenTexture[0].m_fGauge = m_fGauge;
	m_pcbMappedInstScreenTexture[0].m_fTemp = XMFLOAT2(0.0f, 0.0f);
}

void CTextureToScreenShader::ReleaseShaderVariables()
{
	if (m_pd3dInstScreenTexture) m_pd3dInstScreenTexture->Unmap(0, NULL);
	if (m_pd3dInstScreenTexture) m_pd3dInstScreenTexture->Release();
}

void CTextureToScreenShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
	if (m_RectMesh) m_RectMesh->ReleaseUploadBuffers();
}

void CTextureToScreenShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	SetDSVFormat(DXGI_FORMAT_D32_FLOAT);
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dRootSignature);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CTextureToScreenShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);

	m_RectMesh->Render(pd3dCommandList, 0, 1, m_d3dInstancingBufferView);
}

void CTextureToScreenShader::CreateRectTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition)
{
	m_RectMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth, fxPosition, fyPosition, fzPosition);
}

//-------------------------------------------------------------------------------
/*	CTextureToFullScreenShader												   */
//-------------------------------------------------------------------------------
CTextureToFullScreenShader::CTextureToFullScreenShader(CTexture* pTexture)
{
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

CTextureToFullScreenShader::~CTextureToFullScreenShader()
{
}

D3D12_DEPTH_STENCIL_DESC CTextureToFullScreenShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "VSTextureToFullScreen", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "PSTextureToFullScreen", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CTextureToFullScreenShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 1);
	//m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, 1024, 1024, 1, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, ROOT_PARAMETER_TEXTURE, true, true, true, 0, 1);
	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, ROOT_PARAMETER_OUTPUT, true, true, true, 2, 1, 1);
}

void CTextureToFullScreenShader::ReleaseShaderVariables()
{
}

void CTextureToFullScreenShader::ReleaseUploadBuffers()
{
}

void CTextureToFullScreenShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateGraphicsSrvShaderVariable(pd3dCommandList, 0);
	if (m_pTexture) m_pTexture->UpdateGraphicsSrvShaderVariable(pd3dCommandList, 1);
}

void CTextureToFullScreenShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	SetDSVFormat(DXGI_FORMAT_D32_FLOAT);
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dRootSignature);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CTextureToFullScreenShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

//-------------------------------------------------------------------------------
/*	CAddTexturesComputeShader : public CComputeShader						   */
//-------------------------------------------------------------------------------
CAddTexturesComputeShader::CAddTexturesComputeShader()
{
}

CAddTexturesComputeShader::~CAddTexturesComputeShader()
{
}

D3D12_SHADER_BYTECODE CAddTexturesComputeShader::CreateComputeShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "CSAddTextures", "cs_5_1", &m_pd3dGeometryShaderBlob));
}

void CAddTexturesComputeShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CComputeShader::CreateComputePipelineState(pd3dDevice, pd3dRootSignature, cxThreadGroups, cyThreadGroups, czThreadGroups);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CAddTexturesComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1, 0, 2, 1);

	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/InputA.dds", 0); //1024x1024
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/InputC.dds", 1); //1024x1024
	ID3D12Resource* pd3dResource = m_pTexture->GetTexture(0);
	D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();
	m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, d3dResourceDesc.Width, d3dResourceDesc.Height, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXGI_FORMAT_R8G8B8A8_UNORM, 2);

	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_TEX2D_INPUT_A, true, false, true, 0, 2);
	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_RWTEX2D_OUTPUT, true, false, false, 2, 1);

	m_cxThreadGroups = ceil(d3dResourceDesc.Width / 32.0f);
	m_cyThreadGroups = ceil(d3dResourceDesc.Height / 32.0f);
}

void CAddTexturesComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateComputeSrvShaderVariable(pd3dCommandList, 0);
	if (m_pTexture) m_pTexture->UpdateComputeUavShaderVariable(pd3dCommandList, 0);
}

void CAddTexturesComputeShader::ReleaseShaderVariables()
{
	if (m_pTexture) m_pTexture->Release();
}

void CAddTexturesComputeShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CAddTexturesComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
}
//-------------------------------------------------------------------------------
/*	CBrightAreaComputeShader													   */
//-------------------------------------------------------------------------------
CBrightAreaComputeShader::CBrightAreaComputeShader()
{
}
CBrightAreaComputeShader::CBrightAreaComputeShader(wchar_t* texturePath)
{
	pszFileName = texturePath;
}
CBrightAreaComputeShader::~CBrightAreaComputeShader()
{
}

D3D12_SHADER_BYTECODE CBrightAreaComputeShader::CreateComputeShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "CSBrightArea", "cs_5_1", &m_pd3dGeometryShaderBlob));
}

void CBrightAreaComputeShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CComputeShader::CreateComputePipelineState(pd3dDevice, pd3dRootSignature, cxThreadGroups, cyThreadGroups, czThreadGroups);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CBrightAreaComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0, 1, 0, 1, 1);

	if(pszFileName)
		m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pszFileName, 0, true, D3D12_RESOURCE_STATE_GENERIC_READ);
	else
		m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Light3.dds", 0, true, D3D12_RESOURCE_STATE_GENERIC_READ);
	ID3D12Resource* pd3dResource = m_pTexture->GetTexture(0);
	D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();
	m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, d3dResourceDesc.Width, d3dResourceDesc.Height, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_TEX2D_INPUT_A, true, false, true, 0, 1);
	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_RWTEX2D_OUTPUT, true, false, false, 1, 1);

	m_cxThreadGroups = ceil(d3dResourceDesc.Width / 32.0f);
	m_cyThreadGroups = ceil(d3dResourceDesc.Height / 32.0f);
}

void CBrightAreaComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateComputeSrvShaderVariable(pd3dCommandList, 0);
	if (m_pTexture) m_pTexture->UpdateComputeUavShaderVariable(pd3dCommandList, 0);
}

void CBrightAreaComputeShader::ReleaseShaderVariables()
{
	if (m_pTexture) m_pTexture->Release();
}

void CBrightAreaComputeShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CBrightAreaComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	OnPrepareRender(pd3dCommandList);
	if (m_ppd3dPipelineStates[nPipelineState]) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
}

//-------------------------------------------------------------------------------
/*	CGaussian2DBlurComputeShader											   */
//-------------------------------------------------------------------------------
CGaussian2DBlurComputeShader::CGaussian2DBlurComputeShader()
{
}
CGaussian2DBlurComputeShader::CGaussian2DBlurComputeShader(wchar_t* texturePath)
{
	pszFileName = texturePath;
}
CGaussian2DBlurComputeShader::~CGaussian2DBlurComputeShader()
{
}

D3D12_SHADER_BYTECODE CGaussian2DBlurComputeShader::CreateComputeShader()
{
	return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "CSGaussian2DBlur", "cs_5_1", &m_pd3dGeometryShaderBlob));
}

void CGaussian2DBlurComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1, 2, 1, 1);

	if (pszFileName)
		m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pszFileName, 0, true, D3D12_RESOURCE_STATE_GENERIC_READ);
	else
		m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Light3.dds", 0, true, D3D12_RESOURCE_STATE_GENERIC_READ);

	ID3D12Resource* pd3dResource = m_pTexture->GetTexture(0);
	D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();
	m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, d3dResourceDesc.Width, d3dResourceDesc.Height, 1, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
	m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, d3dResourceDesc.Width, d3dResourceDesc.Height, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXGI_FORMAT_R8G8B8A8_UNORM, 2);

	//ID3D12Resource* pd3dSource = m_pTexture->GetTexture(0);
	//ID3D12Resource* pd3dSource = m_pSourceResource;
	//ID3D12Resource* pd3dDestination = m_pTexture->GetTexture(1);
	//pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);

	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_TEX2D_INPUT_A, true, false, true, 1, 1);
	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_RWTEX2D_OUTPUT, true, false, false, 2, 1);

	m_cxThreadGroups = ceil(d3dResourceDesc.Width / 32.0f);
	m_cyThreadGroups = ceil(d3dResourceDesc.Height / 32.0f);
}

void CGaussian2DBlurComputeShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CComputeShader::CreateComputePipelineState(pd3dDevice, pd3dRootSignature, cxThreadGroups, cyThreadGroups, czThreadGroups);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CGaussian2DBlurComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateComputeSrvShaderVariable(pd3dCommandList, 0);
	if (m_pTexture) m_pTexture->UpdateComputeUavShaderVariable(pd3dCommandList, 0);
}

void CGaussian2DBlurComputeShader::ReleaseShaderVariables()
{
	if (m_pTexture) m_pTexture->Release();
}

void CGaussian2DBlurComputeShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CGaussian2DBlurComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	ID3D12Resource* pd3dSource;
	ID3D12Resource* pd3dDestination;
	if (m_pSourceResource)
	{
		pd3dSource = m_pSourceResource;
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dDestination = m_pTexture->GetTexture(1);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		pd3dSource = m_pTexture->GetTexture(0);
		pd3dDestination = m_pTexture->GetTexture(1);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
	}

	OnPrepareRender(pd3dCommandList);
	if (m_ppd3dPipelineStates[nPipelineState]) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);
	UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < 10; i++)
	{
		pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);

		pd3dSource = m_pTexture->GetTexture(2);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dDestination = m_pTexture->GetTexture(1);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGaussian1DBlurComputeShader::CGaussian1DBlurComputeShader()
{
}

CGaussian1DBlurComputeShader::~CGaussian1DBlurComputeShader()
{
}

D3D12_SHADER_BYTECODE CGaussian1DBlurComputeShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0) return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "CSHorizontalBlur", "cs_5_1", ppd3dShaderBlob));
	if (nPipelineState == 1) return(CShader::CompileShaderFromFile((wchar_t*)L"Shaders.hlsl", "CSVerticalBlur", "cs_5_1", ppd3dShaderBlob));
}

void CGaussian1DBlurComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1, 2, 1, 1);

	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/ImageC.dds", 0);
	ID3D12Resource* pd3dResource = m_pTexture->GetTexture(0);
	D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();
	m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, d3dResourceDesc.Width, d3dResourceDesc.Height, 1, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
	m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, NULL, 0, RESOURCE_TEXTURE2D, d3dResourceDesc.Width, d3dResourceDesc.Height, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXGI_FORMAT_R8G8B8A8_UNORM, 2);

	ID3D12Resource* pd3dSource = m_pTexture->GetTexture(0);
	ID3D12Resource* pd3dDestination = m_pTexture->GetTexture(1);
	pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);

	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_TEX2D_INPUT_A, true, false, true, 1, 1);
	CScene::CreateSRVUAVs(pd3dDevice, m_pTexture, CROOT_PARAMETER_TEX2D_OUTPUT_A, true, false, false, 2, 1);
}

void CGaussian1DBlurComputeShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups)
{
	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CComputeShader::CreateComputePipelineState(pd3dDevice, pd3dRootSignature, cxThreadGroups, cyThreadGroups, czThreadGroups);
	CComputeShader::CreateComputePipelineState(pd3dDevice, pd3dRootSignature, cxThreadGroups, cyThreadGroups, czThreadGroups);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CGaussian1DBlurComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateComputeSrvShaderVariable(pd3dCommandList, 0);
	if (m_pTexture) m_pTexture->UpdateComputeUavShaderVariable(pd3dCommandList, 0);
}

void CGaussian1DBlurComputeShader::ReleaseShaderVariables()
{
	if (m_pTexture) m_pTexture->Release();
}

void CGaussian1DBlurComputeShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CGaussian1DBlurComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);

	ID3D12Resource* pd3dResource = m_pTexture->GetTexture(0);
	D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();
	ID3D12Resource* pd3dSource = m_pTexture->GetTexture(2);
	ID3D12Resource* pd3dDestination = m_pTexture->GetTexture(1);
	for (int i = 0; i < 5; i++)
	{
		if (m_ppd3dPipelineStates[0]) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
		pd3dCommandList->Dispatch(ceil(d3dResourceDesc.Width / 256.0f), d3dResourceDesc.Height, 1);

		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if (m_ppd3dPipelineStates[1]) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[1]);
		pd3dCommandList->Dispatch(d3dResourceDesc.Width, ceil(d3dResourceDesc.Height / 256.0f), 1);

		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}