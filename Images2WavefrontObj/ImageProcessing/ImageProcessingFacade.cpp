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
    QImage gradientImage = m_edgeDetector.DetectEdges(p_image, MinimumGradient);
    std::vector<std::unique_ptr<Mesh>>& meshes = m_vertexFinder.FindVerticesFromGradientImage(gradientImage, MinimumGradient);
    m_vertexGapFinder.FindVertexGaps(meshes);
    m_depthCalculator.CalculateDepths(p_image.width(), p_image.height(), meshes);

    m_materialManager.CreateDefaultMaterial();

    m_textureCreator.ResetTextureNumber();
    std::vector<Mesh*> meshesRawPointer;
    for (std::unique_ptr<Mesh>& mesh : meshes)
    {
        meshesRawPointer.push_back(mesh.get());
        std::map<std::string, std::vector<FaceKey>>& texturePaths = m_textureCreator.CreateTextures(p_image, mesh.get());
        m_materialManager.CreateMaterialsBasedOnTextures(texturePaths);
        m_materialManager.UpdateMaterialsInMesh(mesh.get());
    }

    m_vertexAdjuster.NormVertices(meshes, p_image.width(), p_image.height());
    m_normalCalculator.CalculateNormals(meshes);

    for (std::unique_ptr<Mesh>& mesh : meshes)
    {
        m_textureCreator.CreateTextureCoordinates(mesh.get());
    }

    m_wavefrontObjectWriter.WriteWavefrontObject(meshesRawPointer, m_materialManager.GetMaterials());
    m_readableVertexWriter.WriteVertices(meshesRawPointer, p_image.width(), p_image.height());

    return gradientImage;
}