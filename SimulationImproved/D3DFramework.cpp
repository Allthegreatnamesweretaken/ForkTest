#include "D3DFramework.h"
#include <directxcolors.h>
#include <vector>
#include "Resource.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include "DDSTextureLoader.h"
#include <unordered_map>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "directxcollision.h"
//Can be final but want scene changes first.

/*Need to Do list:

---Priority---
- Need to work on redoing everything seperating it all into different files, this could help efficiency and make it easier to read, will also help with the fact that the code is getting very long.
- Need to refine the shaders (if i am keeping them) as the way they are loaded and managed currently is not efficient.
- File manager, for simplicity of being able to find parts of code easier, as in files for shaders etc etc.

-- Maps could be done by having a map loader that loads in the objects and textures, the map loader would be built from config files that would be loaded in at the start of the game, menu would pick between them.


- Remember to Update the README everytime i push a new change.
---Secondary---
- FPS limiter, need to add one as i can hear my fans crying every time i run this. (Maybe find a way to load faster(Or it could be just how DX11 loads?))
- Fix Collision as the player can still currently warp up through the floor.

- Need to think more about how the game will work, while the baseline is okay need more idea on it like multiple guns? More maps? Enemines? and general feeling for the game.
-- Am liking the idea of a base parkour game for the beginning, can add multiple guns with a switch statment to change the way they fire,(And maybe their model) Enemies would be a long way off if not impossible because of the way the game is designed. 

-- Have Implemented new movement as a way around, this movement is only controlled by the W key and the player moving the mouse the player loses controller of the W key while in air and has to reply on the launching to get to the end.
- Add in a way to let the player have in air controls to make teh movement while falling feel smoother, can also change gravity if needed as it right now is strange.
- change the wya the shaders are loaded should be able to be done in a loop.
- Change the way they are added to an object should be able to do that inside of where we create the object.
*/

