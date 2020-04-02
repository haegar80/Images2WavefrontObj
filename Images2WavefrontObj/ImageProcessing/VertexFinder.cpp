#include "VertexFinder.h"
#include <QImage>

std::vector<std::unique_ptr<Mesh>>& VertexFinder::FindVerticesFromGradientImage(const QImage& p_gradientImage, int p_minimumGradient)
{
    m_minimumGradient = p_minimumGradient;
    m_addedEdges.clear();

    for (int xIndex = ImageBorderPixels; xIndex < (p_gradientImage.width() - ImageBorderPixels); xIndex += 5)
    {
        for (int yIndex = ImageBorderPixels; yIndex < (p_gradientImage.height() - ImageBorderPixels); yIndex += 5)
        {
            int imagePixelGray = GetGrayPixel(p_gradientImage, xIndex, yIndex);
            if (imagePixelGray > m_minimumGradient)
            {
                ProcessEdge(p_gradientImage, xIndex, yIndex);
            }
        }
    }

    return m_meshes;
}

void VertexFinder::ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    bool nextVertexFound = false;
    
    SEdgePixels edgePixels;
    edgePixels.startX = p_startX;
    edgePixels.endX = 0;
    edgePixels.startY = p_startY;
    edgePixels.endY = 0;

    std::vector<SEdgePixels> edgePixelsVector;
    bool edgeFound = GetEdges(p_gradientImage, p_startX, p_startY, edgePixelsVector);
    if (edgeFound)
    {
        AddVerticesAndFace(edgePixelsVector);
    }
}

bool VertexFinder::GetEdges(const QImage& p_gradientImage, int p_startX, int p_startY, std::vector<SEdgePixels>& p_edgePixelsVector)
{
    bool edgeFound = false;

    int startX = p_startX;
    int startY = p_startY;

    std::vector<SEdgePixels> edgePixelsVector;
    SEdgePixels updatedEdgePixels;

    int numberOfEdgePixelsFoundTotalX = 0;
    int numberOfEdgePixelsFoundTotalY = 0;
    int numberOfEdgePixelsFoundX = 0;
    int numberOfEdgePixelsFoundY = 0;
    do
    {
        updatedEdgePixels = GetEdgeX(p_gradientImage, startX, startY);
        numberOfEdgePixelsFoundX = updatedEdgePixels.endX - startX;
        numberOfEdgePixelsFoundTotalX += numberOfEdgePixelsFoundX;
        if (numberOfEdgePixelsFoundX > MinimumNumberOfEdgePixels)
        {
            SEdgePixels edgePixels;
            edgePixels.startX = startX;
            edgePixels.startY = startY;
            edgePixels.endX = updatedEdgePixels.endX;
            edgePixels.endY = updatedEdgePixels.endY;
            edgePixelsVector.push_back(edgePixels);
        }
        startX = updatedEdgePixels.endX;
        startY = updatedEdgePixels.endY;
        updatedEdgePixels = GetEdgeY(p_gradientImage, startX, startY);
        numberOfEdgePixelsFoundY = updatedEdgePixels.endY - startY;
        numberOfEdgePixelsFoundTotalY += numberOfEdgePixelsFoundY;
        if (numberOfEdgePixelsFoundY > MinimumNumberOfEdgePixels)
        {
            SEdgePixels edgePixels;
            edgePixels.startX = startX;
            edgePixels.startY = startY;
            edgePixels.endX = updatedEdgePixels.endX;
            edgePixels.endY = updatedEdgePixels.endY;
            edgePixelsVector.push_back(edgePixels);
        }
        startX = updatedEdgePixels.endX;
        startY = updatedEdgePixels.endY;
    } while ((numberOfEdgePixelsFoundX > 0) || (numberOfEdgePixelsFoundY > 0));

    if ((numberOfEdgePixelsFoundTotalX > MinimumNumberOfPixels) || (numberOfEdgePixelsFoundTotalY > MinimumNumberOfPixels))
    {
        for (SEdgePixels edgePixels : edgePixelsVector)
        {
            p_edgePixelsVector.push_back(edgePixels);
        }
        edgeFound = true;
    }

    return edgeFound;
}

