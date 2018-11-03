#pragma once

#include <QImage>

/// author: Copyright by Marcel Fuchs
class EdgeDetector
{
public:
    EdgeDetector();
    virtual ~EdgeDetector() = default;

    QImage DetectEdges(const QImage& p_image);

private:
    static constexpr int KernelNeighbour = 1;
    static constexpr int KernelSize = 3;

    static constexpr int SobelKernelMatrixX[KernelSize][KernelSize] = { { 1,0,-1 },{ 2,0,-2 },{ 1,0,-1 } };
    static constexpr int SobelKernelMatrixY[KernelSize][KernelSize] = { { 1,2,1 },{ 0,0,0 },{ -1,-2,-1 } };

    void GetGradientImage(const QImage& p_image, QImage& p_gradientImage);
    void UpdateGradientImage(QImage& p_gradientImage, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY);
    int ConvoluteMatrices(const int p_ObserveMatrix[KernelSize][KernelSize], const int p_KernelMatrix[KernelSize][KernelSize]);
};

