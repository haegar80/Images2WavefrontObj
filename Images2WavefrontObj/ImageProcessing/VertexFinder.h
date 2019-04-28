#pragma once

#include "../WavefrontObject/Material.h"
#include <memory>
#include <vector>

class Mesh;
class QImage;

class VertexFinder
{
public:
    VertexFinder();
    virtual ~VertexFinder() = default;
    VertexFinder(const VertexFinder&) = delete;
    VertexFinder& operator=(const VertexFinder&) = delete;
    VertexFinder(VertexFinder&&) = default;
    VertexFinder& operator=(VertexFinder&&) = default;

    std::vector<std::unique_ptr<Mesh>>& FindVerticesFromGradientImage(const QImage& p_gradientImage);

private:
    enum EVertexAlreadyAddedResult 
    {
        VertexNew = 0,
        SurfaceAvailable = 1,
        FaceAvailable = 2
    };

    struct SAlreadyAddedVertexData
    {
        Mesh* mesh;
        int vectorIndex;
    };

    static constexpr int ImageBorderPixels = 1;
    static constexpr int MinimumGradient = 50;
    static constexpr int MinimumNumberOfPixels = 3;

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    Material m_dummyMaterial{"DummyMaterial"};
    int m_nextCheckY{ 0 };
    std::map<int, int> m_highGradientRangesX{};
    std::map<int, int> m_highGradientRangesY{};
    std::pair<int, int> m_lastFoundAlreadyAddedVertex{};

    bool ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY);
    int GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    int GetHighGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    int GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY);
    void AddVerticesAndFace(int p_startX, int p_endX, int p_startY, int p_endY);
    bool IsVertexAlreadyAdded(int p_pixelX, int p_pixelY, bool p_isStartFace);
    std::vector<VertexFinder::EVertexAlreadyAddedResult> AreVerticesAlreadyAdded(int p_startX, int p_startY, int p_endX, int p_endY);

    Mesh* GetCurrentMesh(EVertexAlreadyAddedResult p_vertexAlreadyAddedResultStart, EVertexAlreadyAddedResult p_vertexAlreadyAddedResultEnd);
    Mesh* GetMeshBasedOnEdgeX(int p_startX, int p_endX);
    Mesh* GetMeshBasedOnEdgeY(int p_startY, int p_endY);
    bool IsEdgeFound(const Mesh* p_mesh, int p_startEdge, int p_endEdge, bool p_isXAxis);
    void MergeMeshesIfNotInSameMesh(int p_startX, int p_startY, int p_endX, int p_endY);
    void MergeMeshes(Mesh* p_firstMesh, Mesh* p_secondMesh);
    SAlreadyAddedVertexData GetInfoFromLastCheckedVertex();
};

