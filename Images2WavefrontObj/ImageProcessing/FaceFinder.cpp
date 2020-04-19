#include "FaceFinder.h"

void FaceFinder::AddFaces(Mesh* p_mesh, std::vector<int>& p_faceIndices)
{
    m_orderedVerticesWithMinXFirst.clear();
    m_orderedFaceIndicesWithMinXFirst.clear();
    m_handledConsecutiveFaceIndices.clear();
    m_handledNotConsecutiveFaceIndices.clear();

    std::vector<ObjVertexCoords> allVertices = p_mesh->GetVertices();
    std::vector<ObjVertexCoords> verticesToCheck;
    for (int faceIndex : p_faceIndices)
    {
        verticesToCheck.push_back(allVertices.at(faceIndex - 1));
    }

    OrderVerticesWithMinXFirst(verticesToCheck, p_faceIndices);
    FindFaces(p_mesh);
    AddFaces(p_mesh);
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

    if (p_vertex1.X <= p_vertex2.X)
    {
        aboveBelowEdgeInfo = Below;
    }
    
    return aboveBelowEdgeInfo;
}

void FaceFinder::FindFaces(Mesh* p_mesh)
{
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

    HandleLastFace(currentIndex);
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
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(vertexSecondLastIndex));
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(vertexLastIndex));
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(0));
        m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
    }
    else
    {
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(vertexLastIndex));
        handledFaceIndices.push_back(m_orderedFaceIndicesWithMinXFirst.at(0));
        m_handledConsecutiveFaceIndices.push_back(handledFaceIndices);
    }
}

void FaceFinder::AddFaces(Mesh* p_mesh)
{
    for (std::vector<int> faceIndices : m_handledConsecutiveFaceIndices)
    {
        p_mesh->AddFace(&m_dummyMaterial);
        for (int faceIndex : faceIndices)
        {
            p_mesh->AddFaceIndices(faceIndex);
        }
    }

    for (std::vector<int> faceIndices : m_handledNotConsecutiveFaceIndices)
    {
        p_mesh->AddFace(&m_dummyMaterial);
        for (int faceIndex : faceIndices)
        {
            p_mesh->AddFaceIndices(faceIndex);
        }
    }
}