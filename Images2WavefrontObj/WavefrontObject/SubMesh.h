#pragma once

#include "Material.h"
#include <vector>

struct ObjFaceIndices
{
	int VertexIndex;
	int NormalIndex;
	int TextureIndex;
};

struct ObjFace
{
	std::vector<ObjFaceIndices> Indices;
};

class SubMesh
{
public:
    SubMesh(Material* p_Material);
    ~SubMesh() = default;

    void AddNewFace();
    void AddExistingFace(ObjFace p_existingFace);
    void AddFaceIndices(int p_vertexIndex, int p_textureIndex = 0, int p_normalIndex = 0);
    ObjFace DeleteFace(int p_faceVectorIndex);

    Material* GetMaterial() const {
        return m_material;
    }

    std::vector<ObjFace> GetFaces() const {
        return m_faces;
    }

private:
    Material* m_material{ nullptr };
    std::vector<ObjFace> m_faces;

    void AddTriangledFace(ObjFace p_originalFace);
};

