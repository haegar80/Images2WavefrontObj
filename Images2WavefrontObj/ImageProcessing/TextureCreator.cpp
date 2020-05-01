#include "TextureCreator.h"
#include "../WavefrontObject/SubMesh.h"
#include <sstream>
#include <QDir>
#include <cmath>

std::map<std::string, std::vector<FaceKey>>& TextureCreator::CreateTextures(const QImage& p_originalImage, Mesh* p_mesh)
{
    m_diagonalTexturePixels.clear();
    m_identicalColors.clear();
    m_alreadyHandledFaceKeys.clear();
    m_texturePaths.clear();
    m_originalImage = p_originalImage;
    m_currentMesh = p_mesh;

    AnalyzeFaces();

    return m_texturePaths;
}

void TextureCreator::ResetTextureNumber()
{
    m_lastUsedTextureNumber = 0;
}

void TextureCreator::AnalyzeFaces()
{
    std::vector<ObjVertexCoords> vertices = m_currentMesh->GetVertices();
    std::vector<SubMesh*> submeshes = m_currentMesh->GetSubmeshes();
    for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
    {
        std::vector<ObjFace> faces = submeshes.at(submeshIndex)->GetFaces();
        for (int faceIndex = 0; faceIndex < faces.size(); faceIndex++)
        {
            FaceKey faceKey = std::make_pair(submeshIndex, faceIndex);
            AnalyzeFace(vertices, faceKey, faces.at(faceIndex));
        }
    }

    AnalyzeTexturePixels();
    CreateTextureImages();
}

void TextureCreator::AnalyzeFace(std::vector<ObjVertexCoords>& p_vertices, const FaceKey& p_faceKey, ObjFace& p_face)
{
    std::vector<ObjFaceIndices> faceIndices = p_face.Indices;
    ObjVertexCoords vertex1 = p_vertices.at(faceIndices.at(0).VertexIndex - 1);
    ObjVertexCoords vertex2 = p_vertices.at(faceIndices.at(1).VertexIndex - 1);
    
    bool hasThirdFaceIndex = (faceIndices.size() > 2);
    if (hasThirdFaceIndex)
    {
        ObjVertexCoords vertex3 = p_vertices.at(faceIndices.at(2).VertexIndex - 1);
        FindFacePixelsForTextureImage(p_faceKey, vertex1, vertex2, vertex3);
    }
}

void TextureCreator::FindFacePixelsForTextureImage(const FaceKey& p_faceKey, const ObjVertexCoords& p_vertex1, const ObjVertexCoords& p_vertex2, const ObjVertexCoords& p_vertex3)
{
    int sumX = p_vertex1.X + p_vertex2.X + p_vertex3.X;
    int sumY = p_vertex1.Y + p_vertex2.Y + p_vertex3.Y;
    int averageX = static_cast<int>(sumX / 3);
    int averageY = static_cast<int>(sumY / 3);

    int minAngleNumber = 0;
    double minAngle = GetMinAngleOfFaceRad(p_vertex1, p_vertex2, p_vertex3, minAngleNumber);
    int pixelXWithMinAngle = 0;
    int pixelYWithMinAngle = 0;
    if (1 == minAngleNumber)
    {
        pixelXWithMinAngle = p_vertex1.X;
        pixelYWithMinAngle = p_vertex1.Y;
    }
    else if (2 == minAngleNumber)
    {
        pixelXWithMinAngle = p_vertex2.X;
        pixelYWithMinAngle = p_vertex2.Y;
    }
    else
    {
        pixelXWithMinAngle = p_vertex3.X;
        pixelYWithMinAngle = p_vertex3.Y;
    }

    int vectorXDifference = pixelXWithMinAngle - averageX;
    int vectorYDifference = pixelYWithMinAngle - averageY;
    int factorX = 1;
    int factorY = 1;
    if (vectorXDifference < 0)
    {
        factorX = -1;
    }
    if (vectorYDifference < 0)
    {
        factorY = -1;
    }

    int minLengthAverageVector = sqrt(vectorXDifference * vectorXDifference + vectorYDifference * vectorYDifference);
    double angleAverageVector = atan2(vectorYDifference, vectorXDifference);
    double angleDeviationVector = angleAverageVector + minAngle / 2.0;
    int deviationVectorX = static_cast<int>(round(cos(angleDeviationVector) * minLengthAverageVector));
    int deviationVectorY = static_cast<int>(round(sin(angleDeviationVector) * minLengthAverageVector));
 
    int distanceX = averageX - (pixelXWithMinAngle + factorX * deviationVectorX);
    int distanceY = averageY - (pixelYWithMinAngle + factorY * deviationVectorY);
    
    SEdgePixels diagonalTexturePixels;
    diagonalTexturePixels.startX = averageX - static_cast<int>(distanceX / 4);
    diagonalTexturePixels.endX = averageX + static_cast<int>(distanceX / 4);
    diagonalTexturePixels.startY = averageY - static_cast<int>(distanceY / 4);
    diagonalTexturePixels.endY = averageY + static_cast<int>(distanceY / 4);

    SortStartEndPixels(diagonalTexturePixels);
    m_diagonalTexturePixels.insert(std::make_pair(p_faceKey, diagonalTexturePixels));
}

