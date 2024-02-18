#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "ShaderObjects.h"
#include "GameSource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nAnimatedBones];
}
CAnimationSet::~CAnimationSet()
{
	if (m_pfKeyFrameTimes) delete[] m_pfKeyFrameTimes;
	for (int j = 0; j < m_nKeyFrames; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;

	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

void CAnimationSet::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; i++)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData) m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

void CAnimationSet::SetPosition(float fElapsedPosition)
{
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		m_fPosition += fElapsedPosition;
		if (m_fPosition >= m_fLength) m_fPosition = 0.0f;
		//			m_fPosition = fmod(fTrackPosition, m_pfKeyFrameTimes[m_nKeyFrames-1]); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTimes[m_nKeyFrames-1]) * m_pfKeyFrameTimes[m_nKeyFrames-1];
		//			m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
		//			m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
		break;
	}
	case ANIMATION_TYPE_ONCE:
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameTranslationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i + 1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameScaleTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i + 1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameRotationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i + 1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone]), XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i + 1][nBone]), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, NULL, R, T));
#else   
	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_pfKeyFrameTimes[i] <= m_fPosition) && (m_fPosition < m_pfKeyFrameTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i + 1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}
	if (m_fPosition >= m_pfKeyFrameTimes[m_nKeyFrames - 1]) xmf4x4Transform = m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];

#endif
	return(xmf4x4Transform);
}

void CAnimationSet::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSet::SetCallbackKey(int nKeyIndex, float fKeyTime, void* pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationSet::SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSets = new CAnimationSet * [nAnimationSets];
}
CAnimationSets::~CAnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) if (m_pAnimationSets[i]) delete m_pAnimationSets[i];
	if (m_pAnimationSets) delete[] m_pAnimationSets;

	if (m_ppAnimatedBoneFrameCaches) delete[] m_ppAnimatedBoneFrameCaches;
}

void CAnimationSets::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	m_pAnimationSets[nAnimationSet]->m_nCallbackKeys = nCallbackKeys;
	m_pAnimationSets[nAnimationSet]->m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSets::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void* pData)
{
	m_pAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationSets::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler* pCallbackHandler)
{
	m_pAnimationSets[nAnimationSet]->SetAnimationCallbackHandler(pCallbackHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAnimationController::CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel)
{
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource * [m_nSkinnedMeshes];
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void**)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
	}
}
CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Release();
	}
	if (m_ppd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	if (m_ppcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_pAnimationSets) m_pAnimationSets->Release();

	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CAnimationController::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	if (m_pAnimationSets) m_pAnimationSets->SetCallbackKeys(nAnimationSet, nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void* pData)
{
	if (m_pAnimationSets) m_pAnimationSets->SetCallbackKey(nAnimationSet, nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler* pCallbackHandler)
{
	if (m_pAnimationSets) m_pAnimationSets->SetAnimationCallbackHandler(nAnimationSet, pCallbackHandler);
}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks)
	{
		m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
		//		m_pAnimationTracks[nAnimationTrack].m_fPosition = 0.0f;
		//		if (m_pAnimationSets) m_pAnimationSets->m_pAnimationSets[nAnimationSet]->m_fPosition = 0.0f;
	}
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetOneOfTrackEnable(int nAnimationTrack)
{
	if (m_pAnimationTracks)
	{
		if (m_pAnimationTracks[m_nCurrentTrack].m_bEnable)
		{
			m_pAnimationTracks[m_nCurrentTrack].SetEnable(false);
			m_pAnimationTracks[m_nCurrentTrack].SetHandOverPosition(false);
		}

		if(m_nCurrentTrack != 0) // Exception Idle Animation.
			SetTrackPosition(nAnimationTrack, m_pAnimationTracks[m_nCurrentTrack].m_fPosition);
		else
			SetTrackPosition(nAnimationTrack, 0);

		m_pAnimationTracks[nAnimationTrack].SetEnable(true);
		m_pAnimationTracks[nAnimationTrack].SetHandOverPosition(true);
		m_nCurrentTrack = nAnimationTrack;
	}
}

void CAnimationController::SetHandOverPosition(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetHandOverPosition(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
	if (m_pAnimationSets) m_pAnimationSets->m_pAnimationSets[nAnimationTrack]->m_fPosition = fPosition * m_pAnimationSets->m_pAnimationSets[nAnimationTrack]->m_fLength;
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
	}
}

void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject* pRootGameObject)
{
	m_fTime += fTimeElapsed;
	if (m_pAnimationTracks)
	{
		//for (int k = 0; k < m_nAnimationTracks; k++) m_pAnimationTracks[k].m_fPosition += (fTimeElapsed * m_pAnimationTracks[k].m_fSpeed);
		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->SetPosition(fTimeElapsed * m_pAnimationTracks[k].m_fSpeed);

			if (m_pAnimationTracks[k].m_bHandOverPosition)
			{
				m_pAnimationTracks[k].m_fPosition = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->m_fPosition / m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->m_fLength;
				//std::cout << "Tracks: " << k << ", " << m_pAnimationTracks[k].m_fPosition << std::endl;
			}
		}

		for (int j = 0; j < m_pAnimationSets->m_nAnimatedBoneFrames; j++)
		{
			XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Zero();
			for (int k = 0; k < m_nAnimationTracks; k++)
			{
				if (m_pAnimationTracks[k].m_bEnable)
				{
					CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j);
					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
				}
			}
			m_pAnimationSets->m_ppAnimatedBoneFrameCaches[j]->m_xmf4x4Transform = xmf4x4Transform;
		}

		pRootGameObject->UpdateTransform(NULL);

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable) m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->HandleCallback();
		}
	}
}

