#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "ShaderObjects.h"

ID3D12DescriptorHeap* CScene::m_pd3dCbvSrvUavDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavGPUDescriptorNextHandle;

CScene::CScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	m_pd3dComputeRootSignature = CreateComputeRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}
CScene::~CScene()
{
	ReleaseObjects();
	ReleaseShaderVariables();
}

ID3D12RootSignature* CScene::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 2;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0~t1: Texture2D
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; //u0: RWTexture2D
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture2D
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //RWTexture2D
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	ID3D12RootSignature* pd3dComputeRootSignature = CreateRootSignature(pd3dDevice, d3dRootSignatureFlags, _countof(pd3dRootParameters), pd3dRootParameters, 0, NULL);

	return pd3dComputeRootSignature;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[14];
	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 11; //t11: gtxtDetailAlbedoTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 12; //t12: gtxtDetailNormalTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 3;
	pd3dDescriptorRanges[9].BaseShaderRegister = 14; //t14~16: gtxtTerrainTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 1;
	pd3dDescriptorRanges[10].BaseShaderRegister = 1; //t1: gtxtParticleTexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 1;
	pd3dDescriptorRanges[11].BaseShaderRegister = 2; //t2: gtxtRandomTexture
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 3; //t3: gtxtRandomOnSphereTexture
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 4; //t4: ComputeOutput
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::array<D3D12_ROOT_PARAMETER, 21> pd3dRootParameters;
	for (int rootType = 0; rootType < NUM_ROOT_PARAMETER_TYPE; ++rootType)
	{
		switch (rootType) {
		case ROOT_PARAMETER_TEXUV:
			pd3dRootParameters[ROOT_PARAMETER_TEXUV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			pd3dRootParameters[ROOT_PARAMETER_TEXUV].Constants.Num32BitValues = 0;
			pd3dRootParameters[ROOT_PARAMETER_TEXUV].Constants.ShaderRegister = 5; //TextureUV
			pd3dRootParameters[ROOT_PARAMETER_TEXUV].Constants.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_TEXUV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		case ROOT_PARAMETER_CAMERA:
			pd3dRootParameters[ROOT_PARAMETER_CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			pd3dRootParameters[ROOT_PARAMETER_CAMERA].Descriptor.ShaderRegister = 1; //Camera
			pd3dRootParameters[ROOT_PARAMETER_CAMERA].Descriptor.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		case ROOT_PARAMETER_OBJECT:
			pd3dRootParameters[ROOT_PARAMETER_OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			pd3dRootParameters[ROOT_PARAMETER_OBJECT].Constants.Num32BitValues = 33;
			pd3dRootParameters[ROOT_PARAMETER_OBJECT].Constants.ShaderRegister = 2; //GameObject
			pd3dRootParameters[ROOT_PARAMETER_OBJECT].Constants.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		case ROOT_PARAMETER_LIGHT:
			pd3dRootParameters[ROOT_PARAMETER_LIGHT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			pd3dRootParameters[ROOT_PARAMETER_LIGHT].Descriptor.ShaderRegister = 4; //Light
			pd3dRootParameters[ROOT_PARAMETER_LIGHT].Descriptor.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_LIGHT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		default:
			pd3dRootParameters[ROOT_PARAMETER_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[ROOT_PARAMETER_TEXTURE].DescriptorTable.NumDescriptorRanges = 1; //Texture
			pd3dRootParameters[ROOT_PARAMETER_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
			pd3dRootParameters[ROOT_PARAMETER_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
			pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
			pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
			pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
			pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
			pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
			pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
			pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[12].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
			pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
			pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
			pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET].Descriptor.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
			pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM].Descriptor.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			pd3dRootParameters[ROOT_PARAMETER_PARTICLE_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[ROOT_PARAMETER_PARTICLE_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[ROOT_PARAMETER_PARTICLE_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[10]; //t1: gtxtParticleTexture
			pd3dRootParameters[ROOT_PARAMETER_PARTICLE_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			pd3dRootParameters[ROOT_PARAMETER_RANDOM_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[ROOT_PARAMETER_RANDOM_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[ROOT_PARAMETER_RANDOM_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[11]; //t2: gtxtRandomTexture
			pd3dRootParameters[ROOT_PARAMETER_RANDOM_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			pd3dRootParameters[ROOT_PARAMETER_RANDOM_ON_SPHERE_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[ROOT_PARAMETER_RANDOM_ON_SPHERE_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[ROOT_PARAMETER_RANDOM_ON_SPHERE_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[12]; //t3: gtxtRandomOnSphereTexture
			pd3dRootParameters[ROOT_PARAMETER_RANDOM_ON_SPHERE_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			pd3dRootParameters[ROOT_PARAMETER_FRAMEWORK_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			pd3dRootParameters[ROOT_PARAMETER_FRAMEWORK_INFO].Descriptor.ShaderRegister = 3; //Framework Info
			pd3dRootParameters[ROOT_PARAMETER_FRAMEWORK_INFO].Descriptor.RegisterSpace = 0;
			pd3dRootParameters[ROOT_PARAMETER_FRAMEWORK_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			pd3dRootParameters[ROOT_PARAMETER_OUTPUT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			pd3dRootParameters[ROOT_PARAMETER_OUTPUT].DescriptorTable.NumDescriptorRanges = 1;
			pd3dRootParameters[ROOT_PARAMETER_OUTPUT].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[13]; //t4: ComputeOutput
			pd3dRootParameters[ROOT_PARAMETER_OUTPUT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		}
	}

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];
	::ZeroMemory(&pd3dSamplerDescs, sizeof(D3D12_STATIC_SAMPLER_DESC));
	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	ID3D12RootSignature* pd3dGraphicsRootSignature = CreateRootSignature(pd3dDevice, d3dRootSignatureFlags, pd3dRootParameters.size(), pd3dRootParameters.data(), _countof(pd3dSamplerDescs), pd3dSamplerDescs);;

	return pd3dGraphicsRootSignature;
}
ID3D12RootSignature* CScene::GetComputeRootSignature()
{
	return m_pd3dComputeRootSignature;
}
ID3D12RootSignature* CScene::GetGraphicsRootSignature()
{
	return m_pd3dGraphicsRootSignature;
}
ID3D12RootSignature* CScene::CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags, UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters, UINT nStaticSamplerDescs, D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs)
{
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = nRootParameters;
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = nStaticSamplerDescs;
	d3dRootSignatureDesc.pStaticSamplers = pd3dStaticSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3D12RootSignature* pd3dRootSignature = NULL;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbLightsBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbLightsBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbElementBytes = ((sizeof(FRAMEWORK_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbFrameworkInfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dcbFrameworkInfo->Map(0, NULL, (void**)&m_pcbMappedFrameworkInfo);
}
void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLightss, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->gnLights, &m_nLights, sizeof(int));

	m_pcbMappedFrameworkInfo->m_fCurrentTime = m_fCurrentTime;
	m_pcbMappedFrameworkInfo->m_fElapsedTime = m_fElapsedTime;
	m_pcbMappedFrameworkInfo->m_fSecondsPerFirework = 0.4f;
	m_pcbMappedFrameworkInfo->m_nFlareParticlesToEmit = 100;
	m_pcbMappedFrameworkInfo->m_xmf3Gravity = XMFLOAT3(0.0f, PIXEL_KPH(-9.8), 0.0f);
	m_pcbMappedFrameworkInfo->m_nMaxFlareType2Particles = 15 * 1.5f;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbFrameworkInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_FRAMEWORK_INFO, d3dGpuVirtualAddress);
}
void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
	if (m_pd3dcbFrameworkInfo)
	{
		m_pd3dcbFrameworkInfo->Unmap(0, NULL);
		m_pd3dcbFrameworkInfo->Release();
	}
}


//--Build : CScene---------------------------------------------------------------
void CScene::CreateBoundingBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CBoundingBoxObjects* BBShader)
{
	for (int i = 0; i < m_ppShaders.size(); ++i)
	{
		m_ppShaders[i]->CreateBoundingBox(pd3dDevice, pd3dCommandList, BBShader);
	}

	for (int i = 0; i < m_vHierarchicalGameObjects.size(); ++i)
	{
		m_vHierarchicalGameObjects[i]->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);
	}

	//몬스터
	for (int i = 0; i < m_vMonsters.size(); ++i)
	{
		m_vMonsters[i]->CreateBoundingBoxMesh(pd3dDevice, pd3dCommandList, BBShader);
	}
}
void CScene::RunTimeBuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}
void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}
void CScene::BuildLightsAndMaterials()
{
}
void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();

	for (int i = 0; i < m_ppShaders.size(); ++i) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_UIShaders.size(); ++i) m_UIShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_ppComputeShaders.size(); ++i) m_ppComputeShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_vHierarchicalGameObjects.size(); i++) m_vHierarchicalGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_vGroundObjects.size(); i++) m_vGroundObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_vGameObjects.size(); i++) m_vGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_vParticleObjects.size(); i++) m_vParticleObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_vOtherPlayer.size(); i++) m_vOtherPlayer[i]->ReleaseUploadBuffers();
	//몬스터
	for (int i = 0; i < m_vMonsters.size(); i++) m_vMonsters[i]->ReleaseUploadBuffers();
}
void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dComputeRootSignature) m_pd3dComputeRootSignature->Release();
	if (m_pd3dCbvSrvUavDescriptorHeap) 
	{
		m_pd3dCbvSrvUavDescriptorHeap->Release();
		m_pd3dCbvSrvUavDescriptorHeap = NULL;
	}
	if (!m_ppShaders.empty())
	{
		for (int i = 0; i < m_ppShaders.size(); ++i)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
	}
	if (!m_UIShaders.empty())
	{
		for (int i = 0; i < m_UIShaders.size(); ++i)
		{
			m_UIShaders[i]->ReleaseShaderVariables();
			m_UIShaders[i]->ReleaseObjects();
			m_UIShaders[i]->Release();
		}
	}
	if (!m_ppComputeShaders.empty())
	{
		for (int i = 0; i < m_ppComputeShaders.size(); i++)
		{
			m_ppComputeShaders[i]->ReleaseShaderVariables();
			m_ppComputeShaders[i]->Release();
		}
	}
	if (!m_vHierarchicalGameObjects.empty())
	{
		for (int i = 0; i < m_vHierarchicalGameObjects.size(); i++) m_vHierarchicalGameObjects[i]->Release();
	}
	if (!m_vGroundObjects.empty())
	{
		for (int i = 0; i < m_vGroundObjects.size(); i++) m_vGroundObjects[i]->Release();
	}
	if (!m_vGameObjects.empty())
	{
		for (int i = 0; i < m_vGameObjects.size(); i++) m_vGameObjects[i]->Release();
	}
	if (!m_vParticleObjects.empty())
	{
		for (int i = 0; i < m_vParticleObjects.size(); ++i) m_vParticleObjects[i]->Release();
	}
	if (!m_vOtherPlayer.empty())
	{
		for (int i = 0; i < m_vOtherPlayer.size(); ++i) m_vOtherPlayer[i]->Release();
	}

	//몬스터
	if (!m_vMonsters.empty())
	{
		for (int i = 0; i < m_vMonsters.size(); ++i) m_vMonsters[i]->Release();
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;
	if (m_pLights) delete m_pLights;
}

//--ProcessInput : CScene--------------------------------------------------------
bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}
bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_CONTROL:
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
CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	if (!pCamera) return NULL;

	XMFLOAT4X4 xmf4x4View = pCamera->GetViewMatrix();
	XMFLOAT4X4 xmf4x4Projection = pCamera->GetProjectionMatrix();
	D3D12_VIEWPORT d3dViewport = pCamera->GetViewport();

	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
	CGameObject* pIntersectedObject = NULL, * pNearestObject = NULL;
	for (int i = 0; i < m_ppShaders.size(); ++i)
	{
		pIntersectedObject = m_ppShaders[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if (pIntersectedObject && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = pIntersectedObject;
		}
	}

	return pNearestObject;
}

//--ProcessAnimation : CScene----------------------------------------------------
void CScene::Update(float fTotalTime, float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	m_fCurrentTime = fTotalTime;
}
void CScene::Update(float fTimeElapsed)
{
}
void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_ppShaders.size(); ++i)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}

	for (int i = 0; i < m_vHierarchicalGameObjects.size(); ++i)
	{
		if(m_vHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_vHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_vHierarchicalGameObjects[i]->Animate(fTimeElapsed);
	}
	for (int i = 0; i < m_vParticleObjects.size(); i++)
	{
		m_vParticleObjects[i]->Animate(fTimeElapsed);
	}

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Position.y += METER_PER_PIXEL(1.3);
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetCamera()->GetLookVector();
	}
}

