#pragma once
#include "BattleFireDirect.h"

class RenderTarget
{
public:

	RenderTarget(int inWidth, int inHeight);

	void AttachColorBuffer(DXGI_FORMAT inFormat, const float* inClearColor);
	void AttachDSBuffer(DXGI_FORMAT inFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
	RenderTarget* BeginRendering(ID3D12GraphicsCommandList* inCommandList);
	void EndRendering(ID3D12GraphicsCommandList* inCommandList);

	float mClearColor[4];
	DXGI_FORMAT mColorRTFormat,mDSFormat;//D16,D32,D24S8
	ID3D12Resource* mColorBuffer,*mDSBuffer;
	ID3D12DescriptorHeap* mRTVDescriptorHeap,*mDSDescriptorHeap;
	int mWidth, mHeight;
};

