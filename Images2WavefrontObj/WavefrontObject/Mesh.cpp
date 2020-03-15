#include "Mesh.h"

Mesh::Mesh() :
    m_name(std::string("default"))
{
}

Mesh::Mesh(std::string p_name) :
    m_name(p_name)
{
}

Mesh::~Mesh()
{
    for (SubMesh* subMesh : m_submeshes)
    {
        delete subMesh;
    }
}

void Mesh::AddVertex(float p_x, float p_y, float p_z)
{
    ObjVertexCoords vertex;
    vertex.X = p_x;
    vertex.Y = p_y;
    vertex.Z = p_z;
    m_vertices.push_back(vertex);
}

void Mesh::AddNormal(float p_x, float p_y, float p_z)
{
    ObjVertexCoords normal;
    normal.X = p_x;
    normal.Y = p_y;
    normal.Z = p_z;
    m_normals.push_back(normal);
}

void Mesh::AddTexture(float p_u, float p_v)
{
    ObjTextureCoords texture;
    texture.U = p_u;
    texture.V = p_v;
    m_textures.push_back(texture);
}

void Mesh::AddFace(Material* p_material)
{
    FindAndUpdateSubmesh(p_material);
    m_submeshes.back()->AddNewFace();
}

void Mesh::MoveFace(int p_submeshVectorIndex, int p_faceVectorIndex, Material* p_material)
{
    SubMesh* oldSubmesh = m_submeshes.at(p_submeshVectorIndex);
    ObjFace oldFace = oldSubmesh->DeleteFace(p_faceVectorIndex);

    // For new material
    FindAndUpdateSubmesh(p_material);
    m_submeshes.back()->AddExistingFace(std::move(oldFace));
}

void Mesh::AddFaceIndices(unsigned short p_vertexIndex, unsigned short p_textureIndex, unsigned short p_normalIndex)
{
    m_submeshes.back()->AddFaceIndices(p_vertexIndex, p_textureIndex, p_normalIndex);
}

void Mesh::DeleteEmptySubmesh(int p_submeshVectorIndex)
{
    SubMesh* submesh = m_submeshes.at(p_submeshVectorIndex);
    if (0 == submesh->GetFaces().size())
    {
        auto itDelete = m_submeshes.begin() + p_submeshVectorIndex;
        (void)m_submeshes.erase(itDelete);
    }
}

void Mesh::Merge(Mesh* p_mesh)
{
    int numberOfVertices = GetVertices().size();
    std::vector<ObjVertexCoords> verticesToMerge = p_mesh->GetVertices();
    for (ObjVertexCoords vertexToMerge : verticesToMerge)
    {
        AddVertex(vertexToMerge.X, vertexToMerge.Y, vertexToMerge.Z);
    }

    std::vector<SubMesh*> submeshesToMerge = p_mesh->GetSubmeshes();
    for (SubMesh* submeshToMerge : submeshesToMerge)
    {
        Material* materialToMerge = submeshToMerge->GetMaterial();
        std::vector<ObjFace> facesToMerge = submeshToMerge->GetFaces();
        for (ObjFace faceToMerge : facesToMerge)
        {
            AddFace(materialToMerge);
            std::vector<ObjFaceIndices> faceIndicesToMerge = faceToMerge.Indices;
            for (ObjFaceIndices faceIndexToMerge : faceIndicesToMerge)
            {
                int faceIndexToStore = faceIndexToMerge.VertexIndex + numberOfVertices;
                AddFaceIndices(faceIndexToStore, faceIndexToStore);
            }
        }
    }
}

void Mesh::FindAndUpdateSubmesh(Material* p_material)
{
    if (p_material != m_lastUsedMaterial)
    {
        auto subMeshWithMatchingMaterial = std::find_if(m_submeshes.begin(), m_submeshes.end(), [p_material](SubMesh* submesh) {return p_material == submesh->GetMaterial(); });

        if (m_submeshes.end() == subMeshWithMatchingMaterial)
        {
            m_lastUsedMaterial = p_material;
            m_submeshes.push_back(new SubMesh(p_material));
        }
        else {
            SubMesh* submeshMoveToBack = *subMeshWithMatchingMaterial;
            m_submeshes.erase(subMeshWithMatchingMaterial);
            m_submeshes.push_back(submeshMoveToBack);
        }
    }
}