void CAnimationController::SetAnimationBundle(UINT n)
{
	switch (n) {
	case 0:	// empty.
		m_nAnimationBundle[IDLE] = 0;				// Idle
		m_nAnimationBundle[WALK] = 1;				// walk
		m_nAnimationBundle[BACKWARD_WALK] = 2;		// backward walk
		m_nAnimationBundle[RUN] = 3;				// slow run
		m_nAnimationBundle[LEFT_BACKWARD] = 14;
		m_nAnimationBundle[RIGHT_BACKWARD] = 15;
		m_nAnimationBundle[LEFT_FORWARD] = 16;
		m_nAnimationBundle[RIGHT_FORWARD] = 17;
		m_nAnimationBundle[LEFT_WALK] = 18;
		m_nAnimationBundle[RIGHT_WALK] = 19;
		break;
	case 1:	// pick pistol
		m_nAnimationBundle[IDLE] = 4;				// pistol Idle
		m_nAnimationBundle[WALK] = 5;				// pistol walk
		m_nAnimationBundle[BACKWARD_WALK] = 6;		// pistol backward walk
		m_nAnimationBundle[LEFT_BACKWARD] = 14;
		m_nAnimationBundle[RIGHT_BACKWARD] = 15;
		m_nAnimationBundle[LEFT_FORWARD] = 16;
		m_nAnimationBundle[RIGHT_FORWARD] = 17;
		m_nAnimationBundle[LEFT_WALK] = 18;
		m_nAnimationBundle[RIGHT_WALK] = 19;
		break;
	case 2: // pick rifle
		m_nAnimationBundle[IDLE] = 7;				// rifle Idle
		m_nAnimationBundle[WALK] = 9;				// rifle walk
		m_nAnimationBundle[BACKWARD_WALK] = 10;		// rifle backword walk
		m_nAnimationBundle[FIRE] = 8;				// rifle fire
		m_nAnimationBundle[LEFT_BACKWARD] = 20;
		m_nAnimationBundle[RIGHT_BACKWARD] = 21;
		m_nAnimationBundle[LEFT_FORWARD] = 22;
		m_nAnimationBundle[RIGHT_FORWARD] = 23;
		m_nAnimationBundle[LEFT_WALK] = 24;
		m_nAnimationBundle[RIGHT_WALK] = 25;
		break;
	case 3: // aim rifle
		m_nAnimationBundle[IDLE] = 7;				// rifle Idle
		m_nAnimationBundle[WALK] = 11;				// rifle aim walk
		m_nAnimationBundle[BACKWARD_WALK] = 12;		// rifle aim backward walk
		m_nAnimationBundle[FIRE] = 8;				// rifle fire
		m_nAnimationBundle[LEFT_BACKWARD] = 20;
		m_nAnimationBundle[RIGHT_BACKWARD] = 21;
		m_nAnimationBundle[LEFT_FORWARD] = 22;
		m_nAnimationBundle[RIGHT_FORWARD] = 23;
		m_nAnimationBundle[LEFT_WALK] = 24;
		m_nAnimationBundle[RIGHT_WALK] = 25;
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLoadedModelInfo::~CLoadedModelInfo()
{
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters, int nGraphicsSrvGpuHandles, int nComputeSrvGpuHandles, int nComputeUavGpuHandles)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;
	if(nGraphicsSrvGpuHandles || nComputeSrvGpuHandles || nComputeUavGpuHandles)
		m_nGraphicsSrvGpuHandles = nGraphicsSrvGpuHandles;
	else 
		m_nGraphicsSrvGpuHandles = nTextures;
	m_nComputeSrvGpuHandles = nComputeSrvGpuHandles;
	m_nComputeUavGpuHandles = nComputeUavGpuHandles;

	if (m_nTextures > 0)
	{
		if (m_nGraphicsSrvGpuHandles) m_pGraphicsSrvRootArgumentInfos = new GRAPHICS_SRVROOTARGUMENTINFO[m_nGraphicsSrvGpuHandles];
		if (m_nComputeSrvGpuHandles) m_pComputeSrvRootArgumentInfos = new COMPUTE_SRVROOTARGUMENTINFO[m_nComputeSrvGpuHandles];
		if (m_nComputeUavGpuHandles) m_pComputeUavRootArgumentInfos = new COMPUTE_UAVROOTARGUMENTINFO[m_nComputeUavGpuHandles];

		m_ppd3dTextureUploadBuffers = new ID3D12Resource * [m_nTextures];
		m_ppd3dTextures = new ID3D12Resource * [m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_ppd3dTextureUploadBuffers[i] = m_ppd3dTextures[i] = NULL;
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}
CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;

	if (m_pGraphicsSrvRootArgumentInfos) delete m_pGraphicsSrvRootArgumentInfos;
	if (m_pComputeSrvRootArgumentInfos) delete m_pComputeSrvRootArgumentInfos;
	if (m_pComputeUavRootArgumentInfos) delete m_pComputeUavRootArgumentInfos;
}

void CTexture::SetGraphicsSrvRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pGraphicsSrvRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pGraphicsSrvRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}
void CTexture::SetComputeSrvRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pComputeSrvRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pComputeSrvRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}
void CTexture::SetComputeUavRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGpuDescriptorHandle)
{
	m_pComputeUavRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pComputeUavRootArgumentInfos[nIndex].m_d3dUavGpuDescriptorHandle = d3dUavGpuDescriptorHandle;
}
void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateGraphicsShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pGraphicsSrvRootArgumentInfos[0].m_nRootParameterIndex, m_pGraphicsSrvRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nGraphicsSrvGpuHandles; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pGraphicsSrvRootArgumentInfos[i].m_nRootParameterIndex, m_pGraphicsSrvRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}
void CTexture::UpdateComputeShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetComputeRootDescriptorTable(m_pComputeSrvRootArgumentInfos[0].m_nRootParameterIndex, m_pComputeSrvRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nComputeSrvGpuHandles; i++)
		{
			pd3dCommandList->SetComputeRootDescriptorTable(m_pComputeSrvRootArgumentInfos[i].m_nRootParameterIndex, m_pComputeSrvRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}

	for (int i = 0; i < m_nComputeUavGpuHandles; i++)
	{
		pd3dCommandList->SetComputeRootDescriptorTable(m_pComputeUavRootArgumentInfos[i].m_nRootParameterIndex, m_pComputeUavRootArgumentInfos[i].m_d3dUavGpuDescriptorHandle);
	}
}
void CTexture::UpdateGraphicsSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pGraphicsSrvRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pGraphicsSrvRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}
void CTexture::UpdateComputeSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetComputeRootDescriptorTable(m_pComputeSrvRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pComputeSrvRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}
void CTexture::UpdateComputeUavShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetComputeRootDescriptorTable(m_pComputeUavRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pComputeUavRootArgumentInfos[nIndex].m_d3dUavGpuDescriptorHandle);
}
void CTexture::ReleaseShaderVariables()
{
}
void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex, bool bIsDDSFile, D3D12_RESOURCE_STATES d3dResourceStates)
{
	if (bIsDDSFile)
		m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &(m_ppd3dTextureUploadBuffers[nIndex]), d3dResourceStates);
	else
		m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromWICFile(pd3dDevice, pd3dCommandList, pszFileName, &(m_ppd3dTextureUploadBuffers[nIndex]), d3dResourceStates);
}
void CTexture::CreateBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, UINT nIndex)
{
	m_nTextureType = RESOURCE_BUFFER;
	m_dxgiBufferFormat = ndxgiFormat;
	m_nBufferElement = nElements;
	m_nBufferStride = nStride;
	m_ppd3dTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, d3dHeapType, d3dResourceStates, &m_ppd3dTextureUploadBuffers[nIndex]);
}
void CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nDepthOrArraySize, UINT nMipLevels, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, DXGI_FORMAT dxgiFormat, UINT nIndex)
{
	m_nTextureType = nResourceType;
	m_dxgiBufferFormat = dxgiFormat;
	D3D12_RESOURCE_DIMENSION d3dResourceDimension = (nResourceType == RESOURCE_TEXTURE2D) ? D3D12_RESOURCE_DIMENSION_TEXTURE2D : D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	m_ppd3dTextures[nIndex] = ::CreateTextureResource(pd3dDevice, pd3dCommandList, pData, nBytes, d3dResourceDimension, nWidth, nHeight, nDepthOrArraySize, nMipLevels, d3dResourceFlags, dxgiFormat, D3D12_HEAP_TYPE_DEFAULT, d3dResourceStates, &m_ppd3dTextureUploadBuffers[nIndex]);
}
ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex)
{
	m_nTextureType = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, 1, 0, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures;

	m_ppTextures = new CTexture * [m_nTextures];
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = NULL;
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
}

CMaterial::~CMaterial()
{
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}

	if (m_nTextures > 0)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppTextures[i]) m_ppTextures[i]->Release();
		delete[] m_ppTextures;

		if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
	}
}

void CMaterial::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}
void CMaterial::SetTexture(CTexture* pTexture, UINT nTexture)
{
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->Release();
	m_ppTextures[nTexture] = pTexture;
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->AddRef();
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 4, &m_xmf4AmbientColor, 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 4, &m_xmf4AlbedoColor, 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 4, &m_xmf4SpecularColor, 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 4, &m_xmf4EmissiveColor, 28);
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 1, &m_nType, 32);

	if (m_pShader) m_pShader->UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i])
		{
			for (int k = 0; k < m_ppTextures[i]->m_nGraphicsSrvGpuHandles; ++k)
				m_ppTextures[i]->UpdateGraphicsSrvShaderVariable(pd3dCommandList, k);
		}
	}
}
void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader) m_pShader->ReleaseShaderVariables();

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseShaderVariables();
	}
}
void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}

CShader* CMaterial::m_pSkinnedAnimationShader = NULL;
CShader* CMaterial::m_pStandardShader = NULL;
void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	if (!m_pStandardShader)
	{
		m_pStandardShader = new CStandardShader();
		m_pStandardShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_pStandardShader->AddRef();
	}

	if (!m_pSkinnedAnimationShader)
	{
		m_pSkinnedAnimationShader = new CSkinnedAnimationStandardShader();
		m_pSkinnedAnimationShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_pSkinnedAnimationShader->AddRef();
	}
}

