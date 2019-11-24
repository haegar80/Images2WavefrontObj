#include "VertexFinder.h"
#include "../WavefrontObject/Mesh.h"
#include <QImage>

VertexFinder::VertexFinder()
{
}

std::vector<std::unique_ptr<Mesh>>& VertexFinder::FindVerticesFromGradientImage(const QImage& p_gradientImage, int p_minimumGradient)
{
    m_minimumGradient = p_minimumGradient;
    m_highGradientRangesX.clear();
    m_highGradientRangesY.clear();

    for (int xIndex = ImageBorderPixels; xIndex < (p_gradientImage.width() - ImageBorderPixels); xIndex += 5)
    {
        for (int yIndex = ImageBorderPixels; yIndex < (p_gradientImage.height() - ImageBorderPixels); yIndex += 5)
        {
            int imagePixelGray = GetGrayPixel(p_gradientImage, xIndex, yIndex);
            if (imagePixelGray > m_minimumGradient)
            {
                bool nextVertexFound = ProcessEdge(p_gradientImage, xIndex, yIndex);

                if(nextVertexFound)
                {
                    yIndex = m_nextCheckY;
                }
            }
        }
    }

    return m_meshes;
}

bool VertexFinder::ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    bool nextVertexFound = false;
    
    SEdgePixels edgePixels;
    edgePixels.startX = p_startX;
    edgePixels.endX = 0;
    edgePixels.startY = p_startY;
    edgePixels.endY = 0;

    EEdgeFound edgeFound = GetEdge(p_gradientImage, edgePixels);

    if (NoEdgeFound != edgeFound)
    {
        bool findSpaceInXAxis = (XEdgeFound != edgeFound);
        int numberOfSpacePixelsFound = GetSpaceBetweenEdges(p_gradientImage, (p_startX + 1), (p_startY + 1), findSpaceInXAxis);
        if (numberOfSpacePixelsFound  >= MinimumNumberOfPixels)
        {
            AddVerticesAndFace(edgePixels);
        }
    }

    return nextVertexFound;
}

VertexFinder::EEdgeFound VertexFinder::GetEdge(const QImage& p_gradientImage, SEdgePixels& p_edgePixels)
{
    EEdgeFound edgeFound = NoEdgeFound;

    SEdgePixels edgePixels = p_edgePixels;
    SEdgePixels updatedEdgePixels = p_edgePixels;

    int numberOfEdgePixelsFoundTotalX = 0;
    int numberOfEdgePixelsFoundTotalY = 0;
    int numberOfEdgePixelsFoundX = 0;
    int numberOfEdgePixelsFoundY = 0;
    do
    {
        updatedEdgePixels = GetEdgeX(p_gradientImage, edgePixels);
        numberOfEdgePixelsFoundX = updatedEdgePixels.endX - edgePixels.startX;
        numberOfEdgePixelsFoundTotalX += numberOfEdgePixelsFoundX;
        edgePixels.startX = updatedEdgePixels.endX;
        edgePixels.startY = updatedEdgePixels.endY;
        updatedEdgePixels = GetEdgeY(p_gradientImage, edgePixels);
        numberOfEdgePixelsFoundY = updatedEdgePixels.endY - edgePixels.startY;
        numberOfEdgePixelsFoundTotalY += numberOfEdgePixelsFoundY;
        edgePixels.startX = updatedEdgePixels.endX;
        edgePixels.startY = updatedEdgePixels.endY;
    } while ((numberOfEdgePixelsFoundX > 0) && (numberOfEdgePixelsFoundY > 0));

    p_edgePixels.endX = updatedEdgePixels.endX;
    p_edgePixels.endY = updatedEdgePixels.endY;
    
    if ((numberOfEdgePixelsFoundTotalX > MinimumNumberOfPixels) || (numberOfEdgePixelsFoundTotalY > MinimumNumberOfPixels))
    {
        if (numberOfEdgePixelsFoundTotalX > numberOfEdgePixelsFoundTotalY)
        {
            edgeFound = XEdgeFound;
        }
        else
        {
            edgeFound = YEdgeFound;
        }
    }

    return edgeFound;
}

VertexFinder::SEdgePixels VertexFinder::GetEdgeX(const QImage& p_gradientImage, SEdgePixels p_edgePixels)
{
    p_edgePixels.startX++;
    SEdgePixels updatedEdgePixels = GetHighGradientEndX(p_gradientImage, p_edgePixels);
    updatedEdgePixels.startX--;
    updatedEdgePixels.endX--;

    return updatedEdgePixels;
}

