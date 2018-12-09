#include "ImageProcessingFacade.h"
#include <QStringList>

ImageProcessingFacade::ImageProcessingFacade()
{
}

QImage ImageProcessingFacade::CombineImages(const QStringList& p_images)
{
    return m_imageCombiner.CombineImages(p_images);
}

QImage ImageProcessingFacade::Generate3dModel(const QImage& p_image)
{
    QImage gradientImage = m_edgeDetector.DetectEdges(p_image);
    Mesh* mesh = m_vertexFinder.FindVerticesFromGradientImage(gradientImage);

    std::vector<Mesh*> meshes;
    meshes.push_back(mesh);
    std::vector<Material*> materials;

    m_wavefrontObjectWriter.WriteWavefrontObject(meshes, materials);

    return gradientImage;
}