#include "stdafx.h"
#include "GameObject.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	memset(&m_d3dPositionBufferView, 0, sizeof(m_d3dPositionBufferView));
	memset(&m_d3dIndexBufferView, 0, sizeof(m_d3dIndexBufferView));
	memset(&m_d3dVertexBufferView, 0, sizeof(m_d3dVertexBufferView));
}
CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const CMesh& other) : CMesh(pd3dDevice, pd3dCommandList)
{
}
CMesh::~CMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
		if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
	}
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
}
void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;
	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}
void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
}
void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList, NULL);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}
void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList, NULL, d3dInstancingBufferView);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], nInstances, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstances, m_nOffset, 0);
	}
}

int CMesh::CheckRayIntersection(XMFLOAT3& xmf3RayOrigin, XMFLOAT3& xmf3RayDirection, float* pfNearHitDistance, XMFLOAT4X4& xmf4x4World)
{
	int nIntersections = 0;
	BYTE* pbPositions = (BYTE*)m_pxmf3Positions;

	int nOffset = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? 3 : 1;
	int nPrimitives = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nVertices / 3) : (m_nVertices - 2);
	if (m_nSubMeshes > 0) {
		nPrimitives = 0;
		for (int i = 0; i < m_nSubMeshes; ++i) nPrimitives += (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_pnSubSetIndices[i] / 3) : (m_pnSubSetIndices[i] - 2);
	}

	XMVECTOR xmRayOrigin = XMLoadFloat3(&xmf3RayOrigin);
	XMVECTOR xmRayDirection = XMLoadFloat3(&xmf3RayDirection);
	
	bool bIntersected = m_xmBoundingBox.Intersects(xmRayOrigin, xmRayDirection, *pfNearHitDistance);
	if (bIntersected)
	{
		//*pfNearHitDistance = *pfNearHitDistance * ReduceScale * sqrt(xmRayDirection.m128_f32[0] * xmRayDirection.m128_f32[0] + xmRayDirection.m128_f32[1] * xmRayDirection.m128_f32[1] + xmRayDirection.m128_f32[2] * xmRayDirection.m128_f32[2]);
		return bIntersected;
	}

	return nIntersections;
}
bool CMesh::RayIntersectsTriangle(const XMFLOAT3& Origin, const XMFLOAT3& Direction, float& Distance, float ReduceScale)
{
	FXMVECTOR xmOrigin = XMLoadFloat3(&Origin);
	FXMVECTOR xmDirection = XMLoadFloat3(&Direction);
	// 광선과 삼각형의 충돌 검사
	if (m_pxmf3Positions)
	{
		if (0 < m_nSubMeshes)
		{
			const UINT SubMesh0 = 0;	// 현재 프로젝트에서는 0번 SubMesh밖에 없음.
			int nSubIndices = m_pnSubSetIndices[SubMesh0];
			for (int i = 0; i < nSubIndices / 3; ++i)
			{
				float distance;

				UINT iTriangle = i * 3;
				UINT SubIndex0 = m_ppnSubSetIndices[SubMesh0][iTriangle];
				UINT SubIndex1 = m_ppnSubSetIndices[SubMesh0][iTriangle + 1];
				UINT SubIndex2 = m_ppnSubSetIndices[SubMesh0][iTriangle + 2];

				bool intersect = TriangleTests::Intersects(XMLoadFloat3(&Origin), XMLoadFloat3(&Direction),
					XMLoadFloat3(&m_pxmf3Positions[SubIndex0]), XMLoadFloat3(&m_pxmf3Positions[SubIndex1]), XMLoadFloat3(&m_pxmf3Positions[SubIndex2]), distance);

				if (intersect && distance < Distance)
				{
					XMVECTOR xmRayDirection = XMLoadFloat3(&Direction);
					Distance = distance * ReduceScale * 
						sqrt(xmRayDirection.m128_f32[0] * xmRayDirection.m128_f32[0] + 
							xmRayDirection.m128_f32[1] * xmRayDirection.m128_f32[1] + 
							xmRayDirection.m128_f32[2] * xmRayDirection.m128_f32[2]);
					
					//std::cout << "Primitive Index: " << i << std::endl;
					return true;
				}
			}
		}
	}
	return false;
}

//-------------------------------------------------------------------------------
/*	CStandardMesh : public CMesh											   */
//-------------------------------------------------------------------------------
CStandardMesh::CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}
CStandardMesh::~CStandardMesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	if (m_pd3dBiTangentBuffer) m_pd3dBiTangentBuffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3BiTangents) delete[] m_pxmf3BiTangents;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}
void CStandardMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dColorUploadBuffer) m_pd3dColorUploadBuffer->Release();
	m_pd3dColorUploadBuffer = NULL;

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;

	if (m_pd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	m_pd3dTangentUploadBuffer = NULL;

	if (m_pd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	m_pd3dBiTangentUploadBuffer = NULL;
}

void CStandardMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

void CStandardMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	BYTE nStrLength = 0;

	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(m_pstrMeshName, sizeof(char), nStrLength, pInFile);
	m_pstrMeshName[nStrLength] = '\0';

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_xmBoundingBox = BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);
				m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

				m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
				m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

				m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

				m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
				m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
				m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];
				for (int i = 0; i < m_nSubMeshes; ++i)
				{
					m_ppd3dSubSetIndexBuffers[i] = NULL;
					m_ppd3dSubSetIndexUploadBuffers[i] = NULL;
				}

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
					nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
					pstrToken[nStrLength] = '\0';
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile);
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], 1, pInFile);

							m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

							m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

