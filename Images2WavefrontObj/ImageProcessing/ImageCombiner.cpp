#include "ImageCombiner.h"
#include <QStringList>
#include <QImageReader>

ImageCombiner::ImageCombiner()
{
}

QImage ImageCombiner::CombineImages(const QStringList& p_images)
{
    QImage finalImage;
    bool successful = true;
    bool firstImage = true;
    for (QString imageString : p_images)
    {
        QImageReader imageReader(imageString);
        imageReader.setAutoTransform(true);
        QImage tempImage = imageReader.read();
        if (tempImage.isNull())
        {
            successful = false;
        }
        else if (firstImage)
        {
            firstImage = false;
            finalImage = std::move(tempImage);
        }
        else
        {
            successful = CombineTwoImages(finalImage, tempImage);
        }

        if (!successful)
        {
            break;
        }
    }

    if (!successful)
    {
        finalImage = QImage();
    }
    return finalImage;
}

bool ImageCombiner::CombineTwoImages(QImage& p_finalImage, const QImage& p_extensionImage)
{
    bool successful = true;

    int observeMatrix[KernelSize][KernelSize];
    QSize imageSize = QSize(p_finalImage.width(), p_finalImage.height());
    QImage gradientImageX(imageSize, QImage::Format_RGB32);
    QImage gradientImageY(imageSize, QImage::Format_RGB32);

    for (int x = KernelNeighbour; x < (p_finalImage.width() - KernelNeighbour - 1); x++)
    {
        for (int y = KernelNeighbour; y < (p_finalImage.height() - KernelNeighbour -1); y++)
        {
            for (int kernelX = -KernelNeighbour; kernelX <= KernelNeighbour; kernelX++)
            {
                for (int kernelY = -KernelNeighbour; kernelY <= KernelNeighbour; kernelY++)
                {
                    int indexX = kernelX + KernelNeighbour;
                    int indexY = kernelY + KernelNeighbour;
                    observeMatrix[indexX][indexY] = qGray(p_finalImage.pixel(x + indexX, y + indexY));
                }
            }
            UpdateGradientImage(gradientImageX, gradientImageY, observeMatrix, x, y);
        }
    }
    
    p_finalImage = gradientImageX;

    return successful;
}

void ImageCombiner::UpdateGradientImage(QImage& p_gradientImageX, QImage& p_gradientImageY, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY)
{
    int convoluteX = ConvoluteMatrices(p_ObserveMatrix, SobelKernelMatrixX);
    int convoluteY = ConvoluteMatrices(p_ObserveMatrix, SobelKernelMatrixY);
    
    double sum = sqrt(pow(static_cast<double>(convoluteX), 2) + pow(static_cast<double>(convoluteY), 2));

    p_gradientImageX.setPixel(p_ImageX, p_ImageY, qRgb(sum, sum, sum));
    p_gradientImageY.setPixel(p_ImageX, p_ImageY, qRgb(convoluteY, convoluteY, convoluteY));
}

int ImageCombiner::ConvoluteMatrices(const int p_ObserveMatrix[KernelSize][KernelSize], const int p_KernelMatrix[KernelSize][KernelSize])
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