std::unique_ptr<D3DFramework> D3DFramework::_instance = std::make_unique<D3DFramework>();
//Config can stay as a way of loading objects can be changed later on though if can think of better method. Also see if Objects can be laoded that aren't an obj / are more efficient.
bool D3DFramework::LoadConfig(const std::string& pathname) {
	std::ifstream fin(pathname);
	if (!fin) {
		return false;
	}

	std::string tag;
	while (fin >> tag) {
		if (tag == "name") {
			ObjModel tempObject;
			std::string objectName;
			fin >> objectName;
			objectNames.push_back(objectName);
			_modelLoader.LoadOBJ(objectName, tempObject);
			_models.push_back(tempObject);
		}
		else if (tag == "texture") {
			std::string texturePath;
			fin >> texturePath;
			textureNames.push_back(texturePath);
		}
		else if (tag == "position") {
			XMFLOAT3 vertex;
			fin >> vertex.x >> vertex.y >> vertex.z;
			objectPositions.push_back(vertex);
		}
		else if (tag == "player_position") {
			fin >> _firstObjectPosition.x >> _firstObjectPosition.y >> _firstObjectPosition.z;
		}
		else if (tag == "light_direction") {
			fin >> _initialSunLightDirection.x >> _initialSunLightDirection.y >> _initialSunLightDirection.z;
			_lightDirection = _initialSunLightDirection;
		}
		else {
			std::string line;
			std::getline(fin, line);
		}
	}
	return true;
}
void D3DFramework::lockCursor() {
	RECT rect;
	GetClientRect(_hWnd, &rect);
	MapWindowPoints(_hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
	ClipCursor(&rect);
}

void D3DFramework::releaseCursor() {
	ClipCursor(nullptr);
}
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


void D3DFramework::reset() {
	// Reset the application to its initial state / This is not resetting the game as a whole, just the player.
	_firstObjectPosition = { 0.0f, 0.0, 0.0f };
	_firstObjectHorizontalVelocity = 0.0f;
	_firstObjectVerticalVelocity = 0.0f;
	/*_lookAboveObject = false;
	_lookBelowObject = false;*/
	_timeFactor = 1.0f; // Reset the time factor
	updateViewMatrix();
}

void D3DFramework::toggleDaylightTorchMode() {
    // Toggle between daylight and torch mode
    _isFlashlightMode = !_isFlashlightMode;
    updateLightDirection();

    // Output the current light mode
    if (_isFlashlightMode) {
        OutputDebugStringA("Flashlight mode activated\n");
    } else {
        OutputDebugStringA("Sunlight mode activated\n");
    }
}
void D3DFramework::updateLightDirection() {
	if (_isFlashlightMode) {
		// Calculate the flashlight direction based on the player's facing direction
		XMVECTOR playerForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // Assuming the player faces along the Z-axis
		const XMMATRIX playerRotation = XMMatrixRotationY(_firstObjectHorizontalVelocity); // Assuming horizontal velocity affects rotation
		playerForward = XMVector3TransformNormal(playerForward, playerRotation);
		XMStoreFloat3(&_lightDirection, playerForward);
	}
	else {
		// Sun-like light direction
		_lightDirection = _initialSunLightDirection; // Light coming from the top-left, can be changed in GUI and is set in config file.
	}
}



void D3DFramework::adjustTimeFactor(float adjustment) {
	// Adjust the time factor
	_timeFactor = std::clamp(_timeFactor + adjustment, 0.1f, 10.0f);
}
void D3DFramework::updateViewMatrix() {
	// Calculate the look direction based on yaw and pitch
	  // Offset the camera position relative to the player's position
	XMFLOAT3 cameraOffset = XMFLOAT3(0.0f, 1.0f, -1.0f); // Adjust the offset as needed
	XMVECTOR offset = XMLoadFloat3(&cameraOffset);
	XMVECTOR playerPosition = XMLoadFloat3(&_firstObjectPosition);
	XMVECTOR cameraPosition = XMVectorAdd(playerPosition, offset);
	XMStoreFloat3(&_cameraPosition, cameraPosition);



	// Calculate the look direction based on the player's look direction
	XMVECTOR lookDirection = XMVector3Normalize(XMVectorSet(
		cosf(_cameraPitch) * sinf(_cameraYaw),
		sinf(_cameraPitch),
		cosf(_cameraPitch) * cosf(_cameraYaw),
		0.0f
	));

	// Calculate the right direction
	XMVECTOR rightDirection = XMVector3Normalize(XMVectorSet(
		sinf(_cameraYaw - XM_PIDIV2),
		0.0f,
		cosf(_cameraYaw - XM_PIDIV2),
		0.0f
	));

	// Calculate the up direction
	XMVECTOR upDirection = XMVector3Cross(rightDirection, lookDirection);

	// Calculate the look-at position
	XMVECTOR lookAtPosition = XMVectorAdd(XMLoadFloat3(&_cameraPosition), lookDirection);

	// Use the camera position directly
	const XMVECTOR Eye = XMLoadFloat3(&_cameraPosition);
	const XMVECTOR At = lookAtPosition;
	const XMVECTOR Up = upDirection;

	_View = XMMatrixLookAtLH(Eye, At, Up);

	// Update the Constant Buffer
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(_World);
	cb.mView = XMMatrixTranspose(_View);
	cb.mProjection = XMMatrixTranspose(_Projection);
	cb.eyePos = Eye;
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT D3DFramework::initWindow(HINSTANCE hInstance, int nCmdShow) {
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMULATION));
	wcex.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_ARROW));
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"Starter Template";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SIMULATION));
	if (!RegisterClassEx(&wcex))
		return static_cast<HRESULT>(0x80004005L);

	// Create window
	_hInst = hInstance;
	RECT rc = { 0, 0, 1920, 1080 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	_hWnd = CreateWindow(L"Starter Template", L"Platformer Demo",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!_hWnd)
		return static_cast<HRESULT>(0x80004005L);

	ShowWindow(_hWnd, nCmdShow);

	return static_cast<HRESULT>(0L); //S_OK
}