//-------------------------------------------------------------------------------
/*	CBoundingBoxMesh : public CMesh											   */
//-------------------------------------------------------------------------------
CBoundingBoxMesh::CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const XMFLOAT3& Extents, const XMFLOAT3& Center, XMFLOAT3& Scale, XMFLOAT4X4& WorldTransform) : CMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// BoundingBox setting.
	m_xmf3AABBCenter = Center;
	m_xmf3AABBExtents = Extents;

	// Position setting.
	float fx = Extents.x, fy = Extents.y, fz = Extents.z;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, -fz);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fy, -fz);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, +fy, +fz);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, +fy, +fz);
	m_pxmf3Positions[4] = XMFLOAT3(-fx, -fy, -fz);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fy, -fz);
	m_pxmf3Positions[6] = XMFLOAT3(+fx, -fy, +fz);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, -fy, +fz);

	// PositionBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// SubSetIndex setting.
	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	m_pnSubSetIndices[0] = 36;
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];
	m_ppnSubSetIndices[0][0] = 3; m_ppnSubSetIndices[0][1] = 1; m_ppnSubSetIndices[0][2] = 0;
	m_ppnSubSetIndices[0][3] = 2; m_ppnSubSetIndices[0][4] = 1; m_ppnSubSetIndices[0][5] = 3;
	m_ppnSubSetIndices[0][6] = 0; m_ppnSubSetIndices[0][7] = 5; m_ppnSubSetIndices[0][8] = 4;
	m_ppnSubSetIndices[0][9] = 1; m_ppnSubSetIndices[0][10] = 5; m_ppnSubSetIndices[0][11] = 0;
	m_ppnSubSetIndices[0][12] = 3; m_ppnSubSetIndices[0][13] = 4; m_ppnSubSetIndices[0][14] = 7;
	m_ppnSubSetIndices[0][15] = 0; m_ppnSubSetIndices[0][16] = 4; m_ppnSubSetIndices[0][17] = 3;
	m_ppnSubSetIndices[0][18] = 1; m_ppnSubSetIndices[0][19] = 6; m_ppnSubSetIndices[0][20] = 5;
	m_ppnSubSetIndices[0][21] = 2; m_ppnSubSetIndices[0][22] = 6; m_ppnSubSetIndices[0][23] = 1;
	m_ppnSubSetIndices[0][24] = 2; m_ppnSubSetIndices[0][25] = 7; m_ppnSubSetIndices[0][26] = 6;
	m_ppnSubSetIndices[0][27] = 3; m_ppnSubSetIndices[0][28] = 7; m_ppnSubSetIndices[0][29] = 2;
	m_ppnSubSetIndices[0][30] = 6; m_ppnSubSetIndices[0][31] = 4; m_ppnSubSetIndices[0][32] = 5;
	m_ppnSubSetIndices[0][33] = 7; m_ppnSubSetIndices[0][34] = 4; m_ppnSubSetIndices[0][35] = 6;


	// SubSetIndexBuffer setting.
	m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];

	// Apply WorldTransform to BoundingBox 
	XMFLOAT4X4 world = WorldTransform;
	CenterTransform = Matrix4x4::Identity();
	CenterTransform._41 = Center.x; CenterTransform._42 = Center.y; CenterTransform._43 = Center.z;
	CenterTransform._11 = Scale.x; CenterTransform._22 = Scale.y; CenterTransform._33 = Scale.z;
	CenterTransform = Matrix4x4::Multiply(CenterTransform, WorldTransform);
	BoundingScale = Scale;
}
CBoundingBoxMesh::~CBoundingBoxMesh()
{
}

//-------------------------------------------------------------------------------
/*	CSkyBoxMesh : public CMesh												   */
//-------------------------------------------------------------------------------
CSkyBoxMesh::CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 36;
	m_nStride = sizeof(XMFLOAT3);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Vertices setting.
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = m_nStride;
	m_d3dPositionBufferView.SizeInBytes = m_nStride * m_nVertices;
}
CSkyBoxMesh::~CSkyBoxMesh()
{
}

