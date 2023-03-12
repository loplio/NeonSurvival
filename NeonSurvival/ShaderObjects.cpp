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
			for (int n = 0; n < m_BBObjects[i]->GetBBMesh().size(); ++n)
			{
				XMFLOAT4X4 CenterPosition = Matrix4x4::Identity();
				CenterPosition._41 += m_BBObjects[i]->GetBBMesh()[n]->GetAABBCenter().x;
				CenterPosition._42 += m_BBObjects[i]->GetBBMesh()[n]->GetAABBCenter().y;
				CenterPosition._43 += m_BBObjects[i]->GetBBMesh()[n]->GetAABBCenter().z;
				CenterPosition = Matrix4x4::Multiply(CenterPosition, m_BBObjects[i]->m_xmf4x4World);
				m_BBObjects[i]->UpdateShaderVariable(pd3dCommandList, &CenterPosition);
				if (m_BBObjects[i]->GetBBMesh()[n]) m_BBObjects[i]->GetBBMesh()[n]->Render(pd3dCommandList, 0);
			}
		}
	}
}

int CBoundingBoxObjects::IsCollide(CGameObject* obj, ObjectType excludetype)
{
	for (int index = 0; index < m_BBObjects.size(); ++index)
	{
		if (obj->GetObjectType() == m_BBObjects[index]->GetObjectType() ||
			excludetype == m_BBObjects[index]->GetObjectType()) continue;

		for (int nMesh = 0; nMesh < obj->GetMeshNum(); ++nMesh)
		{
			for (int nOthMesh = 0; nOthMesh < m_BBObjects[index]->GetMeshNum(); ++nOthMesh)
			{
				BoundingOrientedBox obb = m_BBObjects[index]->GetMesh(nOthMesh)->GetBoundingBox();
				obb.Transform(obb, XMLoadFloat4x4(&m_BBObjects[index]->m_xmf4x4World));
				if (obj->IsCollide(nMesh, obb)) {
					return index;
				}
			}
		}
	}
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
	m_ppObjects.push_back(new CGameObject(1, 1));

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 17);

	CGameObject* pSuperCobraModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, (char*)"Model/SuperCobra.bin", this);

	m_ppObjects[0] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_ppObjects[0]->SetChild(pSuperCobraModel);
	pSuperCobraModel->AddRef();
	m_ppObjects[0]->SetPosition(300.0f, 250.f, 300.f);
	m_ppObjects[0]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[0]->PrepareAnimate();
	XMFLOAT3 scale = XMFLOAT3(5.0f, 5.0f, 5.0f);
	m_ppObjects[0]->SetScale(scale);
	m_ppObjects[0]->Scale(scale.x, scale.y, scale.z);
	m_ppObjects[0]->SetObjectType(ObjectType::SUPER_COBRA_OBJ);
}

