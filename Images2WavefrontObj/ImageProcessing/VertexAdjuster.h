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
    void NormVertices(std::vector<std::unique_ptr<Mesh>>& p_meshes, int p_originalImageWidth, int p_p_originalImageHeight);

private:
    static constexpr int MaxiumNumberOfVerticesGapPixels = 20;

    std::map<Mesh*, std::vector<ObjVertexCoords>> m_interestingVerticesForGapDetecting;
    std::multimap<Mesh*, Mesh*> m_meshesToMerge;
    std::vector<Mesh*> m_meshesToDelete;

    void FindVerticesForGapDetecting(Mesh* p_mesh);
    void AddVerticesForGapDetecting(Mesh* p_mesh, int p_minXIndex, int p_minYIndex, int p_maxXIndex, int p_maxYIndex);
    void CheckAndAdjustVertexGap(Mesh* p_mesh1, Mesh* p_mesh2);
    bool IsAlreadyPlannedToDelete(Mesh* p_mesh);
    Mesh* FindMeshToMergeInto(Mesh* p_meshToDelete);
    bool HasVertexGap(ObjVertexCoords p_vertex1, ObjVertexCoords p_vertex2);
};