SEdgePixels VertexFinder::GetEdgeX(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    SEdgePixels updatedEdgePixels = GetHighGradientEndX(p_gradientImage, (p_startX + 1), p_startY);
    updatedEdgePixels.startX = p_startX;
    updatedEdgePixels.startY = p_startY;
    updatedEdgePixels.endX--;

    return updatedEdgePixels;
}

SEdgePixels VertexFinder::GetEdgeY(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    SEdgePixels updatedEdgePixels = GetHighGradientEndY(p_gradientImage, p_startX, (p_startY + 1));
    updatedEdgePixels.startX = p_startX;
    updatedEdgePixels.startY = p_startY;
    updatedEdgePixels.endY--;

    return updatedEdgePixels;
}

SEdgePixels VertexFinder::GetLowGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    SEdgePixels updatedEdgePixels;
    updatedEdgePixels.endX = p_nextX;
    updatedEdgePixels.endY = p_nextY;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray < m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(p_nextX, p_nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            updatedEdgePixels = GetLowGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY - 1));

            if (p_nextX == (updatedEdgePixels.endX - 1))
            {
                updatedEdgePixels = GetLowGradientEndX(p_gradientImage, (p_nextX + 1), p_nextY);

                if (p_nextX == (updatedEdgePixels.endX - 1))
                {
                    updatedEdgePixels = GetLowGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
                }
            }
        }
    }

    return updatedEdgePixels;
}

SEdgePixels VertexFinder::GetLowGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    SEdgePixels updatedEdgePixels;
    updatedEdgePixels.endX = p_nextX;
    updatedEdgePixels.endY = p_nextY;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray < m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(p_nextX, p_nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            updatedEdgePixels = GetLowGradientEndY(p_gradientImage, (p_nextX - 1), (p_nextY + 1));

            if (p_nextY == (updatedEdgePixels.endY - 1))
            {
                updatedEdgePixels = GetLowGradientEndY(p_gradientImage, p_nextX, (p_nextY + 1));

                if (p_nextY == (updatedEdgePixels.endY - 1))
                {
                    updatedEdgePixels = GetLowGradientEndY(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
                }
            }
        }
    }

    return updatedEdgePixels;
}

SEdgePixels VertexFinder::GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    SEdgePixels updatedEdgePixels;
    updatedEdgePixels.endX = p_nextX;
    updatedEdgePixels.endY = p_nextY;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray >= m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(p_nextX, p_nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            updatedEdgePixels = GetHighGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY - 1));

            if (p_nextX == (updatedEdgePixels.endX - 1))
            {
                updatedEdgePixels = GetHighGradientEndX(p_gradientImage, (p_nextX + 1), p_nextY);

                if (p_nextX == (updatedEdgePixels.endX - 1))
                {
                    updatedEdgePixels = GetHighGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
                }
            }
        }
    }

    return updatedEdgePixels;
}

SEdgePixels VertexFinder::GetHighGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    SEdgePixels updatedEdgePixels;
    updatedEdgePixels.endX = p_nextX;
    updatedEdgePixels.endY = p_nextY;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray >= m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(p_nextX, p_nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            updatedEdgePixels = GetHighGradientEndY(p_gradientImage, (p_nextX - 1), (p_nextY + 1));

            if (p_nextY == (updatedEdgePixels.endY - 1))
            {
                updatedEdgePixels = GetHighGradientEndY(p_gradientImage, p_nextX, (p_nextY + 1));

                if (p_nextY == (updatedEdgePixels.endY - 1))
                {
                    updatedEdgePixels = GetHighGradientEndY(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
                }
            }
        }
    }

    return updatedEdgePixels;
}

