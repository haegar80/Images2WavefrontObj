#include "ImageProcessingFacade.h"
#include <QStringList>

ImageProcessingFacade::ImageProcessingFacade()
{
}

QImage ImageProcessingFacade::CombineImages(const QStringList& p_images)
{
    return m_imageCombiner.CombineImages(p_images);
}