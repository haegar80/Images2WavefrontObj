#pragma once

#include "../WavefrontObject/Mesh.h"
#include <memory>
#include <vector>
#include <map>

class VertexAdjuster
{
public:
    VertexAdjuster() = default;
    virtual ~VertexAdjuster() = default;
    VertexAdjuster(const VertexAdjuster&) = delete;
    VertexAdjuster& operator=(const VertexAdjuster&) = delete;
    VertexAdjuster(VertexAdjuster&&) = default;
    VertexAdjuster& operator=(VertexAdjuster&&) = default;

    void HandleVerticesGap(std::vector<std::unique_ptr<Mesh>>& p_meshes);

private:
    static constexpr int MaxiumNumberOfVerticesGapPixels = 10;

    std::map<Mesh*, ObjVertexCoords> m_firstVertices;
    std::map<Mesh*, ObjVertexCoords> m_lastVertices;
    std::map<Mesh*, Mesh*> m_meshesToMerge;
    std::vector<Mesh*> m_meshesToDelete;

    void CheckAndAdjustVertexGap(Mesh* p_currentCheckingMesh, ObjVertexCoords p_currentCheckingVertex);
    bool HasVertexGap(ObjVertexCoords p_alreadyCheckedVertex, ObjVertexCoords p_currentCheckingVertex);
};