void CMaterial::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	char pstrTextureName[128] = { '\0' };

	BYTE nStrLength = ::ReadStringFromFile(pInFile, pstrTextureName);

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null"))
	{
		SetMaterialType(nType);

		char pstrFilePath[128] = { '\0' };
		strcpy_s(pstrFilePath, 128, CGameObject::m_pstrTextureFilePath.c_str());
		int PathLength = CGameObject::m_pstrTextureFilePath.size();

		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + PathLength, 128 - PathLength, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		strcpy_s(pstrFilePath + PathLength + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 128 - PathLength - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 128, pstrFilePath, _TRUNCATE);

		//#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif
		if (!bDuplicated)
		{
			*ppTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
			(*ppTexture)->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrTextureName, 0, true);
			if (*ppTexture) (*ppTexture)->AddRef();

			CScene::CreateSRVUAVs(pd3dDevice, *ppTexture, nRootParameter, false);
		}
		else
		{
			if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
				CGameObject* pRootGameObject = pParent;
				*ppTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
				if (*ppTexture) (*ppTexture)->AddRef();
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

CGameObject::CGameObject(int nMaterials)
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf3Scale = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3BoundingScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_xmf3BoundingLocation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3PrevScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_nBoundingCylinderRadius = 0.0f;
	m_IsBoundingCylinder = false;
	m_IsExistBoundingBox = true;
	m_IsCrosshair = false;
	m_NotUseTransform = false;
	m_OnlyOneBoundingBox = false;
	m_Mass = 0;

	m_pMesh = NULL;

	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial * [m_nMaterials];
		for (int i = 0; i < m_nMaterials; ++i)
			m_ppMaterials[i] = NULL;
	}
}
CGameObject::CGameObject(const CGameObject& pGameObject)
{
	strcpy(m_pstrFrameName, pGameObject.m_pstrFrameName);

	m_xmf4x4World = pGameObject.m_xmf4x4World;
	m_xmf4x4Transform = pGameObject.m_xmf4x4Transform;
	m_xmf3Scale = pGameObject.m_xmf3Scale;
	m_xmf3BoundingScale = pGameObject.GetBoundingScale();
	m_xmf3BoundingLocation = pGameObject.GetBoundingLocation();
	m_xmf3PrevScale = pGameObject.m_xmf3PrevScale;
	m_Mass = pGameObject.m_Mass;

	m_nBoundingCylinderRadius = pGameObject.m_nBoundingCylinderRadius;
	m_IsBoundingCylinder = pGameObject.m_IsBoundingCylinder;
	m_IsExistBoundingBox = pGameObject.m_IsExistBoundingBox;
	m_IsCrosshair = pGameObject.m_IsExistBoundingBox;
	m_NotUseTransform = pGameObject.m_NotUseTransform;
	m_OnlyOneBoundingBox = pGameObject.m_OnlyOneBoundingBox;

	m_nMaterials = 0;
	m_ppMaterials = NULL;
	if (pGameObject.m_pMesh)
	{
		m_pMesh = new CMesh();
		BoundingOrientedBox BoundingBox = pGameObject.m_pMesh->GetBoundingBox();
		m_pMesh->SetBoundinBoxCenter(BoundingBox.Center);
		m_pMesh->SetBoundinBoxExtents(BoundingBox.Extents);
	}
	else
		m_pMesh = NULL;


	m_Mobility = pGameObject.m_Mobility;

	if (pGameObject.m_pChild) m_pChild = new CGameObject(*pGameObject.m_pChild);
	if (pGameObject.m_pSibling) m_pSibling = new CGameObject(*pGameObject.m_pSibling);
}
CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_pMesh) m_pMesh->Release();

	if (!m_ppBoundingMeshes.empty())
	{
		for (int i = 0; i < m_ppBoundingMeshes.size(); ++i)
		{
			if (m_ppBoundingMeshes[i]) {
				m_ppBoundingMeshes[i]->Release();
				//m_ppBoundingMeshes[i]->ReleaseUploadBuffers();
			}
			m_ppBoundingMeshes[i] = NULL;
		}
	}
	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;
	if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
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
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}
void CGameObject::ReleaseShaderVariables()
{
}
void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}
void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 16, &xmf4x4World, 0);
}
void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial)
{
}

void CGameObject::SetMaterial(int nMaterial, CMaterial* pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}
void CGameObject::SetShader(CShader* pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	m_ppMaterials[0] = new CMaterial(0);
	m_ppMaterials[0]->SetShader(pShader);
}
void CGameObject::SetShader(int nMaterial, CShader* pShader)
{
	if (!m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial] = new CMaterial();
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}
void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
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

