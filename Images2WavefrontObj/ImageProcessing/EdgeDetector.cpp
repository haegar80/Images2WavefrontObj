#include "EdgeDetector.h"

EdgeDetector::EdgeDetector()
{
}

QImage EdgeDetector::DetectEdges(const QImage& p_image)
{
    QSize imageSize = QSize(p_image.width(), p_image.height());
    QImage gradientImage(imageSize, QImage::Format_RGB32);
    GetGradientImage(p_image, gradientImage);

    return gradientImage;
}

void EdgeDetector::GetGradientImage(const QImage& p_image, QImage& p_gradientImage)
{

    int observeMatrix[KernelSize][KernelSize];
    for (int x = KernelNeighbour; x < (p_image.width() - KernelNeighbour - 1); x++)
    {
        for (int y = KernelNeighbour; y < (p_image.height() - KernelNeighbour - 1); y++)
        {
            for (int kernelX = -KernelNeighbour; kernelX <= KernelNeighbour; kernelX++)
            {
                for (int kernelY = -KernelNeighbour; kernelY <= KernelNeighbour; kernelY++)
                {
                    int indexX = kernelX + KernelNeighbour;
                    int indexY = kernelY + KernelNeighbour;
                    observeMatrix[indexX][indexY] = qGray(p_image.pixel(x + kernelX, y + kernelY));
                }
            }
            UpdateGradientImage(p_gradientImage, observeMatrix, x, y);
        }
    }
}

void EdgeDetector::UpdateGradientImage(QImage& p_gradientImage, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY)
{
    QImage gradientImage;

    int convoluteX = ConvoluteMatrices(p_ObserveMatrix, SobelKernelMatrixX);
    int convoluteY = ConvoluteMatrices(p_ObserveMatrix, SobelKernelMatrixY);

    double sum = sqrt(pow(static_cast<double>(convoluteX), 2) + pow(static_cast<double>(convoluteY), 2));

    p_gradientImage.setPixel(p_ImageX, p_ImageY, qRgb(sum, sum, sum));
}

int EdgeDetector::ConvoluteMatrices(const int p_ObserveMatrix[KernelSize][KernelSize], const int p_KernelMatrix[KernelSize][KernelSize])
{
    int sum = 0;
    for (int x = 0; x < KernelSize; x++)
    {
        for (int y = 0; y < KernelSize; y++)
        {
            sum += p_KernelMatrix[KernelSize - x - 1][KernelSize - y - 1] * p_ObserveMatrix[x][y];
        }
    }

    if (sum > 255)
    {
        sum = 255;
    }
    else if (sum < 100)
    {
        sum = 0;
    }
    return sum;
}