#include "stdafx.h"
#include "Components_Test.h"
#include "GameObject.h"
#include "ShaderObjects.h"

//-------------------------------------------------------------------------------
/*	Scene																	   */
//-------------------------------------------------------------------------------
Scene_Test::Scene_Test(ID3D12Device* pd3dDevice) : CScene(pd3dDevice)
{
}
Scene_Test::~Scene_Test()
{
}

void Scene_Test::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void Scene_Test::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::UpdateShaderVariables(pd3dCommandList);
}
void Scene_Test::ReleaseShaderVariables()
{
	CScene::ReleaseShaderVariables();
}

//--Build : Scene_Test---------------------------------------------------------------
void Scene_Test::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader)
{
	CScene::CreateBoundingBox(pd3dDevice, pd3dCommandList, BBShader);
}
void Scene_Test::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildLightsAndMaterials();

	// SkyBox Build.
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	// Terrain Build.
	XMFLOAT3 xmf3Scale(12.0f, 2.0f, 12.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.1f, 0.0f, 0.0f);
#ifdef _WITH_TERRAIN_PARTITION
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("../Assets/Image/Terrain/HeightMap.raw"), 257, 257, 17, 17, xmf3Scale, xmf4Color);
#else
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
		m_pd3dGraphicsRootSignature, _T("Image/terrain.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color);
#endif

	// ShaderObjects Build.
	m_ppShaders.reserve(5);

	CTexturedObjects* pTexturedObjectShader = new TexturedObjects_1();
	pTexturedObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pTexturedObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders.push_back(pTexturedObjectShader);

	CModelObjects* pModelObjectShader = new ModelObjects_1();
	pModelObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pModelObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders.push_back(pModelObjectShader);

	CBillboardObject_1s* pBillboardObjectShader = new BillboardObjects_1();
	pBillboardObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pBillboardObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders.push_back(pBillboardObjectShader);

	CMultiSpriteObject_1s* pMultiSpriteObjectsShader = new MultiSpriteObjects_1();
	pMultiSpriteObjectsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pMultiSpriteObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders.push_back(pMultiSpriteObjectsShader);

	CBlendTextureObjects* pBlendTextureObjectsShader = new BlendTextureObjects_1();
	pBlendTextureObjectsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pBlendTextureObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders.push_back(pBlendTextureObjectsShader);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// BoundingBoxObjects Build.
	m_pBBObjects->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}
void Scene_Test::BuildLightsAndMaterials()
{
	CScene::BuildLightsAndMaterials();
}
void Scene_Test::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();
}
void Scene_Test::ReleaseObjects()
{
	CScene::ReleaseObjects();
}

//--ProcessInput : Scene_Test--------------------------------------------------------
bool Scene_Test::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}
bool Scene_Test::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_CONTROL:
			break;
		case 'F':
			if (typeid(MultiSpriteObjects_1) == typeid(*m_ppShaders[3])
				&& typeid(CAirplanePlayer) == typeid(*m_pPlayer))
				((CAirplanePlayer*)m_pPlayer.get())->AddObject(m_ppShaders);
			break;
		case 'C':
			if (m_pBBObjects && !m_pBBObjects->m_bCollisionBoxWireFrame) m_pBBObjects->m_bCollisionBoxWireFrame = true;
			else if (m_pBBObjects && m_pBBObjects->m_bCollisionBoxWireFrame) m_pBBObjects->m_bCollisionBoxWireFrame = false;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

//--ProcessAnimation : Scene_Test----------------------------------------------------
void Scene_Test::AnimateObjects(float fTimeElapsed)
{
	CScene::AnimateObjects(fTimeElapsed);
}

//--ProcessOutput : Scene_Test-------------------------------------------------------
void Scene_Test::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::OnPrepareRender(pd3dCommandList, pCamera);
}
void Scene_Test::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::Render(pd3dCommandList, pCamera);
}

//-------------------------------------------------------------------------------
/*	Objects																	   */
//-------------------------------------------------------------------------------

//--CRotatingObject_1--------------------------------------------------------------
CRotatingObject::CRotatingObject(int nMeshes) : CGameObject(nMeshes, 1)
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}
CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

