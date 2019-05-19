#include "TextureCreator.h"
#include "../WavefrontObject/Mesh.h"
#include "../WavefrontObject/SubMesh.h"
#include <sstream>
#include <QDir>

TextureCreator::TextureCreator()
{
}

void TextureCreator::CreateTextures(const QImage& p_originalImage, Mesh* p_mesh)
{
    m_originalImage = p_originalImage;
    AnalyzeFaces(p_mesh);
    SaveTextures();
}

void TextureCreator::AnalyzeFaces(Mesh* p_mesh)
{
    std::vector<SubMesh*> submeshes = p_mesh->GetSubmeshes();
    for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
    {
        std::vector<ObjFace> faces = submeshes.at(submeshIndex)->GetFaces();
        for (int faceIndex = 0; faceIndex < faces.size(); faceIndex++)
        {
            FaceKey faceKey;
            faceKey = std::make_pair(submeshIndex, faceIndex);

            AnalyzeFace(faceKey, p_mesh, faces.at(faceIndex));
        }
        m_lastCheckedVertexIndex = 0;
    }
}

void TextureCreator::AnalyzeFace(FaceKey p_faceKey, Mesh* p_mesh, ObjFace& p_face)
{
    std::vector<ObjVertexCoords> vertices = p_mesh->GetVertices();
    std::vector<ObjFaceIndices> faceIndices = p_face.Indices;
    if (m_lastCheckedVertexIndex == faceIndices.at(0).VertexIndex)
    {
        return;
    }

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
        else if (vertex.X != diagonalPixelX1)
        {
            diagonalPixelX2 = vertex.X;
        }

        if (0 == diagonalPixelY1)
        {
            diagonalPixelY1 = vertex.Y;
        }
        else if (vertex.Y != diagonalPixelY1)
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
    CreateTempTextures(p_faceKey, diagonalEdgePixels);

    m_lastCheckedVertexIndex = faceIndices.at(0).VertexIndex;
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

void TextureCreator::CreateTempTextures(FaceKey p_faceKey, SEdgePixels p_pixelsForTempTexture)
{
    int textureWidth = p_pixelsForTempTexture.endX - p_pixelsForTempTexture.startX + 1;
    int textureHeight = p_pixelsForTempTexture.endY - p_pixelsForTempTexture.startY + 1;
    QSize tempImageSize = QSize(textureWidth, textureHeight);
    QImage tempImage(tempImageSize, QImage::Format_RGB32);

    int tempImageX = 0;
    for (int x = p_pixelsForTempTexture.startX; x <= p_pixelsForTempTexture.endX; x++) {
        int tempImageY = 0;
        for (int y = p_pixelsForTempTexture.startY; y <= p_pixelsForTempTexture.endY; y++) {
            QRgb rgbValue = m_originalImage.pixel(x, y);
            tempImage.setPixel(tempImageX, tempImageY, rgbValue);
            tempImageY++;
        }
        tempImageX++;
    }

    m_tempTextures.insert(std::make_pair(p_faceKey, std::move(tempImage)));
}

void TextureCreator::SaveTextures()
{
    QDir texturetDirPath(QString("textures"));
    if (!texturetDirPath.exists()) {
        texturetDirPath.mkdir(".");
    }

    for (std::pair<FaceKey, QImage> tempTexture : m_tempTextures)
    {
        std::stringstream filePathString;
        filePathString << "texture_" << tempTexture.first.first << "_" << tempTexture.first.second << ".jpg";    
        QFileInfo textureFile(texturetDirPath, filePathString.str().c_str());

        bool successful = tempTexture.second.save(textureFile.absoluteFilePath(), "JPG");
    }
}