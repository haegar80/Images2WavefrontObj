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

void Mesh::AddVertex(int p_x, int p_y, int p_z)
{
    ObjVertexCoords vertex;
    vertex.X = p_x;
    vertex.Y = p_y;
    vertex.Z = p_z;
    m_vertices.push_back(vertex);
}

void Mesh::AddNormal(int p_x, int p_y, int p_z)
{
    ObjVertexCoords normal;
    normal.X = p_x;
    normal.Y = p_y;
    normal.Z = p_z;
    m_normals.push_back(normal);
}

void Mesh::AddTexture(int p_u, int p_v)
{
    ObjTextureCoords texture;
    texture.U = p_u;
    texture.V = p_v;
    m_textures.push_back(texture);
}

void Mesh::UpdateVertex(int p_vertexIndex, int p_x, int p_y, int p_z)
{
    ObjVertexCoords vertex;
    vertex.X = p_x;
    vertex.Y = p_y;
    vertex.Z = p_z;
    m_vertices[p_vertexIndex] = vertex;
}

void Mesh::AddFace(Material* p_material)
{
    FindAndUpdateSubmesh(p_material);
    m_submeshes.back()->AddNewFace();
}

void Mesh::MoveFace(int p_submeshVectorIndex, int p_faceVectorIndex, Material* p_material, bool p_deleteOldFace)
{
    SubMesh* oldSubmesh = m_submeshes.at(p_submeshVectorIndex);

    ObjFace oldFace = oldSubmesh->GetFaces().at(p_faceVectorIndex);
    if (p_deleteOldFace)
    {
        (void) oldSubmesh->DeleteFace(p_faceVectorIndex);
    }

    // For new material
    FindAndUpdateSubmesh(p_material);
    m_submeshes.back()->AddExistingFace(oldFace);
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

bool Mesh::IsEdgeFound(SEdgePixels p_edgePixels)
{
    bool edgeFound = false;

    std::vector<ObjVertexCoords> vertices = GetVertices();
    bool startEdgeFound = false;
    bool endEdgeFound = false;

    std::vector<SubMesh*> submeshes = GetSubmeshes();
    for (SubMesh* submesh : submeshes)
    {
        std::vector<ObjFace> faces = submesh->GetFaces();
        for (ObjFace face : faces)
        {
            std::vector<ObjFaceIndices> faceIndices = face.Indices;
            for (ObjFaceIndices faceIndex : faceIndices)
            {
                int pixelX = vertices.at(faceIndex.VertexIndex - 1).X;
                int pixelY = vertices.at(faceIndex.VertexIndex - 1).Y;

                if ((p_edgePixels.startX == pixelX) && (p_edgePixels.startY == pixelY))
                {
                    startEdgeFound = true;
                }
                else if ((p_edgePixels.endX == pixelX) && (p_edgePixels.endY == pixelY))
                {
                    endEdgeFound = true;
                }
            }

            if (startEdgeFound && endEdgeFound)
            {
                edgeFound = true;
                break;
            }
        }

        if (startEdgeFound && endEdgeFound)
        {
            edgeFound = true;
            break;
        }
    }

    return edgeFound;
}

bool Mesh::IsVertexFound(int p_pixelX, int p_pixelY)
{
    bool vertexFound = false;

    std::vector<ObjVertexCoords> vertices = GetVertices();
    auto itX = std::find_if(vertices.begin(), vertices.end(), [p_pixelX, p_pixelY](ObjVertexCoords vertex) { return (p_pixelX == vertex.X); });
    auto itY = std::find_if(vertices.begin(), vertices.end(), [p_pixelX, p_pixelY](ObjVertexCoords vertex) { return (p_pixelY == vertex.Y); });
    if ((vertices.end() != itX) && (vertices.end() != itY))
    {
        vertexFound = true;
    }

    return vertexFound;
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