VertexFinder::SEdgePixels VertexFinder::GetEdgeY(const QImage& p_gradientImage, SEdgePixels p_edgePixels)
{
    p_edgePixels.startY++;
    SEdgePixels updatedEdgePixels = GetHighGradientEndY(p_gradientImage, p_edgePixels);
    updatedEdgePixels.startY--;
    updatedEdgePixels.endY--;

    return updatedEdgePixels;
}

int VertexFinder::GetSpaceBetweenEdges(const QImage& p_gradientImage, int p_startX, int p_startY, bool p_axisX)
{
    int numberOfSpacePixelsFound = 0;
   
    if (p_axisX)
    {
        int xEndIndex = GetLowGradientEndX(p_gradientImage, (p_startX + 1), p_startY);
        numberOfSpacePixelsFound = xEndIndex - p_startX - 1;
    }
    else
    {
        int yEndIndex = GetLowGradientEndY(p_gradientImage, p_startX, (p_startY + 1));
        numberOfSpacePixelsFound = yEndIndex - p_startY - 1;
    }

    return numberOfSpacePixelsFound;
}

int VertexFinder::GetLowGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    int xEndIndexNew = p_nextX;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray < m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(p_nextX, p_nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            xEndIndexNew = GetLowGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY - 1));

            if (p_nextX == (xEndIndexNew - 1))
            {
                xEndIndexNew = GetLowGradientEndX(p_gradientImage, (p_nextX + 1), p_nextY);

                if (p_nextX == (xEndIndexNew - 1))
                {
                    xEndIndexNew = GetLowGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
                }
            }
        }
    }

    return xEndIndexNew;
}

int VertexFinder::GetLowGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    int yEndIndexNew = p_nextY;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray < m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(p_nextX, p_nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            yEndIndexNew = GetLowGradientEndY(p_gradientImage, (p_nextX - 1), (p_nextY + 1));

            if (p_nextY == (yEndIndexNew - 1))
            {
                yEndIndexNew = GetLowGradientEndY(p_gradientImage, p_nextX, (p_nextY + 1));

                if (p_nextY == (yEndIndexNew - 1))
                {
                    yEndIndexNew = GetLowGradientEndY(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
                }
            }
        }
    }

    return yEndIndexNew;
}

VertexFinder::SEdgePixels VertexFinder::GetHighGradientEndX(const QImage& p_gradientImage, SEdgePixels p_edgePixels)
{
    int nextX = p_edgePixels.startX;
    int nextY = p_edgePixels.startY;

    p_edgePixels.endX = nextX;
    p_edgePixels.endY = nextY;

    SEdgePixels updatedEdgePixels = p_edgePixels;

    int imagePixelGray = GetGrayPixel(p_gradientImage, nextX, nextY);
    if (imagePixelGray >= m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(nextX, nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            p_edgePixels.startX++;
            p_edgePixels.startY--;
            updatedEdgePixels = GetHighGradientEndX(p_gradientImage, p_edgePixels);

            if (nextX == (updatedEdgePixels.endX - 1))
            {
                p_edgePixels.startY++;
                updatedEdgePixels = GetHighGradientEndX(p_gradientImage, p_edgePixels);

                if (nextX == (updatedEdgePixels.endX - 1))
                {
                    p_edgePixels.startY++;
                    updatedEdgePixels = GetHighGradientEndX(p_gradientImage, p_edgePixels);
                }
            }
        }
    }

    return updatedEdgePixels;
}

