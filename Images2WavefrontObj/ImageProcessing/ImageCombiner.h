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
    static constexpr int OverlapPixels = 20;
    static constexpr int AllowedGrayScaleDifference = 20;

    static constexpr int SobelKernelMatrixX[KernelSize][KernelSize] = { {1,0,-1},{2,0,-2},{1,0,-1 } };
    static constexpr int SobelKernelMatrixY[KernelSize][KernelSize] = { {1,2,1},{0,0,0},{-1,-2,-1} };

    int m_overlapXStart{ 0 };
    int m_overlapYStart{ 0 };

    bool CombineTwoImages(QImage& p_finalImage, const QImage& p_extensionImage);
    void GetGradientImage(const QImage& p_Image, QImage& p_gradientImage);
    void UpdateGradientImage(QImage& p_gradientImage, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY);
    int ConvoluteMatrices(const int p_ObserveMatrix[KernelSize][KernelSize], const int p_KernelMatrix[KernelSize][KernelSize]);

    void AnalyzeLeftSide(QImage& p_finalImage, const QImage& p_extensionImage, const QImage& p_gradientFinalImage, const QImage& p_gradientExtensionImage);

    bool FindOverlap(const QImage& p_gradientFinalImage, const QImage& p_gradientExtensionImage, int p_startX, int p_startY);
    bool FindOverlapPixels(const QImage& p_image, QRgb* p_xExpectedPixels, QRgb* p_yExpectedPixels);
    bool HasSameGrayValue(QRgb p_firstValue, QRgb p_secondValue);
};

