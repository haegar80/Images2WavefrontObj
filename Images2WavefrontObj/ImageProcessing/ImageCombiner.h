#pragma once

#include <QImage>
#include <QGenericMatrix>

class QStringList;

class ImageCombiner
{
public:
    ImageCombiner();
    virtual ~ImageCombiner() = default;

    QImage CombineImages(const QStringList& p_images);

private:
    static constexpr int KernelNeighbour = 1;
    static constexpr int KernelSize = 3;

    static constexpr int SobelKernelMatrixX[KernelSize][KernelSize] = { {1,0,-1},{2,0,-2},{1,0,-1 } };
    static constexpr int SobelKernelMatrixY[KernelSize][KernelSize] = { {1,2,1},{0,0,0},{-1,-2,-1} };

    bool CombineTwoImages(QImage& p_finalImage, const QImage& p_extensionImage);
    void UpdateGradientImage(QImage& p_gradientImageX, QImage& p_gradientImageY, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY);
    int ConvoluteMatrices(const int p_ObserveMatrix[KernelSize][KernelSize], const int p_KernelMatrix[KernelSize][KernelSize]);
};