//--ProcessOutput : CScene-------------------------------------------------------
void CScene::RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (int i = 0; i < m_vParticleObjects.size(); i++) m_vParticleObjects[i]->Render(pd3dCommandList, pCamera);
}

void CScene::OnPostRenderParticle()
{
	for (int i = 0; i < m_vParticleObjects.size(); i++) m_vParticleObjects[i]->OnPostRender();
}
void CScene::OnPostReleaseUploadBuffers()
{
	for (int i = 0; i < m_ppShaders.size(); ++i) m_ppShaders[i]->OnPostReleaseUploadBuffers();
}
void CScene::PostRenderParticle(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_vParticleObjects.size(); i++) m_vParticleObjects[i]->PostRender(pd3dCommandList);
}
void CScene::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dComputeRootSignature) pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);
	if (m_pd3dCbvSrvUavDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvUavDescriptorHeap);
	for (int i = 0; i < m_ppComputeShaders.size(); ++i)
	{
		m_ppComputeShaders[i]->Dispatch(pd3dCommandList);
	}
	if (pCamera)
	{
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}
	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress); //Lights
}
void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_vHierarchicalGameObjects.size(); i++)
	{
		m_vHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
	}
	for (int i = 0; i < m_vGroundObjects.size(); i++)
	{
		m_vGroundObjects[i]->Render(pd3dCommandList, pCamera);
	}
	for (int i = 0; i < m_vGameObjects.size(); i++)
	{
		m_vGameObjects[i]->Render(pd3dCommandList, pCamera);
	}
	for (int i = 0; i < m_ppShaders.size(); ++i)
	{
		m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}
	for (int i = 0; i < m_vParticleObjects.size(); i++)
	{
		m_vParticleObjects[i]->Render(pd3dCommandList, pCamera);
	}

	for (int i = 0; i < m_vOtherPlayer.size(); ++i)
	{
		m_vOtherPlayer[i]->Render(pd3dCommandList, pCamera);
	}
	
	//몬스터
	for (int i = 0; i < m_vMonsters.size(); ++i)
	{
		m_vMonsters[i]->Render(pd3dCommandList, pCamera);
	}
}