//-------------------------------------------------------------------------------
/*	CCubeMeshDiffused : public CStandardMesh							       */
//-------------------------------------------------------------------------------
CCubeMeshDiffused::CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 36;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Vertices setting.
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf3Normals = new XMFLOAT3[m_nVertices];

	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, -fz); m_pxmf3Normals[0] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fy, -fz); m_pxmf3Normals[1] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, -fz); m_pxmf3Normals[2] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[3] = XMFLOAT3(-fx, +fy, -fz); m_pxmf3Normals[3] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, -fy, -fz); m_pxmf3Normals[4] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[5] = XMFLOAT3(-fx, -fy, -fz); m_pxmf3Normals[5] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[6] = XMFLOAT3(-fx, +fy, +fz); m_pxmf3Normals[6] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[7] = XMFLOAT3(+fx, +fy, +fz); m_pxmf3Normals[7] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, +fy, -fz); m_pxmf3Normals[8] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[9] = XMFLOAT3(-fx, +fy, +fz); m_pxmf3Normals[9] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[10] = XMFLOAT3(+fx, +fy, -fz); m_pxmf3Normals[10] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, +fy, -fz); m_pxmf3Normals[11] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[12] = XMFLOAT3(-fx, -fy, +fz); m_pxmf3Normals[12] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[13] = XMFLOAT3(+fx, -fy, +fz); m_pxmf3Normals[13] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[14] = XMFLOAT3(+fx, +fy, +fz); m_pxmf3Normals[14] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fy, +fz); m_pxmf3Normals[15] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[16] = XMFLOAT3(+fx, +fy, +fz); m_pxmf3Normals[16] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, +fy, +fz); m_pxmf3Normals[17] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[18] = XMFLOAT3(-fx, -fy, -fz); m_pxmf3Normals[18] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, -fy, -fz); m_pxmf3Normals[19] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fy, +fz); m_pxmf3Normals[20] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[21] = XMFLOAT3(-fx, -fy, -fz); m_pxmf3Normals[21] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, -fy, +fz); m_pxmf3Normals[22] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[23] = XMFLOAT3(-fx, -fy, +fz); m_pxmf3Normals[23] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fy, +fz); m_pxmf3Normals[24] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[25] = XMFLOAT3(-fx, +fy, -fz); m_pxmf3Normals[25] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, -fy, -fz); m_pxmf3Normals[26] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fy, +fz); m_pxmf3Normals[27] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[28] = XMFLOAT3(-fx, -fy, -fz); m_pxmf3Normals[28] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[29] = XMFLOAT3(-fx, -fy, +fz); m_pxmf3Normals[29] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[30] = XMFLOAT3(+fx, +fy, -fz); m_pxmf3Normals[30] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, +fy, +fz); m_pxmf3Normals[31] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[32] = XMFLOAT3(+fx, -fy, +fz); m_pxmf3Normals[32] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	m_pxmf3Positions[33] = XMFLOAT3(+fx, +fy, -fz); m_pxmf3Normals[33] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fy, +fz); m_pxmf3Normals[34] = XMFLOAT3(0.5f, 0.5f, 1.0f);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fy, -fz); m_pxmf3Normals[35] = XMFLOAT3(0.5f, 0.5f, 1.0f);

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// BoundingOrientedBox setting.
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
CCubeMeshDiffused::~CCubeMeshDiffused()
{
}

//-------------------------------------------------------------------------------
/*	CCubeMeshTextured : public CStandardMesh								   */
//-------------------------------------------------------------------------------
CCubeMeshTextured::CCubeMeshTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 36;
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	m_xmf3AABBCenter = XMFLOAT3(0.f, 0.f, 0.f);
	m_xmf3AABBExtents = XMFLOAT3(fx, fy, fz);

	// Vertices setting.
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, -fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fy, -fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, -fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(1.0f, 1.0f);

	m_pxmf3Positions[3] = XMFLOAT3(-fx, +fy, -fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, -fy, -fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[5] = XMFLOAT3(-fx, -fy, -fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(0.0f, 1.0f);

	m_pxmf3Positions[6] = XMFLOAT3(-fx, +fy, +fz); m_pxmf2TextureCoords0[6] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[7] = XMFLOAT3(+fx, +fy, +fz); m_pxmf2TextureCoords0[7] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, +fy, -fz); m_pxmf2TextureCoords0[8] = XMFLOAT2(1.0f, 1.0f);

	m_pxmf3Positions[9] = XMFLOAT3(-fx, +fy, +fz); m_pxmf2TextureCoords0[9] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[10] = XMFLOAT3(+fx, +fy, -fz); m_pxmf2TextureCoords0[10] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, +fy, -fz); m_pxmf2TextureCoords0[11] = XMFLOAT2(0.0f, 1.0f);

	m_pxmf3Positions[12] = XMFLOAT3(-fx, -fy, +fz); m_pxmf2TextureCoords0[12] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[13] = XMFLOAT3(+fx, -fy, +fz); m_pxmf2TextureCoords0[13] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[14] = XMFLOAT3(+fx, +fy, +fz); m_pxmf2TextureCoords0[14] = XMFLOAT2(1.0f, 1.0f);

	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fy, +fz); m_pxmf2TextureCoords0[15] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[16] = XMFLOAT3(+fx, +fy, +fz); m_pxmf2TextureCoords0[16] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, +fy, +fz); m_pxmf2TextureCoords0[17] = XMFLOAT2(0.0f, 1.0f);

	m_pxmf3Positions[18] = XMFLOAT3(-fx, -fy, -fz); m_pxmf2TextureCoords0[18] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, -fy, -fz); m_pxmf2TextureCoords0[19] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fy, +fz); m_pxmf2TextureCoords0[20] = XMFLOAT2(1.0f, 1.0f);

	m_pxmf3Positions[21] = XMFLOAT3(-fx, -fy, -fz); m_pxmf2TextureCoords0[21] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, -fy, +fz); m_pxmf2TextureCoords0[22] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[23] = XMFLOAT3(-fx, -fy, +fz); m_pxmf2TextureCoords0[23] = XMFLOAT2(0.0f, 1.0f);

	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fy, +fz); m_pxmf2TextureCoords0[24] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[25] = XMFLOAT3(-fx, +fy, -fz); m_pxmf2TextureCoords0[25] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, -fy, -fz); m_pxmf2TextureCoords0[26] = XMFLOAT2(1.0f, 1.0f);

	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fy, +fz); m_pxmf2TextureCoords0[27] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[28] = XMFLOAT3(-fx, -fy, -fz); m_pxmf2TextureCoords0[28] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[29] = XMFLOAT3(-fx, -fy, +fz); m_pxmf2TextureCoords0[29] = XMFLOAT2(0.0f, 1.0f);

	m_pxmf3Positions[30] = XMFLOAT3(+fx, +fy, -fz); m_pxmf2TextureCoords0[30] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, +fy, +fz); m_pxmf2TextureCoords0[31] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[32] = XMFLOAT3(+fx, -fy, +fz); m_pxmf2TextureCoords0[32] = XMFLOAT2(1.0f, 1.0f);

	m_pxmf3Positions[33] = XMFLOAT3(+fx, +fy, -fz); m_pxmf2TextureCoords0[33] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fy, +fz); m_pxmf2TextureCoords0[34] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fy, -fz); m_pxmf2TextureCoords0[35] = XMFLOAT2(0.0f, 1.0f);

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	// BoundingOrientedBox setting.
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
CCubeMeshTextured::~CCubeMeshTextured()
{
}

