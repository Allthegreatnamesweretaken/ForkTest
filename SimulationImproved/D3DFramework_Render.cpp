// D3DFramework_Render.cpp
#include "D3DFramework.h"
#include <DirectXColors.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

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
	const ConstantBuffer cb{ XMMatrixTranspose(_WorldMatrices[2]), XMMatrixTranspose(_View), XMMatrixTranspose(_Projection), _lightDirection };
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