//void CGameObject::Conflicted(LPVOID CollisionInfo)
//{
//}
bool CGameObject::Collide(FXMVECTOR Origin, FXMVECTOR Direction, float& Dist)
{
	bool bHit = false;
	if (m_pMesh)
	{
		BoundingOrientedBox OBB = m_pMesh->GetTransformedBoundingBox();
		if (OBB.Intersects(Origin, Direction, Dist))
		{
			if (Dist < METER_PER_PIXEL(1))
			{
				std::cout << "hit!!!!" << std::endl;
				return true;
			}
		}
	}

	if (m_pSibling) bHit = m_pSibling->Collide(Origin, Direction, Dist);
	if (m_pChild && !bHit) bHit = m_pChild->Collide(Origin, Direction, Dist);

	return bHit;
}
bool CGameObject::Collide(const CGameSource& GameSource, CBoundingBoxObjects& BoundingBoxObjects)
{
	assert(GetDisplacement());	// Has a displacement value.
	XMFLOAT3* Displacement = GetDisplacement();

	std::vector<CGameObject*>& BoundingObjects = BoundingBoxObjects.GetBoundingObjects();
	if (m_pMesh)
	{
		BoundingOrientedBox wBoundingBox = m_pMesh->GetTransformedBoundingBox();
		for (int i = 0; i < BoundingObjects.size(); ++i)
		{
			if (BoundingObjects[i]->m_Mobility == Static &&
				BoundingObjects[i]->BeginOverlapBoundingBox(wBoundingBox, Displacement))
			{
				//std::cout << "Prev Player Position: " << GetPosition().x << ", " << GetPosition().y << ", " << GetPosition().z << std::endl;
				//std::cout << "Collide Object[" << i << "]: " << "x = " << (int)BoundingObjects[i]->GetPosition().x << ", y = " << (int)BoundingObjects[i]->GetPosition().y << ", z = " << (int)BoundingObjects[i]->GetPosition().z << std::endl;
				// Displacement operation(Collision)
				//SetPosition(Vector3::Add(GetPosition(), *Displacement));
				return true;
			}
		}
	}
	return false;
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

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

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
void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh && (IsVisible(pCamera) || IsCrosshair()))
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i);
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}
void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i, nInstances, d3dInstancingBufferView);
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera, nInstances, d3dInstancingBufferView);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera, nInstances, d3dInstancingBufferView);
}
void CGameObject::RunTimeBuild(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

XMFLOAT3* CGameObject::GetDisplacement()
{
	return NULL;
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
	m_Mobility = mobility;

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

bool CGameObject::IsVisible(CCamera* pCamera)
{
	if (GetReafObjectType() == SkyBox) return true;

	bool bIsVisible = false;
	BoundingOrientedBox xmBoundingBox = m_pMesh->GetBoundingBox();

	xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
	if (pCamera) bIsVisible = pCamera->IsInFrustum(xmBoundingBox);

	return bIsVisible;
}
bool CGameObject::IsCollide(BoundingOrientedBox& box)
{
	BoundingOrientedBox xmBoundingBox = m_pMesh->GetBoundingBox();

	xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
	return xmBoundingBox.Intersects(box);
}

bool CGameObject::IsCrosshair()
{
	return m_IsCrosshair;
}

void CGameObject::SetOneBoundingBox(bool b, XMFLOAT3 Center, XMFLOAT3 Extent)
{
	m_OnlyOneBoundingBox = b;

	SetBoundingBox(Center, Extent);
}

CGameObject* CGameObject::FindObjectWithMesh()
{
	CGameObject* object = NULL;
	if (m_pMesh) return this;

	if (m_pSibling) object = m_pSibling->FindObjectWithMesh();
	if (object) return object;
	
	if (m_pChild) object = m_pChild->FindObjectWithMesh();
	if (object) return object;

	return NULL;
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
	if (m_pMesh)
	{
		XMFLOAT3 xmf3PickRayOrigin, xmf3PickRayDirection;
		GenerateRayForPicking(xmf3PickPosition, xmf4x4View, &xmf3PickRayOrigin, &xmf3PickRayDirection);

		nIntersected += m_pMesh->CheckRayIntersection(xmf3PickRayOrigin, xmf3PickRayDirection, pfHitDistance, m_xmf4x4World, Vector3::Length(m_xmf3PrevScale) * sqrt(3) / 3);
	}
	return nIntersected;
}

void CGameObject::CreateBoundingBoxMeshSet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	int nBoundingObjects = ((CBoundingBoxObjects*)BBShader)->GetBoundingObjects().size();
	CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);
	if (((CBoundingBoxObjects*)BBShader)->bCreate)
	{
		int nCreateObjects = ((CBoundingBoxObjects*)BBShader)->GetBoundingObjects().size() - nBoundingObjects;
		((CBoundingBoxObjects*)BBShader)->m_ParentObjects.push_back(this);
		((CBoundingBoxObjects*)BBShader)->m_nObjects.push_back(nCreateObjects);
		((CBoundingBoxObjects*)BBShader)->m_StartIndex.push_back(nBoundingObjects);
		((CBoundingBoxObjects*)BBShader)->bCreate = false;
	}
}
void CGameObject::CreateBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	if (!m_IsExistBoundingBox) return;

	if (m_pMesh &&
		EPSILON < m_pMesh->GetAABBExtents().x &&
		EPSILON < m_pMesh->GetAABBExtents().y &&
		EPSILON < m_pMesh->GetAABBExtents().z) {
		XMFLOAT3 extents = m_pMesh->GetAABBExtents();
		XMFLOAT3 center = m_pMesh->GetAABBCenter();
		CBoundingBoxMesh* BBMesh;
		if(GetTopParent()->m_NotUseTransform)
			BBMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList, extents, center, m_xmf3BoundingScale, m_pParent->m_xmf4x4World);
		else
			BBMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList, extents, center, m_xmf3BoundingScale, m_xmf4x4World);
		m_ppBoundingMeshes.push_back(BBMesh);
		m_pMesh->SetBoundinBoxExtents(XMFLOAT3(extents.x * m_xmf3BoundingScale.x, extents.y * m_xmf3BoundingScale.y, extents.z * m_xmf3BoundingScale.z));
		((CBoundingBoxObjects*)BBShader)->AppendBoundingObject(this);
		((CBoundingBoxObjects*)BBShader)->bCreate = true;

		if (!GetTopParent()->m_pTopBoundingMesh) GetTopParent()->m_pTopBoundingMesh = BBMesh;
	}

	if (m_pSibling) m_pSibling->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList,  BBShader);
	if (m_pChild) m_pChild->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);
}
void CGameObject::CreateBoundingBoxInstSet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pGameObject, LPVOID BBShader)
{
	int nBoundingObjects = ((CBoundingBoxObjects*)BBShader)->GetBoundingObjects().size();
	CreateBoundingBoxInst(pd3dDevice, pd3dCommandList, pGameObject, BBShader);
	if (((CBoundingBoxObjects*)BBShader)->bCreate)
	{
		int nCreateObjects = ((CBoundingBoxObjects*)BBShader)->GetBoundingObjects().size() - nBoundingObjects;
		((CBoundingBoxObjects*)BBShader)->m_ParentObjects.push_back(this);
		((CBoundingBoxObjects*)BBShader)->m_nObjects.push_back(nCreateObjects);
		((CBoundingBoxObjects*)BBShader)->m_StartIndex.push_back(nBoundingObjects);
		((CBoundingBoxObjects*)BBShader)->bCreate = false;
	}
}
void CGameObject::CreateBoundingBoxInst(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pGameObject, LPVOID BBShader)
{
	if (!m_IsExistBoundingBox) return;

	if (pGameObject->m_pMesh)
	{
		CMesh* pMesh = pGameObject->m_pMesh;
		if (EPSILON < pMesh->GetAABBExtents().x &&
			EPSILON < pMesh->GetAABBExtents().y &&
			EPSILON < pMesh->GetAABBExtents().z) {
			XMFLOAT3 extents = pMesh->GetAABBExtents();
			XMFLOAT3 center = pMesh->GetAABBCenter();
			CBoundingBoxMesh* BBMesh;
			if (GetTopParent()->m_NotUseTransform)
				BBMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList, extents, center, m_xmf3BoundingScale, m_pParent->m_xmf4x4World);
			else
				BBMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList, extents, center, m_xmf3BoundingScale, m_xmf4x4World);
			m_ppBoundingMeshes.push_back(BBMesh);
			((CBoundingBoxObjects*)BBShader)->AppendBoundingObject(this);
			((CBoundingBoxObjects*)BBShader)->bCreate = true;

			if (!GetTopParent()->m_pTopBoundingMesh) GetTopParent()->m_pTopBoundingMesh = BBMesh;
		}
	}

	if (m_pSibling) m_pSibling->CreateBoundingBoxInst(pd3dDevice, pd3dCommandList, pGameObject->m_pSibling, BBShader);
	if (m_pChild) m_pChild->CreateBoundingBoxInst(pd3dDevice, pd3dCommandList, pGameObject->m_pChild, BBShader);
}
void CGameObject::CreateBoundingBoxObjectSet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	int nBoundingObjects = ((CBoundingBoxObjects*)BBShader)->GetBoundingObjects().size();
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, BBShader);
	if (((CBoundingBoxObjects*)BBShader)->bCreate)
	{
		int nCreateObjects = ((CBoundingBoxObjects*)BBShader)->GetBoundingObjects().size() - nBoundingObjects;
		((CBoundingBoxObjects*)BBShader)->m_ParentObjects.push_back(this);
		((CBoundingBoxObjects*)BBShader)->m_nObjects.push_back(nCreateObjects);
		((CBoundingBoxObjects*)BBShader)->m_StartIndex.push_back(nBoundingObjects);
		((CBoundingBoxObjects*)BBShader)->bCreate = false;
	}
}
void CGameObject::CreateBoundingBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPVOID BBShader)
{
	if (!m_IsExistBoundingBox) return;

	if (m_OnlyOneBoundingBox) {
		FindObjectWithMesh()->CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, BBShader);
		return;
	}

	XMFLOAT3 extents = m_pMesh->GetAABBExtents();
	XMFLOAT3 center = m_pMesh->GetAABBCenter();
	m_ppBoundingMeshes.push_back(new CBoundingBoxMesh(pd3dDevice, pd3dCommandList, extents, center, m_xmf3BoundingScale, m_xmf4x4World));
	m_pMesh->SetBoundinBoxExtents(XMFLOAT3(extents.x * m_xmf3BoundingScale.x, extents.y * m_xmf3BoundingScale.y, extents.z * m_xmf3BoundingScale.z));
	((CBoundingBoxObjects*)BBShader)->AppendBoundingObject(this);
	((CBoundingBoxObjects*)BBShader)->bCreate = true;

	if (!GetTopParent()->m_pTopBoundingMesh) GetTopParent()->m_pTopBoundingMesh = m_ppBoundingMeshes.back();

	SetWorldTransformBoundingBox();
}
void CGameObject::SetWorldTransformBoundingBox()
{
	if (m_pMesh)
	{
		XMFLOAT3 WorldScale = Vector3::OriginScale(m_xmf4x4World);
		BoundingOrientedBox BoundingBox = m_pMesh->GetBoundingBox();
		XMFLOAT3 center = Vector3::Add(Vector3::TransformCoord(BoundingBox.Center, m_xmf4x4World), m_xmf3BoundingLocation);
		XMFLOAT3 extents = XMFLOAT3(BoundingBox.Extents.x * WorldScale.x * m_xmf3BoundingScale.x, BoundingBox.Extents.y * WorldScale.y * m_xmf3BoundingScale.y, BoundingBox.Extents.z * WorldScale.z * m_xmf3BoundingScale.z);
		XMFLOAT4 orientation = Vector4::Orientation(BoundingBox.Orientation, m_xmf4x4World);
		m_pMesh->SetTransformedBoundingBox(center, extents, orientation);
	}

	if (m_pSibling) m_pSibling->SetWorldTransformBoundingBox();
	if (m_pChild) m_pChild->SetWorldTransformBoundingBox();
}
void CGameObject::UpdateWorldTransformBoundingBox()
{
	if (m_pMesh)
	{
		BoundingOrientedBox BoundingBox = m_pMesh->GetBoundingBox();
		XMFLOAT4X4 world = (GetTopParent()->m_NotUseTransform) ? m_pParent->m_xmf4x4World : m_xmf4x4World;
		XMFLOAT3 center = Vector3::Add(Vector3::TransformCoord(BoundingBox.Center, world), m_xmf3BoundingLocation);
		XMFLOAT4 orientation = Vector4::Orientation(BoundingBox.Orientation, world);
		m_pMesh->SetTransformedBoundingBoxCenter(center);
		m_pMesh->SetTransformedBoundingBoxOrientation(orientation);
	}

	if (m_pSibling) m_pSibling->UpdateWorldTransformBoundingBox();
	if (m_pChild) m_pChild->UpdateWorldTransformBoundingBox();
}
void CGameObject::SetBoundingBox(XMFLOAT3& Center, XMFLOAT3& Extent)
{
	if (m_pMesh)
	{
		m_pMesh->SetBoundinBoxCenter(Center);
		m_pMesh->SetBoundinBoxExtents(Extent);
	}

	if (m_pSibling) m_pSibling->SetBoundingBox(Center, Extent);
	if (m_pChild) m_pChild->SetBoundingBox(Center, Extent);
}
void CGameObject::SetBoundingScale(XMFLOAT3& BoundingScale)
{
	m_xmf3BoundingScale = BoundingScale;

	if (m_pSibling) m_pSibling->SetBoundingScale(BoundingScale);
	if (m_pChild) m_pChild->SetBoundingScale(BoundingScale);
}
void CGameObject::SetBoundingScale(XMFLOAT3&& BoundingScale)
{
	SetBoundingScale(BoundingScale);
}
void CGameObject::SetBoundingLocation(XMFLOAT3& BoundingLocation)
{
	m_xmf3BoundingLocation = BoundingLocation;

	if (m_pSibling) m_pSibling->SetBoundingLocation(BoundingLocation);
	if (m_pChild) m_pChild->SetBoundingLocation(BoundingLocation);
}
void CGameObject::SetBoundingLocation(XMFLOAT3&& BoundingLocation)
{
	SetBoundingLocation(BoundingLocation);
}
void CGameObject::SetIsBoundingCylinder(bool bIsCylinder, float fRadius)
{
	m_IsBoundingCylinder = bIsCylinder;
	m_nBoundingCylinderRadius = fRadius;

	if (m_pSibling) m_pSibling->SetIsBoundingCylinder(bIsCylinder, fRadius);
	if (m_pChild) m_pChild->SetIsBoundingCylinder(bIsCylinder, fRadius);
}
bool CGameObject::BeginOverlapBoundingBox(const BoundingOrientedBox& OtherOBB, XMFLOAT3* displacement)
{
	const UINT nDirection = 4;
	bool retval = false;
	if (m_pMesh)
	{
		BoundingOrientedBox OBB = m_pMesh->GetTransformedBoundingBox();
		if (OBB.Intersects(OtherOBB)) {
			XMFLOAT3 ObjectDirection = XMFLOAT3((*displacement).x, 0.0f, (*displacement).z);
			//std::cout << "Prev Displacement: " << (*displacement).x << ", " << (*displacement).y << ", " << (*displacement).z << std::endl;
			//std::cout << "Dpm Length: " << Vector3::Length(*displacement) << std::endl;
			UINT nContains = 0;
			XMFLOAT3 Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 CornersXZ[nDirection] = {
				XMFLOAT3(-OtherOBB.Extents.x, 0.0f, -OtherOBB.Extents.z),	// LB
				XMFLOAT3(-OtherOBB.Extents.x, 0.0f, OtherOBB.Extents.z),		// LT
				XMFLOAT3(OtherOBB.Extents.x, 0.0f, -OtherOBB.Extents.z),		// RB
				XMFLOAT3(OtherOBB.Extents.x, 0.0f, OtherOBB.Extents.z),		// RT
			};
			XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&OtherOBB.Orientation));
			
			for (int i = 0; i < nDirection; ++i)
			{
				// No consider y-value.
				CornersXZ[i] = Vector3::TransformCoord(CornersXZ[i], mtxRotate);
				CornersXZ[i] = Vector3::Add(CornersXZ[i], OtherOBB.Center);
				CornersXZ[i].y = OBB.Center.y;
				if (OBB.Contains(XMLoadFloat3(&CornersXZ[i])))
				{
					Center = Vector3::Add(Center, CornersXZ[i]);
					nContains++;
				}
			}
			//std::cout << " nContains 개수: " << nContains << std::endl;
			if (nContains)
			{
				if (m_IsBoundingCylinder)
				{
					XMFLOAT3 vOBBtoOtherOBBC = Vector3::Subtract(OBB.Center, OtherOBB.Center);
					vOBBtoOtherOBBC.y = 0.0f;
					float OBBRad = (m_nBoundingCylinderRadius > 0.0f) ? m_nBoundingCylinderRadius : (OBB.Extents.x < OBB.Extents.z) ? OBB.Extents.x : OBB.Extents.z;
					float OtherOBBRad = (OtherOBB.Extents.x > OtherOBB.Extents.z) ? OtherOBB.Extents.x : OtherOBB.Extents.z;
					float LengthOBBtoOtherOBBC = Vector3::Length(vOBBtoOtherOBBC);

					if (OBBRad + OtherOBBRad > LengthOBBtoOtherOBBC)
					{
						if (Vector3::Length(ObjectDirection) < EPSILON)
						{
							*displacement = Vector3::Subtract(Vector3::ScalarProduct(*displacement, -1.0f, false), Vector3::Normalize(vOBBtoOtherOBBC));
							(*displacement).y = 0.0f;
							return true;
						}

						XMFLOAT3 SlideDisplacement;
						float degree = XMConvertToDegrees(asin((vOBBtoOtherOBBC.x * ObjectDirection.z - vOBBtoOtherOBBC.z * ObjectDirection.x) / (LengthOBBtoOtherOBBC * (Vector3::Length(ObjectDirection)))));

						if (degree < 0.0f)
						{
							// 방향 벡터를 시계방향으로 90도 회전.
							SlideDisplacement = Vector3::ScalarProduct(Vector3::Normalize(XMFLOAT3(vOBBtoOtherOBBC.z, vOBBtoOtherOBBC.y, -vOBBtoOtherOBBC.x)), Vector3::Length(ObjectDirection)*abs(sin(XMConvertToRadians(degree))), false);
						}
						else
						{
							// 방향 벡터를 반시계방향으로 90도 회전.
							SlideDisplacement = Vector3::ScalarProduct(Vector3::Normalize(XMFLOAT3(-vOBBtoOtherOBBC.z, vOBBtoOtherOBBC.y, vOBBtoOtherOBBC.x)), Vector3::Length(ObjectDirection) * abs(sin(XMConvertToRadians(degree))), false);
						}

						XMFLOAT3 dpm = Vector3::Subtract(Vector3::ScalarProduct(*displacement, -1.0f, false), Vector3::Normalize(vOBBtoOtherOBBC));
						vOBBtoOtherOBBC = Vector3::Subtract(OBB.Center, Vector3::Add(OtherOBB.Center, dpm));
						LengthOBBtoOtherOBBC = Vector3::Length(vOBBtoOtherOBBC);
						if (OBBRad + OtherOBBRad > LengthOBBtoOtherOBBC)
						{
							*displacement = Vector3::Subtract(Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), SlideDisplacement), Vector3::Normalize(vOBBtoOtherOBBC));
							(*displacement).y = 0.0f;
							return true;
						}
						*displacement = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), SlideDisplacement);
						(*displacement).y = 0.0f;
						return true;
					}

					return false;
				}
				else
				{
					Center = Vector3::ScalarProduct(Center, 1.0f / nContains, false);
					Center = Vector3::Normalize(Vector3::Subtract(Center, OBB.Center));

					//if (ObjectDirection.x < EPSILON && ObjectDirection.y < EPSILON && ObjectDirection.z < EPSILON)
					//{
					//	ObjectDirection = XMFLOAT3(0.0f, 0.0f, OtherOBB.Extents.z);
					//	ObjectDirection = Vector3::Normalize(Vector3::TransformCoord(ObjectDirection, mtxRotate));
					//}
					ObjectDirection = Vector3::Normalize(ObjectDirection);

					float RatioXZ = 1.0f / (OBB.Extents.x + OBB.Extents.z);
					XMFLOAT3 OBBDirectionXZ[nDirection] = {				// BoundingBox Normal Vector.
					XMFLOAT3(-(OBB.Extents.x * RatioXZ), 0.0f, 0.0f),		// L
					XMFLOAT3((OBB.Extents.x * RatioXZ), 0.0f, 0.0f),		// R
					XMFLOAT3(0.0f, 0.0f, -(OBB.Extents.z * RatioXZ)),		// B
					XMFLOAT3(0.0f, 0.0f, (OBB.Extents.z * RatioXZ)),		// T
					};
					XMMATRIX mtxOBBRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&OBB.Orientation));

					std::vector<XMFLOAT3> ValidDirection;
					for (int i = 0; i < nDirection; ++i)
					{
						OBBDirectionXZ[i] = Vector3::TransformCoord(OBBDirectionXZ[i], mtxOBBRotate);
						if (Vector3::DotProduct(OBBDirectionXZ[i], ObjectDirection) < 0.0f)
						{
							ValidDirection.push_back(OBBDirectionXZ[i]);
						}
					}

					int ValidDirectionIDX = 0;
					XMFLOAT3 BoundingBoxNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
					if (ValidDirection.size() == 1)
					{
						BoundingBoxNormal = ValidDirection.back();
						ValidDirectionIDX = 0;
					}
					else if (ValidDirection.size() == 2)
					{
						XMFLOAT3 DiagonalDirection = XMFLOAT3(0.0f, 0.0f, 0.0f);
						for (int i = 0; i < ValidDirection.size(); ++i)
						{
							DiagonalDirection = Vector3::Add(DiagonalDirection, ValidDirection[i]);
						}
						DiagonalDirection = Vector3::Normalize(DiagonalDirection);

						float degreeToDiagonal = XMConvertToDegrees(acos(Vector3::DotProduct(ValidDirection.back(), DiagonalDirection) * (1 / (Vector3::Length(ValidDirection.back()) * Vector3::Length(DiagonalDirection)))));
						float degreeToCenter = XMConvertToDegrees(acos(Vector3::DotProduct(ValidDirection.back(), Center) * (1 / (Vector3::Length(ValidDirection.back()) * Vector3::Length(DiagonalDirection)))));
						//std::cout << degreeToDiagonal << ", " << degreeToCenter << std::endl;
						if (degreeToCenter < degreeToDiagonal)
						{
							BoundingBoxNormal = ValidDirection.back();
							ValidDirectionIDX = 1;
						}
						else
						{
							BoundingBoxNormal = ValidDirection.front();
							ValidDirectionIDX = 0;
						}
					}
					else
					{
						return true;
						assert(0);
					}

					XMFLOAT3 PrevObjectDirection = Vector3::Normalize(ObjectDirection);
					XMFLOAT3 SubObjectDirection = ObjectDirection;

					BoundingBoxNormal = Vector3::Normalize(BoundingBoxNormal);
					float scale = Vector3::DotProduct(ObjectDirection, BoundingBoxNormal);
					BoundingBoxNormal = Vector3::ScalarProduct(BoundingBoxNormal, scale, false);
					ObjectDirection = Vector3::Subtract(ObjectDirection, BoundingBoxNormal);

					XMFLOAT3 dpm = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), ObjectDirection);
					for (int i = 0; i < nDirection; ++i)
					{
						// No consider y-value.

						CornersXZ[i] = Vector3::Add(CornersXZ[i], dpm);
						if (OBB.Contains(XMLoadFloat3(&CornersXZ[i])))
						{
							//float degree = XMConvertToDegrees(acos(Vector3::DotProduct(PrevObjectDirection, NObjectDirection) * 1 / (Vector3::Length(PrevObjectDirection) * Vector3::Length(NObjectDirection)) ));
							float degree = XMConvertToDegrees(asin((PrevObjectDirection.x * BoundingBoxNormal.z - PrevObjectDirection.z * BoundingBoxNormal.x) / (Vector3::Length(PrevObjectDirection) * (Vector3::Length(BoundingBoxNormal)))));
							//std::cout << "Degree: " << degree << std::endl;
							//std::cout << "dpm: x - " << dpm.x << ", y - " << dpm.y << ", z - " << dpm.z << std::endl;
							//std::cout << "PrevObjectDirection: x - " << PrevObjectDirection.x << ", z - " << PrevObjectDirection.z << std::endl;
							//std::cout << "BoundingBoxNormal: x - " << BoundingBoxNormal.x << ", z - " << BoundingBoxNormal.z << std::endl;
							if (degree > 0.0f)
							{
								//std::cout << "오른쪽 슬라이딩 -------------------->>>>>>>>>>>>>" << std::endl;
								PrevObjectDirection = XMFLOAT3(PrevObjectDirection.z, PrevObjectDirection.y, -PrevObjectDirection.x);
								//std::cout << "SlidingDirection: x - " << PrevObjectDirection.x << ", z - " << PrevObjectDirection.z << std::endl;
								//ObjectDirection = Vector3::ScalarProduct(PrevObjectDirection, ObjectDirection.x, false);
							}
							else
							{
								//std::cout << "<<<<<<<<<<<<<--------------------왼쪽 슬라이딩" << std::endl;
								PrevObjectDirection = XMFLOAT3(-PrevObjectDirection.z, PrevObjectDirection.y, PrevObjectDirection.x);
								//std::cout << "SlidingDirection: x - " << PrevObjectDirection.x << ", z - " << PrevObjectDirection.z << std::endl;
							}
							//ObjectDirection = Vector3::ScalarProduct(PrevObjectDirection, ObjectDirection.z, false);
							ObjectDirection = Vector3::ScalarProduct(PrevObjectDirection, 2 * Vector3::Length(*displacement), false);
							//ObjectDirection = PrevObjectDirection;
							//std::cout << "모서리에 있어서 슬라이딩 적용 시 오류가 있는 부분." << std::endl;

							CornersXZ[i] = Vector3::Subtract(CornersXZ[i], dpm);
							dpm = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), ObjectDirection);
							CornersXZ[i] = Vector3::Add(CornersXZ[i], dpm);

							if (OBB.Contains(XMLoadFloat3(&CornersXZ[i])))
							{
								if (ValidDirectionIDX)
									BoundingBoxNormal = ValidDirection.front();
								else
									BoundingBoxNormal = ValidDirection.back();

								BoundingBoxNormal = Vector3::Normalize(BoundingBoxNormal);
								float scale = Vector3::DotProduct(SubObjectDirection, BoundingBoxNormal);
								BoundingBoxNormal = Vector3::ScalarProduct(BoundingBoxNormal, scale, false);
								SubObjectDirection = Vector3::Subtract(SubObjectDirection, BoundingBoxNormal);

								//dpm = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), SubObjectDirection);
								ObjectDirection = SubObjectDirection;

								//std::cout << "모서리 지점을 지날 때 오류가 생겨 엉키는 부분~~~~~~~~~~~~~~~~" << std::endl;
							}

							break;
						}
					}

					*displacement = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), ObjectDirection);
					(*displacement).y = 0.0f;
					//std::cout << "ObjectDirection: x - " << ObjectDirection.x << ", y - " << ObjectDirection.y << ", z - " << ObjectDirection.z << std::endl;
					//std::cout << "displacement: x - " << displacement->x << ", y - " << displacement->y << ", z - " << displacement->z << std::endl;
				}
			}
			else
			{
				float RatioXZ = 1.0f / (OBB.Extents.x + OBB.Extents.z);
				XMFLOAT3 OBBDirectionXZ[nDirection] = {				// BoundingBox Normal Vector.
				XMFLOAT3(-(OBB.Extents.x * RatioXZ), 0.0f, 0.0f),		// L
				XMFLOAT3((OBB.Extents.x * RatioXZ), 0.0f, 0.0f),		// R
				XMFLOAT3(0.0f, 0.0f, -(OBB.Extents.z * RatioXZ)),		// B
				XMFLOAT3(0.0f, 0.0f, (OBB.Extents.z * RatioXZ)),		// T
				};
				mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&OBB.Orientation));

				std::vector<XMFLOAT3> ValidDirection;
				for (int i = 0; i < nDirection; ++i)
				{
					OBBDirectionXZ[i] = Vector3::TransformCoord(OBBDirectionXZ[i], mtxRotate);
					if (Vector3::DotProduct(OBBDirectionXZ[i], ObjectDirection) < 0.0f)
					{
						ValidDirection.push_back(OBBDirectionXZ[i]);
					}
				}

				int ValidDirectionIDX = 0;
				XMFLOAT3 BoundingBoxNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
				XMFLOAT3 DiagonalDirection = XMFLOAT3(0.0f, 0.0f, 0.0f);
				if (ValidDirection.empty())
				{
					return true;
					assert(0);
				}
				for (int i = 0; i < ValidDirection.size(); ++i)
				{
					DiagonalDirection = Vector3::Add(DiagonalDirection, ValidDirection[i]);
				}
				DiagonalDirection = Vector3::Normalize(DiagonalDirection);

				float degreeToDiagonal = XMConvertToDegrees(acos(Vector3::DotProduct(ValidDirection.back(), DiagonalDirection) * (1 / (Vector3::Length(ValidDirection.back()) * Vector3::Length(DiagonalDirection)))));
				float degreeToCenter = XMConvertToDegrees(acos(Vector3::DotProduct(ValidDirection.back(), Center) * (1 / (Vector3::Length(ValidDirection.back()) * Vector3::Length(DiagonalDirection)))));

				if (degreeToCenter < degreeToDiagonal)
				{
					BoundingBoxNormal = ValidDirection.back();
					ValidDirectionIDX = 1;
				}
				else
				{
					BoundingBoxNormal = ValidDirection.front();
					ValidDirectionIDX = 0;
				}

				XMFLOAT3 PrevObjectDirection = Vector3::Normalize(ObjectDirection);
				XMFLOAT3 SubObjectDirection = ObjectDirection;

				BoundingBoxNormal = Vector3::Normalize(BoundingBoxNormal);
				float scale = Vector3::DotProduct(ObjectDirection, BoundingBoxNormal);
				BoundingBoxNormal = Vector3::ScalarProduct(BoundingBoxNormal, scale, false);
				ObjectDirection = Vector3::Subtract(ObjectDirection, BoundingBoxNormal);

				XMFLOAT3 dpm = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), ObjectDirection);

				//std::cout << "오른쪽 슬라이딩 -------------------->>>>>>>>>>>>>" << std::endl;
				PrevObjectDirection = XMFLOAT3(PrevObjectDirection.z, PrevObjectDirection.y, -PrevObjectDirection.x);
				//std::cout << "SlidingDirection: x - " << PrevObjectDirection.x << ", z - " << PrevObjectDirection.z << std::endl;

				ObjectDirection = Vector3::ScalarProduct(PrevObjectDirection, 2 * Vector3::Length(*displacement), false);
				dpm = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), ObjectDirection);

				BoundingOrientedBox DpmOBB = OtherOBB;
				DpmOBB.Center.x += dpm.x;
				DpmOBB.Center.z += dpm.z;

				if (OBB.Intersects(DpmOBB))
				{
					//std::cout << "<<<<<<<<<<<<<--------------------왼쪽 슬라이딩" << std::endl;
					PrevObjectDirection = XMFLOAT3(-PrevObjectDirection.x, PrevObjectDirection.y, -PrevObjectDirection.z);
					//std::cout << "SlidingDirection: x - " << PrevObjectDirection.x << ", z - " << PrevObjectDirection.z << std::endl;
	
					ObjectDirection = Vector3::ScalarProduct(PrevObjectDirection, 2 * Vector3::Length(*displacement) , false);
				}

				//std::cout << "모서리에 있어서 슬라이딩 적용 시 오류가 있는 부분." << std::endl;

				*displacement = Vector3::Add(Vector3::ScalarProduct(*displacement, -1.0f, false), ObjectDirection);
				(*displacement).y = 0.0f;
				//std::cout << "ObjectDirection: x - " << ObjectDirection.x << ", y - " << ObjectDirection.y << ", z - " << ObjectDirection.z << std::endl;
				//std::cout << "displacement: x - " << displacement->x << ", y - " << displacement->y << ", z - " << displacement->z << std::endl;
				//std::cout << " 충돌은 했지만 모서리는 충돌하지 않음!			End" << std::endl;
			}

			return true;
		}
	}
	//if (m_pSibling) retval = m_pSibling->BeginOverlapBoundingBox(OtherOBB, displacement);
	//if (m_pChild && !retval) retval = m_pChild->BeginOverlapBoundingBox(OtherOBB, displacement);

	return retval;
}

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}
void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

