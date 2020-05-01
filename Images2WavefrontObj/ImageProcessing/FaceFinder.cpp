#include "FaceFinder.h"

void FaceFinder::AddFaces(Mesh* p_mesh, std::vector<int>& p_faceIndices)
{
    m_mapTempToRealFaceIndices.clear();
    m_orderedVerticesWithMinXFirst.clear();
    m_orderedFaceIndicesWithMinXFirst.clear();
    m_handledConsecutiveFaceIndices.clear();
    m_handledNotConsecutiveFaceIndices.clear();

    std::vector<ObjVertexCoords> allVertices = p_mesh->GetVertices();
    std::vector<int> tempFaceIndicesToCheck;
    std::vector<int> realFaceIndicesToCheck;
    std::vector<ObjVertexCoords> verticesToCheck;

    std::vector<SubMesh*>& submeshes = p_mesh->GetSubmeshes();
    for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
    {
        std::vector<ObjFace>& faces = submeshes.at(submeshIndex)->GetFaces();
        for(int faceVectorIndex = 0; faceVectorIndex < faces.size(); faceVectorIndex++)
        {
            std::vector<ObjFaceIndices>& faceIndices = faces.at(faceVectorIndex).Indices;
            for (ObjFaceIndices& faceIndex : faceIndices)
            {
                auto foundfaceIndex = std::find(realFaceIndicesToCheck.begin(), realFaceIndicesToCheck.end(), faceIndex.VertexIndex);
                if (realFaceIndicesToCheck.end() == foundfaceIndex)
                {
                    verticesToCheck.push_back(allVertices.at(faceIndex.VertexIndex - 1));
                    tempFaceIndicesToCheck.push_back(verticesToCheck.size());
                    realFaceIndicesToCheck.push_back(faceIndex.VertexIndex);
                    m_mapTempToRealFaceIndices.insert(std::make_pair(static_cast<int>(verticesToCheck.size()), faceIndex.VertexIndex));
                }
            }
        }

        p_mesh->DeleteSubmesh(submeshes.at(submeshIndex));
    }

    for (int faceIndex : p_faceIndices)
    {
        auto foundfaceIndex = std::find(realFaceIndicesToCheck.begin(), realFaceIndicesToCheck.end(), faceIndex);
        if (realFaceIndicesToCheck.end() == foundfaceIndex)
        {
           verticesToCheck.push_back(allVertices.at(faceIndex - 1));
           tempFaceIndicesToCheck.push_back(verticesToCheck.size());
           realFaceIndicesToCheck.push_back(faceIndex);
           m_mapTempToRealFaceIndices.insert(std::make_pair(static_cast<int>(verticesToCheck.size()), faceIndex));
        }
    }

    CalculateAverageYPixel(verticesToCheck);
    OrderVerticesWithMinXFirst(verticesToCheck, tempFaceIndicesToCheck);
    FindFaces(p_mesh);
    AddFaces(p_mesh);
}

void FaceFinder::CalculateAverageYPixel(std::vector<ObjVertexCoords>& p_vertices)
{
    int sumY = 0;
    for (ObjVertexCoords& vertex : p_vertices)
    {
        sumY += vertex.Y;
    }
    m_averageYPixel = sumY / p_vertices.size();
}

void FaceFinder::OrderVerticesWithMinXFirst(std::vector<ObjVertexCoords>& p_vertices, std::vector<int>& p_faceIndices)
{
    float minX = p_vertices.at(0).X;
    int minXIndex = 0;

    for (int i = 1; i < p_vertices.size(); i++)
    {
        if (p_vertices.at(i).X < minX)
        {
            minX = p_vertices.at(i).X;
            minXIndex = i;
        }
    }

    if (minXIndex > 0)
    {
        for (int i = minXIndex; i < p_vertices.size(); i++)
        {
            m_orderedVerticesWithMinXFirst.push_back(p_vertices.at(i));
            m_orderedFaceIndicesWithMinXFirst.push_back(p_faceIndices.at(i));
        }
        for (int i = 0; i < minXIndex; i++)
        {
            m_orderedVerticesWithMinXFirst.push_back(p_vertices.at(i));
            m_orderedFaceIndicesWithMinXFirst.push_back(p_faceIndices.at(i));
        }
    }
    else if (0 == minXIndex)
    {
        for (int i = 0; i < p_vertices.size(); i++)
        {
            m_orderedVerticesWithMinXFirst.push_back(p_vertices.at(i));
            m_orderedFaceIndicesWithMinXFirst.push_back(p_faceIndices.at(i));
        }
    }
}

