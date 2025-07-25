#pragma once
#include <DirectXMath.h>

class StaticMeshComponent;

class SceneNode
{
public:

	SceneNode();

	void SetPosition(float inX, float inY, float inZ);
	void SetPosition(const DirectX::XMVECTOR& inPosition);
	void Render(ID3D12GraphicsCommandList* inCommandList);

	bool mbNeedUpdate;
	DirectX::XMVECTOR mPosition;
	StaticMeshComponent* mStaticMeshComponent;
};

