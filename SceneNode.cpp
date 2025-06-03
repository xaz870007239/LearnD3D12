#include "Material.h"
#include "StaticMeshComponent.h"
#include "SceneNode.h"
#include "BattleFireDirect.h"

SceneNode::SceneNode() 
{
	mbNeedUpdate = true;
	mStaticMeshComponent = nullptr;
}

void SceneNode::SetPosition(float inX, float inY, float inZ) 
{
	mbNeedUpdate = true;
	mPosition = DirectX::XMVectorSet(inX, inY, inZ, 1.0f);
}

void SceneNode::SetPosition(const DirectX::XMVECTOR& inPosition) 
{
	mbNeedUpdate = true;
	mPosition = inPosition;
}

void SceneNode::Render(ID3D12GraphicsCommandList* inCommandList) 
{
	if (mbNeedUpdate) 
	{
		DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixTranslationFromVector(mPosition);
		DirectX::XMFLOAT4X4 tempMatrix;
		float matrices[32];
		DirectX::XMStoreFloat4x4(&tempMatrix, modelMatrix);
		memcpy(matrices, &tempMatrix, sizeof(float) * 16);;
		DirectX::XMVECTOR determinant;
		DirectX::XMMATRIX inverseModelMatrix = DirectX::XMMatrixInverse(&determinant, modelMatrix);
		
		if (DirectX::XMVectorGetX(determinant) != 0.0f) 
		{
			DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixTranspose(inverseModelMatrix);
			DirectX::XMStoreFloat4x4(&tempMatrix, modelMatrix);
			memcpy(matrices + 16, &tempMatrix, sizeof(float) * 16);;
		}

		UpdateCPUGPUBuffer(mStaticMeshComponent->mSharedMaterial->mConstantBuffer, matrices, sizeof(float) * 32);
		mbNeedUpdate = false;
	}
	mStaticMeshComponent->Render(inCommandList);
}