bool FaceFinder::GetAboveBelowEdgeInfo(ObjVertexCoords& p_vertex1, ObjVertexCoords& p_vertex2)
{
    bool aboveBelowEdgeInfo = Above;

    int middleY = (p_vertex1.Y + p_vertex2.Y) / 2;
    if(middleY < m_averageYPixel)
    {
        aboveBelowEdgeInfo = Below;
    }
    
    return aboveBelowEdgeInfo;
}

void FaceFinder::FindFaces(Mesh* p_mesh)
{
    if (m_orderedVerticesWithMinXFirst.size() == 2)
    {
        HandleFaceWithTotalTwoFaceIndices();
    }
    else {
        int currentIndex = 0;

        for (; currentIndex < (m_orderedVerticesWithMinXFirst.size() - 1); currentIndex++)
        {
            ObjVertexCoords& vertex1 = m_orderedVerticesWithMinXFirst.at(currentIndex);
            ObjVertexCoords& vertex2 = m_orderedVerticesWithMinXFirst.at(currentIndex + 1);

            float slope = (vertex2.Y - vertex1.Y) / (vertex2.X - vertex1.X);
            bool aboveBelowEdgeInfo = GetAboveBelowEdgeInfo(vertex1, vertex2);

            bool nextNotConsecutiveFaceIndexFound = false;

            int alreadHandledNotConsecutiveVectorIndex = 0;
            bool alreadyHandledAsNotConsecutive = CheckIfFaceIndexHandledAsNotConsecutive(currentIndex + 1, alreadHandledNotConsecutiveVectorIndex);
            if (alreadHandledNotConsecutiveVectorIndex)
            {
                nextNotConsecutiveFaceIndexFound = FindNextNotConsecutiveFaceIndex(currentIndex, currentIndex + 1, slope, aboveBelowEdgeInfo, alreadHandledNotConsecutiveVectorIndex);
            }

            if (!nextNotConsecutiveFaceIndexFound)
            {
                if ((currentIndex + 2) < m_orderedVerticesWithMinXFirst.size())
                {
                    int nextFaceIndex = 0;
                    bool isNextConsecutiveFaceIndexFound = FindNextConsecutiveFaceIndex((currentIndex + 2), slope, aboveBelowEdgeInfo);
                    if (isNextConsecutiveFaceIndexFound)
                    {
                        currentIndex++;
                    }
                }
            }
        }

        HandleFaceWithTwoFaceIndices();

        if (m_orderedVerticesWithMinXFirst.size() > 3)
        {
            HandleLastFace(currentIndex);
        }
    }
}

bool FaceFinder::FindNextNotConsecutiveFaceIndex(int p_faceIndex1, int p_faceIndex2, float p_slope, bool p_aboveBelowEdgeInfo, int p_handledVectorIndex)
{
    bool nextNotConsecutiveFaceIndexFound = false;
    int nextFoundFaceIndex = 0;

    std::vector<int> handledFaceIndices = m_handledNotConsecutiveFaceIndices.at(p_handledVectorIndex);
    for (auto itHandledFaceIndex = handledFaceIndices.rbegin(); handledFaceIndices.rend() != itHandledFaceIndex; ++itHandledFaceIndex)
    {
        int faceIndex = *itHandledFaceIndex - 1;
        if ((faceIndex == p_faceIndex1) || (faceIndex == p_faceIndex2))
        {
            continue;
        }

        // Face index and vertex index are always same
        bool vertexValid = CheckNextVertex(faceIndex, p_slope, p_aboveBelowEdgeInfo);
        if (vertexValid)
        {
            nextFoundFaceIndex = faceIndex;
            nextNotConsecutiveFaceIndexFound = true;
        }
    }

    std::vector<int> faceIndicesToAdd;
    faceIndicesToAdd.push_back(m_orderedFaceIndicesWithMinXFirst.at(p_faceIndex1));
    faceIndicesToAdd.push_back(m_orderedFaceIndicesWithMinXFirst.at(p_faceIndex2));
    if (nextNotConsecutiveFaceIndexFound)
    {
        faceIndicesToAdd.push_back(m_orderedFaceIndicesWithMinXFirst.at(nextFoundFaceIndex));
        m_handledNotConsecutiveFaceIndices.push_back(faceIndicesToAdd);
    }

    return nextNotConsecutiveFaceIndexFound;
}

