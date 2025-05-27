#include "Scene.h"
#include "BattleFireDirect.h"
#include "StaticMeshComponent.h"
#include "Utils.h"
#include "Camera.h"
#include "Material.h"
#include "SceneNode.h"
#include "RenderTarget.h"
struct LightData{
	float mColor[4];
	float mPositionAndIntensity[4];//x,y,z,intensity
};
SceneNode* gSphereNode = nullptr,*gSkyBoxNode=nullptr,*gFSTNode=nullptr;
ID3D12Resource* gLightDataSB=nullptr;
DirectX::XMMATRIX gProjectionMatrix;
Camera gMainCamera;
ID3D12Resource* gTextureCube = nullptr,*gSBA = nullptr,*gSBB = nullptr,*gSBOut = nullptr;
RenderTarget* gRTLDR = nullptr;
ID3D12PipelineState* gCSPSO = nullptr;
ID3D12DescriptorHeap* gDescriptorHeap = nullptr;
void InitScene(int inCanvasWidth, int inCanvasHeight) {
	ID3D12Device* d3dDevice = GetD3DDevice();
	ID3D12GraphicsCommandList* commandList = GetCommandList();
	ID3D12CommandAllocator* commandAllocator = GetCommandAllocator();

	InitRootSignature();
	D3D12_SHADER_BYTECODE csShader;
	CreateShaderFromFile(L"Res/Shader/cs_rwbuffer.hlsl", "MainCS", "cs_5_1", &csShader);
	D3D12_COMPUTE_PIPELINE_STATE_DESC csPSODesc = {};
	csPSODesc.pRootSignature = GetRootSignature();
	csPSODesc.CS = csShader;
	GetD3DDevice()->CreateComputePipelineState(&csPSODesc, IID_PPV_ARGS(&gCSPSO));
	{
		float sbAData[] = {-0.5f,0.0f,0.0f};
		float sbBData[] = {0.5f,0.0f,0.0f};
		gSBA = CreateCPUGPUBufferObject(65536);
		gSBB = CreateCPUGPUBufferObject(65536);
		UpdateCPUGPUBuffer(gSBA, sbAData, sizeof(float) * 3);
		UpdateCPUGPUBuffer(gSBB, sbBData, sizeof(float) * 3);
		D3D12_HEAP_PROPERTIES d3dHeapProperties = {};
		d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		D3D12_RESOURCE_DESC d3d12ResourceDesc = {};
		d3d12ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		d3d12ResourceDesc.Alignment = 0;
		d3d12ResourceDesc.Width = 65536;
		d3d12ResourceDesc.Height = 1;
		d3d12ResourceDesc.DepthOrArraySize = 1;
		d3d12ResourceDesc.MipLevels = 1;
		d3d12ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		d3d12ResourceDesc.SampleDesc.Count = 1;
		d3d12ResourceDesc.SampleDesc.Quality = 0;
		d3d12ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		d3d12ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		d3dDevice->CreateCommittedResource(
			&d3dHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&d3d12ResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&gSBOut)
		);
	}

	gRTLDR = new RenderTarget(inCanvasWidth, inCanvasHeight);
	float clearColor[] = { 0.0f,0.0f,0.0f,0.0f };
	gRTLDR->AttachColorBuffer(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor);
	gRTLDR->AttachDSBuffer();
	const char* images[] = {
		"Res/Image/px.jpg",
		"Res/Image/nx.jpg",
		"Res/Image/py.jpg",
		"Res/Image/ny.jpg",
		"Res/Image/pz.jpg",
		"Res/Image/nz.jpg"
	};
	gTextureCube = CreateTextureCube(commandList, images);
	gLightDataSB = CreateCPUGPUBufferObject(65536);

	gProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		(90.0f * 3.141592f) / 180.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
	gMainCamera.Update(0.0f, 1.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	gSphereNode = new SceneNode;
	gSphereNode->mStaticMeshComponent = new StaticMeshComponent;
	gSphereNode->mStaticMeshComponent->mInstanceCount = 2;
	gSphereNode->mStaticMeshComponent->InitFromFile(commandList, "Res/Model/Sphere.lhsm");
	Material*material = new Material(L"Res/Shader/phong.hlsl");
	gSphereNode->mStaticMeshComponent->mSharedMaterial = material;
	material->InitMaterialData();
	Texture2D* texture2D = LoadTexture2DFromFile(commandList,"Res/Image/earth_d.jpg");
	Texture2D* normalTexture = LoadTexture2DFromFile(commandList, "Res/Image/Normal.png");
	material->SetTexture2D(0, texture2D->mResource,1,texture2D->mFormat);//0~15
	material->SetTexture2D(1, normalTexture->mResource, 1, normalTexture->mFormat);//0~15
	material->SetStructuredBuffer(17, gSBOut,sizeof(float)*3,2);

	gSkyBoxNode = new SceneNode;
	gSkyBoxNode->mStaticMeshComponent = new StaticMeshComponent;
	gSkyBoxNode->mStaticMeshComponent->InitFromFile(commandList, "Res/Model/skybox.lhsm");
	material = new Material(L"Res/Shader/skybox.hlsl");
	gSkyBoxNode->mStaticMeshComponent->mSharedMaterial = material;
	material->InitMaterialData();
	material->SetTextureCube(0, gTextureCube);//0~15
	material->SetStructuredBuffer(17, gLightDataSB, sizeof(LightData), 1);
	material->SetCullMode(D3D12_CULL_MODE_FRONT);
	material->EnableDepthTest(false);

	gFSTNode = new SceneNode;
	FullScreenTriangleComponent* fst = new FullScreenTriangleComponent;
	fst->Init(commandList);
	gFSTNode->mStaticMeshComponent = fst;
	material = new Material(L"Res/Shader/fst.hlsl");
	gFSTNode->mStaticMeshComponent->mSharedMaterial = material;
	material->InitMaterialData();
	material->SetTexture2D(0, gRTLDR->mColorBuffer, 1, gRTLDR->mColorRTFormat);//0~15
	material->SetStructuredBuffer(17, gLightDataSB, sizeof(LightData), 1);
	material->EnableDepthTest(false);

	EndCommandList();
	WaitForCompletionOfCommandList();
}

