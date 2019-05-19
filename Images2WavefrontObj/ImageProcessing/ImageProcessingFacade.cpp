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
    std::vector<std::unique_ptr<Mesh>>& meshes = m_vertexFinder.FindVerticesFromGradientImage(gradientImage);

    std::vector<Mesh*> meshesRawPointer;
    for (std::unique_ptr<Mesh>& mesh : meshes)
    {
        meshesRawPointer.push_back(mesh.get());
        m_textureCreator.CreateTextures(p_image, mesh.get());
    }
    
    std::vector<Material*> materials;
    Material material("DummyMaterial");
    MaterialRGBValue rgbValue;
    rgbValue.R = 1.0;
    rgbValue.G = 1.0;
    rgbValue.B = 1.0;
    material.setDiffuseColor(rgbValue);
    materials.push_back(&material);

    m_wavefrontObjectWriter.WriteWavefrontObject(meshesRawPointer, materials);

    return gradientImage;
}