//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT D3DFramework::compileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	auto dwShaderFlags = static_cast<DWORD>(D3DCOMPILE_ENABLE_STRICTNESS);
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	CComPtr <ID3DBlob> pErrorBlob;
	const auto hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr)) {
		if (pErrorBlob)
			OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
		return hr;
	}

	return static_cast<HRESULT>(0L); //S_OK
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT D3DFramework::initDevice()
{
	//Initialize the time

	_previousTime = std::chrono::high_resolution_clock::now();


	auto hr = static_cast<HRESULT>(0L); //S_OK;	

	RECT rc;
	GetClientRect(_hWnd, &rc);
	const UINT width = rc.right - rc.left;
	const UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	auto numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	auto numFeatureLevels = static_cast<UINT>(ARRAYSIZE(featureLevels));

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &_pd3dDevice, &_featureLevel, &_pImmediateContext);

		if (hr == static_cast<HRESULT>(0x80070057L))
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, _driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, &_pd3dDevice, &_featureLevel, &_pImmediateContext);

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	CComPtr <IDXGIFactory1> dxgiFactory;
	{
		CComPtr <IDXGIDevice> dxgiDevice;
		hr = _pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr)) {
			CComPtr <IDXGIAdapter> adapter;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr)) {
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
			}
		}
	}
	if (FAILED(hr))
		return hr;

	//Im Gui setup
	// // Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(_pd3dDevice, _pImmediateContext);
	// Create swap chain
	CComPtr <IDXGIFactory2> dxgiFactory2;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));

	// DirectX 11.1 or later
	hr = _pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&_pd3dDevice1));
	if (SUCCEEDED(hr)) {
		static_cast<void>(_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&_pImmediateContext1)));
	}

	DXGI_SWAP_CHAIN_DESC1 sd{};
	sd.Width = width;
	sd.Height = height;
	sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;

	hr = dxgiFactory2->CreateSwapChainForHwnd(_pd3dDevice, _hWnd, &sd, nullptr, nullptr, &_swapChain1);
	if (SUCCEEDED(hr)) {
		hr = _swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&_swapChain));
	}

	// Note this tutorial doesn't handle full-screen swap chains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER);

	if (FAILED(hr))
		return hr;

	// Create a render target view
	CComPtr <ID3D11Texture2D> pBackBuffer;
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
	if (FAILED(hr))
		return hr;

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView.p, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(width);
	vp.Height = static_cast<FLOAT>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pImmediateContext->RSSetViewports(1, &vp);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	auto numElements = static_cast<UINT>(ARRAYSIZE(layout));

