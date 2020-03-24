#include "TextureCreator.h"
#include "DepthCalculator.h"
#include "../WavefrontObject/SubMesh.h"
#include <sstream>
#include <QDir>
#include <cmath>

TextureCreator::TextureCreator()
{
}

std::map<std::string, std::vector<FaceKey>>& TextureCreator::CreateTextures(const QImage& p_originalImage, Mesh* p_mesh)
{
    m_diagonalPixels.clear();
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
    std::vector<SubMesh*> submeshes = m_currentMesh->GetSubmeshes();
    for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
    {
        std::vector<ObjFace> faces = submeshes.at(submeshIndex)->GetFaces();
        for (int faceIndex = 0; faceIndex < faces.size(); faceIndex++)
        {
            FaceKey faceKey;
            faceKey = std::make_pair(submeshIndex, faceIndex);

            AnalyzeFace(faceKey, faces.at(faceIndex));
        }
    }

    AnalyzeDiagonalPixels();
    CreateTempTextures();
}

void TextureCreator::AnalyzeFace(FaceKey p_faceKey, ObjFace& p_face)
{
    std::vector<ObjVertexCoords> vertices = m_currentMesh->GetVertices();
    std::vector<ObjFaceIndices> faceIndices = p_face.Indices;

    int diagonalPixelX1 = 0;
    int diagonalPixelX2 = 0;
    int diagonalPixelY1 = 0;
    int diagonalPixelY2 = 0;

    for (auto it = faceIndices.begin(); faceIndices.end() != it; ++it)
    {
        ObjFaceIndices faceIndex = *it;
        ObjVertexCoords vertex = vertices.at(faceIndex.VertexIndex - 1);

        if (0 == diagonalPixelX1)
        {
            diagonalPixelX1 = vertex.X;
        }
        else if (IsCurrentPixelDiagonalEndPoint(diagonalPixelX1, diagonalPixelX2, vertex.X))
        {
            diagonalPixelX2 = vertex.X;
        }

        if (0 == diagonalPixelY1)
        {
            diagonalPixelY1 = vertex.Y;
        }
        else if (IsCurrentPixelDiagonalEndPoint(diagonalPixelY1, diagonalPixelY2, vertex.Y))
        {
            diagonalPixelY2 = vertex.Y;
        }
    }

    SEdgePixels diagonalEdgePixels;
    diagonalEdgePixels.startX = diagonalPixelX1;
    diagonalEdgePixels.endX = diagonalPixelX2;
    diagonalEdgePixels.startY = diagonalPixelY1;
    diagonalEdgePixels.endY = diagonalPixelY2;

    SortStartEndPixels(diagonalEdgePixels);
    m_diagonalPixels.insert(std::make_pair(p_faceKey, diagonalEdgePixels));
}