CSkinnedMesh* CGameObject::FindSkinnedMesh(char* pstrSkinnedMeshName)
{
	CSkinnedMesh* pSkinnedMesh = NULL;
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		pSkinnedMesh = (CSkinnedMesh*)m_pMesh;
		if (!strncmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName, strlen(pstrSkinnedMeshName))) return(pSkinnedMesh);
	}

	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);

	return(NULL);
}
void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh*)m_pMesh;

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

CGameObject* CGameObject::FindFrame(const char* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;
	if (strlen(m_pstrFrameName) == strlen(pstrFrameName) && !strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}
CGameObject* CGameObject::GetTopParent()
{
	CGameObject* parent = this;
	if (m_pParent) parent = m_pParent->GetTopParent();

	return parent;
}

CTexture* CGameObject::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i])
		{
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{
				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName))) return(m_ppMaterials[i]->m_ppTextures[j]);
				}
			}
		}
	}
	CTexture* pTexture = NULL;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader)
{
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial *pMaterial = NULL;

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			pMaterial = new CMaterial(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();
					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}
			SetMaterial(nMaterial, pMaterial);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 5, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, 6, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 7, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, 8, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 9, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, 10, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, 11, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
}
void CGameObject::LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets->m_ppAnimatedBoneFrameCaches = new CGameObject * [pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames];

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppAnimatedBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppAnimatedBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);

					CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet];
					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pInFile);
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}
CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject* pGameObject = new CGameObject();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4Transform, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CGameObject::m_pstrTextureFilePath = "";
CLoadedModelInfo* CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	m_pstrTextureFilePath = pstrFileName;
	m_pstrTextureFilePath = m_pstrTextureFilePath.substr(0, m_pstrTextureFilePath.rfind("/")) + "/Textures/";

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"

				// Update the world matrix of bone.
				pLoadedModel->m_pModelRootObject->UpdateTransform();
				pLoadedModel->m_pModelRootObject->SetPrevScale();
				pLoadedModel->m_pModelRootObject->SetWorldTransformBoundingBox();
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

