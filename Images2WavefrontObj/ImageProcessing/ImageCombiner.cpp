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

            QSize tempImageSize = QSize(OverlapPixels, OverlapPixels);
            QImage tempImage(tempImageSize, QImage::Format_RGB32);
            for (int ii = m_overlapXStartFinalImage; ii < (m_overlapXStartFinalImage + OverlapPixels); ii++) {
                for (int jj = m_overlapYStartFinalImage; jj < (m_overlapYStartFinalImage + OverlapPixels); jj++) {
                    int gray = qGray(finalImage.pixel(ii, jj));
                    tempImage.setPixel(ii - m_overlapXStartFinalImage, jj - m_overlapYStartFinalImage, QColor(gray, gray, gray).rgb());
                }
            }
            finalImage = std::move(tempImage);
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
    bool overlapFound = FindOverlap(p_finalImage, p_extensionImage);
    
    //p_finalImage = gradientFinalImage;

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
                    observeMatrix[indexX][indexY] = qGray(p_Image.pixel(x + kernelX, y + kernelY));
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

bool ImageCombiner::FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    bool overlapFound = false;

    for (int xFinalImageIndex = 1; xFinalImageIndex < (p_finalImage.width() - OverlapPixels); xFinalImageIndex += SearchPixelInterval)
    {
        for (int yFinalImageIndex = 1; yFinalImageIndex < (p_finalImage.height() - OverlapPixels); yFinalImageIndex += SearchPixelInterval)
        {
            overlapFound = FindOverlap(p_finalImage, p_extensionImage, xFinalImageIndex, yFinalImageIndex);
            if (overlapFound)
            {
                m_overlapXStartFinalImage = xFinalImageIndex;
                m_overlapYStartFinalImage = yFinalImageIndex;
                break;
            }
        }
        if (overlapFound)
        {
            break;
        }
    }

    return overlapFound;
}

bool ImageCombiner::FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage, int p_startX, int p_startY)
{
    bool overlapFound = false;

    int expectedPixels[OverlapPixels][OverlapPixels] = { {} };

    bool xUpdated = false;

    for (int xFinalImageIndex = p_startX; xFinalImageIndex < p_startX + OverlapPixels; xFinalImageIndex++)
    {
        for (int yFinalImageIndex = p_startY; yFinalImageIndex < p_startY + OverlapPixels; yFinalImageIndex++)
        {
            QRgb finalImagePixelRGB = p_finalImage.pixel(xFinalImageIndex, yFinalImageIndex);
            int finalImagePixelGray = qGray(finalImagePixelRGB);
         
            expectedPixels[xFinalImageIndex - p_startX][yFinalImageIndex - p_startY] = finalImagePixelGray;
        }
    }

    overlapFound = FindOverlapPixels(p_extensionImage, expectedPixels);

    return overlapFound;
}

bool ImageCombiner::FindOverlapPixels(const QImage& p_image, const int p_expectedPixels[OverlapPixels][OverlapPixels])
{
    bool overlapFound = false;
    int x = 1;
    int y = 1;
    int overlapCounter = 0;
    int overlapGap = 0;
    int lastSuccessfulOverlapX = 0;

    do {
        QRgb imagePixelRGB = p_image.pixel(x, y);
        int imagePixelGray = qGray(imagePixelRGB);

        overlapFound = HasSameGrayValue(imagePixelGray, p_expectedPixels[overlapCounter % OverlapPixels][overlapCounter / OverlapPixels]);
        if (overlapFound)
        {
            overlapCounter++;
        }
        else
        {
            if (overlapCounter > 0)
            {
                overlapGap++;
                if (overlapGap > AllowedOverlapGap)
                {
                    overlapCounter = 0;
                }
            }
            if (lastSuccessfulOverlapX > 0)
            {
                x = lastSuccessfulOverlapX;
                lastSuccessfulOverlapX = 0;
            }
        }
        x++;

        if ((overlapCounter > 0) && (0 == (overlapCounter % OverlapPixels)))
        {
            lastSuccessfulOverlapX = x;
            x -= OverlapPixels;
            if (x < 0)
            {
                x = 0;
            }
            y++;
        }
        if (x >= p_image.width())
        {
            x = 0;
            y++;
        }
    } while (overlapCounter < (OverlapPixels * OverlapPixels) && (y < p_image.height()));

    if ((OverlapPixels * OverlapPixels) == overlapCounter)
    {
        m_overlapXStartExtensionImage = lastSuccessfulOverlapX - OverlapPixels;
        m_overlapYStartExtensionImage = y - OverlapPixels;
        overlapFound = true;
    }

    return overlapFound;
}

bool ImageCombiner::HasSameGrayValue(int p_firstValue, int p_secondValue)
{
    bool hasSameGrayValue = false;

    if (p_firstValue > (p_secondValue - AllowedGrayScaleDifference) && p_firstValue < (p_secondValue + AllowedGrayScaleDifference))
    {
        hasSameGrayValue = true;
    }

    return hasSameGrayValue;
}