void ModelObjects_1::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera);

	//UpdateShaderVariables(pd3dCommandList);
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		if (m_ppObjects[i])
		{
			XMFLOAT3 scale = m_ppObjects[i]->m_xmf3Scale;
			if (m_ppObjects[i]) m_ppObjects[i]->SetLookAt2(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			if (m_ppObjects[i]->m_pChild) m_ppObjects[i]->Scale(scale.x, scale.y, scale.z);

			m_ppObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
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

	//UpdateShaderVariables(pd3dCommandList);

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
	//m_nObjects = xObjects * yObjects * zObjects;
	//m_ppObjects = new CGameObject * [m_nObjects];
	m_ppObjects.resize(xObjects * yObjects * zObjects);

	CTexture* ppTextures[1];
	ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Ceiling.dds", RESOURCE_TEXTURE2D, 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	CreateShaderResourceViews(pd3dDevice, ppTextures[0], 0, 4);

	CMaterial* ppMaterials[1];
	ppMaterials[0] = new CMaterial();
	ppMaterials[0]->SetTexture(ppTextures[0]);

	CCubeMeshTextured* pCubeMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);

	UINT ncbGameObjectBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	XMFLOAT3 xmf3RotateAxis, xmf3SurfaceNormal;
	CRotatingObject* pRotatingObject = NULL;
	for (int i = 0, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				pRotatingObject = new CRotatingObject(1);
				pRotatingObject->SetMaterial(0, ppMaterials[0]);
				pRotatingObject->SetMesh(0, pCubeMesh);
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
/*	CBillboardObject_1s														   */
//-------------------------------------------------------------------------------
CBillboardObject_1s::CBillboardObject_1s()
{
}
CBillboardObject_1s::~CBillboardObject_1s()
{
}

void CBillboardObject_1s::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CBillboardObject_1s::AnimateObjects(float fTimeElapsed)
{
}
void CBillboardObject_1s::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CBillboardObject_1s::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}

void CBillboardObject_1s::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

CGameObject* CBillboardObject_1s::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
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
void BillboardObjects_1::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	object_type = 3;

	CTexture* ppGrassTextures[1];
	ppGrassTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppGrassTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Grass02.dds", RESOURCE_TEXTURE2D, 0);

	CTexture* ppTreeTextures[2];
	ppTreeTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTreeTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Tree01.dds", RESOURCE_TEXTURE2D, 0);
	ppTreeTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTreeTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Tree02.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppGrassMaterials[1];
	ppGrassMaterials[0] = new CMaterial();
	ppGrassMaterials[0]->SetTexture(ppGrassTextures[0]);

	CMaterial* ppTreeMaterials[2];
	ppTreeMaterials[0] = new CMaterial();
	ppTreeMaterials[0]->SetTexture(ppTreeTextures[0]);
	ppTreeMaterials[1] = new CMaterial();
	ppTreeMaterials[1]->SetTexture(ppTreeTextures[1]);

	CTexturedRectMesh* pGrassMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 8.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	CTexturedRectMesh* pTreeMesh01 = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 24.0f, 36.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	UINT ObjectMapWidth = 257, ObjectMapHeight = 257;
	CRawFormatImage* pRawFormatImage = new CRawFormatImage(L"Image/ObjectsMap3.raw", ObjectMapWidth, ObjectMapHeight, true);

	int nGrassObjects = 0, nBlacks = 0, nOthers = 0, nTreeObjects[2] = { 0, 0 };
	for (int z = 2; z <= 254; z++)
	{
		for (int x = 2; x <= 254; x++)
		{
			BYTE nPixel = pRawFormatImage->GetRawImagePixel(x, z);
			switch (nPixel)
			{
			case 102: nGrassObjects++; break;
				//case 128: nGrassObjects++; break;
				//case 153: nFlowerObjects++; break;
				//case 179: nFlowerObjects++; break;
			case 204: nTreeObjects[0]++; break;
			case 225: nTreeObjects[1]++; break;
				//case 255: nTreeObjects[2]++; break;
			case 0: nBlacks++; break;
			default: nOthers++; break;
			}
		}
	}
	//m_nObjects = nGrassObjects + nTreeObjects[0] + nTreeObjects[1];

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 3);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, ppGrassTextures[0], 0, 4);
	CreateShaderResourceViews(pd3dDevice, ppTreeTextures[0], 0, 4);
	CreateShaderResourceViews(pd3dDevice, ppTreeTextures[1], 0, 4);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	int nTerrainWidth = int(pTerrain->GetWidth());
	int nTerrainLength = int(pTerrain->GetLength());

	UINT AspectRatioTraiMapToObjMap = nTerrainWidth / ObjectMapWidth;
	UINT VerticalRatioTraiMapToObjMap = nTerrainLength / ObjectMapHeight;

	XMFLOAT3 xmf3Scale = pTerrain->GetScale();

	//m_ppObjects = new CGameObject * [m_nObjects];
	m_ppObjects.resize(nGrassObjects + nTreeObjects[0] + nTreeObjects[1]);

	CBillboardObject_1* pBillboardObject = NULL;
	for (int nObjects = 0, z = 2; z <= 254; z++)
	{
		for (int x = 2; x <= 254; x++)
		{
			BYTE nPixel = pRawFormatImage->GetRawImagePixel(x, z);

			float fyOffset = 0.0f;

			CMaterial* pMaterial = NULL;
			CMesh* pMesh = NULL;

			switch (nPixel)
			{
			case 102:
				pMesh = pGrassMesh;
				pMaterial = ppGrassMaterials[0];
				fyOffset = 8.0f * 0.5f;
				break;
				//case 128:
				//	pMesh = pGrassMesh;
				//	pMaterial = ppGrassMaterials[1];
				//	fyOffset = 6.0f * 0.5f;
				//	break;
				//case 153:
				//	pMesh = pFlowerMesh;
				//	pMaterial = ppFlowerMaterials[0];
				//	fyOffset = 16.0f * 0.5f;
				//	break;
				//case 179:
				//	pMesh = pFlowerMesh;
				//	pMaterial = ppFlowerMaterials[1];
				//	fyOffset = 16.0f * 0.5f;
				//	break;
			case 204:
				pMesh = pTreeMesh01;
				pMaterial = ppTreeMaterials[0];
				fyOffset = 33.0f * 0.5f;
				break;
			case 225:
				pMesh = pTreeMesh01;
				pMaterial = ppTreeMaterials[1];
				fyOffset = 33.0f * 0.5f;
				break;
				//case 255:
				//	pMesh = pTreeMesh02;
				//	pMaterial = ppTreeMaterials[2];
				//	fyOffset = 40.0f * 0.5f;
				//	break;
			default:
				break;
			}

			if (pMesh && pMaterial)
			{
				pBillboardObject = new CBillboardObject_1();

				pBillboardObject->SetMesh(0, pMesh);
				pBillboardObject->SetMaterial(0, pMaterial);

				float xPosition = x * AspectRatioTraiMapToObjMap;
				float zPosition = z * VerticalRatioTraiMapToObjMap;
				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
				pBillboardObject->SetPosition(xPosition, fHeight + fyOffset, zPosition);
				m_ppObjects[nObjects++] = pBillboardObject;
			}
		}
	}
}
void BillboardObjects_1::AnimateObjects(float fTimeElapsed)
{
	CMaterial** ref = new CMaterial * [object_type];
	UINT type_count = 0;
	for (int i = 0; i < object_type; ++i)
	{
		ref[i] = NULL;
	}

	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		if (type_count < object_type && !ref[type_count]) {		// 동일한 material, texture를 참조하는 object들의 일괄 애니메이션 처리.
			for (int n = 0; n <= type_count; ++n)
			{
				if (ref[n] == *(m_ppObjects[i]->m_ppMaterials)) break;
				if (n == type_count)
				{
					ref[type_count++] = *(m_ppObjects[i]->m_ppMaterials);
					m_ppObjects[i]->BatchAnimate(fTimeElapsed);
					break;
				}
			}
		}
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
	delete ref;
}