bool VertexFinder::HasPixelReachedOutOfBorder(int p_nextX, int p_nextY, int p_width, int p_height)
{
    bool hasPixelReachedOutOfBorder = false;

    if ((p_nextX < ImageBorderPixels) || (p_nextX > (p_width - ImageBorderPixels)))
    {
        hasPixelReachedOutOfBorder = true;
    }
    else if ((p_nextY < ImageBorderPixels) || (p_nextY > (p_height - ImageBorderPixels)))
    {
        hasPixelReachedOutOfBorder = true;
    }

    return hasPixelReachedOutOfBorder;
}

int VertexFinder::GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY)
{
    QRgb imagePixelRGB = p_gradientImage.pixel(p_pixelX, p_pixelY);
    int imagePixelGray = qGray(imagePixelRGB);

    return imagePixelGray;
}

void VertexFinder::AddVerticesAndFace(std::vector<SEdgePixels>& p_edgePixelsVector)
{
    bool faceAdded = false;
    bool faceStartVertexAdded = false;

    for (SEdgePixels edgePixels : p_edgePixelsVector)
    {
        std::vector<VertexFinder::EVertexAlreadyAddedResult> verticesAlreadyAddedResult = AreVerticesAlreadyAdded(edgePixels);
        VertexFinder::EVertexAlreadyAddedResult vertexAlreadyAddedResultStart = verticesAlreadyAddedResult.at(0);
        VertexFinder::EVertexAlreadyAddedResult vertexAlreadyAddedResultEnd = verticesAlreadyAddedResult.at(1);

        bool isStartVertexNew = true;
        bool isEndVertexNew = true;
        Mesh* currentMesh = nullptr;

        if ((VertexFinder::FaceAvailable != vertexAlreadyAddedResultStart) && (VertexFinder::FaceAvailable != vertexAlreadyAddedResultEnd))
        {
            int startEdgeFaceIndex = 0;
            int endEdgeFaceIndex = 0;

            if (VertexFinder::SurfaceAvailable == vertexAlreadyAddedResultStart)
            {
                isStartVertexNew = false;
                currentMesh = GetMeshBasedOnEdge(m_lastFoundAlreadyAddedEdge);
                startEdgeFaceIndex = GetAlreadyAddedVertexIndex(currentMesh, m_lastFoundAlreadyAddedVertex.first, m_lastFoundAlreadyAddedVertex.second);

                edgePixels.startX = m_lastFoundAlreadyAddedEdge.startX;
                edgePixels.startY = m_lastFoundAlreadyAddedEdge.startY;
            }

            if (VertexFinder::SurfaceAvailable == vertexAlreadyAddedResultEnd)
            {
                isEndVertexNew = false;
                currentMesh = GetMeshBasedOnEdge(m_lastFoundAlreadyAddedEdge);
                endEdgeFaceIndex = GetAlreadyAddedVertexIndex(currentMesh, m_lastFoundAlreadyAddedVertex.first, m_lastFoundAlreadyAddedVertex.second);

                edgePixels.endX = m_lastFoundAlreadyAddedEdge.endX;
                edgePixels.endY = m_lastFoundAlreadyAddedEdge.endY;
            }

            if (nullptr == currentMesh)
            {
                std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
                currentMesh = mesh.get();
                m_meshes.push_back(std::move(mesh));
            }

            int numberOfExistingVertices = currentMesh->GetVertices().size();
            int numberOfVerticesAdded = AddVertices(currentMesh, edgePixels, isStartVertexNew, isEndVertexNew);

            if (!faceAdded)
            {
                currentMesh->AddFace(&m_dummyMaterial);
                faceAdded = true;
            }
            
            if (isStartVertexNew)
            {
                startEdgeFaceIndex = numberOfExistingVertices + 1;
            }
            if (isEndVertexNew)
            {
                endEdgeFaceIndex = numberOfExistingVertices + numberOfVerticesAdded;
            }

            if (!faceStartVertexAdded)
            {
                currentMesh->AddFaceIndices(startEdgeFaceIndex);
                faceStartVertexAdded = true;
            }
            currentMesh->AddFaceIndices(endEdgeFaceIndex);
        }
    }
}

