// D3DFramework_Init.cpp
#include "D3DFramework.h"
#include <d3dcompiler.h>
#include <DirectXColors.h>
#include "Resource.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


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