void BillboardObjects_1::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	for (int j = 0; j < m_ppObjects.size(); j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
	}

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

//-------------------------------------------------------------------------------
/*	CMultiSpriteObject_1s														   */
//-------------------------------------------------------------------------------
CMultiSpriteObject_1s::CMultiSpriteObject_1s()
{
}
CMultiSpriteObject_1s::~CMultiSpriteObject_1s()
{
}

void CMultiSpriteObject_1s::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CMultiSpriteObject_1s::ReleaseObjects()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			if (m_ppObjects[i]) m_ppObjects[i]->Release();
	}
}
void CMultiSpriteObject_1s::ReleaseUploadBuffers()
{
	if (!m_ppObjects.empty()) {
		for (int i = 0; i < m_ppObjects.size(); ++i)
			m_ppObjects[i]->ReleaseUploadBuffers();
	}
}
void CMultiSpriteObject_1s::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}
void CMultiSpriteObject_1s::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

CGameObject* CMultiSpriteObject_1s::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
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
void MultiSpriteObjects_1::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_ppSpriteTextures.push_back(new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8));
	m_ppSpriteTextures.push_back(new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 6, 6));
	m_ppSpriteTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);
	m_ppSpriteTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Explosion_6x6.dds", RESOURCE_TEXTURE2D, 0);

	m_ppSpriteMaterials.push_back(new CMaterial());
	m_ppSpriteMaterials.push_back(new CMaterial());
	m_ppSpriteMaterials[0]->SetTexture(m_ppSpriteTextures[0]);
	m_ppSpriteMaterials[1]->SetTexture(m_ppSpriteTextures[1]);

	m_pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, m_ppSpriteTextures[0], 0, 4);
	CreateShaderResourceViews(pd3dDevice, m_ppSpriteTextures[1], 0, 4);

	XMFLOAT3 xmf3Position = XMFLOAT3(1030.0f, 180.0f, 1410.0f);
	AddObject(xmf3Position, 0, 0.5f, false);
	AddObject(xmf3Position, 1, 0.7f, false);
}
void MultiSpriteObjects_1::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	xmf3PlayerPosition.y += 3.0f;
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 50.0f, false));

	GarbageCollector();

	for (int j = 0; j < m_ppObjects.size(); j++)
	{
		if (m_ppObjects[j] && ((CMultiSpriteObject_1*)m_ppObjects[j])->bActive)
		{
			if (((CMultiSpriteObject_1*)m_ppObjects[j])->m_bOwner)
			{
				m_ppObjects[j]->SetPosition(xmf3Position);
				m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			}
			else
			{
				m_ppObjects[j]->UpdateTransform(NULL);
				m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			}
		}
	}

	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		if (m_ppObjects[i] && ((CMultiSpriteObject_1*)m_ppObjects[i])->bActive)
		{
			if (m_ppObjects[i]->m_pChild) m_ppObjects[i]->UpdateTransform(NULL);
			XMFLOAT4X4 xmf4x4World;
			XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
			pd3dCommandList->SetGraphicsRoot32BitConstants(2, 16, &xmf4x4World, 0);
			pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &((CMultiSpriteObject_1*)m_ppObjects[i])->m_xmf4x4Texture, 0);
			m_ppObjects[i]->GetMaterial(0)->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			m_ppObjects[i]->GetMesh(0)->Render(pd3dCommandList, 0);
		}
	}
}