bool FaceFinder::FindNextConsecutiveFaceIndex(int p_startIndex, float p_slope, bool p_aboveBelowEdgeInfo)
{
    bool nextFaceIndexFound = false;
    int nextFaceIndex = p_startIndex;

    for (int i = p_startIndex; i < m_orderedVerticesWithMinXFirst.size(); i++)
    {
        bool vertexValid = CheckNextVertex(i, p_slope, p_aboveBelowEdgeInfo);
        if (vertexValid)
        {
            nextFaceIndexFound = true;
            nextFaceIndex = i;
            break;
        }
    }
   
    bool nextConsecutiveFaceIndexFound = false;
    if (nextFaceIndexFound && (nextFaceIndex == p_startIndex))
    {
        nextConsecutiveFaceIndexFound = true;
    }

    HandleFindNextConsecutiveFaceIndexResult(nextFaceIndexFound, nextFaceIndex, nextConsecutiveFaceIndexFound);

    return nextConsecutiveFaceIndexFound;
}

bool FaceFinder::CheckNextVertex(int p_vertexIndex, float p_slope, bool p_aboveBelowEdgeInfo)
{
    bool faceIndexValid = false;

    ObjVertexCoords& vertex = m_orderedVerticesWithMinXFirst.at(p_vertexIndex);
    bool isAbove = (p_aboveBelowEdgeInfo == Above);
    bool isBelow = !isAbove;
    bool isPositiveSlope = (p_slope >= 0.0f);
    bool isNegativeSlope = !isPositiveSlope;

    int pixelYWithGivenSlope = static_cast<int>(p_slope * vertex.X);

    if (isAbove)
    {
        if (vertex.Y <= pixelYWithGivenSlope)
        {
            faceIndexValid = true;
        }
    }
    else
    {
        if (vertex.Y >= pixelYWithGivenSlope)
        {
            faceIndexValid = true;
        }
    }

    return faceIndexValid;
}

void FaceFinder::HandleFindNextConsecutiveFaceIndexResult(bool p_nextFaceIndexFound, int p_nextFaceIndex, bool p_isConsecutive)
{
    std::vector<int> handledFaceIndices;
    handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(p_nextFaceIndex - 2));
    handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(p_nextFaceIndex - 1));
    if (p_nextFaceIndexFound)
    {
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(p_nextFaceIndex));
        if (p_isConsecutive)
        {
            m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
        }
        else
        {
            m_handledNotConsecutiveFaceIndices.push_back(handledFaceIndices);
        }
    }
    else
    {
        m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
    }
}

bool FaceFinder::CheckIfFaceIndexHandledAsNotConsecutive(int p_faceIndex, int& p_handledVectorIndex)
{
    bool alreadyHandledAsNotConsecutive = false;
    int handledVectorIndex = 0;

    for(int i = 0; i < m_handledNotConsecutiveFaceIndices.size(); i++)
    {
        std::vector<int> handledFaceIndices = m_handledNotConsecutiveFaceIndices.at(i);
        auto foundFaceIndex = std::find_if(handledFaceIndices.begin(), handledFaceIndices.end(), [p_faceIndex](int& handledFaceIndex) { return handledFaceIndex == (p_faceIndex + 1); });
        if (handledFaceIndices.end() != foundFaceIndex)
        {
            alreadyHandledAsNotConsecutive = true;
            p_handledVectorIndex = i;
            break;
        }
     
        if (alreadyHandledAsNotConsecutive)
        {
            break;
        }
    }

    return alreadyHandledAsNotConsecutive;
}

void FaceFinder::HandleFaceWithTotalTwoFaceIndices()
{
    bool checkAddConsecutiveFaceIndices = CheckAlreadyHandledConsecutiveFaceIndices(m_orderedFaceIndicesWithMinXFirst.at(0), m_orderedFaceIndicesWithMinXFirst.at(1));
    if (checkAddConsecutiveFaceIndices)
    {
        std::vector<int> handledFaceIndices;
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(0));
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(1));
        m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
    }
}

