#pragma once

#include "../WavefrontObject/Mesh.h"
#include <QImage>

struct ObjFace;

typedef std::pair<int, int> FaceKey;

class TextureCreator
{
public:
    TextureCreator() = default;
    virtual ~TextureCreator() = default;
    TextureCreator(const TextureCreator&) = delete;
    TextureCreator& operator=(const TextureCreator&) = delete;

    std::map<std::string, std::vector<FaceKey>>& CreateTextures(const QImage& p_originalImage, Mesh* p_mesh);
    void CreateTextureCoordinates(Mesh* p_mesh);
    void ResetTextureNumber();

private:
    static constexpr double PI = 3.141592653589793238463;
    static constexpr int MaxGrayDifference = 20;

    int m_lastUsedTextureNumber{ 0 };
    QImage m_originalImage;
    Mesh* m_currentMesh;

    std::map<FaceKey, SEdgePixels> m_diagonalTexturePixels;
    std::vector<std::vector<FaceKey>> m_identicalColors;
    std::vector<FaceKey> m_alreadyHandledFaceKeys;
    std::map<std::string, std::vector<FaceKey>> m_texturePaths;

    void AnalyzeFaces();
    void AnalyzeFace(std::vector<ObjVertexCoords>& p_vertices, const FaceKey& p_faceKey, ObjFace& p_face);
    void FindFacePixelsForTextureImage(const FaceKey& p_faceKey, const ObjVertexCoords& p_vertex1, const ObjVertexCoords& p_vertex2, const ObjVertexCoords& p_vertex3);
    double GetMinAngleOfFaceRad(const ObjVertexCoords& p_vertex1, const ObjVertexCoords& p_vertex2, const ObjVertexCoords& p_vertex3, int& p_minAngleNumber);
    double GetMinAngleOfFaceRad(int* p_vectorLengthX, int* p_vectorLengthY, int& p_minAngleNumber);
    void SortStartEndPixels(SEdgePixels& p_edgePixels);
    void AnalyzeTexturePixels();
    bool CompareDiagonalTexturePixels(const SEdgePixels& p_diagonalTexturePixels1, const SEdgePixels& p_diagonalTexturePixels2);
    void FindNumberOfTexturePixelsToCompare(const SEdgePixels& p_diagonalTexturePixels1, const SEdgePixels& p_diagonalTexturePixels2, int& p_numberOfPixelsX, int& p_numberOfPixelsY);
    void AddIdenticalColorFaceKeys(const FaceKey& p_faceKey1, const FaceKey& p_faceKey2);
    void RemoveIdenticalColorFaceKey(const FaceKey& p_faceKey, int p_vectorIndex);
    bool FindIdenticalColorFaceKey(const FaceKey& p_faceKey, int& p_mainVectorIndexFaceKey);
    bool GetIdenticalColorFaceKeys(const FaceKey& p_faceKey, std::vector<FaceKey>& p_otherFaceKeys);
    void FindTexturePixels(std::vector<FaceKey>& p_faceKeys, SEdgePixels& p_texturePixels);
    void CreateTextureImages();
    void CreateTextureImages(std::vector<FaceKey>& p_faceKeys, SEdgePixels p_pixelsForTexture);
    void SaveTexture(const QImage& p_textureImage, std::vector<FaceKey>& p_faceKeys);
};

