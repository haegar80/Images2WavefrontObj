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

    std::vector<std::unique_ptr<Mesh>>& FindVerticesFromGradientImage(const QImage& p_gradientImage, int p_minimumGradient);

private:
    enum EVertexAlreadyAddedResult 
    {
        VertexNew = 0,
        SurfaceAvailable = 1,
        FaceAvailable = 2
    };

    struct SEdgePixels
    {
        int startX;
        int endX;
        int startY;
        int endY;
    };

    static constexpr int ImageBorderPixels = 2;
    static constexpr int MinimumNumberOfPixels = 100;
    static constexpr int MinimumNumberOfEdgePixels = 1;
    static constexpr int NumberOfCheckingNeighbouredPixels = 10;

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    Material m_dummyMaterial{"DummyMaterial"};
    int m_minimumGradient{ 0 };
    std::pair<int, int> m_lastFoundAlreadyAddedVertex;
    SEdgePixels m_lastFoundAlreadyAddedEdge;
    std::vector<SEdgePixels> m_addedEdges;

    void ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY);
    bool GetEdges(const QImage& p_gradientImage, int p_startX, int p_startY, std::vector<SEdgePixels>& p_edgePixelsVector);
    SEdgePixels GetEdgeX(const QImage& p_gradientImage, int p_startX, int p_startY);
    SEdgePixels GetEdgeY(const QImage& p_gradientImage, int p_startX, int p_startY);
    VertexFinder::SEdgePixels GetLowGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    VertexFinder::SEdgePixels GetLowGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    SEdgePixels GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    SEdgePixels GetHighGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY);
    bool HasPixelReachedOutOfBorder(int p_nextX, int p_nextY, int p_width, int p_height);
    int GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY);
    void AddVerticesAndFace(std::vector<SEdgePixels>& p_edgePixelsVector);
    int AddVertices(Mesh* p_mesh, SEdgePixels p_edgePixels, bool p_isStartVertexNew, bool p_isEndVertexNew);
    bool IsVertexAlreadyAdded(int p_pixelX, int p_pixelY);
    std::vector<VertexFinder::EVertexAlreadyAddedResult> AreVerticesAlreadyAdded(SEdgePixels p_facePixels);

    Mesh* GetMeshBasedOnVertex(int p_pixelX, int p_pixelY);
    Mesh* GetMeshBasedOnEdge(SEdgePixels p_edgePixels);
    bool IsEdgeFoundInMesh(const Mesh* p_mesh, SEdgePixels p_edgePixels);
    void MergeMeshesIfEdgeInDifferentMeshes(SEdgePixels p_edgePixels);
    void DeleteMesh(Mesh* p_mesh);

    int GetAlreadyAddedVertexIndex(Mesh* p_currentMesh, int p_pixelX, int p_pixelY);
};

