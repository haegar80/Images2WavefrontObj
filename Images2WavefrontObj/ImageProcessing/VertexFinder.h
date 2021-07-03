#pragma once

#include "FaceFinder.h"
#include "../WavefrontObject/Mesh.h"
#include <memory>
#include <vector>

class QImage;

class VertexFinder
{
public:
    VertexFinder() = default;
    virtual ~VertexFinder() = default;
    VertexFinder(const VertexFinder&) = delete;
    VertexFinder& operator=(const VertexFinder&) = delete;

    std::vector<std::unique_ptr<Mesh>>& FindVerticesFromGradientImage(const QImage& p_gradientImage, int p_minimumGradient);

private:
    enum EVertexAlreadyAddedResult 
    {
        VertexNew = 0,
        SurfaceAvailable = 1,
        FaceAvailable = 2
    };

    static constexpr int ImageBorderPixels = 2;
    static constexpr int MinimumNumberOfPixels = 10;
    static constexpr int MinimumNumberOfEdgePixels = 1;
    static constexpr int NumberOfCheckingNeighbouredPixels = 10;

    FaceFinder m_faceFinder;
    std::vector<std::unique_ptr<Mesh>> m_meshes;
    int m_minimumGradient{ 0 };
    std::pair<int, int> m_lastFoundAlreadyAddedVertex;
    SEdgePixels m_lastFoundAlreadyAddedEdge;
    std::vector<SEdgePixels> m_addedEdges;

    void ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY);
    bool GetEdges(const QImage& p_gradientImage, int p_startX, int p_startY, std::vector<SEdgePixels>& p_edgePixelsVector);
    SEdgePixels GetEdgeX(const QImage& p_gradientImage, int p_startX, int p_startY);
    SEdgePixels GetEdgeY(const QImage& p_gradientImage, int p_startX, int p_startY);
    SEdgePixels GetHighGradientEndX(const QImage& p_gradientImage, int p_startX, int p_nextX, int p_nextY);
    SEdgePixels GetHighGradientEndY(const QImage& p_gradientImage, int p_startY, int p_nextX, int p_nextY);
    bool HasPixelReachedOutOfBorder(int p_nextX, int p_nextY, int p_width, int p_height);
    int GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY);
    void AddVerticesAndFaces(std::vector<SEdgePixels>& p_edgePixelsVector);
    int AddVertices(Mesh* p_mesh, SEdgePixels p_edgePixels, bool p_isStartVertexNew, bool p_isEndVertexNew);
    bool IsVertexAlreadyAdded(int p_pixelX, int p_pixelY);
    std::vector<VertexFinder::EVertexAlreadyAddedResult> AreVerticesAlreadyAdded(Mesh* p_currentMesh, SEdgePixels p_facePixels);

    Mesh* GetMeshBasedOnVertex(int p_pixelX, int p_pixelY);
    void MergeMeshesIfEdgeInDifferentMeshes(SEdgePixels p_edgePixels);
    void MergeOtherMeshWithCurrentMeshIfDifferent(Mesh* p_currentMesh, int p_pixelX, int p_pixelY);
    void DeleteMesh(Mesh* p_mesh);

    int GetAlreadyAddedVertexIndex(Mesh* p_currentMesh, int p_pixelX, int p_pixelY);
};

