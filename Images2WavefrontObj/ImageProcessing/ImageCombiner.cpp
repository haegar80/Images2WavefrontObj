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
    m_overlapXStartPixelsFinalImage.clear();
    m_overlapYStartPixelsFinalImage.clear();
    m_overlapXStartPixelsExtensionImage.clear();
    m_overlapYStartPixelsExtensionImage.clear();

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
    int correctOverlapIndex = FindCorrectOneOfOverlapCandidates(p_finalImage, p_extensionImage);
    if (OverlapNotFound == correctOverlapIndex)
    {
        successful = false;
    }

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

void ImageCombiner::AnalyzeTop(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    bool overlapFound = false;

    int ConstantY = ImageBorderPixels;
    for (int xFinalImageIndex = ImageBorderPixels; xFinalImageIndex < (p_finalImage.width() - ImageBorderPixels - OverlapPixels); xFinalImageIndex += SearchPixelInterval)
    {
        overlapFound = FindOverlap(p_finalImage, p_extensionImage, xFinalImageIndex, ConstantY);
        if (overlapFound)
        {
            m_overlapXStartPixelsFinalImage.push_back(xFinalImageIndex);
            m_overlapYStartPixelsFinalImage.push_back(ConstantY);
        }
    }
}

void ImageCombiner::AnalyzeLeft(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    int ConstantX = ImageBorderPixels;
    for (int yFinalImageIndex = ImageBorderPixels + OverlapPixels; yFinalImageIndex < (p_finalImage.height() - ImageBorderPixels - OverlapPixels); yFinalImageIndex += SearchPixelInterval)
    {
        bool overlapFound = FindOverlap(p_finalImage, p_extensionImage, ConstantX, yFinalImageIndex);
        if (overlapFound)
        {
            m_overlapXStartPixelsFinalImage.push_back(ConstantX);
            m_overlapYStartPixelsFinalImage.push_back(yFinalImageIndex);
        }
    }
}

void ImageCombiner::AnalyzeRight(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    int ConstantX = p_finalImage.width() - ImageBorderPixels - OverlapPixels;
    for (int yFinalImageIndex = ImageBorderPixels + OverlapPixels; yFinalImageIndex < (p_finalImage.height() - ImageBorderPixels - OverlapPixels); yFinalImageIndex += SearchPixelInterval)
    {
        bool overlapFound = FindOverlap(p_finalImage, p_extensionImage, ConstantX, yFinalImageIndex);
        if (overlapFound)
        {
            m_overlapXStartPixelsFinalImage.push_back(ConstantX);
            m_overlapYStartPixelsFinalImage.push_back(yFinalImageIndex);
        }
    }
}

void ImageCombiner::AnalyzeBottom(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    int ConstantY = p_finalImage.height() - ImageBorderPixels - OverlapPixels;
    for (int xFinalImageIndex = ImageBorderPixels; xFinalImageIndex < (p_finalImage.width() - ImageBorderPixels - OverlapPixels); xFinalImageIndex += SearchPixelInterval)
    {
       bool overlapFound = FindOverlap(p_finalImage, p_extensionImage, xFinalImageIndex, ConstantY);
       if (overlapFound)
       {
           m_overlapXStartPixelsFinalImage.push_back(xFinalImageIndex);
           m_overlapYStartPixelsFinalImage.push_back(ConstantY);
       }
    }
}

