#include "Material.h"
#include "Utils.h"

#define StructuredBufferIndexStart 16

Material::Material(LPCTSTR inShaderFilePath) 
{
	ID3D12Device* d3dDevice = GetD3DDevice();
	CreateShaderFromFile(inShaderFilePath, "MainVS", "vs_5_1", &mVertexShader);
	CreateShaderFromFile(inShaderFilePath, "MainPS", "ps_5_1", &mPixelShader);

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDescSRV = {};
	d3dDescriptorHeapDescSRV.NumDescriptors = 32;
	d3dDescriptorHeapDescSRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDescSRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDescSRV, IID_PPV_ARGS(&mDescriptorHeap));

	mConstantBuffer = CreateCPUGPUBufferObject(65536);
	mMaterialDataSB = CreateCPUGPUBufferObject(65536);

	mCullMode = D3D12_CULL_MODE_BACK;
	mbEnableDepthTest = true;
	mbNeedUpdatePSO = true;
}

void Material::EnableDepthTest(bool inEnableDepthTest) 
{
	mbEnableDepthTest = inEnableDepthTest;
	mbNeedUpdatePSO = true;
}

void Material::SetCullMode(D3D12_CULL_MODE inCullMode) 
{
	mCullMode = inCullMode;
	mbNeedUpdatePSO = true;
}

void Material::SetTexture2D(int inSRVIndex, ID3D12Resource* inResource, int inMipMapLevelCount,DXGI_FORMAT inFormat) 
{
	ID3D12Device* d3dDevice = GetD3DDevice();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = inFormat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = inMipMapLevelCount;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHeapPtr = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapPtr.ptr += inSRVIndex * d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	d3dDevice->CreateShaderResourceView(inResource, &srvDesc, srvHeapPtr);
}

void Material::SetTextureCube(int inSRVIndex, ID3D12Resource* inResource, int inMipMapLevelCount, DXGI_FORMAT inFormat) 
{
	ID3D12Device* d3dDevice = GetD3DDevice();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = inFormat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = inMipMapLevelCount;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHeapPtr = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapPtr.ptr += inSRVIndex * d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	d3dDevice->CreateShaderResourceView(inResource, &srvDesc, srvHeapPtr);
}

void Material::SetStructuredBuffer(int inSRVIndex, ID3D12Resource* inResource, int inPerElementSize, int inElementCount) 
{
	ID3D12Device* d3dDevice = GetD3DDevice();

	D3D12_SHADER_RESOURCE_VIEW_DESC sbSRBDesc = {};
	sbSRBDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	sbSRBDesc.Format = DXGI_FORMAT_UNKNOWN;
	sbSRBDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	sbSRBDesc.Buffer.FirstElement = 0;
	sbSRBDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	sbSRBDesc.Buffer.NumElements = inElementCount;
	sbSRBDesc.Buffer.StructureByteStride = inPerElementSize;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHeapPtr = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapPtr.ptr += inSRVIndex * d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	d3dDevice->CreateShaderResourceView(inResource, &sbSRBDesc, srvHeapPtr);
}

void Material::Active(ID3D12GraphicsCommandList* inCommandList) 
{
	if (mbNeedUpdatePSO) 
	{
		mPSO = CreatePSO(GetRootSignature(), mVertexShader, mPixelShader, mCullMode,mbEnableDepthTest);
		mbNeedUpdatePSO = false;
	}

	ID3D12DescriptorHeap* descriptorHeaps[] = { mDescriptorHeap };

	inCommandList->SetPipelineState(mPSO);
	inCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	inCommandList->SetGraphicsRootConstantBufferView(1, mConstantBuffer->GetGPUVirtualAddress());
	inCommandList->SetGraphicsRootDescriptorTable(2, mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	inCommandList->SetGraphicsRootShaderResourceView(3, mMaterialDataSB->GetGPUVirtualAddress());
}

void Material::InitMaterialData() 
{
	float* materialDatas = new float[3000];
	memset(materialDatas, 0, sizeof(float) * 3000);
	UpdateCPUGPUBuffer(mMaterialDataSB, materialDatas, sizeof(float) * 3000);
	SetStructuredBuffer(StructuredBufferIndexStart, mMaterialDataSB, sizeof(float), 3000);
}