void CCubeMeshTextured::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
}

//-------------------------------------------------------------------------------
/*	CMeshIlluminated : public CMesh											   */
//-------------------------------------------------------------------------------
CMeshIlluminated::CMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}
CMeshIlluminated::~CMeshIlluminated()
{
}

void CMeshIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices)
{
	int nPrimitives = nVertices / 3;
	UINT nIndex0, nIndex1, nIndex2;
	for (int i = 0; i < nPrimitives; ++i)
	{
		nIndex0 = i * 3 + 0;
		nIndex1 = i * 3 + 1;
		nIndex2 = i * 3 + 2;
		XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
		XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
		pxmf3Normals[nIndex0] = pxmf3Normals[nIndex1] = pxmf3Normals[nIndex2] = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
	}
}
void CMeshIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices / 3) : (nVertices / 3);
	XMFLOAT3 xmf3SumOfNormal, xmf3Edge01, xmf3Edge02, xmf3Normal;
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; ++j)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; ++i)
		{
			nIndex0 = pnIndices[i * 3 + 0];
			nIndex1 = pnIndices[i * 3 + 1];
			nIndex2 = pnIndices[i * 3 + 2];
			if (pnIndices && ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j)))
			{
				xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
				xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
				xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, false);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}
void CMeshIlluminated::CalculateTriangleStripVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices - 2) : (nVertices - 2);
	XMFLOAT3 xmf3SumOfNormal(0.0f, 0.0f, 0.0f);
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; ++j)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; ++i)
		{
			nIndex0 = ((i % 2) == 0) ? (i + 0) : (i + 1);
			if (pnIndices) nIndex0 = pnIndices[nIndex0];
			nIndex1 = ((i % 2) == 0) ? (i + 1) : (i + 0);
			if (pnIndices) nIndex1 = pnIndices[nIndex1];
			nIndex2 = (pnIndices) ? pnIndices[i + 2] : (i + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
				XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
				XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}
void CMeshIlluminated::CalculateVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices, UINT* pnIndices, int nIndices)
{
	switch (m_d3dPrimitiveTopology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (pnIndices)
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		else
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices);
		break;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		CalculateTriangleStripVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------------------
/*	CCrosshairMesh : public CMesh											   */
//-------------------------------------------------------------------------------
CCrosshairMesh::CCrosshairMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fthickness, float flength, float interval, float radDot, bool bDot) : CMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = (bDot) ? 30 : 24;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Position setting.
	float thickness = fthickness * 0.5f, length = flength * 0.5f, fx = 0.5f;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	// Left Cross
	m_pxmf3Positions[0] = XMFLOAT3(-length - interval, +thickness, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+length - interval, +thickness, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-length - interval, -thickness, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-length - interval, -thickness, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+length - interval, +thickness, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+length - interval, -thickness, +fx);
	// Right Cross										
	m_pxmf3Positions[6] = XMFLOAT3(-length + interval, +thickness, +fx);
	m_pxmf3Positions[7] = XMFLOAT3(+length + interval, +thickness, +fx);
	m_pxmf3Positions[8] = XMFLOAT3(-length + interval, -thickness, +fx);
	m_pxmf3Positions[9] = XMFLOAT3(-length + interval, -thickness, +fx);
	m_pxmf3Positions[10] = XMFLOAT3(+length + interval, +thickness, +fx);
	m_pxmf3Positions[11] = XMFLOAT3(+length + interval, -thickness, +fx);
	// Top Cross										
	m_pxmf3Positions[12] = XMFLOAT3(-thickness, +length - interval, +fx);
	m_pxmf3Positions[13] = XMFLOAT3(+thickness, +length - interval, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-thickness, -length - interval, +fx);
	m_pxmf3Positions[15] = XMFLOAT3(-thickness, -length - interval, +fx);
	m_pxmf3Positions[16] = XMFLOAT3(+thickness, +length - interval, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(+thickness, -length - interval, +fx);
	// Bottom Cross										
	m_pxmf3Positions[18] = XMFLOAT3(-thickness, +length + interval, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+thickness, +length + interval, +fx);
	m_pxmf3Positions[20] = XMFLOAT3(-thickness, -length + interval, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(-thickness, -length + interval, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+thickness, +length + interval, +fx);
	m_pxmf3Positions[23] = XMFLOAT3(+thickness, -length + interval, +fx);
	// Dot Cross									
	if (bDot)
	{
		m_pxmf3Positions[24] = XMFLOAT3(-radDot, +radDot, +fx);
		m_pxmf3Positions[25] = XMFLOAT3(+radDot, +radDot, +fx);
		m_pxmf3Positions[26] = XMFLOAT3(-radDot, -radDot, +fx);
		m_pxmf3Positions[27] = XMFLOAT3(-radDot, -radDot, +fx);
		m_pxmf3Positions[28] = XMFLOAT3(+radDot, +radDot, +fx);
		m_pxmf3Positions[29] = XMFLOAT3(+radDot, -radDot, +fx);
	}

	// PositionBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = m_nStride;
	m_d3dPositionBufferView.SizeInBytes = m_nStride * m_nVertices;
}
CCrosshairMesh::~CCrosshairMesh()
{
}

//-------------------------------------------------------------------------------
/*	CRawFormatImage															   */
//-------------------------------------------------------------------------------
CRawFormatImage::CRawFormatImage(LPCTSTR pFileName, int nWidth, int nLength, bool bFlipY)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	BYTE* pRawImagePixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pRawImagePixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	if (bFlipY)
	{
		m_pRawImagePixels = new BYTE[m_nWidth * m_nLength];
		for (int z = 0; z < m_nLength; z++)
		{
			for (int x = 0; x < m_nWidth; x++)
			{
				m_pRawImagePixels[x + ((m_nLength - 1 - z) * m_nWidth)] = pRawImagePixels[x + (z * m_nWidth)];
			}
		}

		if (pRawImagePixels) delete[] pRawImagePixels;
	}
	else
	{
		m_pRawImagePixels = pRawImagePixels;
	}
}
CRawFormatImage::~CRawFormatImage()
{
	if (m_pRawImagePixels) delete[] m_pRawImagePixels;
	m_pRawImagePixels = NULL;
}

