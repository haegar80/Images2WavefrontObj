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

    std::map<FaceKey, std::string>& CreateTextures(const QImage& p_originalImage, Mesh* p_mesh);

private:
    QImage m_originalImage{};
    std::map<FaceKey, QImage> m_tempTextures{};
    std::map<FaceKey, std::string> m_texturePaths{};
    int m_lastCheckedVertexIndex{ 0 };

    void AnalyzeFaces(Mesh* p_mesh);
    void AnalyzeFace(FaceKey p_faceKey, Mesh* p_mesh, ObjFace& p_face);
    void SortStartEndPixels(SEdgePixels& p_edgePixels);
    void CreateTempTextures(FaceKey p_faceKey, SEdgePixels p_pixelsForTempTexture);
    void SaveTextures(Mesh* p_mesh);
};