void RenderOneFrame(float inDeltaFrameTime, float inTimeSinceAppStart) {
	GlobalConstants globalConstants;
	DirectX::XMFLOAT4X4 tempMatrix;
	DirectX::XMStoreFloat4x4(&tempMatrix, gProjectionMatrix);
	memcpy(globalConstants.mProjectionMatrix, &tempMatrix, sizeof(float) * 16);
	DirectX::XMStoreFloat4x4(&tempMatrix, gMainCamera.mViewMatrix);
	memcpy(globalConstants.mViewMatrix, &tempMatrix, sizeof(float) * 16);
	globalConstants.mMisc[0] = inTimeSinceAppStart;

	ID3D12GraphicsCommandList* commandList = GetCommandList();
	ID3D12CommandAllocator* commandAllocator = GetCommandAllocator();
	commandAllocator->Reset();
	commandList->Reset(commandAllocator, nullptr);

	D3D12_RESOURCE_BARRIER barrier = InitResourceBarrier(
		gSBOut,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(1, &barrier);
	commandList->SetPipelineState(gCSPSO);
	commandList->SetComputeRootSignature(GetRootSignature());
	commandList->SetComputeRootShaderResourceView(3, gSBA->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(4, gSBB->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(5, gSBOut->GetGPUVirtualAddress());
	commandList->Dispatch(1, 1, 1);
	barrier = InitResourceBarrier(
		gSBOut,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_GENERIC_READ);//srv

	RenderTarget* currentRT = gRTLDR->BeginRendering(commandList);
	commandList->SetGraphicsRootSignature(GetRootSignature());
	commandList->SetGraphicsRoot32BitConstants(0, 36, &globalConstants, 0);
	commandList->SetGraphicsRootShaderResourceView(4, gSBOut->GetGPUVirtualAddress());

	gSkyBoxNode->SetPosition(gMainCamera.mPosition);
	gSkyBoxNode->Render(commandList); //over draw
	gSphereNode->Render(commandList);

	gRTLDR->EndRendering(commandList);
	EndCommandList();
	WaitForCompletionOfCommandList();

	commandAllocator->Reset();
	commandList->Reset(commandAllocator, nullptr);
	commandList->SetGraphicsRootSignature(GetRootSignature());
	commandList->SetGraphicsRoot32BitConstants(0, 36, &globalConstants, 0);
	commandList->SetGraphicsRootShaderResourceView(4, gLightDataSB->GetGPUVirtualAddress());
	BeginRenderToSwapChain(commandList);
	gFSTNode->Render(commandList);
	EndRenderToSwapChain(commandList);
	EndCommandList();
}