//-------------------------------------------------------------------------------
/*	Object Type  															   */
//-------------------------------------------------------------------------------
StaticObject::StaticObject()
{
	m_Mobility = Static;
}
StaticObject::StaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel) : StaticObject()
{
	CLoadedModelInfo* pMapModel = pModel;
	SetChild(pMapModel->m_pModelRootObject, true);
}
StaticObject::~StaticObject()
{
}

//-------------------------------------------------------------------------------
DynamicObject::DynamicObject()
{
	m_Mobility = Moveable;
}
DynamicObject::DynamicObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel) : DynamicObject()
{
	CLoadedModelInfo* pMapModel = pModel;
	SetChild(pMapModel->m_pModelRootObject, true);
}
DynamicObject::~DynamicObject()
{
}

MonsterObject::MonsterObject()
{
	m_Mobility = Moveable;
}
MonsterObject::MonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel) : MonsterObject()
{
	CLoadedModelInfo* pMapModel = pModel;
	SetChild(pMapModel->m_pModelRootObject, true);

	//HP바
	m_pHPMaterial = new CMaterial();
	m_pHPMaterial->AddRef();
	CTexture* pBulletTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pBulletTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"UI/hpds.dds", 0);
	m_pHPMaterial->SetTexture(pBulletTexture);
	CScene::CreateSRVUAVs(pd3dDevice, pBulletTexture, ROOT_PARAMETER_TEXTURE, true);
	m_pHPObject = new CRectTextureObject(pd3dDevice, pd3dCommandList, m_pHPMaterial);
	m_pHPObject->SetPosition(GetPosition().x, GetPosition().y + 10, GetPosition().z);
	SetChild(m_pHPObject, true);
}
MonsterObject::~MonsterObject()
{
}

