#include "GameObject.h"
#include <algorithm>

CGameObject::CGameObject(int nMaterials)
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf3Scale = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3PrevScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_Mass = 0;

	m_nMaterials = nMaterials;
}
CGameObject::CGameObject(const CGameObject& pGameObject)
{
	m_xmf4x4World = pGameObject.m_xmf4x4World;
	m_xmf4x4Transform = pGameObject.m_xmf4x4Transform;
	m_xmf3Scale = pGameObject.m_xmf3Scale;
	m_xmf3PrevScale = pGameObject.m_xmf3PrevScale;
	m_Mass = pGameObject.m_Mass;

	m_nMaterials = 0;

	if (pGameObject.m_pChild) m_pChild = new CGameObject(*pGameObject.m_pChild);
	if (pGameObject.m_pSibling) m_pSibling = new CGameObject(*pGameObject.m_pSibling);
}
CGameObject::~CGameObject()
{
	ReleaseShaderVariables();
}

void CGameObject::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}
void CGameObject::Release()
{
	if (m_pSibling) m_pSibling->Release();
	if (m_pChild) m_pChild->Release();

	if (--m_nReferences <= 0) delete this;
}
void CGameObject::ReleaseUploadBuffers()
{
	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::UpdateShaderVariable(XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
}

void CGameObject::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void CGameObject::OnPrepareAnimate()
{
}
void CGameObject::Update(float fTimeElapsed)
{
}
void CGameObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	OnPrepareRender();

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}
void CGameObject::BatchAnimate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_pSibling) m_pSibling->BatchAnimate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->BatchAnimate(fTimeElapsed, &m_xmf4x4World);
}

void CGameObject::OnPrepareRender()
{
}


XMFLOAT3 CGameObject::GetPosition()
{
	return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}
XMFLOAT3 CGameObject::GetLook()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33));
}
XMFLOAT3 CGameObject::GetUp()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23));
}
XMFLOAT3 CGameObject::GetRight()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13));
}

CGameObject* CGameObject::GetRootParentObject()
{
	if (!m_pParent) return this;

	return m_pParent->GetRootParentObject();
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}
void CGameObject::UpdateMobility(Mobility mobility)
{
	if (m_pSibling) m_pSibling->UpdateMobility(mobility);
	if (m_pChild) m_pChild->UpdateMobility(mobility);
}
void CGameObject::SetPrevScale(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf3PrevScale.x = m_xmf4x4World._11;
	m_xmf3PrevScale.y = m_xmf4x4World._22;
	m_xmf3PrevScale.z = m_xmf4x4World._33;

	if (m_pSibling) m_pSibling->SetPrevScale(pxmf4x4Parent);
	if (m_pChild) m_pChild->SetPrevScale(&m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}
void CGameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}
void CGameObject::SetScale(float width, float height, float depth)
{
	XMMATRIX mtxScale = XMMatrixScaling(width, height, depth);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

	UpdateTransform(NULL);
}
void CGameObject::SetScale(const XMFLOAT3& scale)
{
	m_xmf3Scale = scale;
	if (m_pSibling) m_pSibling->SetScale(scale);
	if (m_pChild) m_pChild->SetScale(scale);
}
void CGameObject::SetMass(float mass)
{
	m_Mass = mass;
}
void CGameObject::SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3&& xmf3Up)
{
	XMFLOAT3 xmf3Position(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(xmf3Position, xmf3Target, xmf3Up);
	m_xmf4x4World._11 = mtxLookAt._11; m_xmf4x4World._12 = mtxLookAt._21; m_xmf4x4World._13 = mtxLookAt._31;
	m_xmf4x4World._21 = mtxLookAt._12; m_xmf4x4World._22 = mtxLookAt._22; m_xmf4x4World._23 = mtxLookAt._32;
	m_xmf4x4World._31 = mtxLookAt._13; m_xmf4x4World._32 = mtxLookAt._23; m_xmf4x4World._33 = mtxLookAt._33;
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;

	UpdateTransform(NULL);
}
void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
void CGameObject::SetTransform(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& look, const XMFLOAT3& pos)
{
	m_xmf4x4Transform._11 = right.x; m_xmf4x4Transform._12 = right.y; m_xmf4x4Transform._13 = right.z;
	m_xmf4x4Transform._21 = up.x; m_xmf4x4Transform._22 = up.y; m_xmf4x4Transform._23 = up.z;
	m_xmf4x4Transform._31 = look.x; m_xmf4x4Transform._32 = look.y; m_xmf4x4Transform._33 = look.z;
	m_xmf4x4Transform._41 = pos.x; m_xmf4x4Transform._42 = pos.y; m_xmf4x4Transform._43 = pos.z;

	UpdateTransform(NULL);
}

void CGameObject::GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection)
{
	XMFLOAT4X4 xmf4x4WorldView = Matrix4x4::Multiply(m_xmf4x4World, xmf4x4View);
	XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(xmf4x4WorldView);
	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);

	*pxmf3PickRayOrigin = Vector3::TransformCoord(xmf3CameraOrigin, xmf4x4Inverse);
	*pxmf3PickRayDirection = Vector3::TransformCoord(xmf3PickPosition, xmf4x4Inverse);
	*pxmf3PickRayDirection = Vector3::Normalize(Vector3::Subtract(*pxmf3PickRayDirection, *pxmf3PickRayOrigin));
}
int CGameObject::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance)
{
	int nIntersected = 0;
	return nIntersected;
}

