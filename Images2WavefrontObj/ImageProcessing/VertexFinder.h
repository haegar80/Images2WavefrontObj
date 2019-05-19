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
        int faceIndex;
    };

    struct SEdgePixels
    {
        int startX;
        int endX;
        int startY;
        int endY;
    };

    static constexpr int ImageBorderPixels = 2;
    static constexpr int MinimumGradient = 50;
    static constexpr int MinimumNumberOfPixels = 3;
    static constexpr int FaceIndexStartPixels = 0;
    static constexpr int FaceIndexEndPixels = 2;

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    Material m_dummyMaterial{"DummyMaterial"};
    int m_nextCheckY{ 0 };
    std::map<int, int> m_highGradientRangesX{};
    std::map<int, int> m_highGradientRangesY{};
    std::pair<int, int> m_lastFoundAlreadyAddedVertex{};

    bool ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY);
    int GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    int GetHighGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    bool HasPixelReachedOutOfBorder(int p_nextX, int p_nextY, int p_width, int p_height);
    int GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY);
    void AddVerticesAndFace(SEdgePixels p_edgePixels);
    int AddVertices(Mesh* p_mesh, SEdgePixels p_edgePixels, bool p_isStartVertexNew, bool p_isEndVertexNew);
    bool IsVertexAlreadyAdded(int p_pixelX, int p_pixelY, bool p_isNewVertexBeginningFace);
    void HandleAlreadyAddedVertex(SEdgePixels p_edgePixels, bool p_isNewVertexBeginningFace);
    std::vector<VertexFinder::EVertexAlreadyAddedResult> AreVerticesAlreadyAdded(SEdgePixels p_facePixels);

    Mesh* GetCurrentMesh(EVertexAlreadyAddedResult p_vertexAlreadyAddedResultStart, EVertexAlreadyAddedResult p_vertexAlreadyAddedResultEnd);
    Mesh* GetMeshBasedOnEdgeX(int p_startX, int p_endX);
    Mesh* GetMeshBasedOnEdgeY(int p_startY, int p_endY);
    bool IsEdgeFoundInMesh(const Mesh* p_mesh, int p_startEdge, int p_endEdge, bool p_isXAxis);
    void MergeMeshesIfEdgesNotInSameMesh(SEdgePixels p_edgePixels);
    void MergeMeshes(Mesh* p_firstMesh, Mesh* p_secondMesh);
    void DeleteMesh(Mesh* p_mesh);
    SAlreadyAddedVertexData GetInfoFromLastCheckedVertex();
};

