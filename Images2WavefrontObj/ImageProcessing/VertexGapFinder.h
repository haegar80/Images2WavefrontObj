#pragma once

#include "FaceFinder.h"
#include "../WavefrontObject/Mesh.h"
#include <memory>
#include <vector>
#include <map>

class VertexGapFinder
{
public:
    VertexGapFinder() = default;
    ~VertexGapFinder() = default;
    VertexGapFinder(const VertexGapFinder&) = delete;
    VertexGapFinder& operator=(const VertexGapFinder&) = delete;

    void FindVertexGaps(std::vector<std::unique_ptr<Mesh>>& p_meshes);

private:
    static constexpr int MaxiumNumberOfVerticesGapPixels = 20;

    FaceFinder m_faceFinder;
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

