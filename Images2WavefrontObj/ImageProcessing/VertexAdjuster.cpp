#include "VertexAdjuster.h"
#include "DepthCalculator.h"

void VertexAdjuster::HandleVerticesGap(std::vector<std::unique_ptr<Mesh>>& p_meshes)
{
    m_interestingVerticesForGapDetecting.clear();
    m_meshesToMerge.clear();
    m_meshesToDelete.clear();

    for (std::unique_ptr<Mesh>& mesh : p_meshes)
    {
        if (mesh)
        {
            FindVerticesForGapDetecting(mesh.get());
        }
    }

    for (auto it1 = m_interestingVerticesForGapDetecting.begin(); m_interestingVerticesForGapDetecting.end() != it1; ++it1)
    {
        for (auto it2 = std::next(it1); m_interestingVerticesForGapDetecting.end() != it2; ++it2)
        {
            CheckAndAdjustVertexGap((*it1).first, (*it2).first);
        }
    }

    for (auto itMesh = m_meshesToMerge.rbegin(); m_meshesToMerge.rend() != itMesh; ++itMesh)
    {
        itMesh->first->Merge(itMesh->second);
    }

    for (Mesh* meshToDelete : m_meshesToDelete)
    {
        auto itDelete = find_if(p_meshes.begin(), p_meshes.end(), [&, meshToDelete](std::unique_ptr<Mesh>& mesh) { return mesh.get() == meshToDelete; });

        if (p_meshes.end() != itDelete) {
            (void)std::move(*itDelete);
            p_meshes.erase(itDelete);
        }
    }
}

void VertexAdjuster::NormVertices(std::vector<std::unique_ptr<Mesh>>& p_meshes, int p_originalImageWidth, int p_p_originalImageHeight)
{
    for (std::unique_ptr<Mesh>& mesh : p_meshes)
    {
        if (mesh)
        {
            std::vector<ObjVertexCoords> vertices = mesh->GetVertices();
            for (int i = 0; i < vertices.size(); i++)
            {
                ObjVertexCoords& vertex = vertices.at(i);
                float normedX = vertex.X / p_originalImageWidth;
                float normedY = vertex.Y / p_p_originalImageHeight;
                float normedZ = vertex.Z / (DepthCalculator::GetZPixelFarest() - DepthCalculator::GetZPixelNearest()) * -1.0f;
                mesh->UpdateVertex(i, normedX, normedY, normedZ);
            }
        }
    }
}

void VertexAdjuster::FindVerticesForGapDetecting(Mesh* p_mesh)
{
    std::vector<ObjVertexCoords> vertices = p_mesh->GetVertices();

    int minX = vertices.at(0).X;
    int minY = vertices.at(0).Y;
    int maxX = vertices.at(0).X;
    int maxY = vertices.at(0).Y;
    int minXIndex = 0;
    int minYIndex = 0;
    int maxXIndex = 0;
    int maxYIndex = 0;

    for (int vertexIndex = 1; vertexIndex < vertices.size(); vertexIndex++)
    {
        ObjVertexCoords& vertex = vertices.at(vertexIndex);
        if (vertex.X < minX)
        {
            minX = vertex.X;
            minXIndex = vertexIndex;
        }
        else if (vertex.X > maxX) {
            maxX = vertex.X;
            maxXIndex = vertexIndex;
        }

        if (vertex.Y < minY)
        {
            minY = vertex.Y;
            minYIndex = vertexIndex;
        }
        else if (vertex.Y > maxY) {
            maxY = vertex.Y;
            maxYIndex = vertexIndex;
        }
    }

    AddVerticesForGapDetecting(p_mesh, minXIndex, minYIndex, maxXIndex, maxYIndex);
}

void VertexAdjuster::AddVerticesForGapDetecting(Mesh* p_mesh, int p_minXIndex, int p_minYIndex, int p_maxXIndex, int p_maxYIndex)
{
    std::vector<ObjVertexCoords> vertices = p_mesh->GetVertices();

    std::vector<ObjVertexCoords> interestingVertices;
    interestingVertices.push_back(vertices.at(p_minXIndex));

    if (p_minXIndex != p_minYIndex)
    {
        interestingVertices.push_back(vertices.at(p_minYIndex));
    }
    if (p_minYIndex != p_maxXIndex)
    {
        interestingVertices.push_back(vertices.at(p_maxXIndex));
    }
    if (p_maxXIndex != p_maxYIndex)
    {
        interestingVertices.push_back(vertices.at(p_maxYIndex));
    }
    m_interestingVerticesForGapDetecting.insert(std::make_pair(p_mesh, std::move(interestingVertices)));
}

void VertexAdjuster::CheckAndAdjustVertexGap(Mesh* p_mesh1, Mesh* p_mesh2)
{
    bool hasVertexGap = false;
    for (ObjVertexCoords vertex1 : m_interestingVerticesForGapDetecting.at(p_mesh1)) {
        for (ObjVertexCoords vertex2 : m_interestingVerticesForGapDetecting.at(p_mesh2)) {
            hasVertexGap = HasVertexGap(vertex1, vertex2);
            if (hasVertexGap)
            {
                m_meshesToMerge.insert(std::make_pair(p_mesh1, p_mesh2));
                m_meshesToDelete.push_back(p_mesh2);
                break;
            }
        }
        if (hasVertexGap)
        {
            break;
        }
    }
}

bool VertexAdjuster::HasVertexGap(ObjVertexCoords p_vertex1, ObjVertexCoords p_vertex2)
{
    bool hasVertexGap = false;

    int pixelXDifference = 0;
    int pixelYDifference = 0;

    if (p_vertex2.X >= p_vertex1.X)
    {
        pixelXDifference = p_vertex2.X - p_vertex1.X;
    }
    else
    {
        pixelXDifference = p_vertex1.X - p_vertex2.X;
    }

    if (p_vertex2.Y >= p_vertex1.Y)
    {
        pixelYDifference = p_vertex2.Y - p_vertex1.Y;
    }
    else
    {
        pixelYDifference = p_vertex1.Y - p_vertex2.Y;
    }

    if ((pixelXDifference < MaxiumNumberOfVerticesGapPixels) && (pixelYDifference < MaxiumNumberOfVerticesGapPixels))
    {
        hasVertexGap = true;
    }

    return hasVertexGap;
}