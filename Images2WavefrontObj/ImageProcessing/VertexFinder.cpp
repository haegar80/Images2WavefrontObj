#include "VertexFinder.h"
#include "../WavefrontObject/Mesh.h"
#include <QImage>

VertexFinder::VertexFinder()
{
}

std::vector<std::unique_ptr<Mesh>>& VertexFinder::FindVerticesFromGradientImage(const QImage& p_gradientImage)
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

    return m_meshes;
}

bool VertexFinder::ProcessEdge(const QImage& p_gradientImage, int p_startX, int p_startY)
{
    bool nextVertexFound = false;

    int xEndIndex = GetHighGradientEndX(p_gradientImage, p_startX, p_startY);
    xEndIndex -= 1;

    int yEndIndex = 0;

    if ((xEndIndex - p_startX) >= MinimumNumberOfPixels)
    {
        yEndIndex = GetHighGradientEndY(p_gradientImage, p_startX, p_startY);
        yEndIndex -= 1;

        if ((yEndIndex - p_startY) >= MinimumNumberOfPixels)
        {
            nextVertexFound = true;
        }
    }

    if (nextVertexFound)
    {
        AddVerticesAndFace(p_startX, xEndIndex, p_startY, yEndIndex);
    }

    return nextVertexFound;
}

int VertexFinder::GetHighGradientEndX(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    int xEndIndex = p_nextX;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray > MinimumGradient)
    {
        int xEndIndexNew = xEndIndex + 1;
        if ((p_nextY - 1) >= ImageBorderPixels)
        {
            xEndIndexNew = GetHighGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY - 1));
        }

        if (xEndIndex == (xEndIndexNew - 1))
        {
            xEndIndexNew = GetHighGradientEndX(p_gradientImage, (p_nextX + 1), p_nextY);

            if (xEndIndex == (xEndIndexNew - 1))
            {
                xEndIndexNew = GetHighGradientEndX(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
            }
        }

        xEndIndex = xEndIndexNew;
    }

    return xEndIndex;
}

int VertexFinder::GetHighGradientEndY(const QImage& p_gradientImage, int p_nextX, int p_nextY)
{
    int yEndIndex = p_nextY;

    int imagePixelGray = GetGrayPixel(p_gradientImage, p_nextX, p_nextY);
    if (imagePixelGray > MinimumGradient)
    {
        int yEndIndexNew = yEndIndex + 1;
        if ((p_nextX - 1) >= ImageBorderPixels)
        {
            yEndIndexNew = GetHighGradientEndY(p_gradientImage, (p_nextX - 1), (p_nextY + 1));
        }

        if (yEndIndex == (yEndIndexNew - 1))
        {
            yEndIndexNew = GetHighGradientEndY(p_gradientImage, p_nextX, (p_nextY + 1));

            if (yEndIndex == (yEndIndexNew - 1))
            {
                yEndIndexNew = GetHighGradientEndY(p_gradientImage, (p_nextX + 1), (p_nextY + 1));
            }
        }

        yEndIndex = yEndIndexNew;
    }

    return yEndIndex;
}

int VertexFinder::GetGrayPixel(const QImage& p_gradientImage, int p_pixelX, int p_pixelY)
{
    QRgb imagePixelRGB = p_gradientImage.pixel(p_pixelX, p_pixelY);
    int imagePixelGray = qGray(imagePixelRGB);

    return imagePixelGray;
}