void MonsterObject::UpdaetHP()
{
	
}
void MonsterObject::Conflicted(float damage)
{
	HP -= damage;
	std::cout << "Monster Life: " << HP << std::endl;
}

void MonsterObject::InitAnimPosition(int nAnimationTrack)
{
	m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, 0.0f);
}

bool MonsterObject::IsEndAnimPosition()
{
	float CurrentAnimPosition = m_pSkinnedAnimationController->GetTrackPosition();

	if (0.95f < CurrentAnimPosition)
	{
		bActivate = false;
		return true;
	}

	return false;
}

bool MonsterObject::IsAttackAnimPosition()
{ 
	float CurrentAnimPosition = m_pSkinnedAnimationController->GetTrackPosition();

	if (AttackAnimPosition <= CurrentAnimPosition && AttackAnimToggle == false)
	{
		AttackAnimToggle = true;
		return true;
	}
	if(CurrentAnimPosition < AttackAnimPosition && AttackAnimToggle)
		AttackAnimToggle = false;

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
CRectTextureObject::CRectTextureObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial)
{
	SetMaterial(0, pMaterial);
	m_pMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, XMFLOAT3(0.0f, 0.0f/*METER_PER_PIXEL(1.5)*/, 0.0f), METER_PER_PIXEL(6), METER_PER_PIXEL(0.15), 1.0f);
}
CRectTextureObject::~CRectTextureObject()
{
}

void CRectTextureObject::Update(float fTimeElapsed)
{
}
//-------------------------------------------------------------------------------
CHPBarTextureObject::CHPBarTextureObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial, float hp, float maxhp) : CRectTextureObject(pd3dDevice, pd3dCommandList, pMaterial)
{
	HP = hp;
	MAXHP = maxhp;
}
CHPBarTextureObject::~CHPBarTextureObject()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
CParticleObject::CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles)
{
}
CParticleObject::~CParticleObject()
{
	if (m_pRandowmValueTexture) m_pRandowmValueTexture->Release();
	if (m_pRandowmValueOnSphereTexture) m_pRandowmValueOnSphereTexture->Release();
}

void CParticleObject::ReleaseUploadBuffers()
{
	if (m_pRandowmValueTexture) m_pRandowmValueTexture->ReleaseUploadBuffers();
	if (m_pRandowmValueOnSphereTexture) m_pRandowmValueOnSphereTexture->ReleaseUploadBuffers();

	CGameObject::ReleaseUploadBuffers();
}

void CParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	for (int i = 0; i < m_nMaterials; ++i)
	{
		if (m_ppMaterials[i])
		{
			if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->OnPrepareRender(pd3dCommandList, 0);
			for (int t = 0; t < m_ppMaterials[i]->m_nTextures; ++t)
			{
				if (m_ppMaterials[i]->m_ppTextures[t]) m_ppMaterials[i]->m_ppTextures[t]->UpdateGraphicsShaderVariables(pd3dCommandList);
			}

			if (m_pRandowmValueTexture) m_pRandowmValueTexture->UpdateGraphicsShaderVariables(pd3dCommandList);
			if (m_pRandowmValueOnSphereTexture) m_pRandowmValueOnSphereTexture->UpdateGraphicsShaderVariables(pd3dCommandList);
		}
	}

	UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh) m_pMesh->PreRender(pd3dCommandList, 0); //Stream Output
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, 0); //Stream Output
	if (m_pMesh) m_pMesh->PostRender(pd3dCommandList, 0); //Stream Output

	for (int i = 0; i < m_nMaterials; ++i)
		if (m_ppMaterials[i] && m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->OnPrepareRender(pd3dCommandList, 1);

	if (m_pMesh) m_pMesh->PreRender(pd3dCommandList, 1); //Draw
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, 1); //Draw
}
void CParticleObject::OnPostRender()
{
	if (m_pMesh) m_pMesh->OnPostRender(0); //Read Stream Output Buffer Filled Size
}

void CParticleObject::PostRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pMesh) m_pMesh->PostRender(pd3dCommandList, 1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

CBoundingBox::CBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject()
{
}
CBoundingBox::~CBoundingBox()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pszFileName) : CGameObject()
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 20.0f);
	SetMesh(pSkyBoxMesh);

	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pszFileName, 0);

	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateSRVUAVs(pd3dDevice, pSkyBoxTexture, PARAMETER_SKYBOX_CUBE_TEXTURE, false);

	CMaterial* pSkyBoxMaterial = new CMaterial();
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}
CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);
	CGameObject::Render(pd3dCommandList, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, wchar_t* baseTexture, wchar_t* detailTexture, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject()
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	m_xmf3PrevScale = xmf3Scale;
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh* pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, this, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pHeightMapGridMesh);

	UINT nTexture = 2;

	CTexture* pTerrainTexture = new CTexture(nTexture, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, baseTexture, 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, detailTexture, 1);

	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	CScene::CreateSRVUAVs(pd3dDevice, pTerrainTexture, PARAMETER_MAP_TEXTURE, false);

	CMaterial* pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);
	pTerrainMaterial->SetShader(pTerrainShader);
	SetMaterial(0, pTerrainMaterial);
}
CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGroundObject::CGroundObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nWidth, int nLength) : StaticObject()
{
	CLoadedModelInfo* pGroundModel = pModel;
	SetChild(pGroundModel->m_pModelRootObject, true);
	if (pGroundModel->m_pModelRootObject->m_ppMaterials)
	{
		pGroundModel->m_pModelRootObject->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.45f, 0.45f, 0.6f, 1.0f);
	}

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_pHeightBuffer = new std::vector<XMFLOAT3>[nWidth * nLength];	
}
CGroundObject::~CGroundObject()
{
	if (m_pHeightBuffer) delete[] m_pHeightBuffer;
	m_pHeightBuffer = NULL;
}

