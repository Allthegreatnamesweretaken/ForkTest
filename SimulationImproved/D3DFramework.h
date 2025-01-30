#pragma once
// Simulation template, based on the Microsoft DX11 tutorial 04


#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <atlbase.h>
#include <fstream>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "directxcollision.h"
#include "ModelLoader.h"
#include "TextureManager.h"
#include "ObjModel.h"

using namespace DirectX;

#define COMPILE_CSO

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};


struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	DirectX::XMFLOAT3 LightDirection;
	XMVECTOR eyePos;
	

};

class D3DFramework final {

	HINSTANCE _hInst = nullptr;
	HWND _hWnd = nullptr;
	D3D_DRIVER_TYPE _driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL _featureLevel = D3D_FEATURE_LEVEL_11_1;
	CComPtr <ID3D11Device> _pd3dDevice;
	CComPtr <ID3D11Device1> _pd3dDevice1;
	CComPtr <ID3D11DeviceContext> _pImmediateContext;
	CComPtr <ID3D11DeviceContext1> _pImmediateContext1;
	CComPtr <IDXGISwapChain1> _swapChain;
	CComPtr <IDXGISwapChain1> _swapChain1;
	CComPtr <ID3D11RenderTargetView> _pRenderTargetView;
	CComPtr <ID3D11VertexShader> _pVertexShader;
	CComPtr <ID3D11PixelShader> _pPixelShader;
	CComPtr <ID3D11InputLayout> _pVertexLayout;
	//Skybox
	CComPtr <ID3D11VertexShader> _skyboxVertexShader;
	CComPtr <ID3D11PixelShader> _skyboxPixelShader;
	CComPtr <ID3D11InputLayout> _skyboxVertexLayout;
	CComPtr <ID3D11Buffer> _pVertexBuffer;
	CComPtr <ID3D11Buffer> _pIndexBuffer;
	CComPtr <ID3D11Buffer> _pConstantBuffer;
	CComPtr <ID3D11Buffer> _pConstantBuffer2;
	XMMATRIX _World = {};
	//XMMATRIX _World2 = {};
	XMMATRIX _View = {};
	XMMATRIX _Projection = {};
	CComPtr <ID3D11SamplerState> g_pSamplerLinear = nullptr;

	std::chrono::high_resolution_clock::time_point _previousTime;
	std::chrono::high_resolution_clock::time_point _currentTime;
	float _deltaTime = 0.0f;

	CComPtr <ID3D11ShaderResourceView> wood_TextureRV = nullptr;
	CComPtr <ID3D11ShaderResourceView> wood_TextureRV2 = nullptr;
	CComPtr <ID3D11SamplerState> wood_Sampler = nullptr;

	CComPtr  <ID3D11ShaderResourceView> _skyboxTextureRV = nullptr;

	CComPtr <ID3D11Texture2D> g_pDepthStencil = nullptr;
	CComPtr <ID3D11DepthStencilView> g_pDepthStencilView = nullptr;
	CComPtr <ID3D11DepthStencilState> _skyboxDepthStencilState = nullptr;
	CComPtr <ID3D11DepthStencilState> _objectsDepthStencilState = nullptr;

	// Add this line to store the rasterizer state
	CComPtr<ID3D11RasterizerState> _skyboxRasterizerState = nullptr;
	CComPtr<ID3D11RasterizerState> _objectsRasterizerState = nullptr;

	//Environment Variables
	CComPtr<ID3D11VertexShader> _envMapVertexShader;
	CComPtr<ID3D11PixelShader> _envMapPixelShader;
	CComPtr<ID3D11InputLayout> _envMapVertexLayout;
	CComPtr<ID3D11ShaderResourceView> _envMapTextureRV;
	CComPtr<ID3D11SamplerState> _envMapSampler;

	//toon Variables
	CComPtr<ID3D11VertexShader> _toonVertexShader;
	CComPtr<ID3D11PixelShader> _toonPixelShader;
	CComPtr<ID3D11InputLayout> _toonVertexLayout;



	//float _rotation = 0.0f;
	
	static std::unique_ptr<D3DFramework> _instance;
public:

