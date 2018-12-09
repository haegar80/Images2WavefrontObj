#include "VertexFinder.h"
#include <QImage>

VertexFinder::VertexFinder()
{
    m_mesh = std::make_unique<Mesh>();
}

Mesh* VertexFinder::FindVerticesFromGradientImage(const QImage& p_gradientImage)
{
    m_HighGradientRangesX.clear();
    m_HighGradientRangesY.clear();

    for (int xIndex = ImageBorderPixels; xIndex < (p_gradientImage.width() - ImageBorderPixels); xIndex += 2)
    {
        for (int yIndex = ImageBorderPixels; yIndex < (p_gradientImage.height() - ImageBorderPixels); yIndex += 2)
        {
            int imagePixelGray = GetGrayPixel(p_gradientImage, xIndex, yIndex);
            if (imagePixelGray > 0)
            {
                bool vertexAdded = ProcessEdge(p_gradientImage, xIndex, yIndex);

                if(vertexAdded)
                {
                    xIndex = m_NextCheckingX;
                }
            }
        }

        m_NextCheckingX = xIndex;
    }

    return m_mesh.get();
}

bool VertexFinder::ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    bool nextVertexFound = false;

    int xEndIndex = GetHighGradientEndX(p_gradientImage, (p_startX + 1), p_startY);
    int yEndIndex = 0;

    if (xEndIndex != p_startX)
    {
        yEndIndex = GetHighGradientEndY(p_gradientImage, p_startX, (p_startY + 1));
        if (yEndIndex != p_startY)
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
        if (0 == imagePixelGray)
        {
            xEndIndex = xIndex - 1;
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
        if (0 == imagePixelGray)
        {
            yEndIndex = yIndex - 1;
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
    if (!IsVertexAlreadyAdded(p_startX, p_startY))
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

        m_HighGradientRangesX.insert(std::make_pair(p_startX, p_endX));
        m_HighGradientRangesY.insert(std::make_pair(p_startY, p_endY));
        m_NextCheckingX = p_endX;
    }
}

bool VertexFinder::IsVertexAlreadyAdded(int p_pixelX, int p_pixelY)
{
    bool vertexAlreadyAdded = false;
    bool xFound = false;
    bool yFound = false;
    int tempNextCheckingX = 0;

    for (std::pair<int, int> xPair : m_HighGradientRangesX)
    {
        if (p_pixelX >= xPair.first && p_pixelX <= xPair.second)
        {
            tempNextCheckingX = xPair.second + 1;
            xFound = true;
            break;
        }
    }

    for (std::pair<int, int> yPair : m_HighGradientRangesY)
    {
        if (p_pixelY > yPair.first && p_pixelY < yPair.second)
        {
            yFound = true;
            break;
        }
    }

    if (xFound && yFound)
    {
        m_NextCheckingX = tempNextCheckingX;
        vertexAlreadyAdded = true;
    }

    return vertexAlreadyAdded;
}