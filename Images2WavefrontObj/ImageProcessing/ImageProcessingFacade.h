#pragma once

#include "ImageCombiner.h"
#include "EdgeDetector.h"
#include "VertexFinder.h"
#include "TextureCreator.h"
#include "WavefrontObject\WavefrontObjectWriter.h"
#include <QImage>

class QStringList;

/// author: Copyright by Marcel Fuchs
class ImageProcessingFacade
{
public:
    ImageProcessingFacade();
    virtual ~ImageProcessingFacade() = default;

    QImage CombineImages(const QStringList& p_images);
    QImage Generate3dModel(const QImage& p_image);

private:
    ImageCombiner m_imageCombiner{};
    EdgeDetector m_edgeDetector{};
    VertexFinder m_vertexFinder{};
    TextureCreator m_textureCreator{};

    WavefrontObjectWriter m_wavefrontObjectWriter{"Wavefront", "wavefront"};
};

