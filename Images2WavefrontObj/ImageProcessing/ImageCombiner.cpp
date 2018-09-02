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

    QSize finalImageSize = QSize(p_finalImage.width(), p_finalImage.height());
    QImage gradientFinalImage(finalImageSize, QImage::Format_RGB32);
    GetGradientImage(p_finalImage, gradientFinalImage);
    
    QSize extensionImageSize = QSize(p_extensionImage.width(), p_extensionImage.height());
    QImage gradientExtensionImage(extensionImageSize, QImage::Format_RGB32);
    GetGradientImage(p_extensionImage, gradientExtensionImage);
    AnalyzeLeftSide(p_finalImage, p_extensionImage, gradientFinalImage, gradientExtensionImage);
    
    p_finalImage = gradientFinalImage;

    return successful;
}

void ImageCombiner::GetGradientImage(const QImage& p_Image, QImage& p_gradientImage)
{
    int observeMatrix[KernelSize][KernelSize];
    for (int x = KernelNeighbour; x < (p_Image.width() - KernelNeighbour - 1); x++)
    {
        for (int y = KernelNeighbour; y < (p_Image.height() - KernelNeighbour - 1); y++)
        {
            for (int kernelX = -KernelNeighbour; kernelX <= KernelNeighbour; kernelX++)
            {
                for (int kernelY = -KernelNeighbour; kernelY <= KernelNeighbour; kernelY++)
                {
                    int indexX = kernelX + KernelNeighbour;
                    int indexY = kernelY + KernelNeighbour;
                    observeMatrix[indexX][indexY] = qGray(p_Image.pixel(x + indexX, y + indexY));
                }
            }
            UpdateGradientImage(p_gradientImage, observeMatrix, x, y);
        }
    }
}

void ImageCombiner::UpdateGradientImage(QImage& p_gradientImage, int p_ObserveMatrix[KernelSize][KernelSize], int p_ImageX, int p_ImageY)
{
    int convoluteX = ConvoluteMatrices(p_ObserveMatrix, SobelKernelMatrixX);
    int convoluteY = ConvoluteMatrices(p_ObserveMatrix, SobelKernelMatrixY);
    
    double sum = sqrt(pow(static_cast<double>(convoluteX), 2) + pow(static_cast<double>(convoluteY), 2));

    p_gradientImage.setPixel(p_ImageX, p_ImageY, qRgb(sum, sum, sum));
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

void ImageCombiner::AnalyzeLeftSide(QImage& p_finalImage, const QImage& p_extensionImage, const QImage& p_gradientFinalImage, const QImage& p_gradientExtensionImage)
{
    for (int y = 0; (y + OverlapPixels) < p_gradientFinalImage.height(); y += OverlapPixels)
    {
        bool overlapFound = FindOverlap(p_gradientFinalImage, p_gradientExtensionImage, 0, y);

        if (overlapFound)
        {
            break;
        }
    }

}

bool ImageCombiner::FindOverlap(const QImage& p_gradientFinalImage, const QImage& p_gradientExtensionImage, int p_startX, int p_startY)
{
    bool overlapFound = false;

    QRgb xExpectedPixels[OverlapPixels] = {};
    QRgb yExpectedPixels[OverlapPixels] = {};
    bool xUpdated = false;

    for (int xFinalImageIndex = p_startX; xFinalImageIndex < p_startX + OverlapPixels; xFinalImageIndex++)
    {
        for (int yFinalImageIndex = p_startY; yFinalImageIndex < p_startY + OverlapPixels; yFinalImageIndex++)
        {
            QRgb finalImagePixel = p_gradientFinalImage.pixel(xFinalImageIndex, yFinalImageIndex);
         
            if (!xUpdated)
            {
                xExpectedPixels[xFinalImageIndex - p_startX] = finalImagePixel;
                xUpdated = true;
            }
            yExpectedPixels[yFinalImageIndex - p_startY] = finalImagePixel;
        }
        xUpdated = false;
    }

    overlapFound = FindOverlapPixels(p_gradientExtensionImage, xExpectedPixels, yExpectedPixels);

    return overlapFound;
}

bool ImageCombiner::FindOverlapPixels(const QImage& p_image, QRgb* p_xExpectedPixels, QRgb* p_yExpectedPixels)
{
    bool overlapFound = false;

    int y = 0;
    for (int x = 0; x < p_image.width() - OverlapPixels; x++)
    {
        for (; y < p_image.height() - OverlapPixels; y++)
        {
            int yExpectedPixelIndex = 0;
            do {
                QRgb imagePixel = p_image.pixel(x, y + yExpectedPixelIndex);
                overlapFound = HasSameGrayValue(imagePixel, p_yExpectedPixels[yExpectedPixelIndex]);
                yExpectedPixelIndex++;
            } while (overlapFound || yExpectedPixelIndex < OverlapPixels);
            
            if (overlapFound)
            {
                break;
            }
        }

        if (overlapFound)
        {
            int xExpectedPixelIndex = 0;
            do {
                QRgb imagePixel = p_image.pixel(x + xExpectedPixelIndex, y);
                overlapFound = HasSameGrayValue(imagePixel, p_xExpectedPixels[xExpectedPixelIndex]);
                xExpectedPixelIndex++;
            } while (overlapFound || xExpectedPixelIndex < OverlapPixels);

            if (overlapFound)
            {
                m_overlapXStart = x;
                m_overlapYStart = y;
                break;
            }
        }
    }

    return overlapFound;
}

bool ImageCombiner::HasSameGrayValue(QRgb p_firstValue, QRgb p_secondValue)
{
    bool hasSameGrayValue = false;

    if (p_firstValue > (p_secondValue - AllowedGrayScaleDifference) && p_firstValue < (p_secondValue + AllowedGrayScaleDifference))
    {
        hasSameGrayValue = true;
    }

    return hasSameGrayValue;
}