bool ImageCombiner::FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    bool overlapFound = false;

    AnalyzeTop(p_finalImage, p_extensionImage);
    AnalyzeLeft(p_finalImage, p_extensionImage);
    AnalyzeRight(p_finalImage, p_extensionImage);
    AnalyzeBottom(p_finalImage, p_extensionImage);

    if (m_overlapXStartPixelsFinalImage.size() > 0)
    {
        overlapFound = true;
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
    int x = ImageBorderPixels;
    int y = ImageBorderPixels;
    int overlapCounter = 0;
    int overlapGap = 0;
    int lastSuccessfulOverlapX = 0;
    int MaxImageX = p_image.width() - ImageBorderPixels;
    int MaxImageY = p_image.height() - ImageBorderPixels;
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
        }
        x++;

        if ((overlapCounter > 0) && (0 == (overlapCounter % OverlapPixels)))
        {
            x -= OverlapPixels;
            lastSuccessfulOverlapX = x;
            y++;
        }
        if (x >= MaxImageX)
        {
            // Fill overlap counter to number of OverlapPixels in x-direction
            if (overlapCounter > MinOverlapPixelsAtBorder)
            {
                x -= (overlapCounter % OverlapPixels);
                lastSuccessfulOverlapX = x;
                overlapCounter += (OverlapPixels - (overlapCounter % OverlapPixels));
                
            }
            x = 0;
            y++;
        }
    } while (overlapCounter < (OverlapPixels * OverlapPixels) && (y < MaxImageY));

    if (((OverlapPixels * OverlapPixels) == overlapCounter) || (MaxImageY == y && overlapCounter > MinOverlapPixelsAtBorder))
    {
        m_overlapXStartPixelsExtensionImage.push_back(lastSuccessfulOverlapX);
        m_overlapYStartPixelsExtensionImage.push_back(y - (overlapCounter / OverlapPixels));
        overlapFound = true;
    }
    else
    {
        overlapFound = false;
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

int ImageCombiner::FindCorrectOneOfOverlapCandidates(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    int correctIndex = FindCorrectOneOfOverlapCandidatesX(p_finalImage, p_extensionImage);
    if (OverlapNotFound != correctIndex)
    {
        correctIndex = FindCorrectOneOfOverlapCandidatesY(p_finalImage, p_extensionImage);
    }

    return correctIndex;
}

int ImageCombiner::FindCorrectOneOfOverlapCandidatesX(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    int correctIndex = OverlapNotFound;
    bool overlapSuccessful = false;

    for (int i = 0; i < m_overlapXStartPixelsFinalImage.size(); i++)
    {
        int currentPixelXFinalImage = m_overlapXStartPixelsFinalImage.at(i) + OverlapPixels;
        int currentPixelXExtensionImage = m_overlapXStartPixelsExtensionImage.at(i) + OverlapPixels;
        int currentPixelYFinalImage = m_overlapYStartPixelsFinalImage.at(i);
        int currentPixelYExtensionImage = m_overlapYStartPixelsExtensionImage.at(i);

        while (currentPixelXFinalImage < (p_finalImage.width() - ImageBorderPixels) && currentPixelXExtensionImage < (p_extensionImage.width() - ImageBorderPixels))
        {
            QRgb finalImagePixelRGB = p_finalImage.pixel(currentPixelXFinalImage, currentPixelYFinalImage);
            int finalImagePixelGray = qGray(finalImagePixelRGB);
            QRgb extensionImagePixelRGB = p_extensionImage.pixel(currentPixelXExtensionImage, currentPixelYExtensionImage);
            int extensionImagePixelGray = qGray(extensionImagePixelRGB);

            if (HasSameGrayValue(finalImagePixelGray, extensionImagePixelGray))
            {
                overlapSuccessful = true;
            }
            else
            {
                overlapSuccessful = false;
                break;
            }
            currentPixelXFinalImage++;
            currentPixelXExtensionImage++;
        };

        if (overlapSuccessful)
        {
            correctIndex = i;
            break;
        }
    }

    return correctIndex;
}

int ImageCombiner::FindCorrectOneOfOverlapCandidatesY(const QImage& p_finalImage, const QImage& p_extensionImage)
{
    int correctIndex = OverlapNotFound;
    bool overlapSuccessful = false;

    for (int i = 0; i < m_overlapXStartPixelsFinalImage.size(); i++)
    {
        int currentPixelXFinalImage = m_overlapXStartPixelsFinalImage.at(i);
        int currentPixelXExtensionImage = m_overlapXStartPixelsExtensionImage.at(i);
        int currentPixelYFinalImage = m_overlapYStartPixelsFinalImage.at(i) + OverlapPixels;
        int currentPixelYExtensionImage = m_overlapYStartPixelsExtensionImage.at(i) + OverlapPixels;

        while (currentPixelYFinalImage < (p_finalImage.height() - ImageBorderPixels) && currentPixelYExtensionImage < (p_extensionImage.height() - ImageBorderPixels))
        {
            QRgb finalImagePixelRGB = p_finalImage.pixel(currentPixelXFinalImage, currentPixelYFinalImage);
            int finalImagePixelGray = qGray(finalImagePixelRGB);
            QRgb extensionImagePixelRGB = p_extensionImage.pixel(currentPixelXExtensionImage, currentPixelYExtensionImage);
            int extensionImagePixelGray = qGray(extensionImagePixelRGB);

            if (HasSameGrayValue(finalImagePixelGray, extensionImagePixelGray))
            {
                overlapSuccessful = true;
            }
            else
            {
                overlapSuccessful = false;
                break;
            }
            currentPixelYFinalImage++;
            currentPixelYExtensionImage++;
        };

        if (overlapSuccessful)
        {
            correctIndex = i;
            break;
        }
    }

    return correctIndex;
}