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

    m_materialManager.CreateDefaultMaterial();

    std::vector<Mesh*> meshesRawPointer;
    for (std::unique_ptr<Mesh>& mesh : meshes)
    {
        meshesRawPointer.push_back(mesh.get());
        std::map<FaceKey, std::string>& texturePaths = m_textureCreator.CreateTextures(p_image, mesh.get());
        m_materialManager.CreateMaterialsBasedOnTextures(texturePaths);
        m_materialManager.UpdateMaterialsInMesh(mesh.get());
    }

    m_wavefrontObjectWriter.WriteWavefrontObject(meshesRawPointer, m_materialManager.GetMaterials());

    return gradientImage;
}