#ifdef COMPILE_CSO
	//This can 100% be made more efficient or it can be changed to be loaded in from the config so it loops.
	// Need to do this soon will make the code look simplier and easier to read.
	// Compile the vertex shader
	CComPtr <ID3DBlob> pVSBlob;
	hr = compileShaderFromFile(L"Simulation.fx", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error Primary Vertex Shader", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader.p);
	if (FAILED(hr)) {
		return hr;
	}

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &_pVertexLayout.p);
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	CComPtr <ID3DBlob> pPSBlob;
	hr = compileShaderFromFile(L"Simulation.fx", "PS", "ps_5_0", &pPSBlob);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error Primary Pixel Shader", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader.p);
	if (FAILED(hr))
		return hr;

	//SKYBOX VERTEX SHADER
	//THIS CAUSES ONE OF THE WARNING POPUPS if you dont use a new psblob
	CComPtr <ID3DBlob> skyVSBlob;
	hr = compileShaderFromFile(L"Sky.fx", "VS", "vs_5_0", &skyVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error Skybox Vertex Shader", MB_OK);
		return hr;
	}
	hr = _pd3dDevice->CreateVertexShader(skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), nullptr, &_skyboxVertexShader.p);
	if (FAILED(hr))
		return hr;

	//Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), &_skyboxVertexLayout.p);
	if (FAILED(hr))
		return hr;

	CComPtr <ID3DBlob> skyPSBlob;
	hr = compileShaderFromFile(L"Sky.fx", "PS", "ps_5_0", &skyPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error SKybox Pixel shader", MB_OK);
		return hr;
	}
	hr = _pd3dDevice->CreatePixelShader(skyPSBlob->GetBufferPointer(), skyPSBlob->GetBufferSize(), nullptr, &_skyboxPixelShader.p);


	// Compile the vertex shader
	CComPtr<ID3DBlob> envpVSBlob;
	hr = compileShaderFromFile(L"Enviroment.fx", "VS", "vs_5_0", &envpVSBlob);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", L"Error Environment Map Vertex Shader", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(envpVSBlob->GetBufferPointer(), envpVSBlob->GetBufferSize(), nullptr, &_envMapVertexShader);
	if (FAILED(hr)) {
		return hr;
	}

	// Compile the pixel shader
	CComPtr<ID3DBlob> envpPSBlob;
	hr = compileShaderFromFile(L"Enviroment.fx", "PS", "ps_5_0", &envpPSBlob);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", L"Error Environment Map Pixel Shader", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(envpPSBlob->GetBufferPointer(), envpPSBlob->GetBufferSize(), nullptr, &_envMapPixelShader);
	if (FAILED(hr)) {
		return hr;
	}
	//Had to remake this because it complained when using the other one.
	D3D11_INPUT_ELEMENT_DESC envlayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT envnumElements = ARRAYSIZE(layout);

	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &_envMapVertexLayout);
	if (FAILED(hr)) {
		return hr;
	}
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 0;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = _pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil.p);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = _pd3dDevice->CreateDepthStencilView(g_pDepthStencil.p, &descDSV, &g_pDepthStencilView.p);
	if (FAILED(hr))
		return hr;

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView.p, g_pDepthStencilView.p);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Create depth stencil parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	//Disable depth buffer for the sky state
	_pd3dDevice->CreateDepthStencilState(&dsDesc, &_skyboxDepthStencilState.p);

	//Enable depth buffer for the cube state
	dsDesc.DepthEnable = true;
	_pd3dDevice->CreateDepthStencilState(&dsDesc, &_objectsDepthStencilState.p);

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.ScissorEnable = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	hr = _pd3dDevice->CreateRasterizerState(&rasterDesc, &_skyboxRasterizerState.p);
	if (FAILED(hr))
		return hr;

	rasterDesc.CullMode = D3D11_CULL_BACK;
	hr = _pd3dDevice->CreateRasterizerState(&rasterDesc, &_objectsRasterizerState.p);
	if (FAILED(hr))
		return hr;

	//_pImmediateContext->RSSetState(_objectsRasterizerState);
	//This also should be done from the config file.
    hr = _textureManager.loadDDSTexture(_pd3dDevice, L"Skymap.dds", &_skyboxTextureRV.p); //Skymap.dds
	if (FAILED(hr))
		return hr;

	// Create the sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = _pd3dDevice->CreateSamplerState(&sampDesc, &wood_Sampler.p);
	if (FAILED(hr)) {
		return hr;
	}
	

#else
	{
		const std::string fileName{ "..\\Debug\\Simulation_PS.cso" };
		std::ifstream fin(fileName, std::ios::binary);
		if (!fin) {
			MessageBox(nullptr, L"The CSO file cannot be found.", L"Error", MB_OK);
			return E_FAIL;
		}
		std::vector<unsigned char> byteCode(std::istreambuf_iterator<char>(fin), {});

		hr = _pd3dDevice->CreatePixelShader(&byteCode[0], byteCode.size(), nullptr, &_pPixelShader);
		if (FAILED(hr)) {
			return hr;
		}
	}