///////////////////////////////////////////////////////
void Obstacle::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void Obstacle::SetScale(float width, float height, float depth)
{
	XMMATRIX mtxScale = XMMatrixScaling(width, height, depth);
	m_xmf4x4World = Matrix4x4::Multiply(mtxScale, m_xmf4x4World);
}

void Obstacle::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

bool Obstacle::IsIntersectingL(XMFLOAT3 start, XMFLOAT3 end) {
	if (Vector3::Length(Vector3::Subtract(end, start)) < EPSILON) return true;

	XMMATRIX worldTransform = XMLoadFloat4x4(&m_xmf4x4World);
	
	XMMATRIX invWorld = XMMatrixInverse(NULL, worldTransform);

	// 선분을 OBB의 로컬 좌표로 변환
	XMFLOAT3 localStart = Vector3::TransformCoord(start, invWorld);
	XMFLOAT3 localEnd = Vector3::TransformCoord(end, invWorld);
	localStart.y = 0.0f;
	localEnd.y = 0.0f;

	BoundingOrientedBox boundingBox = BoundingOrientedBox(/*m_xmf3Center*/XMFLOAT3(0.0f,0.0f,0.0f), m_xmf3Extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	float dist = 0.0f;
	XMFLOAT3 vLocalLine = Vector3::Normalize(Vector3::Subtract(localEnd, localStart));
	if (boundingBox.Intersects(XMLoadFloat3(&localStart), XMLoadFloat3(&vLocalLine), dist))
	{
		float fLength = Vector3::Length(Vector3::Subtract(localEnd, localStart));
		if (dist < fLength)
		{
			return true;
		}
	}

	return false;
}

bool Obstacle::IsIntersectingV(const XMFLOAT3 start, const XMFLOAT3 direction) {

	XMMATRIX worldTransform = XMLoadFloat4x4(&m_xmf4x4World);
	
	// 월드 변환 행렬 추출
	XMMATRIX inverseWorldTransform = XMMatrixInverse(NULL, worldTransform);

	// 시작점을 OBB의 로컬 좌표로 변환
	XMVECTOR localStart = XMVector3TransformCoord(XMVectorSet(start.x, 0.0f, start.z, 1.0f), inverseWorldTransform);

	// 방향 벡터를 OBB의 로컬 좌표로 변환
	XMVECTOR localDirection = XMVector3TransformNormal(XMVectorSet(direction.x, 0.0f, direction.z, 0.0f), inverseWorldTransform);

	// OBB 축에 대한 투영 길이 계산
	float tmin = (-m_xmf3Extents.x - XMVectorGetX(localStart)) / XMVectorGetX(localDirection);
	float tmax = (m_xmf3Extents.x - XMVectorGetX(localStart)) / XMVectorGetX(localDirection);

	if (tmin > tmax) {
		std::swap(tmin, tmax);
	}

	float tzmin = (-m_xmf3Extents.z - XMVectorGetZ(localStart)) / XMVectorGetZ(localDirection);
	float tzmax = (m_xmf3Extents.z - XMVectorGetZ(localStart)) / XMVectorGetZ(localDirection);

	if (tzmin > tzmax) {
		std::swap(tzmin, tzmax);
	}

	if ((tmin > tzmax) || (tzmin > tmax)) {
		return false;
	}

	return true;
}

void Obstacle::SetCorner(XMFLOAT3& Extents, XMFLOAT3& Center, float scale)
{
	m_xmf3Extents = Extents;
	m_xmf3Center = Center;

	float interval = 1.0f / 2;	// 10PIXEL_PER 1M, 2.0 => 20CM
	float Ratio = interval / scale;
	float fx = Extents.x  + Ratio, fy = Extents.y, fz = Extents.z + Ratio;

	diagonalLength = Vector3::Length(Extents);
	
	corner.LB = Vector3::TransformCoord(XMFLOAT3(-fx, 0.0, -fz), m_xmf4x4World);
	corner.LT = Vector3::TransformCoord(XMFLOAT3(-fx, 0.0, +fz), m_xmf4x4World);
	corner.RB = Vector3::TransformCoord(XMFLOAT3(+fx, 0.0, -fz), m_xmf4x4World);
	corner.RT = Vector3::TransformCoord(XMFLOAT3(+fx, 0.0, +fz), m_xmf4x4World);
}
