#include "VertexFinder.h"
#include <QImage>

VertexFinder::VertexFinder()
{
    m_mesh = std::make_unique<Mesh>();
}

Mesh* VertexFinder::FindVerticesFromGradientImage(const QImage& p_gradientImage)
{
    m_highGradientRangesX.clear();
    m_highGradientRangesY.clear();

    for (int xIndex = ImageBorderPixels; xIndex < (p_gradientImage.width() - ImageBorderPixels); xIndex += 2)
    {
        for (int yIndex = ImageBorderPixels; yIndex < (p_gradientImage.height() - ImageBorderPixels); yIndex += 2)
        {
            int imagePixelGray = GetGrayPixel(p_gradientImage, xIndex, yIndex);
            if (imagePixelGray > 0)
            {
                bool nextVertexFound = ProcessEdge(p_gradientImage, xIndex, yIndex);

                if(nextVertexFound)
                {
                    yIndex = m_nextCheckY;
                }
            }
        }
    }

    return m_mesh.get();
}

bool VertexFinder::ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    bool nextVertexFound = false;

    int xEndIndex = GetHighGradientEndX(p_gradientImage, (p_startX + 1), p_startY);
    int yEndIndex = 0;

    if ((xEndIndex - p_startX) >= MinimumNumberOfPixels)
    {
        yEndIndex = GetHighGradientEndY(p_gradientImage, p_startX, (p_startY + 1));
        if ((yEndIndex - p_startY) >= MinimumNumberOfPixels)
        {
            nextVertexFound = true;
        }
    }

    if (nextVertexFound)
    {
        AddVertices(p_startX, xEndIndex, p_startY, yEndIndex);
    }

    return nextVertexFound;
}

int VertexFinder::GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_pixelY)
{
    int xEndIndex = p_nextX - 1;

    for (int xIndex = p_nextX; xIndex < (p_gradientImage.width() - ImageBorderPixels); xIndex++)
    {
        int imagePixelGray = GetGrayPixel(p_gradientImage, xIndex, p_pixelY);
        if (imagePixelGray > MinimumGradient)
        {
            xEndIndex = xIndex;
            break;
        }
    }

    return xEndIndex;
}

int VertexFinder::GetHighGradientEndY(const QImage& p_gradientImage, int p_pixelX, int p_nextY)
{
    int yEndIndex = p_nextY - 1;

    for (int yIndex = p_nextY; yIndex < (p_gradientImage.height() - ImageBorderPixels); yIndex++)
    {
        int imagePixelGray = GetGrayPixel(p_gradientImage, p_pixelX, yIndex);
        if (imagePixelGray > MinimumGradient)
        {
            yEndIndex = yIndex;
            break;
        }
    }

    return yEndIndex;
}

int VertexFinder::GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY)
{
    QRgb imagePixelRGB = p_gradientImage.pixel(p_pixelX, p_pixelY);
    int imagePixelGray = qGray(imagePixelRGB);

    return imagePixelGray;
}

void VertexFinder::AddVertices(int p_startX, int p_endX, int p_startY, int p_endY)
{
    if (!IsVertexAlreadyAdded(p_startX, p_startY) && !IsVertexAlreadyAdded(p_endX, p_endY))
    {
        m_mesh->AddVertex(p_startX, p_startY, 0);
        m_mesh->AddVertex(p_endX, p_startY, 0);
        m_mesh->AddVertex(p_endX, p_endY, 0);
        m_mesh->AddVertex(p_startX, p_endY, 0);

        m_mesh->AddFace(&m_dummyMaterial);
        int faceIndex = m_mesh->GetVertices().size();
        for (int i = faceIndex; i < (faceIndex + 4); i++)
        {
            m_mesh->AddFaceIndices(i);
        }

        m_highGradientRangesX.insert(std::make_pair(p_startX, p_endX));
        m_highGradientRangesY.insert(std::make_pair(p_startY, p_endY));
        m_nextCheckY = p_endY;
    }
}

bool VertexFinder::IsVertexAlreadyAdded(int p_pixelX, int p_pixelY)
{
    bool vertexAlreadyAdded = false;
    bool xFound = false;
    bool yFound = false;
    int tempNextCheckY = 0;

    for (std::pair<int, int> xPair : m_highGradientRangesX)
    {
        if (p_pixelX >= xPair.first && p_pixelX <= xPair.second)
        {
            xFound = true;
            break;
        }
    }

    for (std::pair<int, int> yPair : m_highGradientRangesY)
    {
        if (p_pixelY > yPair.first && p_pixelY < yPair.second)
        {
            tempNextCheckY = yPair.second + 1;
            yFound = true;
            break;
        }
    }

    if (xFound && yFound)
    {
        m_nextCheckY = tempNextCheckY;
        vertexAlreadyAdded = true;
    }

    return vertexAlreadyAdded;
}