bool TextureCreator::IsCurrentPixelDiagonalEndPoint(int p_startDiagonal, int p_endDiagonalTemp, int p_currentPixel)
{
    bool isDiagonalPixel = false;

    if (0 == p_endDiagonalTemp)
    {
        isDiagonalPixel = true;
    }
    else if ((p_endDiagonalTemp < p_startDiagonal) && (p_currentPixel < p_endDiagonalTemp))
    {
        isDiagonalPixel = true;
    }
    else if ((p_endDiagonalTemp > p_startDiagonal) && (p_currentPixel > p_endDiagonalTemp))
    {
        isDiagonalPixel = true;
    }

    return isDiagonalPixel;
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

void TextureCreator::AnalyzeDiagonalPixels()
{
    for (auto itCurrentPixel = m_diagonalPixels.begin(); m_diagonalPixels.end() != itCurrentPixel; ++itCurrentPixel)
    {
        m_identicalColors.emplace_back();
        auto itOtherPixel = itCurrentPixel;
        itOtherPixel++;
        for (; m_diagonalPixels.end() != itOtherPixel; ++itOtherPixel)
        {
            bool identicalColors = CompareDiagonalPixels((*itCurrentPixel).second, (*itOtherPixel).second);
            if (identicalColors)
            {
                std::vector<FaceKey>& identicalColorsVector = m_identicalColors.back();
                FaceKey currentFaceKey = (*itCurrentPixel).first;
                auto findFaceKey1 = std::find_if(identicalColorsVector.begin(), identicalColorsVector.end(), [currentFaceKey](const FaceKey& faceKey) {return ((faceKey.first == currentFaceKey.first) && (faceKey.second == currentFaceKey.second)); });
                if (identicalColorsVector.end() == findFaceKey1)
                {
                    identicalColorsVector.push_back((*itCurrentPixel).first);
                }
                FaceKey otherFaceKey = (*itOtherPixel).first;
                auto findFaceKey2 = std::find_if(identicalColorsVector.begin(), identicalColorsVector.end(), [otherFaceKey](const FaceKey& faceKey) {return ((faceKey.first == otherFaceKey.first)  && (faceKey.second == otherFaceKey.second)); });
                if (identicalColorsVector.end() == findFaceKey2)
                {
                    identicalColorsVector.push_back((*itOtherPixel).first);
                }
            }
        }
    }
}

bool TextureCreator::CompareDiagonalPixels(SEdgePixels p_diagonalPixels1, SEdgePixels p_diagonalPixels2)
{
    bool identicalColors = false;

    int comparePixelX = 0;
    int comparePixelY = 0;
    int numberOfPixelX1 = p_diagonalPixels1.endX - p_diagonalPixels1.startX;
    int numberOfPixelX2 = p_diagonalPixels2.endX - p_diagonalPixels2.startX;
    int numberOfPixelY1 = p_diagonalPixels1.endY - p_diagonalPixels1.startY;
    int numberOfPixelY2 = p_diagonalPixels2.endY - p_diagonalPixels2.startY;

    if (numberOfPixelX1 < numberOfPixelX2)
    {
        comparePixelX = numberOfPixelX1;
    }
    else
    {
        comparePixelX = numberOfPixelX2;
    }

    if (numberOfPixelY1 < numberOfPixelY2)
    {
        comparePixelY = numberOfPixelY1;
    }
    else
    {
        comparePixelY = numberOfPixelY2;
    }

    identicalColors = true;
    for (int x = 0; x < comparePixelX; x++)
    {
        for (int y = 0; y < comparePixelY; y++)
        {
            int grayValue1 = qGray(m_originalImage.pixel((x + p_diagonalPixels1.startX), (y + p_diagonalPixels1.startY)));
            int grayValue2 = qGray(m_originalImage.pixel((x + p_diagonalPixels2.startX), (y + p_diagonalPixels2.startY)));

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

bool TextureCreator::HasIdenticalColorsWithOtherFaceKeys(FaceKey p_faceKey, std::vector<FaceKey>& p_otherFaceKeys)
{
    bool hasIdenticalColors = false;

    for (std::vector<FaceKey> identicalColors : m_identicalColors)
    {
        auto findFaceKey = std::find_if(identicalColors.begin(), identicalColors.end(), [p_faceKey](const FaceKey& faceKey) {return ((faceKey.first == p_faceKey.first) && (faceKey.second == p_faceKey.second)); });
        if (identicalColors.end() != findFaceKey)
        {
            hasIdenticalColors = true;
            for (FaceKey faceKey : identicalColors)
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

void TextureCreator::CreateTempTextures()
{
    for (std::pair<FaceKey, SEdgePixels> diagonalPixel : m_diagonalPixels)
    {
        FaceKey currentFaceKey = diagonalPixel.first;
        auto findFaceKey = std::find_if(m_alreadyHandledFaceKeys.begin(), m_alreadyHandledFaceKeys.end(), [currentFaceKey](const FaceKey& faceKey) {return ((faceKey.first == currentFaceKey.first) && (faceKey.second == currentFaceKey.second)); });
        if (m_alreadyHandledFaceKeys.end() == findFaceKey)
        {
            std::vector<FaceKey> allFaceKeys;
            allFaceKeys.push_back(currentFaceKey);

            std::vector<FaceKey> otherFaceKeys;
            bool hasIdenticalColors = HasIdenticalColorsWithOtherFaceKeys(diagonalPixel.first, otherFaceKeys);
           
            if (hasIdenticalColors)
            {
                for (FaceKey otherFaceKey : otherFaceKeys)
                {
                    allFaceKeys.push_back(otherFaceKey);
                }    
            }
            CreateTempTextures(allFaceKeys, diagonalPixel.second);
        }
    }
}

void TextureCreator::CreateTempTextures(std::vector<FaceKey>& p_faceKeys, SEdgePixels p_pixelsForTexture)
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
    CreateTextureCoordinates(tempImage, p_faceKeys);
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

void TextureCreator::CreateTextureCoordinates(const QImage& p_textureImage, std::vector<FaceKey>& p_faceKeys)
{
    constexpr double PI = 3.141592653589793238463;
    int textureWidth = p_textureImage.width();
    int textureHeight = p_textureImage.height();

    std::vector<ObjVertexCoords> vertices = m_currentMesh->GetVertices();
    
    for (FaceKey& faceKey : p_faceKeys)
    {
        std::vector<SubMesh*> submeshes = m_currentMesh->GetSubmeshes();
        std::vector<ObjFace> faces = submeshes.at(faceKey.first)->GetFaces();
        ObjFace& face = faces.at(faceKey.second);
        for (ObjFaceIndices& faceIndex : face.Indices)
        {
            ObjVertexCoords vertex = vertices.at(faceIndex.VertexIndex - 1);
            double pixelX = static_cast<double>(vertex.X) / m_originalImage.width();
            double pixelY = static_cast<double>(vertex.Y) / m_originalImage.height();
            double pixelZ = static_cast<double>(vertex.Z) / (DepthCalculator::GetZPixelFarest() - DepthCalculator::GetZPixelNearest());

            double phi = atan2(pixelZ, pixelX);
            double theta = asin(pixelY);
            double pixelU = 0.5 + phi / (2 * PI);
            pixelU *= m_originalImage.width();
            double pixelV = 0.5 - theta / PI;
            pixelV *= m_originalImage.height();

            m_currentMesh->AddTexture(static_cast<int>(pixelU), static_cast<int>(pixelV));
            faceIndex.TextureIndex = m_currentMesh->GetTextures().size();
            submeshes.at(faceKey.first)->UpdateExistingFace(faceKey.second, face);
        }
    }
}