//-------------------------------------------------------------------------------
/*	CHeightMapImage															   */
//-------------------------------------------------------------------------------
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale) : CRawFormatImage(pFileName, nWidth, nLength, true)
{
	m_xmf3Scale = xmf3Scale;
}
CHeightMapImage::~CHeightMapImage()
{
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) 
		return XMFLOAT3(0.0f, 1.0f, 0.0f);

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	
	float y1 = (float)m_pRawImagePixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pRawImagePixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pRawImagePixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return xmf3Normal;
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz)
{
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return 0.0f;
	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pRawImagePixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pRawImagePixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pRawImagePixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pRawImagePixels[(x + 1) + ((z + 1) * m_nWidth)];

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	bool bRightToLeft = ((z % 2) != 0);
	if (bRightToLeft)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return fHeight;
}

//-------------------------------------------------------------------------------
/*	CHeightMapGridMesh : public CMesh										   */
//-------------------------------------------------------------------------------
CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CHeightMapTerrain* heightMapTerrain, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_nVertices = nWidth * nLength;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;

	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	int cxHeightMap = pHeightMapImage->GetRawImageWidth();
	int czHeightMap = pHeightMapImage->GetRawImageLength();

	// Vertices setting.
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];
	m_pxmf3Normals = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];

	float detailMap_Row = 12, detailMap_Col = 20;
	float detailMap_wPixel = cxHeightMap / detailMap_Row;
	float detailMap_hPixel = czHeightMap / detailMap_Col;

	fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);
			m_pxmf3Positions[i] = XMFLOAT3((x * m_xmf3Scale.x), fHeight, (z * m_xmf3Scale.z));
			m_pxmf4Colors[i] = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);
			m_pxmf3Normals[i] = heightMapTerrain->GetNormal(x, z);
			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / detailMap_wPixel, float(z) / detailMap_hPixel);
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	int fxMax = std::max_element(m_pxmf3Positions, m_pxmf3Positions + m_nVertices, [](XMFLOAT3& lhs, XMFLOAT3 rhs) {return lhs.x < rhs.x; })->x;
	int fxMin = std::min_element(m_pxmf3Positions, m_pxmf3Positions + m_nVertices, [](XMFLOAT3& lhs, XMFLOAT3 rhs) {return lhs.x < rhs.x; })->x;
	int fyMax = std::max_element(m_pxmf3Positions, m_pxmf3Positions + m_nVertices, [](XMFLOAT3& lhs, XMFLOAT3 rhs) {return lhs.y < rhs.y; })->y;
	int fyMin = std::min_element(m_pxmf3Positions, m_pxmf3Positions + m_nVertices, [](XMFLOAT3& lhs, XMFLOAT3 rhs) {return lhs.y < rhs.y; })->y;
	int fzMax = std::max_element(m_pxmf3Positions, m_pxmf3Positions + m_nVertices, [](XMFLOAT3& lhs, XMFLOAT3 rhs) {return lhs.z < rhs.z; })->z;
	int fzMin = std::min_element(m_pxmf3Positions, m_pxmf3Positions + m_nVertices, [](XMFLOAT3& lhs, XMFLOAT3 rhs) {return lhs.z < rhs.z; })->z;

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);
	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);
	m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	m_pnSubSetIndices[0] = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];

	// BoundingOrientedBox setting.
	float fx = (fxMax - fxMin) * 0.5f, fy = (fyMax - fyMin) * 0.5f, fz = (fzMax - fzMin) * 0.5f;
	if (fxMax - fxMin < FLT_MIN) fx = 0.1f;
	if (fyMax - fyMin < FLT_MIN) fy = 0.1f;
	if (fzMax - fzMin < FLT_MIN) fz = 0.1f;

	m_xmf3AABBCenter = XMFLOAT3(fx, fyMax, fz);
	m_xmf3AABBExtents = XMFLOAT3(fx, fy, fz);
	m_xmBoundingBox = BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