void CScene::DrawUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (int i = 0; i < m_UIShaders.size(); ++i)
	{
		m_UIShaders[i]->Render(pd3dCommandList, pCamera);
	}
}

void CScene::SetLight(LIGHT& light, XMFLOAT4 xmf4Ambient, XMFLOAT4 xmf4Diffuse, XMFLOAT4 xmf4Specular,
	XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, XMFLOAT3 xmf3Attenuation,
	float fFalloff, float fTheta, float fPhi, bool bEnable, int nType, float fRange, float padding)
{
	light.m_xmf4Ambient = xmf4Ambient;
	light.m_xmf4Diffuse = xmf4Diffuse;
	light.m_xmf4Specular = xmf4Specular;
	light.m_xmf3Position = xmf3Position;
	light.m_fFalloff = fFalloff;
	light.m_xmf3Direction = xmf3Direction;
	light.m_fTheta = fTheta;
	light.m_xmf3Attenuation = xmf3Attenuation;
	light.m_fPhi = fPhi;
	light.m_bEnable = bEnable;
	light.m_nType = nType;
	light.m_fRange = fRange;
	light.padding = padding;
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvUavDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dUavCPUDescriptorNextHandle.ptr = m_d3dUavCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (nConstantBufferViews + nShaderResourceViews));
	m_d3dUavGPUDescriptorNextHandle.ptr = m_d3dUavGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (nConstantBufferViews + nShaderResourceViews));
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType, int nBufferElement = 0, int nBufferStride = 0, DXGI_FORMAT nBufferFormat = DXGI_FORMAT_UNKNOWN)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE1D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		d3dShaderResourceViewDesc.Texture1D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture1D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = nBufferFormat;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = nBufferElement;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	case RESOURCE_STRUCTURED_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = nBufferFormat;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = nBufferElement;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = nBufferStride;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}
D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType, int nBufferElement = 0, int nBufferStride = 0, DXGI_FORMAT nBufferFormat = DXGI_FORMAT_UNKNOWN)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc;
	d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		d3dUnorderedAccessViewDesc.Texture2DArray.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		d3dUnorderedAccessViewDesc.Texture2DArray.PlaneSlice = 0;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dUnorderedAccessViewDesc.Format = nBufferFormat;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		d3dUnorderedAccessViewDesc.Buffer.FirstElement = 0;
		d3dUnorderedAccessViewDesc.Buffer.NumElements = 0;
		d3dUnorderedAccessViewDesc.Buffer.StructureByteStride = 0;
		d3dUnorderedAccessViewDesc.Buffer.CounterOffsetInBytes = 0;
		d3dUnorderedAccessViewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	}
	return(d3dUnorderedAccessViewDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateSRVUAVs(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement, bool IsGraphics, bool IsSrv, UINT startIndex, UINT nViews, UINT nRepetition)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		DXGI_FORMAT nBufferFormat = pTexture->GetBufferFormat();
		int nBufferElement = pTexture->GetBufferElement();
		int nBufferStride = pTexture->GetBufferStride();
		int lastIndex = (nViews) ? startIndex + nViews : nTextures;
		for (int i = startIndex; i < lastIndex; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

			int argumentIndex = i - startIndex;
			if (IsSrv)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType, nBufferElement, nBufferStride, nBufferFormat);
				pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
				m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

				if (IsGraphics) 
					pTexture->SetGraphicsSrvRootArgument(argumentIndex + nRepetition, (bAutoIncrement) ? (nRootParameter + argumentIndex) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
				else 
					pTexture->SetComputeSrvRootArgument(argumentIndex + nRepetition, (bAutoIncrement) ? (nRootParameter + argumentIndex) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
				m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			}
			else
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = GetUnorderedAccessViewDesc(d3dResourceDesc, nTextureType, nBufferElement, nBufferStride, nBufferFormat);
				pd3dDevice->CreateUnorderedAccessView(pShaderResource, NULL, &d3dUnorderedAccessViewDesc, m_d3dUavCPUDescriptorNextHandle);
				m_d3dUavCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

				pTexture->SetComputeUavRootArgument(argumentIndex + nRepetition, (bAutoIncrement) ? (nRootParameter + argumentIndex) : nRootParameter, m_d3dUavGPUDescriptorNextHandle);
				m_d3dUavGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			}
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}