//--COceanObject_1-----------------------------------------------------------------
COceanObject_1::COceanObject_1() : CGameObject(1, 1)
{
}
COceanObject_1::~COceanObject_1()
{
}

void COceanObject_1::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
}
void COceanObject_1::BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; ++i)
		{
			if (m_ppMaterials[i] && m_ppMaterials[i]->m_pTexture)
			{
				m_fTime += fTimeElapsed;
				if (m_fTime >= m_fSpeed) m_fTime = 0.0f;
				WaterFlows(&m_ppMaterials[i]->m_pTexture->m_xmf4x4Texture, m_fTime);
			}
		}
	}
}
void COceanObject_1::WaterFlows(XMFLOAT4X4* textureUV, float fTime)
{
	if (fTime == 0.0f)
	{
		textureUV->_13 -= 0.1f;
		textureUV->_23 += 0.1f;
		if (textureUV->_13 < 0.f) textureUV->_13 = 1.f;
		if (textureUV->_23 > 1.f) textureUV->_23 = 0.f;
	}
}

//--CMultiSpriteObject_1-----------------------------------------------------------
CMultiSpriteObject_1::CMultiSpriteObject_1(bool bAct, int col, int row) : CGameObject(1, 1)
{
	bActive = bAct; m_nCols = col; m_nRows = row;
}
CMultiSpriteObject_1::~CMultiSpriteObject_1()
{
}

void CMultiSpriteObject_1::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (/*m_nMaterials > 0 && */bActive)
	{
		//for (int i = 0; i < m_nMaterials; ++i)
		//{
			//if (m_ppMaterials[i] && m_ppMaterials[i]->m_pTexture)
			//{
		m_fTime += fTimeElapsed;
		{
			int sprite_num = m_nCols * m_nRows;
			float normalizeTime = m_fTime / cooltime;
			int current_sprite = int(normalizeTime * sprite_num);

			m_xmf4x4Texture._11 = 1.0f / float(m_nRows);
			m_xmf4x4Texture._22 = 1.0f / float(m_nCols);
			m_xmf4x4Texture._13 = float(current_sprite % m_nCols) / float(m_nCols);
			m_xmf4x4Texture._23 = float(current_sprite / m_nRows) / float(m_nRows);

			if (m_fTime > cooltime)
			{
				bActive = false;
				m_fTime = 0.0f;
			}
		}
		//m_ppMaterials[i]->m_pTexture->AnimateRowColumn(bActive, m_fTime, cooltime);
	//}
	//}
	}
}


//--CBillboardObject_1-------------------------------------------------------------
CBillboardObject_1::CBillboardObject_1() : CGameObject(1, 1)
{
}
CBillboardObject_1::~CBillboardObject_1()
{
}

void CBillboardObject_1::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_fRotationAngle <= -1.5f) m_fRotationDelta = 1.0f;
	if (m_fRotationAngle >= +1.5f) m_fRotationDelta = -1.0f;
	m_fRotationAngle += m_fRotationDelta * fTimeElapsed;

	Rotate(0.0f, 0.0f, m_fRotationAngle);
}
void CBillboardObject_1::BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; ++i)
		{
			if (m_ppMaterials[i] && m_ppMaterials[i]->m_pTexture)
			{
				m_fTime += fTimeElapsed;
				if (m_fTime >= m_fSpeed) m_fTime = 0.0f;
				SwayInTheWind(&m_ppMaterials[i]->m_pTexture->m_xmf4x4Texture, m_fTime);
			}
		}
	}
}
void CBillboardObject_1::SwayInTheWind(XMFLOAT4X4* textureUV, float fTime)
{
	const float side_push = 0.35f;
	static bool turn = false;
	if (fTime == 0.0f)
	{
		if (turn)
		{
			textureUV->_12 += 0.02f;
			textureUV->_13 -= 0.02f;
			if (textureUV->_12 > side_push) turn = false;
		}
		else
		{
			textureUV->_12 -= 0.02f;
			textureUV->_13 += 0.02f;
			if (textureUV->_12 < -side_push) turn = true;
		}
	}
}