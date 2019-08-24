#pragma once

#include <QImage>

/// author: Copyright by Marcel Fuchs
class EdgeDetector
{
public:
    EdgeDetector();
    virtual ~EdgeDetector() = default;

    QImage DetectEdges(const QImage& p_image, int p_minimumConvolutionSum);

private:
    enum ERGBColor
    {
        RGBColor_Red = 0,
        RGBColor_Green = 1,
        RGBColor_Blue = 2
    };

    static constexpr int KernelNeighbour = 1;
    static constexpr int KernelSize = 3;

    static constexpr int SobelKernelMatrixX[KernelSize][KernelSize] = { { 1,0,-1 },{ 2,0,-2 },{ 1,0,-1 } };
    static constexpr int SobelKernelMatrixY[KernelSize][KernelSize] = { { 1,2,1 },{ 0,0,0 },{ -1,-2,-1 } };
    int m_minimumConvolutionSum{ 0 };
    double m_maximumSobelResult{ 0.0 };

    void GetGradientImage(const QImage& p_image, QImage& p_gradientImage);
    void UpdateGradientImage(QImage& p_gradientImage, int p_observeMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY);
    int ConvoluteMatrices(const int p_observeMatrix[KernelSize][KernelSize], const int p_kernelMatrix[KernelSize][KernelSize]);
    double FindMaxResult();
};