int VertexFinder::AddVertices(Mesh* p_mesh, SEdgePixels p_edgePixels, bool p_isStartVertexNew, bool p_isEndVertexNew)
{
    int numberOfVerticesAdded = 0;

    if (p_isStartVertexNew || p_isEndVertexNew)
    {
        if (p_isStartVertexNew)
        {
            numberOfVerticesAdded++;
            p_mesh->AddVertex(static_cast<float>(p_edgePixels.startX), static_cast<float>(p_edgePixels.startY), 0.0f);
        }

        if (p_isEndVertexNew)
        {
            numberOfVerticesAdded++;
            p_mesh->AddVertex(static_cast<float>(p_edgePixels.endX), static_cast<float>(p_edgePixels.endY), 0.0f);
        }

        m_addedEdges.push_back(p_edgePixels);
    }

    return numberOfVerticesAdded;
}

bool VertexFinder::IsVertexAlreadyAdded(int p_pixelX, int p_pixelY)
{
    bool vertexAlreadyAdded = false;

    for (SEdgePixels addedEdge : m_addedEdges)
    {
        int smallerX = addedEdge.startX;
        int greaterX = addedEdge.endX;
        if (addedEdge.endX < smallerX)
        {
            smallerX = addedEdge.endX;
            greaterX = addedEdge.startX;
        }
        int smallerY = addedEdge.startY;
        int greaterY = addedEdge.endY;
        if (addedEdge.endY < smallerY)
        {
            smallerY = addedEdge.endY;
            greaterY = addedEdge.startY;
        }

        if (p_pixelX >= smallerX && p_pixelX <= greaterX)
        {
            if (p_pixelY >= smallerY && p_pixelY <= greaterY)
            {
                //Todo: check if the point is on the line
                m_lastFoundAlreadyAddedEdge.startX = addedEdge.startX;
                m_lastFoundAlreadyAddedEdge.endX = addedEdge.endX;
                m_lastFoundAlreadyAddedEdge.startY = addedEdge.startY;
                m_lastFoundAlreadyAddedEdge.endY = addedEdge.endY;

                int xFoundVertex = 0;
                int yFoundVertex = 0;
                if ((p_pixelX - addedEdge.startX) < (addedEdge.endX - p_pixelX))
                {
                    xFoundVertex = addedEdge.startX;
                }
                else
                {
                    xFoundVertex = addedEdge.endX;
                }
                if ((p_pixelY - addedEdge.startY) < (addedEdge.endY - p_pixelY))
                {
                    yFoundVertex = addedEdge.startY;
                }
                else
                {
                    yFoundVertex = addedEdge.endY;
                }

                m_lastFoundAlreadyAddedVertex = std::make_pair(xFoundVertex, yFoundVertex);
                vertexAlreadyAdded = true;
                break;
            }
        }
    }

    return vertexAlreadyAdded;
}