void CGroundObject::SetDefaultHeight(float hHeight)
{
	m_fHeight = hHeight;
}
void CGroundObject::SetHeightBuffer()
{
	XMFLOAT4X4 transform = m_pChild->m_xmf4x4Transform;
	XMFLOAT3 Center = Vector3::TransformCoord(m_pChild->m_pMesh->GetAABBCenter(), transform);
	XMFLOAT3 Extents = Vector3::TransformCoord(m_pChild->m_pMesh->GetAABBExtents(), transform);
	XMFLOAT3 position = GetPosition();

	CStandardMesh* pMesh = (CStandardMesh*)m_pChild->m_pMesh;
	XMFLOAT3* pPositionBuffer = pMesh->GetPositionBuffer();
	XMFLOAT3* pNormalBuffer = pMesh->GetNormalBuffer();
	int nPosition = pMesh->GetVerticesNum();

	m_Rect.left = position.x - Extents.x + Center.x;
	m_Rect.top = position.z + Extents.z + Center.z;
	m_Rect.right = position.x + Extents.x + Center.x;
	m_Rect.bottom = position.z - Extents.z + Center.z;

	m_xInterval = Extents.x * 2.f / m_nWidth;
	m_zInterval = Extents.z * 2.f / m_nLength;

	for (int i = 0; i < nPosition; ++i)
	{
		XMFLOAT3 vertice = Vector3::Add(Vector3::TransformCoord(pPositionBuffer[i], transform), GetPosition());
		float xGap = vertice.x - m_Rect.left;
		float zGap = vertice.z - m_Rect.bottom;
		if (pNormalBuffer[i].y > 0)
		{
			int width = int(xGap / m_xInterval);
			int length = int(zGap / m_zInterval);
			float x = width * m_xInterval + m_Rect.left;
			float z = length * m_zInterval + m_Rect.bottom;
			if (m_nWidth - width < EPSILON) width -= 1;
			if (m_nLength - length< EPSILON) length -= 1;

			m_pHeightBuffer[width + (length * m_nWidth)].push_back(vertice);
			if (width > 0)
				m_pHeightBuffer[(width - 1) + length * m_nWidth].push_back(vertice);
			if (width > 0 && length + 1 < m_nLength)
				m_pHeightBuffer[(width - 1) + (length + 1) * m_nWidth].push_back(vertice);
			if (width + 1 < m_nWidth)
				m_pHeightBuffer[(width + 1) + length * m_nWidth].push_back(vertice);
			if(width + 2 < m_nWidth)
				m_pHeightBuffer[(width + 2) + length * m_nWidth].push_back(vertice);
			
			if(length > 0)
				m_pHeightBuffer[width + (length - 1) * m_nWidth].push_back(vertice);
			if (width + 1 < m_nWidth && length > 0)
				m_pHeightBuffer[(width + 1) + (length - 1) * m_nWidth].push_back(vertice);
			if (length + 1 < m_nLength)
				m_pHeightBuffer[width + (length + 1) * m_nWidth].push_back(vertice);
			if(length + 2 < m_nLength)
				m_pHeightBuffer[width + (length + 2) * m_nWidth].push_back(vertice);
			
			if (width + 1 < m_nWidth && length + 1 < m_nLength)
				m_pHeightBuffer[(width + 1) + (length + 1) * m_nWidth].push_back(vertice);
			if(width + 2 < m_nWidth && length + 1 < m_nLength)
				m_pHeightBuffer[(width + 2) + (length + 1) * m_nWidth].push_back(vertice);
			if (width + 1 < m_nWidth && length + 2 < m_nLength)
				m_pHeightBuffer[(width + 1) + (length + 2) * m_nWidth].push_back(vertice);
		}
	}

	for (int width = 0; width < m_nWidth; ++width)
	{
		for (int length = 0; length < m_nLength; ++length)
		{
			int nBuffer = width + (length * m_nWidth);
			float x = width * m_xInterval + m_Rect.left;
			float z = length * m_zInterval + m_Rect.bottom;
			XMFLOAT3 LB = XMFLOAT3(x - m_xInterval, m_fHeight, z - m_zInterval);
			XMFLOAT3 LT = XMFLOAT3(x - m_xInterval, m_fHeight, z + m_zInterval);
			XMFLOAT3 RB = XMFLOAT3(x + m_xInterval, m_fHeight, z - m_zInterval);
			XMFLOAT3 RT = XMFLOAT3(x + m_xInterval, m_fHeight, z + m_zInterval);
			float lbLength = FLT_MAX, ltLength = FLT_MAX, rbLength = FLT_MAX, rtLength = FLT_MAX;
			for (int i = 0; i < m_pHeightBuffer[nBuffer].size(); ++i)
			{
				XMFLOAT3 vertice = m_pHeightBuffer[nBuffer][i];
				float s = pow(x - vertice.x, 2) + pow(z - vertice.z, 2);
				if (vertice.x < x && vertice.z < z && s < lbLength)
				{
					lbLength = s;
					LB = vertice;
				}
				else if (vertice.x < x && vertice.z >= z && s < ltLength)
				{
					ltLength = s;
					LT = vertice;
				}
				else if (vertice.x >= x && vertice.z < z && s < rbLength)
				{
					rbLength = s;
					RB = vertice;
				}
				else if (vertice.x >= x && vertice.z >= z && s < rtLength)
				{
					rtLength = s;
					RT = vertice;
				}
			}
			float interpolation = (abs(LB.z - LT.z) < EPSILON) ? 0 : abs(LB.z - z) / abs(LB.z - LT.z);
			XMFLOAT3 fLeftHeight = Vector3::Add(Vector3::ScalarProduct(LB, (1 - interpolation), false), Vector3::ScalarProduct(LT, interpolation, false));
			interpolation = (abs(RB.z - RT.z) < EPSILON) ? 0 : abs(RB.z - z) / abs(RB.z - RT.z);
			XMFLOAT3 fRightHeight = Vector3::Add(Vector3::ScalarProduct(RB, (1 - interpolation), false), Vector3::ScalarProduct(RT, interpolation, false));
			interpolation = (abs(fLeftHeight.x - fRightHeight.x) < EPSILON) ? 0 : abs(fLeftHeight.x - x) / abs(fLeftHeight.x - fRightHeight.x);
			float fHeight = fLeftHeight.y * (1 - interpolation) + fRightHeight.y * (interpolation);
			m_pHeightBuffer[nBuffer].push_back(XMFLOAT3(x, fHeight, z));
		}
	}
}
float CGroundObject::GetHeight(float fx, float fz)
{
	if ((fx < m_Rect.left) || (fz < m_Rect.bottom) || (fx >= m_Rect.right) || (fz >= m_Rect.top)) return 0.0f;
	int x = int((fx - m_Rect.left) / m_xInterval);
	int z = int((fz - m_Rect.bottom) / m_zInterval);
	float fxPercent = 0.0f;
	float fzPercent = 0.0f;

	//if (!m_pHeightBuffer[x + (z * m_nWidth)].empty())
	//	std::cout << "index = " << x + (z * m_nWidth) << ", width: " << x << ", length: " << z << ", vertice: " << m_pHeightBuffer[x + (z * m_nWidth)].back().x
	//	<< ", " << m_pHeightBuffer[x + (z * m_nWidth)].back().y << ", " << m_pHeightBuffer[x + (z * m_nWidth)].back().z << std::endl;

	float fBottomLeft = (!m_pHeightBuffer[x + (z * m_nWidth)].empty()) ? m_pHeightBuffer[x + (z * m_nWidth)].back().y : 0.0f;
	float fBottomRight = (x + 1 < m_nWidth && !m_pHeightBuffer[(x + 1) + z * m_nWidth].empty()) ? m_pHeightBuffer[(x + 1) + z * m_nWidth].back().y : 0.0f;
	float fTopLeft = (z + 1 < m_nLength && !m_pHeightBuffer[x + (z + 1) * m_nWidth].empty()) ? m_pHeightBuffer[x + (z + 1) * m_nWidth].back().y : 0.0f;
	float fTopRight = (x + 1 < m_nWidth && z + 1 < m_nLength && !m_pHeightBuffer[(x + 1) + (z + 1) * m_nWidth].empty()) ? m_pHeightBuffer[(x + 1) + (z + 1) * m_nWidth].back().y : 0.0f;

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
	//float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	//float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	//float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;
	float fHeight = fBottomLeft;
	return fHeight;
}