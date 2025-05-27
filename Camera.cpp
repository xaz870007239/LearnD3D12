#include "Camera.h"

void Camera::Update(float inPositionX, float inPositionY, float inPositionZ, float inFocusPointX, float inFocusPointY, float inFocusPointZ, float inUpX, float inUpY, float inUpZ) {
	mPosition = DirectX::XMVectorSet(inPositionX, inPositionY, inPositionZ, 1.0f);
	mFocusPoint = DirectX::XMVectorSet(inFocusPointX, inFocusPointY, inFocusPointZ, 1.0f);
	mViewMatrix = DirectX::XMMatrixLookAtLH(mPosition, mFocusPoint, DirectX::XMVectorSet(inUpX, inUpY, inUpZ, 0.0f));
}