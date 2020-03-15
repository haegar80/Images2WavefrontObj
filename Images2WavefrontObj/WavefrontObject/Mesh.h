#pragma once

#include "SubMesh.h"
#include <string>
#include <vector>

struct ObjVertexCoords
{
	int X;
    int Y;
    int Z;
};

struct ObjTextureCoords
{
    int U;
    int V;
};

struct SEdgePixels
{
    int startX;
    int endX;
    int startY;
    int endY;
};

class Mesh
{
public:
    Mesh();
    Mesh(std::string p_name);
    virtual ~Mesh();

    void AddVertex(int p_x, int p_y, int p_z);
    void AddNormal(int p_x, int p_y, int p_z);
    void AddTexture(int p_u, int p_v);
    void UpdateVertex(int p_vertexIndex, int p_x, int p_y, int p_z);
    void AddFace(Material* p_material);
    void MoveFace(int p_submeshVectorIndex, int p_faceVectorIndex, Material* p_material);
    void AddFaceIndices(unsigned short p_vertexIndex, unsigned short p_textureIndex = 0, unsigned short p_normalIndex = 0);
    void DeleteEmptySubmesh(int p_submeshVectorIndex);
    void Merge(Mesh* p_mesh);
    bool IsEdgeFound(SEdgePixels p_edgePixels);
    bool IsVertexFound(int p_pixelX, int p_pixelY);

    std::string getName() const
    {
        return m_name;
    }

    std::vector<ObjVertexCoords> GetVertices() const {
        return m_vertices;
    }
    std::vector<ObjVertexCoords> GetNormals() const {
        return m_normals;
    }
    std::vector<ObjTextureCoords> GetTextures() const {
        return m_textures;
    }
    std::vector<SubMesh*> GetSubmeshes() const {
        return m_submeshes;
    }

private:
    std::string m_name;
    std::vector<ObjVertexCoords> m_tempVertices;
    std::vector<ObjVertexCoords> m_tempNormals;
    std::vector<ObjTextureCoords> m_tempTextures;

    std::vector<ObjVertexCoords> m_vertices;
    std::vector<ObjVertexCoords> m_normals;
    std::vector<ObjTextureCoords> m_textures;
    std::vector<SubMesh*> m_submeshes;
    unsigned int m_totalFaceIndices{ 1 }; // References to vertices/normals/textures start always with 1
    Material* m_lastUsedMaterial{ nullptr };

    void FindAndUpdateSubmesh(Material* p_material);
};

