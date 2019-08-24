#include "EdgeDetector.h"

EdgeDetector::EdgeDetector()
{
}

QImage EdgeDetector::DetectEdges(const QImage& p_image, int p_minimumConvolutionSum)
{
    m_minimumConvolutionSum = p_minimumConvolutionSum;
    m_maximumSobelResult = FindMaxResult();

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

void EdgeDetector::UpdateGradientImage(QImage& p_gradientImage, int p_observeMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY)
{
    QImage gradientImage;

    int convoluteX = ConvoluteMatrices(p_observeMatrix, SobelKernelMatrixX);
    int convoluteY = ConvoluteMatrices(p_observeMatrix, SobelKernelMatrixY);

    double sum = sqrt(pow(static_cast<double>(convoluteX), 2) + pow(static_cast<double>(convoluteY), 2));

    int normalizedSum = static_cast<int>(255 * (sum / m_maximumSobelResult));
    if (normalizedSum < m_minimumConvolutionSum)
    {
        normalizedSum = 0;
    }

    p_gradientImage.setPixel(p_ImageX, p_ImageY, qRgb(normalizedSum, normalizedSum, normalizedSum));
}

int EdgeDetector::ConvoluteMatrices(const int p_observeMatrix[KernelSize][KernelSize], const int p_kernelMatrix[KernelSize][KernelSize])
{
    int sum = 0;
    for (int x = 0; x < KernelSize; x++)
    {
        for (int y = 0; y < KernelSize; y++)
        {
            sum += p_kernelMatrix[KernelSize - x - 1][KernelSize - y - 1] * p_observeMatrix[x][y];
        }
    }

    return sum;
}

double EdgeDetector::FindMaxResult()
{
    int observeMatrixMax[KernelSize][KernelSize] = { { 0, 0, 0 },{ 128, 128, 128 },{ 255, 255, 255 } };

    int resultX = ConvoluteMatrices(observeMatrixMax, SobelKernelMatrixX);
    int resultY = ConvoluteMatrices(observeMatrixMax, SobelKernelMatrixY);

    double result = sqrt(pow(static_cast<double>(resultX), 2) + pow(static_cast<double>(resultY), 2));
    return result;
}