#endif		
	LoadConfig("objects.txt");
	// Iterate over each .obj file in the objFiles vector
	for (const auto& file : objectNames) {
		// Create an ObjModel instance to store the loaded model data
		ObjModel model;

		// Load the .obj file into the model
		if (!_modelLoader.LoadOBJ(file, model)) {
			// If loading fails, display an error message and return E_FAIL
			const std::wstring errorMsg = L"Failed to load .obj file: " + std::wstring(file.begin(), file.end());
			MessageBox(nullptr, errorMsg.c_str(), L"Error Object Loader", MB_OK);
			return static_cast<HRESULT>(0x80004005L);
		}
		// Create vertex buffer
		std::vector<SimpleVertex> vertices;
		for (size_t i = 0; i < model.vertices.size(); ++i) {
			SimpleVertex vertex;
			vertex.Pos = model.vertices[i];
			vertex.Normal = (i < model.normals.size()) ? model.normals[i] : XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.TexCoord = (i < model.textures.size()) ? model.textures[i] : XMFLOAT2(0.0f, 0.0f);

		
			vertices.push_back(vertex);
		}
		// Describe the vertex buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;


		// Initialize the vertex buffer with the vertex data
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices.data();
		CComPtr<ID3D11Buffer> vertexBuffer;
		hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &vertexBuffer);
		if (FAILED(hr)) {
			OutputDebugStringA("Failed to create vertex buffer\n");
			return hr;
		}

		// Add the created vertex buffer to the _vertexBuffers vector
		_vertexBuffers.push_back(vertexBuffer);
		OutputDebugStringA("Created vertex buffer\n");

		// Create index buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * model.indices.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = model.indices.data();
		CComPtr<ID3D11Buffer> indexBuffer;
		hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &indexBuffer);
		if (FAILED(hr)) {
			OutputDebugStringA("Failed to create index buffer\n");
			return hr;
		}

		// Add the created index buffer to the _indexBuffers vector
		_indexBuffers.push_back(indexBuffer);
		OutputDebugStringA("Created index buffer\n");
		// Release the previous constant buffer if it exists
		if (_pConstantBuffer) {
			_pConstantBuffer.Release();
		}

		// Create the constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer.p);
		if (FAILED(hr))
			return hr;

		
	}

	//Rasterizer State
	ID3D11RasterizerState* m_rasterState;
	//D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	//rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.ScissorEnable = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	hr = _pd3dDevice->CreateRasterizerState(&rasterDesc, &m_rasterState);

	_pImmediateContext->RSSetState(m_rasterState);

	_WorldMatrices.resize(_models.size(), XMMatrixIdentity());

	// Initialize the world matrix
	_World = XMMatrixIdentity();

	updateViewMatrix();

	// Initialize the projection matrix
	_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / static_cast<FLOAT>(height), 0.01f, 100.0f);

	return static_cast<HRESULT>(0L);
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
D3DFramework::~D3DFramework() {
	try {
		if (_pImmediateContext)
			_pImmediateContext->ClearState();
	}
	catch (...) {

	}
}
//--------------------------------------------------------------------------------------
// Time Calculator
//--------------------------------------------------------------------------------------
void D3DFramework::calculateDeltaTime() {
	_currentTime = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<float> elapsedTime = _currentTime - _previousTime;
	_deltaTime = elapsedTime.count() * _timeFactor; // Adjust delta time by the time factor
	_previousTime = _currentTime;
}
void D3DFramework::zoomIn() {
	if (_zoomFactor > 0.1f) {
		_zoomFactor -= 0.1f; // Decrease zoom factor, but not less than 0.1
		updateViewMatrix();
	}
}
size_t i = 0;
void D3DFramework::zoomOut() {
	if (_zoomFactor < 10.0f) {
		_zoomFactor += 0.1f; // Increase zoom factor, but not more than 10.0
		updateViewMatrix();
	}
}
//OutputDebugStringA("Shifted\n");
//Temp Test code
void D3DFramework::updateWorldMatrix(float deltaTime) {
	static float t = 0.0f;
	t += deltaTime;

	_firstObjectVerticalVelocity += _gravity * deltaTime;
	_firstObjectPosition.y += _firstObjectVerticalVelocity * deltaTime;

	_firstObjectPosition.x += _firstObjectHorizontalVelocity * deltaTime;


	_firstObjectPosition.z += _firstObjectHorizontalVelocityZ * deltaTime;



	XMMATRIX rotationMatrix = XMMatrixRotationY(-_cameraYaw);
	XMMATRIX translationMatrix = XMMatrixTranslation(_firstObjectPosition.x, _firstObjectPosition.y, _firstObjectPosition.z);
	_WorldMatrices[0] = rotationMatrix * translationMatrix;

	BoundingBox firstObjectBox;
	firstObjectBox.Center = _firstObjectPosition;
	firstObjectBox.Extents = XMFLOAT3(1.0f, 1.0f, 1.0f);

	_isOnSurface = false;

	for (size_t i = 1; i < _models.size(); ++i) {
		BoundingBox otherObjectBox;

		if (i == 1 || i == 2) {
			continue;
		}

		XMVECTOR scale, rotation, translation;
		XMMatrixDecompose(&scale, &rotation, &translation, _WorldMatrices[i]);
		XMStoreFloat3(&otherObjectBox.Center, translation);

		XMFLOAT3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
		XMFLOAT3 maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (const auto& vertex : _models[i].vertices) {
			if (vertex.x < minPoint.x) minPoint.x = vertex.x;
			if (vertex.y < minPoint.y) minPoint.y = vertex.y;
			if (vertex.z < minPoint.z) minPoint.z = vertex.z;

			if (vertex.x > maxPoint.x) maxPoint.x = vertex.x;
			if (vertex.y > maxPoint.y) maxPoint.y = vertex.y;
			if (vertex.z > maxPoint.z) maxPoint.z = vertex.z;
		}

		otherObjectBox.Extents = XMFLOAT3(
			(maxPoint.x - minPoint.x) / 2.0f,
			(maxPoint.y - minPoint.y),
			(maxPoint.z - minPoint.z) / 2.0f
		);

		otherObjectBox.Center = XMFLOAT3(
			(maxPoint.x + minPoint.x) / 2.0f,
			(maxPoint.y + minPoint.y) / 2.0f,
			(maxPoint.z + minPoint.z) / 2.0f
		);

		BoundingBox transformedBox;
		otherObjectBox.Transform(transformedBox, _WorldMatrices[i]);

		if (firstObjectBox.Intersects(transformedBox)) {
			if (_firstObjectPosition.y > transformedBox.Center.y + transformedBox.Extents.y &&
				_firstObjectPosition.x + firstObjectBox.Extents.x > transformedBox.Center.x - transformedBox.Extents.x &&
				_firstObjectPosition.x - firstObjectBox.Extents.x < transformedBox.Center.x + transformedBox.Extents.x) {
				const float targetY = transformedBox.Center.y + transformedBox.Extents.y;
				if (_firstObjectPosition.y - targetY < 0.1f) {
					if (_firstObjectVerticalVelocity < 0) {
						_firstObjectVerticalVelocity = 0.0f;
					}
				}
				_isOnSurface = true;
			}
			else {
				if (_firstObjectPosition.y > transformedBox.Center.y - transformedBox.Extents.y &&
					_firstObjectPosition.y < transformedBox.Center.y + transformedBox.Extents.y) {
					if (_firstObjectPosition.x < transformedBox.Center.x - transformedBox.Extents.x) {
						_firstObjectPosition.x = transformedBox.Center.x - transformedBox.Extents.x - firstObjectBox.Extents.x;
					}
					else if (_firstObjectPosition.x > transformedBox.Center.x + transformedBox.Extents.x) {
						_firstObjectPosition.x = transformedBox.Center.x + transformedBox.Extents.x + firstObjectBox.Extents.x;
					}
				}

				if (abs(_firstObjectPosition.x - transformedBox.Center.x) <= transformedBox.Extents.x) {
					if (_firstObjectVerticalVelocity < 0) {
						const float targetY = transformedBox.Center.y + transformedBox.Extents.y;
						if (_firstObjectPosition.y - targetY < 0.1f) {
							_firstObjectPosition.y = targetY;
							_firstObjectVerticalVelocity = 0.0f;
						}
						_isOnSurface = true;
					}
				}
			}
		}
	}

	for (size_t i = 2; i < _WorldMatrices.size(); ++i) {
		if (i < objectPositions.size() + 2) {
			const XMMATRIX translationMatrix = XMMatrixTranslation(objectPositions[i - 2].x, objectPositions[i - 2].y, objectPositions[i - 2].z);
			const XMMATRIX rotationMatrix = XMMatrixRotationY(t);

			if (spinnning == true) {
				_WorldMatrices[i] = translationMatrix * rotationMatrix;
			}
			else {
				_WorldMatrices[i] = translationMatrix;
			}
		}
	}

	if (_WorldMatrices.size() > 2) {
		const XMMATRIX translationMatrix = XMMatrixTranslation(objectPositions[2 - 2].x + (-10.0f), objectPositions[2 - 2].y + (-5.0f), objectPositions[2 - 2].z);
		_WorldMatrices[2] = translationMatrix * _WorldMatrices[2];
	}

	_gravity = -9.8f;

	updateViewMatrix();
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void D3DFramework::render() {
	// Calculate delta time
	calculateDeltaTime();

	//Process inputs?
	//processMouseInput(_deltaTime);
	// Update the world matrix for the moving object
	updateWorldMatrix(_deltaTime);

	// Update the light direction based on the current mode
	updateLightDirection();

	//When the sahders are set properly i will be able to fix these.
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, DirectX::Colors::MidnightBlue);
	_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// Set primitive topology
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_pImmediateContext->OMSetDepthStencilState(_skyboxDepthStencilState.p, 1);
	_pImmediateContext->RSSetState(_skyboxRasterizerState.p);
	const auto stride = static_cast<UINT>(sizeof(SimpleVertex));
	const auto offset = static_cast<UINT>(0);
	_pImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffers[1].p, &stride, &offset);
	_pImmediateContext->PSSetShaderResources(1, 1, &_skyboxTextureRV.p);
	// Set index buffer
	_pImmediateContext->IASetIndexBuffer(_indexBuffers[1], DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->VSSetShader(_skyboxVertexShader.p, nullptr, 0);
	_pImmediateContext->PSSetShader(_skyboxPixelShader.p, nullptr, 0);
	_pImmediateContext->IASetInputLayout(_skyboxVertexLayout.p);

	_pImmediateContext->DrawIndexed(static_cast<UINT>(_models[1].indices.size()), 0, 0);

	_pImmediateContext->OMSetDepthStencilState(_objectsDepthStencilState.p, 1);
	_pImmediateContext->RSSetState(_objectsRasterizerState.p);

	//Enviroment Map
	// Set environment map shaders
	 // Set environment map shaders
	const ConstantBuffer cb{ XMMatrixTranspose(_WorldMatrices[2]), XMMatrixTranspose(_View), XMMatrixTranspose(_Projection), _lightDirection};
	_pImmediateContext->UpdateSubresource(_pConstantBuffer.p, 0, nullptr, &cb, 0, 0);

	// Set shaders
	_pImmediateContext->VSSetShader(_envMapVertexShader.p, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer.p);
	_pImmediateContext->PSSetShader(_envMapPixelShader.p, nullptr, 0);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer.p);

	// Set vertex buffer
	_pImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffers[2].p, &stride, &offset);

	_pImmediateContext->PSSetShaderResources(0, 1, &_skyboxTextureRV.p);
	_pImmediateContext->PSSetSamplers(0, 1, &wood_Sampler.p);

	// Set index buffer
	_pImmediateContext->IASetIndexBuffer(_indexBuffers[2], DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw the model
	_pImmediateContext->DrawIndexed(static_cast<UINT>(_models[2].indices.size()), 0, 0);


	_pImmediateContext->OMSetDepthStencilState(_objectsDepthStencilState.p, 1);
	_pImmediateContext->RSSetState(_objectsRasterizerState.p);

	static std::vector<ID3D11ShaderResourceView*> textures; // <-- THIS

	if (textures.empty()) { // <-- THIS
		textures.reserve(textureNames.size()); // <-- THIS
		for (const auto& textureName : textureNames) { // <-- THIS
			ID3D11ShaderResourceView* texture = nullptr; // <-- THIS
			const std::wstring wTextureName(textureName.begin(), textureName.end()); // <-- THIS
			if (SUCCEEDED(_textureManager.loadDDSTexture(_pd3dDevice, wTextureName, &texture))) { // <-- THIS
				textures.push_back(texture); // <-- THIS
			} // <-- THIS
		} // <-- THIS
	} // <-- THIS

	// Render all models
	for (size_t i = 0; i < _models.size(); ++i) {
		// Update variables
		if (i == 1 || i == 2) {
			continue;
		}
		const ConstantBuffer cb{ XMMatrixTranspose(_WorldMatrices[i]), XMMatrixTranspose(_View), XMMatrixTranspose(_Projection), _lightDirection };
		_pImmediateContext->UpdateSubresource(_pConstantBuffer.p, 0, nullptr, &cb, 0, 0);

		// Set shaders
		_pImmediateContext->VSSetShader(_pVertexShader.p, nullptr, 0);
		_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer.p);
		_pImmediateContext->PSSetShader(_pPixelShader.p, nullptr, 0);
		_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer.p);

		// Set vertex buffer
		const auto stride = static_cast<UINT>(sizeof(SimpleVertex));
		const auto offset = static_cast<UINT>(0);
		_pImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffers[i].p, &stride, &offset);

		_pImmediateContext->PSSetShaderResources(0, 1, &textures[i]); // ITS HERE
		_pImmediateContext->PSSetSamplers(0, 1, &wood_Sampler.p);

		// Set index buffer
		_pImmediateContext->IASetIndexBuffer(_indexBuffers[i], DXGI_FORMAT_R16_UINT, 0);

		// Set primitive topology
		_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Draw the model
		_pImmediateContext->DrawIndexed(static_cast<UINT>(_models[i].indices.size()), 0, 0);
	}

	bool is_Window_Open = true;
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow(); // Show demo window! :)
	//Leaving this in.

	//For this i want to seperate it for simplicity and readability of the render function.
	ImGui::Begin("Control Panel", &is_Window_Open, ImGuiWindowFlags_MenuBar);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	// Calculate total number of polygons
	int totalPolygons = 0;
	for (const auto& model : _models) {
		totalPolygons += static_cast<int>(model.indices.size() / 3);
	}
	ImGui::Text("Total Polygons: %d", totalPolygons);


	if (ImGui::BeginTabBar("Control Tabs")) {
		if (ImGui::BeginTabItem("Camera Controls")) {
			ImGui::Checkbox("Track First Object", &_trackFirstObject);
			ImGui::Checkbox("Make them spin?", &spinnning);
			if (ImGui::InputFloat3("Light Direction", reinterpret_cast<float*>(&_lightDirection))) {
				if (!_isFlashlightMode) {
					_initialSunLightDirection = _lightDirection;
				}
			}
			if (ImGui::SliderFloat3("Camera Position", reinterpret_cast<float*>(&_cameraPosition), -100.0f, 100.0f)) {
				if (!_trackFirstObject) {
					updateViewMatrix();
				}
			}
			ImGui::SliderFloat("Zoom Factor", &_zoomFactor, 0.1f, 10.0f);
			ImGui::SliderFloat("Game Speed", &_timeFactor, 0.1f, 10.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Debug Info")) {
			ImGui::Text("Player Position: (%.2f, %.2f, %.2f)", _firstObjectPosition.x, _firstObjectPosition.y, _firstObjectPosition.z);
			ImGui::Text("Player Velocity: (%.2f, %.2f)", _firstObjectHorizontalVelocity, _firstObjectVerticalVelocity);
			//ImGui::Text("Player Look Direction: (%.2f, %.2f, %.2f)", _firstObjectLookDirection.x, _firstObjectLookDirection.y, _firstObjectLookDirection.z);
			ImGui::Text("Sprint Factor: %.2f", sprintFactor); // Add this line to show the current sprint factor

			ImGui::Separator();
			ImGui::Text("Object Positions:");
			for (size_t i = 0; i < objectPositions.size(); ++i) {
				ImGui::Text("Object %zu: (%.2f, %.2f, %.2f)", i, objectPositions[i].x, objectPositions[i].y, objectPositions[i].z);
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Object Locations Controls")) {
			ImGui::Text("Modify Object Positions:");
			for (size_t i = 0; i < objectPositions.size(); ++i) {
				ImGui::PushID(static_cast<int>(i));
				ImGui::DragFloat3("Position", reinterpret_cast<float*>(&objectPositions[i]), 0.1f);
				ImGui::PopID();
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present our back buffer to our front buffer
	_swapChain->Present(0, 0);
}