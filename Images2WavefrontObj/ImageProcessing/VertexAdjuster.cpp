#include "VertexAdjuster.h"

void VertexAdjuster::HandleVerticesGap(std::vector<std::unique_ptr<Mesh>>& p_meshes)
{
    m_firstVertices.clear();
    m_lastVertices.clear();
    m_meshesToMerge.clear();
    m_meshesToDelete.clear();

    for (std::unique_ptr<Mesh>& mesh : p_meshes)
    {
        if (mesh)
        {
            std::vector<ObjVertexCoords> vertices = mesh.get()->GetVertices();
            ObjVertexCoords firstVertex = vertices.front();
            ObjVertexCoords lastVertex = vertices.back();

            CheckAndAdjustVertexGap(mesh.get(), firstVertex);
            CheckAndAdjustVertexGap(mesh.get(), lastVertex);

            m_firstVertices.insert(std::make_pair(mesh.get(), firstVertex));
            m_lastVertices.insert(std::make_pair(mesh.get(), lastVertex));
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

void VertexAdjuster::CheckAndAdjustVertexGap(Mesh* p_currentCheckingMesh, ObjVertexCoords p_currentCheckingVertex)
{
    bool hasVertexGap = false;
    for (std::pair<Mesh*, ObjVertexCoords> firstVertex : m_firstVertices)
    {
        hasVertexGap = HasVertexGap(firstVertex.second, p_currentCheckingVertex);
        if (hasVertexGap)
        {
            m_meshesToMerge.insert(std::make_pair(firstVertex.first, p_currentCheckingMesh));
            m_meshesToDelete.push_back(p_currentCheckingMesh);
            break;
        }
    }

    for (std::pair<Mesh*, ObjVertexCoords> lastVertex : m_lastVertices)
    {
        if (!hasVertexGap)
        {
            hasVertexGap = HasVertexGap(lastVertex.second, p_currentCheckingVertex);
            if (hasVertexGap)
            {
                m_meshesToMerge.insert(std::make_pair(lastVertex.first, p_currentCheckingMesh));
                m_meshesToDelete.push_back(p_currentCheckingMesh);
                break;
            }
        }
    }
}

bool VertexAdjuster::HasVertexGap(ObjVertexCoords p_alreadyCheckedVertex, ObjVertexCoords p_currentCheckingVertex)
{
    bool hasVertexGap = false;

    int pixelXDifference = 0;
    int pixelYDifference = 0;

    if (p_currentCheckingVertex.X >= p_alreadyCheckedVertex.X)
    {
        pixelXDifference = p_currentCheckingVertex.X - p_alreadyCheckedVertex.X;
    }
    else
    {
        pixelXDifference = p_alreadyCheckedVertex.X - p_currentCheckingVertex.X;
    }

    if (p_currentCheckingVertex.Y >= p_alreadyCheckedVertex.Y)
    {
        pixelYDifference = p_currentCheckingVertex.Y - p_alreadyCheckedVertex.Y;
    }
    else
    {
        pixelYDifference = p_alreadyCheckedVertex.Y - p_currentCheckingVertex.Y;
    }

    if ((pixelXDifference < MaxiumNumberOfVerticesGapPixels) && (pixelYDifference < MaxiumNumberOfVerticesGapPixels))
    {
        hasVertexGap = true;
    }

    return hasVertexGap;
}