void VertexFinder::AddVerticesAndFace(int p_startX, int p_endX, int p_startY, int p_endY)
{
    std::vector<VertexFinder::EVertexAlreadyAddedResult> verticesAlreadyAddedResult = AreVerticesAlreadyAdded(p_startX, p_startY, p_endX, p_endY);
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
        int numberOfNewVertices = 0;

        if (VertexFinder::SurfaceAvailable == vertexAlreadyAddedResultStart)
        {
            isStartVertexNew = false;
            SAlreadyAddedVertexData alreadyAddedVertexData = GetInfoFromLastCheckedVertex();
            faceIndices[0] = alreadyAddedVertexData.vectorIndex;
        }

        if (VertexFinder::SurfaceAvailable == vertexAlreadyAddedResultEnd)
        {
            isEndVertexNew = false;
            SAlreadyAddedVertexData alreadyAddedVertexData = GetInfoFromLastCheckedVertex();
            faceIndices[3] = alreadyAddedVertexData.vectorIndex;
        }
       
        if (isStartVertexNew || isEndVertexNew)
        {
            if (isStartVertexNew)
            {
                numberOfNewVertices++;
                currentMesh->AddVertex(p_startX, p_startY, 0);
            }
            numberOfNewVertices += 2;
            currentMesh->AddVertex(p_startX, p_endY, 0);
            currentMesh->AddVertex(p_endX, p_startY, 0);
            if (isEndVertexNew)
            {
                numberOfNewVertices++;
                currentMesh->AddVertex(p_endX, p_endY, 0);
            }

            m_highGradientRangesX.insert(std::make_pair(p_startX, p_endX));
            m_highGradientRangesY.insert(std::make_pair(p_startY, p_endY));
        }
        
        currentMesh->AddFace(&m_dummyMaterial);

        int numberOfAddingFaceIndices = numberOfNewVertices;
        for (int i = 0; i < numberOfAddingFaceIndices; i++)
        {
            if ((!isStartVertexNew) && (0 == i))
            {
                numberOfAddingFaceIndices++;
                continue;
            }
            else if ((!isEndVertexNew) && (3 == i))
            {
                continue;;
            }

            faceIndices[i] = numberOfExistingVertices + i;
        }

        for (int i = 0; i < 4; i++)
        {
            currentMesh->AddFaceIndices(faceIndices[i] + 1);
        }

        m_nextCheckY = p_endY;
    }
}

bool VertexFinder::IsVertexAlreadyAdded(int p_pixelX, int p_pixelY, bool p_isStartFace)
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
        int xFoundVertex = 0;
        int yFoundVertex = 0;

        MergeMeshesIfNotInSameMesh(xFoundPairFirst, yFoundPairFirst, xFoundPairSecond, yFoundPairSecond);

        if (p_isStartFace)
        {
            xFoundVertex = xFoundPairFirst;
            yFoundVertex = yFoundPairFirst;
        } 
        else
        {
            xFoundVertex = xFoundPairSecond;
            yFoundVertex = yFoundPairSecond;
        }


        m_nextCheckY = tempNextCheckY;
        m_lastFoundAlreadyAddedVertex = std::make_pair(xFoundVertex, yFoundVertex);
  
        vertexAlreadyAdded = true;
    }

    return vertexAlreadyAdded;
}

std::vector<VertexFinder::EVertexAlreadyAddedResult> VertexFinder::AreVerticesAlreadyAdded(int p_startX, int p_startY, int p_endX, int p_endY)
{
    VertexFinder::EVertexAlreadyAddedResult resultStartVertex = VertexFinder::VertexNew;
    VertexFinder::EVertexAlreadyAddedResult resultEndVertex = VertexFinder::VertexNew;

    bool isStartVertexAlreadyAdded = IsVertexAlreadyAdded(p_startX, p_startY, true);
    bool isEndVertexAlreadyAdded = IsVertexAlreadyAdded(p_endX, p_endY, false);

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
        meshFound = IsEdgeFound(mesh.get(), p_startX, p_endX, true);

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
        meshFound = IsEdgeFound(mesh.get(), p_startY, p_endY, false);

        if (meshFound)
        {
            foundMesh = mesh.get();
            break;
        }
    }

    return foundMesh;
}

bool VertexFinder::IsEdgeFound(const Mesh* p_mesh, int p_startEdge, int p_endEdge, bool p_isXAxis)
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

void VertexFinder::MergeMeshesIfNotInSameMesh(int p_startX, int p_startY, int p_endX, int p_endY)
{
    Mesh* meshXAxis = GetMeshBasedOnEdgeX(p_startX, p_endX);
    Mesh* meshYAxis = GetMeshBasedOnEdgeY(p_startY, p_endY);

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
                p_firstMesh->AddFaceIndices(faceIndex.VertexIndex + numberOfVerticesFirstMesh);
            }
        }
    }

    auto itDelete = m_meshes.begin();
    for (; itDelete < m_meshes.end(); itDelete++)
    {
        if (p_secondMesh == (*itDelete).get())
        {
            break;
        }
    }

    if (m_meshes.end() != itDelete)
    {
        (void) m_meshes.erase(itDelete);
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
            alreadyAddedVertexData.vectorIndex = itX - vertices.begin();
            break;
        }
    }

    return alreadyAddedVertexData;
}