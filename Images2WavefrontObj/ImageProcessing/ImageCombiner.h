#pragma once

#include <QImage>

class QStringList;

class ImageCombiner
{
public:
    ImageCombiner();
    virtual ~ImageCombiner() = default;

    QImage CombineImages(const QStringList& p_images);
};

