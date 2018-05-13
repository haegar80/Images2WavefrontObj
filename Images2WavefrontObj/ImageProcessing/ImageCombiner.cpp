#include "ImageCombiner.h"
#include <QStringList>
#include <QImageReader>

ImageCombiner::ImageCombiner()
{
}

QImage ImageCombiner::CombineImages(const QStringList& p_images)
{
    QImageReader imageReader(p_images.at(0));
    imageReader.setAutoTransform(true);
    const QImage image = imageReader.read();

    return std::move(image);
}