	D3DFramework() = default;
	D3DFramework(D3DFramework&) = delete;
	D3DFramework(D3DFramework&&) = delete;
	D3DFramework& operator=(D3DFramework&) = delete;
	D3DFramework& operator=(D3DFramework&&) = delete;
	~D3DFramework();
	
	static D3DFramework& getInstance() { return *_instance; }

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HRESULT initWindow(HINSTANCE hInstance, int nCmdShow);
	static HRESULT compileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT initDevice();
	void calculateDeltaTime();
	void updateWorldMatrix(float deltaTime);
	void render();
	void updateViewMatrix();
	void reset();
	void toggleDaylightTorchMode();
	void updateLightDirection();
	void adjustTimeFactor(float adjustment);
	void zoomIn();
	void zoomOut();

	bool LoadConfig(const std::string& pathname);

	void lockCursor();
	void releaseCursor();

	//HRESULT loadDDSTexture(const std::wstring& fileName, ID3D11ShaderResourceView** textureRV) const;



	

private:
	std::vector<ObjModel> _models;
	std::vector<CComPtr<ID3D11Buffer>> _vertexBuffers;
	std::vector<CComPtr<ID3D11Buffer>> _indexBuffers;
	std::vector<XMMATRIX> _WorldMatrices;
	//XMFLOAT3 _firstObjectPosition;
	//float _firstObjectVerticalVelocity;

	ModelLoader _modelLoader;
	TextureManager _textureManager;

	std::vector<std::string> objectNames;
	std::vector<std::string> textureNames;
	std::vector<XMFLOAT3> objectPositions;
	bool _isFlashlightMode = false; // Add this line to track the light mode


	//Player Variables
	XMFLOAT3 _firstObjectPosition;
	float _firstObjectVerticalVelocity = 0.0f;
	float _firstObjectHorizontalVelocity = 0.0f;
	float _firstObjectHorizontalVelocityZ = 0.0f;
	float _gravity = 0.0f; // Gravity constant
	const float _jumpVelocity = 10.0f; // Initial jump velocity
	const float _groundLevel = 0.0f; // Ground level
	const float _moveSpeed = 5.0f; // Horizontal movement speed
	const float _decelerationFactor = 0.95f; // Deceleration factor
	float _zoomFactor = 1.0f; // Zoom factor for the camera
	bool _isOnSurface = false;
	bool _jumped = false;
	XMFLOAT3 _firstObjectLookDirection = XMFLOAT3(0.0f, 0.0f, 1.0f); // Initial look direction

	bool _decelerateLeft = false;
	bool _decelerateForward = false;
	bool _decelerateRight = false;
	bool _decelerateBackward = false;

	// Camera variables
	XMFLOAT3 _cameraPosition = XMFLOAT3(0.0f, 1.0f, -5.0f); // Initial camera
	bool _trackFirstObject = true; // Flag to indicate whether the camer
	//XMFLOAT3 _originalCameraPosition = XMFLOAT3(0.0f, 1.0f, -5.0f); // Store the original camera position
	//bool _lookAboveObject = false; // Flag to indicate whether the camera should look above the object
	//bool _lookBelowObject = false; // Flag to indicate whether the camera should look below the object
	// Camera control variables
	float _cameraYaw = 0.0f;
	float _cameraPitch = 0.0f;
	bool _mouseCaptured = false;
	POINT _lastMousePos;

	// Add these methods
	void processMouseInput(float deltaTime);
	void processKeyboardInput(float deltaTime);


	//GUI Buttons
	bool spinnning = false;

	//Extras
	float _timeFactor = 1.0f;
	bool _isDaylight = true;

	// Add this line to store the light direction
	//XMFLOAT3 _lightDirection = XMFLOAT3(-0.577f, 0.577f, -0.577f);
	XMFLOAT3 _lightDirection;// = XMFLOAT3(-0.577f, 0.577f, -0.577f);
	XMFLOAT3 _lightPosition; // Add this line
	XMFLOAT3 _initialSunLightDirection; // Add this line to store the initial sun-like light direction
	float _cutoffAngle = XM_PIDIV4; // 45 degrees cutoff angle for the spotlight
};


