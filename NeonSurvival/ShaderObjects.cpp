#include "stdafx.h"
#include "ShaderObjects.h"
#include "Frameworks.h"

//-------------------------------------------------------------------------------
/*	CBoundingBoxObjects : public CShader									   */
//-------------------------------------------------------------------------------
CBoundingBoxObjects::CBoundingBoxObjects()
{
	m_BBObjects.reserve(64);
}
CBoundingBoxObjects::~CBoundingBoxObjects()
{
}

void CBoundingBoxObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (!m_bCollisionBoxWireFrame) return;

	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_BBObjects.size(); ++i)
	{
		if (m_BBObjects[i])
		{
			for (int n = 0; n < m_BBObjects[i]->GetBoundingBoxMesh().size(); ++n)
			{
				if (m_BBObjects[i]->GetBoundingBoxMesh()[n])
				{
					XMFLOAT4X4 CenterPosition = Matrix4x4::Identity();
					CenterPosition._41 += m_BBObjects[i]->GetBoundingBoxMesh()[n]->GetAABBCenter().x;
					CenterPosition._42 += m_BBObjects[i]->GetBoundingBoxMesh()[n]->GetAABBCenter().y;
					CenterPosition._43 += m_BBObjects[i]->GetBoundingBoxMesh()[n]->GetAABBCenter().z;
					CenterPosition = Matrix4x4::Multiply(CenterPosition, m_BBObjects[i]->m_xmf4x4World);
					m_BBObjects[i]->UpdateShaderVariable(pd3dCommandList, &CenterPosition);
					m_BBObjects[i]->GetBoundingBoxMesh()[n]->Render(pd3dCommandList, 0);
				}
			}
		}
	}
}

int CBoundingBoxObjects::IsCollide(CGameObject* obj/*, ObjectType excludetype*/)
{
	//for (int index = 0; index < m_BBObjects.size(); ++index)
	//{
	//	if (obj->GetObjectType() == m_BBObjects[index]->GetObjectType() ||
	//		excludetype == m_BBObjects[index]->GetObjectType()) continue;

	//	BoundingOrientedBox obb = m_BBObjects[index]->m_pMesh->GetBoundingBox();
	//	obb.Transform(obb, XMLoadFloat4x4(&m_BBObjects[index]->m_xmf4x4World));
	//	if (obj->IsCollide(obb)) {
	//		return index;
	//	}
	//}
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
	ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Ceiling.dds", 0, true);

	CScene::CreateShaderResourceViews(pd3dDevice, ppTextures[0], 4, true);

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

//--Concrete_2-------------------------------------------------------------------
void TexturedObjects_2::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
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