void FaceFinder::HandleFaceWithTwoFaceIndices()
{
    std::vector<int> vectorIndicesWithTwoFaceIndices;
    std::vector<int> vectorIndicesWithMoreThanTwoFaceIndices;

    if (m_handledConsecutiveFaceIndices.size() > 1)
    {
        for (int i = 0; i < m_handledConsecutiveFaceIndices.size(); i++)
        {
            if (2 == m_handledConsecutiveFaceIndices.at(i).size())
            {
                vectorIndicesWithTwoFaceIndices.push_back(i);
            }
            else if(m_handledConsecutiveFaceIndices.at(i).size() > 2)
            {
                vectorIndicesWithMoreThanTwoFaceIndices.push_back(i);
            }
        }

        if (vectorIndicesWithTwoFaceIndices.size() > 0)
        {
            for (int i = 0; i < m_handledConsecutiveFaceIndices.size(); i++)
            {
                auto itFoundWithTwoFaceIndices = std::find(vectorIndicesWithTwoFaceIndices.begin(), vectorIndicesWithTwoFaceIndices.end(), i);
                if (vectorIndicesWithTwoFaceIndices.end() != itFoundWithTwoFaceIndices)
                {
                    bool vertexValid1 = false;
                    bool vertexValid2 = false;
                    int faceIndex1 = 0;
                    int faceIndex2 = 0;

                    for (int j = 0; j < m_handledConsecutiveFaceIndices.size(); j++)
                    {
                        auto itFoundWithMoreThanTwoFaceIndices = std::find(vectorIndicesWithMoreThanTwoFaceIndices.begin(), vectorIndicesWithMoreThanTwoFaceIndices.end(), j);
                        if (vectorIndicesWithMoreThanTwoFaceIndices.end() != itFoundWithMoreThanTwoFaceIndices)
                        {
                            int faceIndex1 = m_handledConsecutiveFaceIndices.at(*itFoundWithMoreThanTwoFaceIndices).at(0);
                            int faceIndex2 = m_handledConsecutiveFaceIndices.at(*itFoundWithMoreThanTwoFaceIndices).at(1);
                            ObjVertexCoords& vertex1 = m_orderedVerticesWithMinXFirst.at(faceIndex1 - 1);
                            ObjVertexCoords& vertex2 = m_orderedVerticesWithMinXFirst.at(faceIndex2 - 1);
                            float slope = (vertex2.Y - vertex1.Y) / (vertex2.X - vertex1.X);
                            bool aboveBelowEdgeInfo = GetAboveBelowEdgeInfo(vertex1, vertex2);

                            int faceIndex3 = m_handledConsecutiveFaceIndices.at(*itFoundWithTwoFaceIndices).at(0);
                            int faceIndex4 = m_handledConsecutiveFaceIndices.at(*itFoundWithTwoFaceIndices).at(1);
                            if (!vertexValid1)
                            {
                                if ((faceIndex3 != faceIndex1) && (faceIndex3 != faceIndex2))
                                {
                                    vertexValid1 = CheckNextVertex(faceIndex3 - 1, slope, aboveBelowEdgeInfo);
                                }
                                if (vertexValid1)
                                {
                                    bool checkAddConsecutiveFaceIndices = CheckAlreadyHandledConsecutiveFaceIndices(faceIndex1, faceIndex2, faceIndex3);
                                    if (checkAddConsecutiveFaceIndices)
                                    {
                                        std::vector<int> handledFaceIndices;
                                        handledFaceIndices.push_back(faceIndex1);
                                        handledFaceIndices.push_back(faceIndex2);
                                        handledFaceIndices.push_back(faceIndex3);
                                        m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
                                    }
                                }
                            }

                            if (!vertexValid2)
                            {
                                if ((faceIndex4 != faceIndex1) && (faceIndex4 != faceIndex2))
                                {
                                    vertexValid2 = CheckNextVertex(faceIndex4 - 1, slope, aboveBelowEdgeInfo);
                                }
                                if (vertexValid2)
                                {
                                    bool checkAddConsecutiveFaceIndices = CheckAlreadyHandledConsecutiveFaceIndices(faceIndex1, faceIndex2, faceIndex4);
                                    if (checkAddConsecutiveFaceIndices)
                                    {
                                        std::vector<int> handledFaceIndices;
                                        handledFaceIndices.push_back(faceIndex1);
                                        handledFaceIndices.push_back(faceIndex2);
                                        handledFaceIndices.push_back(faceIndex4);
                                        m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
                                    }
                                }
                            }
                            if (vertexValid1 || vertexValid2)
                            {
                                m_handledConsecutiveFaceIndices.erase(m_handledConsecutiveFaceIndices.begin() + i);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void FaceFinder::HandleLastFace(int p_lastHandledFaceIndex)
{
    int vertexSecondLastIndex;
    int vertexLastIndex;

    if (p_lastHandledFaceIndex == (m_orderedVerticesWithMinXFirst.size() - 2))
    {
        vertexSecondLastIndex = p_lastHandledFaceIndex;
        vertexLastIndex = p_lastHandledFaceIndex + 1;
    }
    else if (p_lastHandledFaceIndex == (m_orderedVerticesWithMinXFirst.size() - 1))
    {
        vertexSecondLastIndex = p_lastHandledFaceIndex - 1;
        vertexLastIndex = p_lastHandledFaceIndex;
    }

    ObjVertexCoords& vertex1 = m_orderedVerticesWithMinXFirst.at(vertexSecondLastIndex);
    ObjVertexCoords& vertex2 = m_orderedVerticesWithMinXFirst.at(vertexLastIndex);
    float slope = (vertex2.X - vertex1.X) / (vertex1.Y - vertex2.Y);
    bool aboveBelowEdgeInfo = GetAboveBelowEdgeInfo(vertex1, vertex2);

    std::vector<int> handledFaceIndices;
    bool vertexValid = CheckNextVertex(0, slope, aboveBelowEdgeInfo);
    if (vertexValid)
    {
        bool checkAddConsecutiveFaceIndices = CheckAlreadyHandledConsecutiveFaceIndices(vertexSecondLastIndex, vertexLastIndex, 0);
        if (checkAddConsecutiveFaceIndices)
        {
            handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(vertexSecondLastIndex));
            handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(vertexLastIndex));
            handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(0));
            m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
        }
    }
    else
    {
        bool checkAddConsecutiveFaceIndices = CheckAlreadyHandledConsecutiveFaceIndices(vertexLastIndex, 0);
        if (checkAddConsecutiveFaceIndices)
        {
            handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(vertexLastIndex));
            handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(0));
            m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
        }
    }
}

bool FaceFinder::CheckAlreadyHandledConsecutiveFaceIndices(int p_faceIndex1, int p_faceIndex2)
{
    bool foundFaceIndex1 = false;
    bool foundFaceIndex2 = false;

    for (std::vector<int>& faceIndices : m_handledConsecutiveFaceIndices)
    {
        for (int faceIndex : faceIndices)
        {
            if (faceIndex == p_faceIndex1)
            {
                foundFaceIndex1 = true;
            }
            if (faceIndex == p_faceIndex2)
            {
                foundFaceIndex2 = true;
            }
        }
    }

    bool checkSuccessful = !(foundFaceIndex1 && foundFaceIndex2);

    return checkSuccessful;
}

bool FaceFinder::CheckAlreadyHandledConsecutiveFaceIndices(int p_faceIndex1, int p_faceIndex2, int p_faceIndex3)
{
    bool foundFaceIndex1 = false;
    bool foundFaceIndex2 = false;
    bool foundFaceIndex3 = false;

    for (std::vector<int>& faceIndices : m_handledConsecutiveFaceIndices)
    {
        for (int faceIndex : faceIndices)
        {
            if (faceIndex == p_faceIndex1)
            {
                foundFaceIndex1 = true;
            }
            if (faceIndex == p_faceIndex2)
            {
                foundFaceIndex2 = true;
            }
            if (faceIndex == p_faceIndex3)
            {
                foundFaceIndex3 = true;
            }
        }
    }

    bool checkSuccessful = !(foundFaceIndex1 && foundFaceIndex2 && foundFaceIndex3);

    return checkSuccessful;
}

void FaceFinder::AddFaces(Mesh* p_mesh)
{
    for (std::vector<int> faceIndices : m_handledConsecutiveFaceIndices)
    {
        p_mesh->AddFace(&m_dummyMaterial);
        for (int faceIndex : faceIndices)
        {
            p_mesh->AddFaceIndices(m_mapTempToRealFaceIndices.at(faceIndex));
        }
    }

    for (std::vector<int> faceIndices : m_handledNotConsecutiveFaceIndices)
    {
        p_mesh->AddFace(&m_dummyMaterial);
        for (int faceIndex : faceIndices)
        {
            p_mesh->AddFaceIndices(m_mapTempToRealFaceIndices.at(faceIndex));
        }
    }
}