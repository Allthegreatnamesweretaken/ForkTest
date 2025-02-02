// D3DFramework_Input.cpp
#include "D3DFramework.h"
#include <Windows.h>
//#include <imgui.h>


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK D3DFramework::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	const PAINTSTRUCT ps;

	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	auto& app = D3DFramework::getInstance();

	const std::string msg;
	switch (message) {
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MOUSEMOVE:
		if (!app._mouseCaptured) {
			POINT currentMousePos;
			GetCursorPos(&currentMousePos);

			float deltaX = static_cast<float>(currentMousePos.x - app._lastMousePos.x);
			float deltaY = static_cast<float>(currentMousePos.y - app._lastMousePos.y);

			app._cameraYaw += deltaX * 0.002f; // Adjust sensitivity as needed
			app._cameraPitch += -deltaY * 0.002f;
			app._cameraPitch = std::clamp(app._cameraPitch, -XM_PIDIV2, XM_PIDIV2); // Limit pitch to avoid flipping

			RECT rect;
			GetClientRect(hWnd, &rect);
			POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
			ClientToScreen(hWnd, &center);
			SetCursorPos(center.x, center.y);

			app._lastMousePos = center;
		}
		break;

	case WM_RBUTTONDOWN:
	{
		// Release the cursor to allow free movement
		app.releaseCursor();
		app._mouseCaptured = false;
	}
	break;

	case WM_LBUTTONDOWN:
	{
		XMVECTOR lookDirection = XMVector3Normalize(XMVectorSet(
			cosf(app._cameraPitch) * sinf(app._cameraYaw),
			sinf(app._cameraPitch),
			cosf(app._cameraPitch) * cosf(app._cameraYaw),
			0.0f
		));

		app._firstObjectHorizontalVelocity = -XMVectorGetX(lookDirection) * 20;
		app._firstObjectHorizontalVelocityZ = -XMVectorGetZ(lookDirection) * 20;
		app._firstObjectVerticalVelocity = -XMVectorGetY(lookDirection) * 20;
	}
	break;

	case WM_KEYDOWN:
	{
		XMVECTOR lookDirection = XMVector3Normalize(XMVectorSet(
			sinf(app._cameraYaw),
			0.0f,
			cosf(app._cameraYaw),
			0.0f
		));

		switch (wParam) {
		case 'W':
			if (app._isOnSurface)
			{
				app.sprintFactor = app.sprintFactor + 0.01f;
				app._firstObjectHorizontalVelocityZ = XMVectorGetZ(lookDirection) * (app._moveSpeed + app.sprintFactor);
				app._firstObjectHorizontalVelocity = XMVectorGetX(lookDirection) * (app._moveSpeed + app.sprintFactor);
				app._decelerateForward = false;
			}

			break;
		case VK_ESCAPE:
			msg == "ESC pressed";
			OutputDebugStringA("ESC pressed\n");
			PostQuitMessage(0); // Exit the application
			break;
		case 'R':
			msg == "R pressed";
			OutputDebugStringA("R pressed\n");
			app.reset(); // Reset the application to its initial state
			break;
		case 'T':
			msg == "T pressed";
			OutputDebugStringA("T pressed\n");
			OutputDebugStringA("Toggled Light\n");
			app.toggleDaylightTorchMode(); // Toggle between daylight and torch mode
			break;
		case VK_OEM_COMMA: // '<' key
			msg == "< pressed";
			OutputDebugStringA("< pressed\n");
			app.adjustTimeFactor(-0.1f); // Decrease the time factor
			break;
		case VK_OEM_PERIOD: // '>' key
			msg == "> pressed";
			OutputDebugStringA("> pressed\n");
			app.adjustTimeFactor(0.1f); // Increase the time factor
			break;
		case VK_OEM_PLUS: // '=' key
			msg == "= pressed";
			OutputDebugStringA("= pressed\n");
			app.zoomIn(); // Zoom in
			break;
		case VK_OEM_MINUS: // '-' key
			msg == "- pressed";
			OutputDebugStringA("- pressed\n");
			app.zoomOut(); // Zoom out
			break;
		default:
			break;
		}
		break;
	}

	case WM_KEYUP:
		switch (wParam) {
		default:
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}