double TextureCreator::GetMinAngleOfFaceRad(const ObjVertexCoords& p_vertex1, const ObjVertexCoords& p_vertex2, const ObjVertexCoords& p_vertex3, int& p_minAngleNumber)
{
    int diffX1 = p_vertex1.X - p_vertex2.X;
    int diffY1 = p_vertex1.Y - p_vertex2.Y;
    int diffX2 = p_vertex2.X - p_vertex3.X;
    int diffY2 = p_vertex2.Y - p_vertex3.Y;
    int diffX3 = p_vertex1.X - p_vertex3.X;
    int diffY3 = p_vertex1.Y - p_vertex3.Y;

    int vectorLengthX[3] = { diffX1, diffX2, diffX3 };
    int vectorLengthY[3] = { diffY1, diffY2, diffY3 };
    double minAngle = GetMinAngleOfFaceRad(vectorLengthX, vectorLengthY, p_minAngleNumber);

    return minAngle;
}

double TextureCreator::GetMinAngleOfFaceRad(int* p_vectorLengthX, int* p_vectorLengthY, int& p_minAngleNumber)
{
    double angleTemp1 = atan2(p_vectorLengthY[0], p_vectorLengthX[0]);
    double angleTemp2 = atan2(p_vectorLengthY[1], p_vectorLengthX[1]);
    double angleTemp3 = atan2(p_vectorLengthY[2], p_vectorLengthX[2]);
    double angle1 = angleTemp3 - angleTemp1;
    double angle2 = angleTemp1 - angleTemp2;
    double angle3 = angleTemp2 - angleTemp3;

    double minAngle = angle1;
    p_minAngleNumber = 1;
    if (((angle2 >= 0) && (angle2 < minAngle)) || (angle2 < 0) && (angle2 > minAngle))
    {
        minAngle = angle2;
        p_minAngleNumber = 2;
    }
    if (((angle3 >= 0) && (angle3 < minAngle)) || (angle3 < 0) && (angle3 > minAngle))
    {
        minAngle = angle3;
        p_minAngleNumber = 3;
    }

    return minAngle;
}

void TextureCreator::SortStartEndPixels(SEdgePixels& p_edgePixels)
{
    int tmp = 0;
    if (p_edgePixels.endX < p_edgePixels.startX)
    {
        tmp = p_edgePixels.endX;
        p_edgePixels.endX = p_edgePixels.startX;
        p_edgePixels.startX = tmp;
    }
    if (p_edgePixels.endY < p_edgePixels.startY)
    {
        tmp = p_edgePixels.endY;
        p_edgePixels.endY = p_edgePixels.startY;
        p_edgePixels.startY = tmp;
    }
}

void TextureCreator::AnalyzeTexturePixels()
{
    for (auto it1 = m_diagonalTexturePixels.begin(); m_diagonalTexturePixels.end() != it1; ++it1)
    {
        for (auto it2 = std::next(it1); m_diagonalTexturePixels.end() != it2; ++it2)
        {
            bool identicalColors = CompareDiagonalTexturePixels((*it1).second, (*it2).second);
            if (identicalColors)
            {
                AddIdenticalColorFaceKeys((*it1).first, (*it2).first);
            }
        }
    }
}

bool TextureCreator::CompareDiagonalTexturePixels(const SEdgePixels& p_diagonalTexturePixels1, const SEdgePixels& p_diagonalTexturePixels2)
{
    bool identicalColors = false;

    int comparePixelX = 0;
    int comparePixelY = 0;
    FindNumberOfTexturePixelsToCompare(p_diagonalTexturePixels1, p_diagonalTexturePixels2, comparePixelX, comparePixelY);

    identicalColors = true;
    for (int x = 0; x < comparePixelX; x++)
    {
        for (int y = 0; y < comparePixelY; y++)
        {
            int grayValue1 = qGray(m_originalImage.pixel((x + p_diagonalTexturePixels1.startX), (y + p_diagonalTexturePixels1.startY)));
            int grayValue2 = qGray(m_originalImage.pixel((x + p_diagonalTexturePixels2.startX), (y + p_diagonalTexturePixels2.startY)));

            int grayDifference = grayValue1 - grayValue2;
            if (grayDifference < -MaxGrayDifference || grayDifference > MaxGrayDifference)
            {
                identicalColors = false;
                break;
            }
        }
        if (!identicalColors)
        {
            break;
        }
    }

    return identicalColors;
}