VertexFinder::SEdgePixels VertexFinder::GetHighGradientEndY(const QImage& p_gradientImage, SEdgePixels p_edgePixels)
{
    int nextX = p_edgePixels.startX;
    int nextY = p_edgePixels.startY;

    p_edgePixels.endX = nextX;
    p_edgePixels.endY = nextY;

    SEdgePixels updatedEdgePixels = p_edgePixels;

    int imagePixelGray = GetGrayPixel(p_gradientImage, nextX, nextY);
    if (imagePixelGray >= m_minimumGradient)
    {
        bool hasPixelReachedOutOfBorder = HasPixelReachedOutOfBorder(nextX, nextY, p_gradientImage.width(), p_gradientImage.height());

        if (!hasPixelReachedOutOfBorder)
        {
            p_edgePixels.startX--;
            p_edgePixels.startY++;
            updatedEdgePixels = GetHighGradientEndY(p_gradientImage, p_edgePixels);

            if (nextY == (updatedEdgePixels.endY - 1))
            {
                p_edgePixels.startX++;
                updatedEdgePixels = GetHighGradientEndY(p_gradientImage, p_edgePixels);

                if (nextY == (updatedEdgePixels.endY - 1))
                {
                    p_edgePixels.startX;
                    updatedEdgePixels = GetHighGradientEndY(p_gradientImage, p_edgePixels);
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

void VertexFinder::AddVerticesAndFace(SEdgePixels p_edgePixels)
{
    std::vector<VertexFinder::EVertexAlreadyAddedResult> verticesAlreadyAddedResult = AreVerticesAlreadyAdded(p_edgePixels);
    VertexFinder::EVertexAlreadyAddedResult vertexAlreadyAddedResultStart = verticesAlreadyAddedResult.at(0);
    VertexFinder::EVertexAlreadyAddedResult vertexAlreadyAddedResultEnd = verticesAlreadyAddedResult.at(1);

    bool isStartVertexNew = true;
    bool isEndVertexNew = true;
    int faceIndices[4];
    Mesh* currentMesh = nullptr;

    if ((VertexFinder::FaceAvailable != vertexAlreadyAddedResultStart) && (VertexFinder::FaceAvailable != vertexAlreadyAddedResultEnd))
    {
        currentMesh = GetCurrentMesh(vertexAlreadyAddedResultStart, vertexAlreadyAddedResultEnd);
        int numberOfExistingVertices = currentMesh->GetVertices().size();

        if (VertexFinder::SurfaceAvailable == vertexAlreadyAddedResultStart)
        {
            isStartVertexNew = false;
            SAlreadyAddedVertexData alreadyAddedVertexData = GetInfoFromLastCheckedVertex();
            faceIndices[FaceIndexStartPixels] = alreadyAddedVertexData.faceIndex;
        }

        if (VertexFinder::SurfaceAvailable == vertexAlreadyAddedResultEnd)
        {
            isEndVertexNew = false;
            SAlreadyAddedVertexData alreadyAddedVertexData = GetInfoFromLastCheckedVertex();
            faceIndices[FaceIndexEndPixels] = alreadyAddedVertexData.faceIndex;
        }
       
        int numberOfNewVertices = AddVertices(currentMesh, p_edgePixels, isStartVertexNew, isEndVertexNew);
        
        currentMesh->AddFace(&m_dummyMaterial);

        int startVertexIndexOffset = 0;
        if (!isStartVertexNew)
        {
            startVertexIndexOffset = 1;
        }
        int endVertexIndexOffset = 0;

        for (int i = 0; i < numberOfNewVertices; i++)
        {
            if (!isEndVertexNew && (FaceIndexEndPixels == i))
            {
                endVertexIndexOffset++;
            }

            faceIndices[i + startVertexIndexOffset + endVertexIndexOffset] = numberOfExistingVertices + i + 1;
        }

        for (int i = 0; i < 4; i++)
        {
            currentMesh->AddFaceIndices(faceIndices[i], faceIndices[i]);
        }

        if (p_edgePixels.endY > m_nextCheckY)
        {
            m_nextCheckY = p_edgePixels.endY;
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
            p_mesh->AddVertex(p_edgePixels.startX, p_edgePixels.startY, 0);
        }
        numberOfVerticesAdded++;
        p_mesh->AddVertex(p_edgePixels.startX, p_edgePixels.endY, 0);

        if (p_isEndVertexNew)
        {
            numberOfVerticesAdded++;
            p_mesh->AddVertex(p_edgePixels.endX, p_edgePixels.endY, 0);
        }
        numberOfVerticesAdded++;
        p_mesh->AddVertex(p_edgePixels.endX, p_edgePixels.startY, 0);


        m_highGradientRangesX.insert(std::make_pair(p_edgePixels.startX, p_edgePixels.endX));
        m_highGradientRangesY.insert(std::make_pair(p_edgePixels.startY, p_edgePixels.endY));
    }

    return numberOfVerticesAdded;
}

bool VertexFinder::IsVertexAlreadyAdded(int p_pixelX, int p_pixelY, bool p_isNewVertexBeginningFace)
{
    bool vertexAlreadyAdded = false;
    bool xFound = false;
    bool yFound = false;
    int xFoundPairFirst = 0;
    int xFoundPairSecond = 0;
    int yFoundPairFirst = 0;
    int yFoundPairSecond = 0;
    int tempNextCheckY = 0;

    for (std::pair<int, int> xPair : m_highGradientRangesX)
    {
        if (p_pixelX >= xPair.first && p_pixelX <= xPair.second)
        {
            xFound = true;
            xFoundPairFirst = xPair.first;
            xFoundPairSecond = xPair.second;

            break;
        }
    }

    for (std::pair<int, int> yPair : m_highGradientRangesY)
    {
        if (p_pixelY >= yPair.first && p_pixelY <= yPair.second)
        {
            tempNextCheckY = yPair.second + 1;
            yFound = true;
            yFoundPairFirst = yPair.first;
            yFoundPairSecond = yPair.second;

            break;
        }
    }

    if (xFound && yFound)
    {   
        SEdgePixels edgePixels;
        edgePixels.startX = xFoundPairFirst;
        edgePixels.endX = xFoundPairSecond;
        edgePixels.startY = yFoundPairFirst;
        edgePixels.endY = yFoundPairSecond;
        HandleAlreadyAddedVertex(edgePixels, p_isNewVertexBeginningFace);

        m_nextCheckY = tempNextCheckY;

        vertexAlreadyAdded = true;
    }

    return vertexAlreadyAdded;
}

void VertexFinder::HandleAlreadyAddedVertex(SEdgePixels p_edgePixels, bool p_isNewVertexBeginningFace)
{
    MergeMeshesIfEdgesNotInSameMesh(p_edgePixels);

    int xFoundVertex = 0;
    int yFoundVertex = 0;

    if (p_isNewVertexBeginningFace)
    {
        xFoundVertex = p_edgePixels.startX;
        yFoundVertex = p_edgePixels.startY;
    }
    else
    {
        xFoundVertex = p_edgePixels.endX;
        yFoundVertex = p_edgePixels.endY;
    }

    m_lastFoundAlreadyAddedVertex = std::make_pair(xFoundVertex, yFoundVertex);
}

std::vector<VertexFinder::EVertexAlreadyAddedResult> VertexFinder::AreVerticesAlreadyAdded(SEdgePixels p_edgePixels)
{
    VertexFinder::EVertexAlreadyAddedResult resultStartVertex = VertexFinder::VertexNew;
    VertexFinder::EVertexAlreadyAddedResult resultEndVertex = VertexFinder::VertexNew;

    bool isStartVertexAlreadyAdded = IsVertexAlreadyAdded(p_edgePixels.startX, p_edgePixels.startY, true);
    bool isEndVertexAlreadyAdded = IsVertexAlreadyAdded(p_edgePixels.endX, p_edgePixels.endY, false);

    if (isStartVertexAlreadyAdded && isEndVertexAlreadyAdded)
    {
        resultStartVertex = VertexFinder::FaceAvailable;
        resultEndVertex = VertexFinder::FaceAvailable;
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

Mesh* VertexFinder::GetCurrentMesh(EVertexAlreadyAddedResult p_vertexAlreadyAddedResultStart, EVertexAlreadyAddedResult p_vertexAlreadyAddedResultEnd)
{
    Mesh* currentMesh = nullptr;

    if ((VertexFinder::VertexNew == p_vertexAlreadyAddedResultStart) && (VertexFinder::VertexNew == p_vertexAlreadyAddedResultEnd))
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
        currentMesh = mesh.get();
        m_meshes.push_back(std::move(mesh));
    }
    else if ((VertexFinder::SurfaceAvailable == p_vertexAlreadyAddedResultStart) || (VertexFinder::SurfaceAvailable == p_vertexAlreadyAddedResultEnd))
    {
        SAlreadyAddedVertexData alreadyAddedVertexData = GetInfoFromLastCheckedVertex();
        currentMesh = alreadyAddedVertexData.mesh;
    }

    if (nullptr == currentMesh)
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
        currentMesh = mesh.get();
        m_meshes.push_back(std::move(mesh));
    }

    return currentMesh;
}

Mesh* VertexFinder::GetMeshBasedOnEdgeX(int p_startX, int p_endX)
{
    Mesh* foundMesh = nullptr;
    bool meshFound = false;

    for (std::unique_ptr<Mesh>& mesh : m_meshes)
    {
        meshFound = IsEdgeFoundInMesh(mesh.get(), p_startX, p_endX, true);

        if (meshFound)
        {
            foundMesh = mesh.get();
            break;
        }
    }

    return foundMesh;
}

Mesh* VertexFinder::GetMeshBasedOnEdgeY(int p_startY, int p_endY)
{
    Mesh* foundMesh = nullptr;
    bool meshFound = false;

    for (std::unique_ptr<Mesh>& mesh : m_meshes)
    {
        meshFound = IsEdgeFoundInMesh(mesh.get(), p_startY, p_endY, false);

        if (meshFound)
        {
            foundMesh = mesh.get();
            break;
        }
    }

    return foundMesh;
}

bool VertexFinder::IsEdgeFoundInMesh(const Mesh* p_mesh, int p_startEdge, int p_endEdge, bool p_isXAxis)
{
    bool edgeFound = false;

    std::vector<ObjVertexCoords> vertices = p_mesh->GetVertices();
    bool startEdgeFound = false;
    bool endEdgeFound = false;

    std::vector<SubMesh*> submeshes = p_mesh->GetSubmeshes();
    for (SubMesh* submesh : submeshes)
    {
        std::vector<ObjFace> faces = submesh->GetFaces();
        for (ObjFace face : faces)
        {
            std::vector<ObjFaceIndices> faceIndices = face.Indices;
            for (ObjFaceIndices faceIndex : faceIndices)
            {
                int pixel = 0;
                if (p_isXAxis)
                {
                    pixel = vertices.at(faceIndex.VertexIndex - 1).X;
                }
                else
                {
                    pixel = vertices.at(faceIndex.VertexIndex - 1).Y;
                }

                if (p_startEdge == pixel)
                {
                    startEdgeFound = true;
                }
                if (p_endEdge == pixel)
                {
                    endEdgeFound = true;
                }
            }

            if (startEdgeFound && endEdgeFound)
            {
                edgeFound = true;
                break;
            }
        }

        if (startEdgeFound && endEdgeFound)
        {
            break;
        }
    }

    return edgeFound;
}

void VertexFinder::MergeMeshesIfEdgesNotInSameMesh(SEdgePixels p_edgePixels)
{
    Mesh* meshXAxis = GetMeshBasedOnEdgeX(p_edgePixels.startX, p_edgePixels.endX);
    Mesh* meshYAxis = GetMeshBasedOnEdgeY(p_edgePixels.startY, p_edgePixels.endY);

    if ((nullptr != meshXAxis) && (nullptr != meshYAxis) && (meshXAxis != meshYAxis))
    {
        MergeMeshes(meshXAxis, meshYAxis);
    }
}

void VertexFinder::MergeMeshes(Mesh* p_firstMesh, Mesh* p_secondMesh)
{
    int numberOfVerticesFirstMesh = p_firstMesh->GetVertices().size();
    std::vector<ObjVertexCoords> vertices = p_secondMesh->GetVertices();
    for (ObjVertexCoords vertex : vertices)
    {
        p_firstMesh->AddVertex(vertex.X, vertex.Y, vertex.Z);
    }

    std::vector<SubMesh*> submeshes = p_secondMesh->GetSubmeshes();
    for (SubMesh* submesh : submeshes)
    {
        Material* material = submesh->GetMaterial();
        std::vector<ObjFace> faces = submesh->GetFaces();
        for (ObjFace face : faces)
        {
            p_firstMesh->AddFace(material);
            std::vector<ObjFaceIndices> faceIndices = face.Indices;
            for (ObjFaceIndices faceIndex : faceIndices)
            {
                int faceIndexToStore = faceIndex.VertexIndex + numberOfVerticesFirstMesh;
                p_firstMesh->AddFaceIndices(faceIndexToStore, faceIndexToStore);
            }
        }
    }

    DeleteMesh(p_secondMesh);
}

void VertexFinder::DeleteMesh(Mesh* p_mesh)
{
    auto itDelete = m_meshes.begin();
    for (; itDelete < m_meshes.end(); itDelete++)
    {
        if (p_mesh == (*itDelete).get())
        {
            break;
        }
    }

    if (m_meshes.end() != itDelete)
    {
        (void)m_meshes.erase(itDelete);
    }
}

VertexFinder::SAlreadyAddedVertexData VertexFinder::GetInfoFromLastCheckedVertex()
{
    SAlreadyAddedVertexData alreadyAddedVertexData;
    alreadyAddedVertexData.mesh = nullptr;

    for (std::unique_ptr<Mesh>& mesh : m_meshes)
    {
        std::vector<ObjVertexCoords> vertices = mesh.get()->GetVertices();
        auto itX = std::find_if(vertices.begin(), vertices.end(), [this](ObjVertexCoords vertex) { return (m_lastFoundAlreadyAddedVertex.first == vertex.X); });
        auto itY = std::find_if(vertices.begin(), vertices.end(), [this](ObjVertexCoords vertex) { return (m_lastFoundAlreadyAddedVertex.second == vertex.Y); });
        if ((vertices.end() != itX) && (vertices.end() != itY))
        {
            alreadyAddedVertexData.mesh = mesh.get();
            alreadyAddedVertexData.faceIndex = itX - vertices.begin() + 1;
            break;
        }
    }

    return alreadyAddedVertexData;
}