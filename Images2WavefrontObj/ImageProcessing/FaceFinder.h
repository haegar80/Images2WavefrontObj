#pragma once

#include "../WavefrontObject/Mesh.h"
#include "../WavefrontObject/Material.h"
#include <map>
#include <vector>

class FaceFinder
{
public:
    FaceFinder() = default;
    virtual ~FaceFinder() = default;
    FaceFinder(const FaceFinder&) = delete;
    FaceFinder& operator=(const FaceFinder&) = delete;

    void AddFaces(Mesh* p_mesh, std::vector<int>& p_faceIndices);

private:
    static constexpr bool Above = true;
    static constexpr bool Below = false;

    int m_averageYPixel{ 0 };
    std::map<int, int> m_mapTempToRealFaceIndices;
    std::vector<ObjVertexCoords> m_orderedVerticesWithMinXFirst;
    std::vector<int> m_orderedFaceIndicesWithMinXFirst;
    std::vector<std::vector<int>> m_handledConsecutiveFaceIndices;
    std::vector<std::vector<int>> m_handledNotConsecutiveFaceIndices;
    Material m_dummyMaterial{ "DummyMaterial" };

    void CalculateAverageYPixel(std::vector<ObjVertexCoords>& p_vertices);
    void OrderVerticesWithMinXFirst(std::vector<ObjVertexCoords>& p_vertices, std::vector<int>& p_faceIndices);
    bool GetAboveBelowEdgeInfo(ObjVertexCoords& p_vertex1, ObjVertexCoords& p_vertex2);
    void FindFaces(Mesh* p_mesh);
    bool FindNextNotConsecutiveFaceIndex(int p_faceIndex1, int p_faceIndex2, float p_slope, bool p_aboveBelowEdgeInfo, int p_handledVectorIndex);
    bool FindNextConsecutiveFaceIndex(int p_startIndex, float p_slope, bool p_aboveBelowEdgeInfo);
    bool CheckNextVertex(int p_vertexIndex, float p_slope, bool p_aboveBelowEdgeInfo);
    void HandleFindNextConsecutiveFaceIndexResult(bool p_nextFaceIndexFound, int p_nextFaceIndex, bool p_isConsecutive);
    bool CheckIfFaceIndexHandledAsNotConsecutive(int p_faceIndex, int& p_handledVectorIndex);
    void HandleFaceWithTotalTwoFaceIndices();
    void HandleFaceWithTwoFaceIndices();
    void HandleLastFace(int p_lastHandledFaceIndex);
    bool CheckAlreadyHandledConsecutiveFaceIndices(int p_faceIndex1, int p_faceIndex2);
    bool CheckAlreadyHandledConsecutiveFaceIndices(int p_faceIndex1, int p_faceIndex2, int p_faceIndex3);
    void AddFaces(Mesh* p_mesh);
};

