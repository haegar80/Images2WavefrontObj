#pragma once

#include "../WavefrontObject/Mesh.h"
#include "../WavefrontObject/Material.h"
#include <memory>

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

    Mesh* FindVerticesFromGradientImage(const QImage& p_gradientImage);

private:
    static constexpr int ImageBorderPixels = 1;

    std::unique_ptr<Mesh> m_mesh;
    Material m_dummyMaterial{"default"};
    int m_NextCheckingX{ 0 };
    std::map<int, int> m_HighGradientRangesX{};
    std::map<int, int> m_HighGradientRangesY{};

    bool ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY);
    int GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_pixelY);
    int GetHighGradientEndY(const QImage& p_gradientImage, int p_pixelX, int p_nextY);
    int GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY);
    void AddVertices(int p_startX, int p_endX, int p_startY, int p_endY);
    bool IsVertexAlreadyAdded(int p_pixelX, int p_pixelY);
};

