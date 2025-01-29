#include "ModelLoader.h"

const bool ModelLoader::LoadOBJ(const std::string& pathname, ObjModel& model) const {
    std::ifstream fin(pathname);
    if (!fin) {
        std::cerr << "Error: Failed to open file " << pathname << " for reading" << std::endl;
        return false;
    }

    std::vector<XMFLOAT3> vertices;
    std::vector<XMFLOAT2> textures;
    std::vector<XMFLOAT3> normals;
    std::unordered_map<std::tuple<WORD, WORD, WORD>, WORD, VertexHash, VertexEqual> uniqueVertices;

    std::string tag;
    while (fin >> tag) {
        if (tag == "v") {
            XMFLOAT3 vertex;
            fin >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (tag == "vt") {
            XMFLOAT2 texture;
            fin >> texture.x >> texture.y;
            texture.y = 1.0f - texture.y;
            textures.push_back(texture);
        }
        else if (tag == "vn") {
            XMFLOAT3 normal;
            fin >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (tag == "f") {
            std::string face;
            std::getline(fin, face);
            std::istringstream iss(face);
            std::string vertex;
            std::vector<WORD> faceIndices;
            while (iss >> vertex) {
                std::replace(vertex.begin(), vertex.end(), '/', ' ');
                std::istringstream viss(vertex);
                WORD v, t, n;
                viss >> v >> t >> n;
                const auto vertexKey = std::make_tuple(v - 1, t - 1, n - 1);
                if (uniqueVertices.find(vertexKey) == uniqueVertices.end()) {
                    uniqueVertices[vertexKey] = static_cast<WORD>(model.vertices.size());
                    model.vertices.push_back(vertices[v - 1]);
                    model.textures.push_back(textures[t - 1]);
                    model.normals.push_back(normals[n - 1]);
                }
                faceIndices.push_back(uniqueVertices[vertexKey]);
            }
            for (size_t i = 1; i < faceIndices.size() - 1; ++i) {
                model.indices.push_back(faceIndices[0]);
                model.indices.push_back(faceIndices[i]);
                model.indices.push_back(faceIndices[i + 1]);
            }
        }
        else {
            std::string line;
            std::getline(fin, line);
        }
    }

    return true;
}