void TextureCreator::FindNumberOfTexturePixelsToCompare(const SEdgePixels& p_diagonalTexturePixels1, const SEdgePixels& p_diagonalTexturePixels2, int& p_numberOfPixelsX, int& p_numberOfPixelsY)
{
    int numberOfPixelX1 = p_diagonalTexturePixels1.endX - p_diagonalTexturePixels1.startX + 1;
    int numberOfPixelX2 = p_diagonalTexturePixels2.endX - p_diagonalTexturePixels2.startX + 1;
    int numberOfPixelY1 = p_diagonalTexturePixels1.endY - p_diagonalTexturePixels1.startY + 1;
    int numberOfPixelY2 = p_diagonalTexturePixels2.endY - p_diagonalTexturePixels2.startY + 1;

    if (numberOfPixelX1 < numberOfPixelX2)
    {
        p_numberOfPixelsX = numberOfPixelX1;
    }
    else
    {
        p_numberOfPixelsX = numberOfPixelX2;
    }

    if (numberOfPixelY1 < numberOfPixelY2)
    {
        p_numberOfPixelsY = numberOfPixelY1;
    }
    else
    {
        p_numberOfPixelsY = numberOfPixelY2;
    }
}

void TextureCreator::AddIdenticalColorFaceKeys(const FaceKey& p_faceKey1, const FaceKey& p_faceKey2)
{
    int mainVectorIndexFaceKey1 = 0;
    int mainVectorIndexFaceKey2 = 0;

    bool faceKey1Found = FindIdenticalColorFaceKey(p_faceKey1, mainVectorIndexFaceKey1);
    bool faceKey2Found = FindIdenticalColorFaceKey(p_faceKey2, mainVectorIndexFaceKey2);

    if (faceKey1Found && faceKey2Found)
    {
        RemoveIdenticalColorFaceKey(p_faceKey2, mainVectorIndexFaceKey2);
    }

    if (faceKey1Found)
    {
        std::vector<FaceKey>& identicalColorsVector = m_identicalColors.at(mainVectorIndexFaceKey1);
        identicalColorsVector.push_back(p_faceKey2);
    }
    else if (faceKey2Found)
    {
        std::vector<FaceKey>& identicalColorsVector = m_identicalColors.at(mainVectorIndexFaceKey2);
        identicalColorsVector.push_back(p_faceKey1);
    }
    else
    {
        m_identicalColors.emplace_back();
        std::vector<FaceKey>& identicalColorsVector = m_identicalColors.back();
        identicalColorsVector.push_back(p_faceKey1);
        identicalColorsVector.push_back(p_faceKey2);
    }
}

void TextureCreator::RemoveIdenticalColorFaceKey(const FaceKey& p_faceKey, int p_vectorIndex)
{
    std::vector<FaceKey>& identicalColorsVector = m_identicalColors.at(p_vectorIndex);
    (void)identicalColorsVector.erase(std::remove_if(identicalColorsVector.begin(), identicalColorsVector.end(), [p_faceKey](const FaceKey& faceKey) {return ((faceKey.first == p_faceKey.first) && (faceKey.second == p_faceKey.second)); }), identicalColorsVector.end());
    if (identicalColorsVector.empty())
    {
        auto itDelete = m_identicalColors.begin() + p_vectorIndex;
        (void)m_identicalColors.erase(itDelete);
    }
}

bool TextureCreator::FindIdenticalColorFaceKey(const FaceKey& p_faceKey, int& p_mainVectorIndexFaceKey)
{
    bool faceKeyFound = false;

    for (int i = 0; i < m_identicalColors.size(); i++)
    {
        std::vector<FaceKey>& identicalColors = m_identicalColors.at(i);
        auto findFaceKey = std::find_if(identicalColors.begin(), identicalColors.end(), [p_faceKey](const FaceKey& faceKey) {return ((faceKey.first == p_faceKey.first) && (faceKey.second == p_faceKey.second)); });
        if (identicalColors.end() != findFaceKey)
        {
            faceKeyFound = true;
            p_mainVectorIndexFaceKey = i;
            break;
        }
    }

    return faceKeyFound;
}

