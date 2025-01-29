#pragma once
#pragma once
#include <string>
#include <d3d11.h>
#include <wrl/client.h>
#include "DDSTextureLoader.h"

class TextureManager final {
public:
    HRESULT loadDDSTexture(ID3D11Device* device, const std::wstring& fileName, ID3D11ShaderResourceView** textureRV) const;
};