CHeightMapGridMesh::~CHeightMapGridMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dTextureCoord1Buffer) m_pd3dTextureCoord1Buffer->Release();
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();

	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}
void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
	if (m_pd3dColorUploadBuffer) m_pd3dColorUploadBuffer->Release();
	m_pd3dColorUploadBuffer = NULL;
	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;
	if (m_pd3dTextureCoord1UploadBuffer) m_pd3dTextureCoord1UploadBuffer->Release();
	m_pd3dTextureCoord1UploadBuffer = NULL;
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dColorBufferView, m_d3dNormalBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetRawImagePixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetRawImageWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	
	return fHeight;
}
XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);

	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);

	return xmf4Color;
}

//-------------------------------------------------------------------------------
/*	CTexturedRectMesh : public CStandardMesh								   */
//-------------------------------------------------------------------------------
CTexturedRectMesh::CTexturedRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 offset, float fWidth, float fHeight, float fDepth) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Position setting.
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	m_pxmf3Positions[0] = XMFLOAT3(+fx + offset.x, +fy + offset.y, fz + offset.z), m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[1] = XMFLOAT3(+fx + offset.x, -fy + offset.y, fz + offset.z), m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[2] = XMFLOAT3(-fx + offset.x, -fy + offset.y, fz + offset.z), m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf3Positions[3] = XMFLOAT3(-fx + offset.x, -fy + offset.y, fz + offset.z), m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf3Positions[4] = XMFLOAT3(-fx + offset.x, +fy + offset.y, fz + offset.z), m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[5] = XMFLOAT3(+fx + offset.x, +fy + offset.y, fz + offset.z), m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	// BoundingOrientedBox setting.
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
CTexturedRectMesh::CTexturedRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Position setting.
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	float fx = fWidth / FRAME_BUFFER_WIDTH, fy = fHeight / FRAME_BUFFER_HEIGHT, fz = fDepth;
	float fxp = (fxPosition - FRAME_BUFFER_WIDTH * 0.5f) / (FRAME_BUFFER_WIDTH * 0.5f);
	float fyp = (fyPosition - FRAME_BUFFER_HEIGHT * 0.5f) / (FRAME_BUFFER_HEIGHT * 0.5f);

	m_pxmf3Positions[0] = XMFLOAT3(+fx + fxp, +fy - fyp, fz), m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[1] = XMFLOAT3(+fx + fxp, -fy - fyp, fz), m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[2] = XMFLOAT3(-fx + fxp, -fy - fyp, fz), m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf3Positions[3] = XMFLOAT3(-fx + fxp, -fy - fyp, fz), m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf3Positions[4] = XMFLOAT3(-fx + fxp, +fy - fyp, fz), m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[5] = XMFLOAT3(+fx + fxp, +fy - fyp, fz), m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	// BoundingOrientedBox setting.
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
CTexturedRectMesh::~CTexturedRectMesh()
{
}

void CTexturedRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}

void CTexturedRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, d3dInstancingBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, nInstances, m_nOffset, 0);
}