void MultiSpriteObjects_1::AddObject(const XMFLOAT3& position, int index, float cooltime, bool btemporary, bool bOwner)
{
	int row = (index) ? 6 : 8;
	int col = (index) ? 6 : 8;

	CMultiSpriteObject_1* pSpriteObject = NULL;
	pSpriteObject = new CMultiSpriteObject_1(btemporary, col, row);
	pSpriteObject->SetMesh(0, m_pSpriteMesh);
	pSpriteObject->SetMaterial(0, m_ppSpriteMaterials[index]);
	pSpriteObject->SetPosition(XMFLOAT3(position.x, position.y, position.z));
	pSpriteObject->cooltime = cooltime;
	pSpriteObject->temporary = btemporary;
	pSpriteObject->m_bOwner = bOwner;
	m_ppObjects.push_back(pSpriteObject);
}
void MultiSpriteObjects_1::GarbageCollector()
{
	for (int i = 0; i < m_ppObjects.size(); ++i)
	{
		if (((CMultiSpriteObject_1*)m_ppObjects[i])->temporary && !((CMultiSpriteObject_1*)m_ppObjects[i])->bActive)
		{
			m_ppObjects[i]->Release();
			m_ppObjects.erase(m_ppObjects.begin() + i);
		}
	}
}
void MultiSpriteObjects_1::ExecuteActive(int index, bool bOwner)
{
	((CMultiSpriteObject_1*)m_ppObjects[index])->bActive = true;
	((CMultiSpriteObject_1*)m_ppObjects[index])->m_fTime = 0.0f;
	((CMultiSpriteObject_1*)m_ppObjects[index])->m_bOwner = bOwner;
}

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
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fTerrainWidth = pTerrain->GetWidth(), fTerrainLength = pTerrain->GetLength();
	float fxPitch = 50.f;
	float fyPitch = 50.f;
	float fzPitch = 50.f;
	//사각형을 지형 표면에 그리고 지형보다 높은 위치에 일정한 간격으로 배치한다.
	int xObjects = int(fTerrainWidth / fxPitch), yObjects = 1, zObjects = int(fTerrainLength / fzPitch);
	//m_nObjects = xObjects * yObjects * zObjects;
	//m_ppObjects = new CGameObject * [m_nObjects];
	m_ppObjects.resize(xObjects * yObjects * zObjects);

	CTexture* ppWaterTextures[1];
	ppWaterTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppWaterTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Image/Water-0341.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppSpriteMaterials[1];
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppWaterTextures[0]);

	CTexturedRectMesh* pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 0.0f, 50.0f, 0.0f, 0.0f, 0.0f);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, ppWaterTextures[0], 0, 4);

	COceanObject_1* pGameObject = NULL;
	for (int i = 0, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				pGameObject = new COceanObject_1();

				pGameObject->SetMesh(0, pSpriteMesh);
				pGameObject->SetMaterial(0, ppSpriteMaterials[0]);

				float xPosition = (x + 0.5f) * fxPitch;
				float zPosition = (z + 0.5f) * fzPitch;
				pGameObject->SetPosition(XMFLOAT3(xPosition, 100.f, zPosition));

				m_ppObjects[i++] = pGameObject;
			}
		}
	}
}
void BlendTextureObjects_1::AnimateObjects(float fTimeElapsed)
{
	if (!m_ppObjects.empty()) m_ppObjects[0]->BatchAnimate(fTimeElapsed);
}