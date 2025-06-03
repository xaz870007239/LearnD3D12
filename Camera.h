#pragma once

#include <DirectXMath.h>

class Camera
{
public:

	void Update(float inPositionX,float inPositionY,float inPositionZ,
		float inFocusPointX,float inFocusPointY,float inFocusPointZ,
		float inUpX, float inUpY, float inUpZ);

	DirectX::XMMATRIX mViewMatrix;
	DirectX::XMVECTOR mPosition, mFocusPoint;
};

