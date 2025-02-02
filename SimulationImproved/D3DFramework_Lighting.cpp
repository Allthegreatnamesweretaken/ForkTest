// D3DFramework_Lighting.cpp
#include "D3DFramework.h"

//--------------------------------------------------------------------------------------
// Update the light direction based on the current mode
//--------------------------------------------------------------------------------------
void D3DFramework::updateLightDirection() {
    if (_isFlashlightMode) {
        // Calculate the flashlight direction based on the player's facing direction
        XMVECTOR lookDirection = XMVector3Normalize(XMVectorSet(
            cosf(_cameraPitch) * sinf(_cameraYaw),
            sinf(_cameraPitch),
            cosf(_cameraPitch) * cosf(_cameraYaw),
            0.0f
        ));
        XMStoreFloat3(&_lightDirection, lookDirection);
    }
    else {
        // Use the initial sunlight direction
        _lightDirection = _initialSunLightDirection;
    }
}

//--------------------------------------------------------------------------------------
// Toggle between daylight and torch mode
//--------------------------------------------------------------------------------------
void D3DFramework::toggleDaylightTorchMode() {
    _isFlashlightMode = !_isFlashlightMode;
    updateLightDirection();
}

//--------------------------------------------------------------------------------------
// Adjust the time factor
//--------------------------------------------------------------------------------------
void D3DFramework::adjustTimeFactor(float adjustment) {
    _timeFactor = std::clamp(_timeFactor + adjustment, 0.1f, 10.0f);
}