std::vector<VertexFinder::EVertexAlreadyAddedResult> VertexFinder::AreVerticesAlreadyAdded(SEdgePixels p_edgePixels)
{
    VertexFinder::EVertexAlreadyAddedResult resultStartVertex = VertexFinder::VertexNew;
    VertexFinder::EVertexAlreadyAddedResult resultEndVertex = VertexFinder::VertexNew;

    bool isStartVertexAlreadyAdded = IsVertexAlreadyAdded(p_edgePixels.startX, p_edgePixels.startY);
    std::pair<int, int> lastFoundAlreadyAddedVertexCopy = m_lastFoundAlreadyAddedVertex;
    bool isEndVertexAlreadyAdded = IsVertexAlreadyAdded(p_edgePixels.endX, p_edgePixels.endY);

    if (isStartVertexAlreadyAdded && isEndVertexAlreadyAdded)
    {
        resultStartVertex = VertexFinder::FaceAvailable;
        resultEndVertex = VertexFinder::FaceAvailable;
        SEdgePixels edgePixelsToMerge;
        edgePixelsToMerge.startX = lastFoundAlreadyAddedVertexCopy.first;
        edgePixelsToMerge.startY = lastFoundAlreadyAddedVertexCopy.second;
        edgePixelsToMerge.endX = m_lastFoundAlreadyAddedVertex.first;
        edgePixelsToMerge.endY = m_lastFoundAlreadyAddedVertex.second;
        MergeMeshesIfEdgeInDifferentMeshes(edgePixelsToMerge);
    }
    else if (isStartVertexAlreadyAdded)
    {
        resultStartVertex = VertexFinder::SurfaceAvailable;
    }
    else if (isEndVertexAlreadyAdded)
    {
        resultEndVertex = VertexFinder::SurfaceAvailable;
    }

    std::vector<VertexFinder::EVertexAlreadyAddedResult> resultVertices;
    resultVertices.push_back(resultStartVertex);
    resultVertices.push_back(resultEndVertex);
  
    return resultVertices;
}

Mesh* VertexFinder::GetMeshBasedOnVertex(int p_pixelX, int p_pixelY)
{
    Mesh* foundMesh = nullptr;

    for (std::unique_ptr<Mesh>& mesh : m_meshes)
    {
        if (mesh)
        {
            bool isVertexFoundInMesh = mesh.get()->IsVertexFound(p_pixelX, p_pixelY);
            if (isVertexFoundInMesh)
            {
                foundMesh = mesh.get();
                break;
            }
        }
    }

    return foundMesh;
}

Mesh* VertexFinder::GetMeshBasedOnEdge(SEdgePixels p_edgePixels)
{
    Mesh* foundMesh = nullptr;
    bool meshFound = false;

    for (std::unique_ptr<Mesh>& mesh : m_meshes)
    {
        if (mesh)
        {
            meshFound = mesh.get()->IsEdgeFound(p_edgePixels);
        }

        if (meshFound)
        {
            foundMesh = mesh.get();
            break;
        }
    }

    return foundMesh;
}

void VertexFinder::MergeMeshesIfEdgeInDifferentMeshes(SEdgePixels p_edgePixels)
{
    Mesh* meshStartVertex = GetMeshBasedOnVertex(p_edgePixels.startX, p_edgePixels.startY);
    Mesh* meshEndVertex = GetMeshBasedOnVertex(p_edgePixels.endX, p_edgePixels.endY);

    if ((nullptr != meshStartVertex) && (nullptr != meshEndVertex) && (meshStartVertex != meshEndVertex))
    {
        meshStartVertex->Merge(meshEndVertex);
        DeleteMesh(meshEndVertex);
    }
}

void VertexFinder::DeleteMesh(Mesh* p_mesh)
{
    auto itDelete = find_if(m_meshes.begin(), m_meshes.end(), [&, p_mesh](std::unique_ptr<Mesh>& mesh) { return mesh.get() == p_mesh; });

    if (m_meshes.end() != itDelete) {
        (void)std::move(*itDelete);
        m_meshes.erase(itDelete);
    }
}

int VertexFinder::GetAlreadyAddedVertexIndex(Mesh* p_currentMesh, int p_pixelX, int p_pixelY)
{
    int alreadyAddedVertexIndex = 0;

    std::vector<ObjVertexCoords> vertices = p_currentMesh->GetVertices();
    auto itX = std::find_if(vertices.begin(), vertices.end(), [p_pixelX, p_pixelY](ObjVertexCoords vertex) { return (p_pixelX == vertex.X); });
    auto itY = std::find_if(vertices.begin(), vertices.end(), [p_pixelX, p_pixelY](ObjVertexCoords vertex) { return (p_pixelY == vertex.Y); });
    if ((vertices.end() != itX) && (vertices.end() != itY))
    {
        alreadyAddedVertexIndex = itX - vertices.begin() + 1;
    }

    return alreadyAddedVertexIndex;
}