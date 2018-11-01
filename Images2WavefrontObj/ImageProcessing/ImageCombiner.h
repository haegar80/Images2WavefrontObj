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
    static constexpr int OverlapPixels = 200;
    static constexpr int SearchPixelInterval = 50;
    static constexpr int AllowedOverlapGap = 2;
    static constexpr int AllowedGrayScaleDifference = 10;

    static constexpr int SobelKernelMatrixX[KernelSize][KernelSize] = { {1,0,-1},{2,0,-2},{1,0,-1 } };
    static constexpr int SobelKernelMatrixY[KernelSize][KernelSize] = { {1,2,1},{0,0,0},{-1,-2,-1} };

    int m_overlapXStartFinalImage{ 0 };
    int m_overlapYStartFinalImage{ 0 };
    int m_overlapXStartExtensionImage{ 0 };
    int m_overlapYStartExtensionImage{ 0 };

    bool CombineTwoImages(QImage& p_finalImage, const QImage& p_extensionImage);
    void GetGradientImage(const QImage& p_Image, QImage& p_gradientImage);
    void UpdateGradientImage(QImage& p_gradientImage, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY);
    int ConvoluteMatrices(const int p_ObserveMatrix[KernelSize][KernelSize], const int p_KernelMatrix[KernelSize][KernelSize]);

    bool FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage);
    bool FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage, int p_startX, int p_startY);
    bool FindOverlapPixels(const QImage& p_image, const int p_expectedPixels[OverlapPixels][OverlapPixels]);
    bool HasSameGrayValue(int p_firstValue, int p_secondValue);
};

