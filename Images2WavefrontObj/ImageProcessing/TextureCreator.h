#pragma once

#include "../WavefrontObject/Mesh.h"
#include <QImage>

struct ObjFace;

typedef std::pair<int, int> FaceKey;

class TextureCreator
{
public:
    TextureCreator();
    virtual ~TextureCreator() = default;
    TextureCreator(const TextureCreator&) = delete;
    TextureCreator& operator=(const TextureCreator&) = delete;

    std::map<std::string, std::vector<FaceKey>>& CreateTextures(const QImage& p_originalImage, Mesh* p_mesh);
    void CreateTextureCoordinates(Mesh* p_mesh);
    void ResetTextureNumber();

private:
    static constexpr int MaxGrayDifference = 20;

    int m_lastUsedTextureNumber{ 0 };
    QImage m_originalImage;
    Mesh* m_currentMesh;

    std::map<FaceKey, SEdgePixels> m_diagonalPixels;
    std::vector<std::vector<FaceKey>> m_identicalColors;
    std::vector<FaceKey> m_alreadyHandledFaceKeys;
    std::map<std::string, std::vector<FaceKey>> m_texturePaths;

    void AnalyzeFaces();
    void AnalyzeFace(FaceKey p_faceKey, ObjFace& p_face);
    bool IsCurrentPixelDiagonalEndPoint(int p_startDiagonal, int p_endDiagonalTemp, int p_currentPixel);
    void SortStartEndPixels(SEdgePixels& p_edgePixels);
    void AnalyzeDiagonalPixels();
    bool CompareDiagonalPixels(SEdgePixels p_diagonalPixels1, SEdgePixels p_diagonalPixels2);
    bool HasIdenticalColorsWithOtherFaceKeys(FaceKey p_faceKey, std::vector<FaceKey>& p_otherFaceKeys);
    void CreateTempTextures();
    void CreateTempTextures(std::vector<FaceKey>& p_faceKeys, SEdgePixels p_pixelsForTexture);
    void SaveTexture(const QImage& p_textureImage, std::vector<FaceKey>& p_faceKeys);
};

