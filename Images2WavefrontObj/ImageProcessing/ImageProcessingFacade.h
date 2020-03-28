#pragma once

#include "ImageCombiner.h"
#include "DepthCalculator.h"
#include "EdgeDetector.h"
#include "NormalCalculator.h"
#include "VertexAdjuster.h"
#include "VertexFinder.h"
#include "TextureCreator.h"
#include "MaterialProcessing/MaterialManager.h"
#include "WavefrontObject/WavefrontObjectWriter.h"
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
    static constexpr int MinimumGradient = 30;

    ImageCombiner m_imageCombiner;
    DepthCalculator m_depthCalculator;
    EdgeDetector m_edgeDetector;
    NormalCalculator m_normalCalculator;
    VertexAdjuster m_vertexAdjuster;
    VertexFinder m_vertexFinder;
    TextureCreator m_textureCreator;
    MaterialManager m_materialManager;

    WavefrontObjectWriter m_wavefrontObjectWriter{"Wavefront", "wavefront"};
};

