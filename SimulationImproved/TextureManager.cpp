#include "TextureManager.h"

HRESULT TextureManager::loadDDSTexture(ID3D11Device* device, const std::wstring& fileName, ID3D11ShaderResourceView** textureRV) const {
    const HRESULT hr = DirectX::CreateDDSTextureFromFile(device, fileName.c_str(), nullptr, textureRV);
    if (((static_cast<HRESULT>(hr)) < 0)) {
        const std::wstring errorMsg = L"Failed to load DDS texture: " + fileName;
        MessageBox(nullptr, errorMsg.c_str(), reinterpret_cast<LPCTSTR>(L"Error DDS"), MB_OK);
        return hr;
    }
    return static_cast<HRESULT>(0L);
}
