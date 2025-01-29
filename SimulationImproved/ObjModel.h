#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <directxcollision.h>

using namespace DirectX;

// Structure to hold the parsed data
struct ObjModel {
    std::vector<XMFLOAT3> vertices;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> textures;
    std::vector<WORD> indices;
    ID3D11ShaderResourceView* textureRV = nullptr;
    ID3D11ShaderResourceView* diffuse_map = nullptr;
    BoundingBox boundingBox;

    // Default constructor
    ObjModel() = default;

    // Copy constructor
    ObjModel(const ObjModel& other) = default;

    // Copy assignment operator
    ObjModel& operator=(const ObjModel& other) = default;
};

// Hash function for a tuple of three WORDs
struct VertexHash {
    std::size_t operator()(const std::tuple<WORD, WORD, WORD>& vertex) const {
        return std::hash<WORD>()(std::get<0>(vertex)) ^ std::hash<WORD>()(std::get<1>(vertex)) ^ std::hash<WORD>()(std::get<2>(vertex));
    }
};

// Equality function for a tuple of three WORDs
struct VertexEqual {
    bool operator()(const std::tuple<WORD, WORD, WORD>& lhs, const std::tuple<WORD, WORD, WORD>& rhs) const {
        return lhs == rhs;
    }
};