bool TextureCreator::GetIdenticalColorFaceKeys(const FaceKey& p_faceKey, std::vector<FaceKey>& p_otherFaceKeys)
{
    bool hasIdenticalColors = false;

    for (std::vector<FaceKey> identicalColorsVector : m_identicalColors)
    {
        auto findFaceKey = std::find_if(identicalColorsVector.begin(), identicalColorsVector.end(), [p_faceKey](const FaceKey& faceKey) {return ((faceKey.first == p_faceKey.first) && (faceKey.second == p_faceKey.second)); });
        if (identicalColorsVector.end() != findFaceKey)
        {
            hasIdenticalColors = true;
            for (FaceKey faceKey : identicalColorsVector)
            {
                if (p_faceKey != faceKey)
                {
                    p_otherFaceKeys.push_back(faceKey);
                }
            }
        }
    }

    return hasIdenticalColors;
}

void TextureCreator::CreateTextureImages()
{
    std::vector<FaceKey> allFaceKeys;

    std::vector<SubMesh*> submeshes = m_currentMesh->GetSubmeshes();
    for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
    {
        std::vector<ObjFace> faces = submeshes.at(submeshIndex)->GetFaces();
        for (int faceIndex = 0; faceIndex < faces.size(); faceIndex++)
        {
            FaceKey localFaceKey = std::make_pair(submeshIndex, faceIndex);

            auto findFaceKey = std::find_if(m_alreadyHandledFaceKeys.begin(), m_alreadyHandledFaceKeys.end(), [localFaceKey](const FaceKey& faceKey) {return ((faceKey.first == localFaceKey.first) && (faceKey.second == localFaceKey.second)); });
            if (m_alreadyHandledFaceKeys.end() == findFaceKey)
            {
                allFaceKeys.push_back(localFaceKey);

                std::vector<FaceKey> otherFaceKeys;
                bool hasIdenticalColors = GetIdenticalColorFaceKeys(localFaceKey, otherFaceKeys);

                if (hasIdenticalColors)
                {
                    for (FaceKey otherFaceKey : otherFaceKeys)
                    {
                        allFaceKeys.push_back(otherFaceKey);
                    }
                }

                auto itFoundTexturePixels = m_diagonalTexturePixels.find(localFaceKey);
                if (m_diagonalTexturePixels.end() != itFoundTexturePixels)
                {
                    SEdgePixels texturePixels;
                    FindTexturePixels(allFaceKeys, texturePixels);
                    CreateTextureImages(allFaceKeys, texturePixels);
                    allFaceKeys.clear();
                }
            }
        }
    }
}

bool TextureCreator::FindTexturePixels(std::vector<FaceKey>& p_faceKeys, SEdgePixels& p_texturePixels)
{
    bool foundTexturePixels = false;

    int minNumberOfPixels = 0;
    for (auto itFaceKey1 = p_faceKeys.begin(); p_faceKeys.end() != itFaceKey1; ++itFaceKey1)
    {
        for (auto itFaceKey2 = std::next(itFaceKey1); p_faceKeys.end() != itFaceKey2; ++itFaceKey2)
        {
            auto itFoundTexturePixels1 = m_diagonalTexturePixels.find(*itFaceKey1);
            auto itFoundTexturePixels2 = m_diagonalTexturePixels.find(*itFaceKey2);
            if ((m_diagonalTexturePixels.end() != itFoundTexturePixels1) && (m_diagonalTexturePixels.end() != itFoundTexturePixels2))
            {
                int numberOfPixels1 = ((*itFoundTexturePixels1).second.endX - (*itFoundTexturePixels1).second.startX) * ((*itFoundTexturePixels1).second.endY - (*itFoundTexturePixels1).second.startY);
                int numberOfPixels2 = ((*itFoundTexturePixels2).second.endX - (*itFoundTexturePixels2).second.startX) * ((*itFoundTexturePixels2).second.endY - (*itFoundTexturePixels2).second.startY);
               
                if ((0 == minNumberOfPixels) || (numberOfPixels1 < minNumberOfPixels))
                {
                    minNumberOfPixels = numberOfPixels1;
                    p_texturePixels = (*itFoundTexturePixels1).second;
                }
                if ((0 == minNumberOfPixels) || (numberOfPixels2 < minNumberOfPixels))
                {
                    minNumberOfPixels = numberOfPixels2;
                    p_texturePixels = (*itFoundTexturePixels2).second;
                }
                foundTexturePixels = true;
            }
            else if (m_diagonalTexturePixels.end() != itFoundTexturePixels1)
            {
                int numberOfPixels1 = ((*itFoundTexturePixels1).second.endX - (*itFoundTexturePixels1).second.startX) * ((*itFoundTexturePixels1).second.endY - (*itFoundTexturePixels1).second.startY);
                if ((0 == minNumberOfPixels) || (numberOfPixels1 < minNumberOfPixels))
                {
                    minNumberOfPixels = numberOfPixels1;
                    p_texturePixels = (*itFoundTexturePixels1).second;
                }
                foundTexturePixels = true;
            }
            else if (m_diagonalTexturePixels.end() != itFoundTexturePixels2)
            {
                int numberOfPixels2 = ((*itFoundTexturePixels2).second.endX - (*itFoundTexturePixels2).second.startX) * ((*itFoundTexturePixels2).second.endY - (*itFoundTexturePixels2).second.startY);
                if ((0 == minNumberOfPixels) || (numberOfPixels2 < minNumberOfPixels))
                {
                    minNumberOfPixels = numberOfPixels2;
                    p_texturePixels = (*itFoundTexturePixels2).second;
                }
                foundTexturePixels = true;
            }
        }
    }

    return foundTexturePixels;
}

