#pragma once

#include "ImageCombiner.h"
#include <QImage>

class QStringList;

class ImageProcessingFacade
{
public:
    ImageProcessingFacade();
    virtual ~ImageProcessingFacade() = default;

    QImage CombineImages(const QStringList& p_images);

private:
    ImageCombiner m_imageCombiner;
};

