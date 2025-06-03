#pragma once
#include "BattleFireDirect.h"
struct MaterialData 
{
	float mDiffuseMaterial[4];
	float mSpecularMaterial[4];
};

class Material
{
public:

	Material(LPCTSTR inShaderFilePath);

	void EnableDepthTest(bool inEnableDepthTest);
	void SetCullMode(D3D12_CULL_MODE inCullMode);
	void SetTexture2D(int inSRVIndex, ID3D12Resource* inResource, int inMipMapLevelCount=1, DXGI_FORMAT inFormat=DXGI_FORMAT_R8G8B8A8_UNORM);
	void SetTextureCube(int inSRVIndex, ID3D12Resource* inResource, int inMipMapLevelCount = 1, DXGI_FORMAT inFormat = DXGI_FORMAT_R8G8B8A8_UNORM);
	void SetStructuredBuffer(int inSRVIndex, ID3D12Resource* inResource, int inPerElementSize, int inElementCount);
	void Active(ID3D12GraphicsCommandList*inCommandList);
	void InitMaterialData();

	ID3D12Resource* mConstantBuffer;
	ID3D12DescriptorHeap* mDescriptorHeap;
	ID3D12Resource* mMaterialDataSB;
	ID3D12PipelineState* mPSO;
	D3D12_SHADER_BYTECODE mVertexShader,mPixelShader;
	D3D12_CULL_MODE mCullMode;
	bool mbEnableDepthTest;
	bool mbNeedUpdatePSO;
};