void TextureCreator::CreateTextureImages(std::vector<FaceKey>& p_faceKeys, SEdgePixels p_pixelsForTexture)
{
    bool createdTempTexture = false;

    int textureWidth = p_pixelsForTexture.endX - p_pixelsForTexture.startX + 1;
    int textureHeight = p_pixelsForTexture.endY - p_pixelsForTexture.startY + 1;
    QSize tempImageSize = QSize(textureWidth, textureHeight);
    QImage tempImage(tempImageSize, QImage::Format_RGB32);

    int tempImageX = 0;
    for (int x = p_pixelsForTexture.startX; x <= p_pixelsForTexture.endX; x++) {
        int tempImageY = 0;
        for (int y = p_pixelsForTexture.startY; y <= p_pixelsForTexture.endY; y++) {
            QRgb rgbValue = m_originalImage.pixel(x, y);
            tempImage.setPixel(tempImageX, tempImageY, rgbValue);
            tempImageY++;
        }
        tempImageX++;
    }
    
    SaveTexture(tempImage, p_faceKeys);
}

void TextureCreator::SaveTexture(const QImage& p_textureImage, std::vector<FaceKey>& p_faceKeys)
{
    QDir textureDirPath(QString("textures"));
    if (!textureDirPath.exists()) {
        textureDirPath.mkdir(".");
    }

    m_lastUsedTextureNumber++;

    std::stringstream filePathStringStream;
    filePathStringStream << "texture_" << m_lastUsedTextureNumber << ".jpg";
    QFileInfo textureFile(textureDirPath, filePathStringStream.str().c_str());

    // Directly call of toStdString() fails
    QString& filePath = textureFile.filePath();
    QByteArray tmp = filePath.toLocal8Bit();
    std::string filePathString = std::string(tmp.constData());
    bool successful = p_textureImage.save(filePathString.c_str(), "JPG");

    if (successful)
    {
        for (FaceKey faceKey : p_faceKeys)
        {
            m_alreadyHandledFaceKeys.push_back(faceKey);
        }

        m_texturePaths.emplace(std::make_pair(filePathString, p_faceKeys));
    }
}

void TextureCreator::CreateTextureCoordinates(Mesh* p_mesh)
{
    std::vector<ObjVertexCoords> vertices = p_mesh->GetVertices();
    std::vector<SubMesh*>& submeshes = p_mesh->GetSubmeshes();
    for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
    {
        std::vector<ObjFace>& faces = submeshes.at(submeshIndex)->GetFaces();
        for (int faceVectorIndex = 0; faceVectorIndex < faces.size(); faceVectorIndex++)
        {
            ObjFace& face = faces.at(faceVectorIndex);
            for (ObjFaceIndices& faceIndex : face.Indices)
            {
                ObjVertexCoords vertex = vertices.at(faceIndex.VertexIndex - 1);
                double phi = atan2(vertex.Z, vertex.X);
                double theta = asin(vertex.Y);
                double pixelU = 0.5 + phi / (2 * PI);
                double pixelV = 0.5 - theta / PI;

                p_mesh->AddTexture(pixelU, pixelV);
                faceIndex.TextureIndex = p_mesh->GetTextures().size();
                submeshes.at(submeshIndex)->UpdateExistingFace(faceVectorIndex, face);
            }
        }
    }
}