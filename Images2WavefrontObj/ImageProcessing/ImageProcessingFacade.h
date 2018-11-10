#pragma once

#include "ImageCombiner.h"
#include "EdgeDetector.h"
#include <QImage>

class QStringList;

/// author: Copyright by Marcel Fuchs
class ImageProcessingFacade
{
public:
    ImageProcessingFacade();
    virtual ~ImageProcessingFacade() = default;

    QImage CombineImages(const QStringList& p_images);
    QImage DetectEdges(const QImage& p_image);

private:
    ImageCombiner m_imageCombiner{};
    EdgeDetector m_edgeDetector{};
};

