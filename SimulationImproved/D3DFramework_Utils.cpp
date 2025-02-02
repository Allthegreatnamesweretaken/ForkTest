// D3DFramework_Utils.cpp
#include "D3DFramework.h"

//--------------------------------------------------------------------------------------
// Calculate delta time
//--------------------------------------------------------------------------------------
void D3DFramework::calculateDeltaTime() {
    _currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedTime = _currentTime - _previousTime;
    _deltaTime = elapsedTime.count() * _timeFactor;
    _previousTime = _currentTime;
}

//--------------------------------------------------------------------------------------
// Reset the application state
//--------------------------------------------------------------------------------------
void D3DFramework::reset() {
    _firstObjectPosition = { 0.0f, 0.0f, 0.0f };
    _firstObjectHorizontalVelocity = 0.0f;
    _firstObjectVerticalVelocity = 0.0f;
    _timeFactor = 1.0f;
    updateViewMatrix();
}

//--------------------------------------------------------------------------------------
// Lock the cursor within the application window
//--------------------------------------------------------------------------------------
void D3DFramework::lockCursor() {
    RECT rect;
    GetClientRect(_hWnd, &rect);
    MapWindowPoints(_hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
    ClipCursor(&rect);
}

//--------------------------------------------------------------------------------------
// Release the cursor
//--------------------------------------------------------------------------------------
void D3DFramework::releaseCursor() {
    ClipCursor(nullptr);
}