//-------------------------------------------------------------------------------
/*	CSkinnedMesh : public CStandardMesh									       */
//-------------------------------------------------------------------------------
CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}
CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const CSkinnedMesh& other) : CSkinnedMesh(pd3dDevice, pd3dCommandList)
{
	m_nBonesPerVertex = other.m_nBonesPerVertex;
	m_RootBoneIndex = other.m_RootBoneIndex;

	memcpy(m_pxmn4BoneIndices, other.m_pxmn4BoneIndices, sizeof(XMINT4));
	memcpy(m_pxmf4BoneWeights, other.m_pxmf4BoneWeights, sizeof(XMFLOAT4));

//	ID3D12Resource* m_pd3dBoneIndexBuffer = NULL;
//	ID3D12Resource* m_pd3dBoneIndexUploadBuffer = NULL;
//	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneIndexBufferView;
//
//	ID3D12Resource* m_pd3dBoneWeightBuffer = NULL;
//	ID3D12Resource* m_pd3dBoneWeightUploadBuffer = NULL;
//	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneWeightBufferView;
//
//public:
//	int								m_nSkinningBones = 0;
//
//	char(*m_ppstrSkinningBoneNames)[64];
//	CGameObject** m_ppSkinningBoneFrameCaches = NULL; //[m_nSkinningBones]
//
//	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL; //Transposed
//
//	ID3D12Resource* m_pd3dcbBindPoseBoneOffsets = NULL;
//	XMFLOAT4X4* m_pcbxmf4x4MappedBindPoseBoneOffsets = NULL;
//
//	ID3D12Resource* m_pd3dcbSkinningBoneTransforms = NULL; //Pointer Only
//	XMFLOAT4X4* m_pcbxmf4x4MappedSkinningBoneTransforms = NULL;
}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pxmn4BoneIndices) delete[] m_pxmn4BoneIndices;
	if (m_pxmf4BoneWeights) delete[] m_pxmf4BoneWeights;

	if (m_ppSkinningBoneFrameCaches) delete[] m_ppSkinningBoneFrameCaches;
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;
	if (m_pd3dcbBindPoseBoneOffsets) m_pd3dcbBindPoseBoneOffsets->Release();

	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();

	ReleaseShaderVariables();
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dcbBindPoseBoneOffsets)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBindPoseBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_BONE_OFFSET, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets
	}

	if (m_pd3dcbSkinningBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbSkinningBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_BONE_TRANSFORM, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms

		for (int j = 0; j < m_nSkinningBones; j++)
		{
			XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j], XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[j]->m_xmf4x4World)));
		}
	}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer) m_pd3dBoneIndexUploadBuffer->Release();
	m_pd3dBoneIndexUploadBuffer = NULL;

	if (m_pd3dBoneWeightUploadBuffer) m_pd3dBoneWeightUploadBuffer->Release();
	m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::PrepareSkinning(CGameObject* pModelRootObject)
{
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		m_ppSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
			m_xmBoundingBox = BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				m_ppSkinningBoneFrameCaches = new CGameObject * [m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
					m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<RootBoneIndex>:"))
		{
			m_RootBoneIndex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
				m_pd3dcbBindPoseBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
				m_pd3dcbBindPoseBoneOffsets->Map(0, NULL, (void**)&m_pcbxmf4x4MappedBindPoseBoneOffsets);

				for (int i = 0; i < m_nSkinningBones; i++)
				{
					XMStoreFloat4x4(&m_pcbxmf4x4MappedBindPoseBoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[7] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, pVertexBufferViews);
}
void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[8] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView, d3dInstancingBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 8, pVertexBufferViews);
}

//-------------------------------------------------------------------------------
/*	CParticleMesh : public CMesh											   */
//-------------------------------------------------------------------------------
CParticleMesh::CParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles, UINT nParticleType) : CMesh(pd3dDevice, pd3dCommandList)
{
	CreateVertexBuffer(pd3dDevice, pd3dCommandList, xmf3Position, xmf3Velocity, fLifetime, xmf3Acceleration, xmf3Color, xmf2Size, nParticleType);
	CreateStreamOutputBuffer(pd3dDevice, pd3dCommandList, nMaxParticles);
}

void CParticleMesh::CreateVertexBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nParticleType)
{
	m_nVertices = 1;
	m_nStride = sizeof(CParticleVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	CParticleVertex pVertices[1];

	pVertices[0].m_xmf3Position = xmf3Position;
	pVertices[0].m_xmf3Velocity = xmf3Velocity;
	pVertices[0].m_fLifetime = fLifetime;
	pVertices[0].m_nType = nParticleType;

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	// BoundingOrientedBox setting.
	float fx = xmf2Size.x * 0.5f;
	float fy = xmf2Size.y * 0.5f;
	float fz = 0.1f;
	m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3AABBExtents = XMFLOAT3(fx, fy, fz);
	m_xmBoundingBox = BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

void CParticleMesh::CreateStreamOutputBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaxParticles)
{
	m_nMaxParticles = nMaxParticles;

	m_pd3dStreamOutputBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_nMaxParticles), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);
	m_pd3dDrawBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_nMaxParticles), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	UINT64 nBufferFilledSize = 0;
	m_pd3dDefaultBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, &nBufferFilledSize, sizeof(UINT64), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

	m_pd3dUploadBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dUploadBufferFilledSize->Map(0, NULL, (void**)&m_pnUploadBufferFilledSize);

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	D3D12_QUERY_HEAP_DESC d3dQueryHeapDesc = { };
	d3dQueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
	d3dQueryHeapDesc.Count = 1;
	d3dQueryHeapDesc.NodeMask = 0;
	pd3dDevice->CreateQueryHeap(&d3dQueryHeapDesc, __uuidof(ID3D12QueryHeap), (void**)&m_pd3dSOQueryHeap);

	m_pd3dSOQueryBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(D3D12_QUERY_DATA_SO_STATISTICS), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
#else
	m_pd3dReadBackBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
#endif
}

CParticleMesh::~CParticleMesh()
{
	if (m_pd3dStreamOutputBuffer) m_pd3dStreamOutputBuffer->Release();
	if (m_pd3dDrawBuffer) m_pd3dDrawBuffer->Release();
	if (m_pd3dDefaultBufferFilledSize) m_pd3dDefaultBufferFilledSize->Release();
	if (m_pd3dUploadBufferFilledSize) m_pd3dUploadBufferFilledSize->Release();

	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	if (m_pd3dSOQueryBuffer) m_pd3dSOQueryBuffer->Release();
	if (m_pd3dSOQueryHeap) m_pd3dSOQueryHeap->Release();
#else
	if (m_pd3dReadBackBufferFilledSize) m_pd3dReadBackBufferFilledSize->Release();
#endif
}
void CParticleMesh::ReleaseUploadBuffers()
{
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;
	m_pd3dIndexUploadBuffer = NULL;
}

