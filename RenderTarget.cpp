#include "RenderTarget.h"

RenderTarget::RenderTarget(int inWidth, int inHeight) 
{
	mWidth = inWidth;
	mHeight = inHeight;
}

void RenderTarget::AttachColorBuffer(DXGI_FORMAT inFormat, const float* inClearColor) 
{
	mColorRTFormat = inFormat;

	D3D12_HEAP_PROPERTIES d3dHeapProperties = {};
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC d3d12ResourceDesc = {};
	d3d12ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3d12ResourceDesc.Alignment = 0;
	d3d12ResourceDesc.Width = mWidth;
	d3d12ResourceDesc.Height = mHeight;
	d3d12ResourceDesc.DepthOrArraySize = 1;
	d3d12ResourceDesc.MipLevels = 1;
	d3d12ResourceDesc.Format = mColorRTFormat;
	d3d12ResourceDesc.SampleDesc.Count = 1;
	d3d12ResourceDesc.SampleDesc.Quality = 0;
	d3d12ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3d12ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = mColorRTFormat;
	memcpy(mClearColor, inClearColor, sizeof(float) * 4);
	memcpy(clearValue.Color, inClearColor, sizeof(float) * 4);
	GetD3DDevice()->CreateCommittedResource(&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&d3d12ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearValue,
		IID_PPV_ARGS(&mColorBuffer)
	);

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDescRTV = {};
	d3dDescriptorHeapDescRTV.NumDescriptors = 1;
	d3dDescriptorHeapDescRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	GetD3DDevice()->CreateDescriptorHeap(&d3dDescriptorHeapDescRTV, IID_PPV_ARGS(&mRTVDescriptorHeap));
	
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = mColorRTFormat;

	GetD3DDevice()->CreateRenderTargetView(mColorBuffer, &rtvDesc, mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void RenderTarget::AttachDSBuffer(DXGI_FORMAT inFormat) 
{
	mDSFormat = inFormat;
	ID3D12Device*device=GetD3DDevice();
	D3D12_HEAP_PROPERTIES d3dHeapProperties = {};
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC d3d12ResourceDesc = {};
	d3d12ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3d12ResourceDesc.Alignment = 0;
	d3d12ResourceDesc.Width = mWidth;
	d3d12ResourceDesc.Height = mHeight;
	d3d12ResourceDesc.DepthOrArraySize = 1;
	d3d12ResourceDesc.MipLevels = 1;
	d3d12ResourceDesc.Format = mDSFormat;
	d3d12ResourceDesc.SampleDesc.Count = 1;
	d3d12ResourceDesc.SampleDesc.Quality = 0;
	d3d12ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3d12ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE dsClearValue = {};
	dsClearValue.Format = mDSFormat;
	dsClearValue.DepthStencil.Depth = 1.0f;
	dsClearValue.DepthStencil.Stencil = 0;

	device->CreateCommittedResource(&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&d3d12ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&dsClearValue,
		IID_PPV_ARGS(&mDSBuffer)
	);
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDescDSV = {};
	d3dDescriptorHeapDescDSV.NumDescriptors = 1;
	d3dDescriptorHeapDescDSV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	device->CreateDescriptorHeap(&d3dDescriptorHeapDescDSV, IID_PPV_ARGS(&mDSDescriptorHeap));

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDSViewDesc = {};
	d3dDSViewDesc.Format = mDSFormat;
	d3dDSViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(mDSBuffer, &d3dDSViewDesc, mDSDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

RenderTarget* RenderTarget::BeginRendering(ID3D12GraphicsCommandList* inCommandList) 
{
	D3D12_RESOURCE_BARRIER barriers[2];
	barriers[0] = InitResourceBarrier(
		mColorBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	barriers[1] = InitResourceBarrier(
		mDSBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);
	inCommandList->ResourceBarrier(_countof(barriers), barriers);

	D3D12_CPU_DESCRIPTOR_HANDLE colorRT = mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = mDSDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	inCommandList->OMSetRenderTargets(1, &colorRT, FALSE, &dsv);
	D3D12_VIEWPORT viewport = { 0.0f,0.0f,float(mWidth),float(mHeight)};
	D3D12_RECT scissorRect = { 0,0,mWidth,mHeight };
	inCommandList->RSSetViewports(1, &viewport);
	inCommandList->RSSetScissorRects(1, &scissorRect);

	inCommandList->ClearRenderTargetView(colorRT, mClearColor, 0, nullptr);
	inCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	return this;
}

void RenderTarget::EndRendering(ID3D12GraphicsCommandList* inCommandList) 
{
	D3D12_RESOURCE_BARRIER barriers[2];
	barriers [0] = InitResourceBarrier(
		mColorBuffer, 
		D3D12_RESOURCE_STATE_RENDER_TARGET, 
		D3D12_RESOURCE_STATE_GENERIC_READ);
	barriers[1] = InitResourceBarrier(
		mDSBuffer,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_GENERIC_READ);
	inCommandList->ResourceBarrier(_countof(barriers), barriers);

}