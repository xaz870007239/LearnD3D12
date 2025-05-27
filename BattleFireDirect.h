#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>

D3D12_RESOURCE_BARRIER InitResourceBarrier(
	ID3D12Resource* inResource, D3D12_RESOURCE_STATES inPrevState,
	D3D12_RESOURCE_STATES inNextState);
ID3D12RootSignature* InitRootSignature();
ID3D12RootSignature* GetRootSignature();
void CreateShaderFromFile(
	LPCTSTR inShaderFilePath,
	const char* inMainFunctionName,
	const char* inTarget,//"vs_5_0","ps_5_0","vs_4_0"
	D3D12_SHADER_BYTECODE* inShader);
ID3D12Resource* CreateCPUGPUBufferObject(int inDataLen);
void UpdateCPUGPUBuffer(ID3D12Resource* inCB, void* inData, int inDataLen);
ID3D12Resource* CreateBufferObject(ID3D12GraphicsCommandList* inCommandList,
	void* inData, int inDataLen, D3D12_RESOURCE_STATES inFinalResourceState);
ID3D12PipelineState* CreatePSO(ID3D12RootSignature* inID3D12RootSignature,
	D3D12_SHADER_BYTECODE inVertexShader, D3D12_SHADER_BYTECODE inPixelShader,
	D3D12_CULL_MODE inCullMode = D3D12_CULL_MODE_BACK,bool inEnableDepthTest=true);
bool InitD3D12(HWND inHWND, int inWidth, int inHeight);
ID3D12GraphicsCommandList* GetCommandList();
ID3D12CommandAllocator* GetCommandAllocator();
void WaitForCompletionOfCommandList();
void EndCommandList();
void BeginRenderToSwapChain(ID3D12GraphicsCommandList* inCommandList);
void EndRenderToSwapChain(ID3D12GraphicsCommandList* inCommandList);
void SwapD3D12Buffers();
ID3D12Resource* CreateTexture2D(ID3D12GraphicsCommandList* inCommandList,
	const void*inPixelData,int inDataSizeInBytes,int inWidth,int inHeight,
	DXGI_FORMAT inFormat);
ID3D12Resource* CreateTextureCube(ID3D12GraphicsCommandList* inCommandList, const char** inFilePath);
ID3D12Device* GetD3DDevice();
struct Texture2D {
	ID3D12Resource* mResource;
	DXGI_FORMAT mFormat;
};
Texture2D* LoadTexture2DFromFile(ID3D12GraphicsCommandList* inCommandList, const char* inFilePath);