void CParticleMesh::PreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 0)
	{
		if (m_bStart)
		{
			m_bStart = false;

			m_nVertices = 1;

			m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
			m_d3dVertexBufferView.StrideInBytes = m_nStride;
			m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
		}
		else
		{
			m_d3dVertexBufferView.BufferLocation = m_pd3dDrawBuffer->GetGPUVirtualAddress();
			m_d3dVertexBufferView.StrideInBytes = m_nStride;
			m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
		}
		m_d3dStreamOutputBufferView.BufferLocation = m_pd3dStreamOutputBuffer->GetGPUVirtualAddress();
		m_d3dStreamOutputBufferView.SizeInBytes = m_nStride * m_nMaxParticles;
		m_d3dStreamOutputBufferView.BufferFilledSizeLocation = m_pd3dDefaultBufferFilledSize->GetGPUVirtualAddress();

		//		*m_pnUploadBufferFilledSize = m_nStride * m_nVertices;
		*m_pnUploadBufferFilledSize = 0;

		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST);
		pd3dCommandList->CopyResource(m_pd3dDefaultBufferFilledSize, m_pd3dUploadBufferFilledSize);
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);
	}
	else if (nPipelineState == 1)
	{
		m_d3dVertexBufferView.BufferLocation = m_pd3dDrawBuffer->GetGPUVirtualAddress();
		m_d3dVertexBufferView.StrideInBytes = m_nStride;
		m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
	}
}

void CParticleMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstances)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, nInstances, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstances, m_nOffset, 0);
	}
}
void CParticleMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 0)
	{
		D3D12_STREAM_OUTPUT_BUFFER_VIEW pStreamOutputBufferViews[1] = { m_d3dStreamOutputBufferView };
		pd3dCommandList->SOSetTargets(0, 1, pStreamOutputBufferViews);

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
		pd3dCommandList->BeginQuery(m_pd3dSOQueryHeap, D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
		CMesh::Render(pd3dCommandList);
		pd3dCommandList->EndQuery(m_pd3dSOQueryHeap, D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);

		pd3dCommandList->ResolveQueryData(m_pd3dSOQueryHeap, D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0, 1, m_pd3dSOQueryBuffer, 0);
#else
		CParticleMesh::Render(pd3dCommandList); //Stream Output to m_pd3dStreamOutputBuffer

		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dCommandList->CopyResource(m_pd3dReadBackBufferFilledSize, m_pd3dDefaultBufferFilledSize);
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);
#endif
	}
	else if (nPipelineState == 1)
	{
		pd3dCommandList->SOSetTargets(0, 1, NULL);

		CParticleMesh::Render(pd3dCommandList); //Render m_pd3dDrawBuffer 
	}
}

void CParticleMesh::PostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 1)
	{
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDrawBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_STREAM_OUT);

		::SwapResourcePointer(&m_pd3dDrawBuffer, &m_pd3dStreamOutputBuffer);
	}
}

#define _WITH_DEBUG_STREAM_OUTPUT_VERTICES

void CParticleMesh::OnPostRender(int nPipelineState)
{
	if (nPipelineState == 0)
	{
#ifdef _WITH_QUERY_DATA_SO_STATISTICS
		D3D12_RANGE d3dReadRange = { 0, 0 };
		UINT8* pBufferDataBegin = NULL;
		m_pd3dSOQueryBuffer->Map(0, &d3dReadRange, (void**)&m_pd3dSOQueryDataStatistics);
		if (m_pd3dSOQueryDataStatistics) m_nVertices = (UINT)m_pd3dSOQueryDataStatistics->NumPrimitivesWritten;
		m_pd3dSOQueryBuffer->Unmap(0, NULL);
#else
		UINT64* pnReadBackBufferFilledSize = NULL;
		m_pd3dReadBackBufferFilledSize->Map(0, NULL, (void**)&pnReadBackBufferFilledSize);
		m_nVertices = UINT(*pnReadBackBufferFilledSize) / m_nStride;
		m_pd3dReadBackBufferFilledSize->Unmap(0, NULL);
#endif

		//::gnCurrentParticles = m_nVertices;
#ifdef _WITH_DEBUG_STREAM_OUTPUT_VERTICES
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Stream Output Vertices = %d\n"), m_nVertices);
		OutputDebugString(pstrDebug);
#endif
		if ((m_nVertices == 0) || (m_nVertices >= MAX_PARTICLES)) m_bStart = true;
	}
}


CNeonParticleMesh::CNeonParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles)
	: CParticleMesh(pd3dDevice, pd3dCommandList, xmf3Position, xmf3Velocity, fLifetime, xmf3Acceleration, xmf3Color, xmf2Size, nMaxParticles, PARTICLE_TYPE_STAR)
{
}
CNeonParticleMesh::~CNeonParticleMesh()
{
}

//-------------------------------------------------------------------------------
/*	CPistolBulletMesh : public CMesh										   */
//-------------------------------------------------------------------------------
CPistolBulletMesh::CPistolBulletMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT2 xmf2Size) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
	// default setting.
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Position setting.
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	float fx = xmf2Size.x * 0.5f, fy = xmf2Size.y * 0.5f, fz = 0.0f;

	m_pxmf3Positions[0] = XMFLOAT3(+fx, +fy, fz), m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, -fy, fz), m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fy, fz), m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fy, fz), m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf3Positions[4] = XMFLOAT3(-fx, +fy, fz), m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, +fy, fz), m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);

	// VertexBuffer setting.
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	// BoundingOrientedBox setting.
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
CPistolBulletMesh::~CPistolBulletMesh()
{
}

void CPistolBulletMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}