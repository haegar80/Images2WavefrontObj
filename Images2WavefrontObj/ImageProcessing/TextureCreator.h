#pragma once

#include <QImage>

class Mesh;
struct ObjFace;

typedef std::pair<int, int> FaceKey;

struct SEdgePixels
{
    int startX;
    int endX;
    int startY;
    int endY;
};

class TextureCreator
{
public:
    TextureCreator();
    virtual ~TextureCreator() = default;

    void CreateTextures(const QImage& p_originalImage, Mesh* p_mesh);

private:
    QImage m_originalImage{};
    std::map<FaceKey, QImage> m_tempTextures{};
    int m_lastCheckedVertexIndex{ 0 };

    void AnalyzeFaces(Mesh* p_mesh);
    void AnalyzeFace(FaceKey p_faceKey, Mesh* p_mesh, ObjFace& p_face);
    void SortStartEndPixels(SEdgePixels& p_edgePixels);
    void CreateTempTextures(FaceKey p_faceKey, SEdgePixels p_pixelsForTempTexture);
    void SaveTextures();
};

