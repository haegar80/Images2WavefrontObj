#pragma once

#include <QImage>
#include <QGenericMatrix>

class QStringList;

/// author: Copyright by Marcel Fuchs
class ImageCombiner
{
public:
    ImageCombiner();
    virtual ~ImageCombiner() = default;

    QImage CombineImages(const QStringList& p_images);

private:
    static constexpr int OverlapPixels = 150;
    static constexpr int MinOverlapPixelsAtBorder = 50;
    static constexpr int SearchPixelInterval = 50;
    static constexpr int AllowedOverlapGap = 10;
    static constexpr int AllowedGrayScaleDifference = 10;
    static constexpr int ImageBorderPixels = 1;
    static constexpr int OverlapNotFound = -1;

    int m_overlapXStartFinalImage{ 0 };
    int m_overlapYStartFinalImage{ 0 };
    int m_overlapXStartExtensionImage{ 0 };
    int m_overlapYStartExtensionImage{ 0 };
    std::vector<int> m_overlapXStartPixelsFinalImage{};
    std::vector<int> m_overlapYStartPixelsFinalImage{};
    std::vector<int> m_overlapXStartPixelsExtensionImage{};
    std::vector<int> m_overlapYStartPixelsExtensionImage{};

    bool CombineTwoImages(QImage& p_finalImage, const QImage& p_extensionImage);
    void AnalyzeTop(const QImage& p_finalImage, const QImage& p_extensionImage);
    void AnalyzeLeft(const QImage& p_finalImage, const QImage& p_extensionImage);
    void AnalyzeRight(const QImage& p_finalImage, const QImage& p_extensionImage);
    void AnalyzeBottom(const QImage& p_finalImage, const QImage& p_extensionImage);
    bool FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage);
    bool FindOverlap(const QImage& p_finalImage, const QImage& p_extensionImage, int p_startX, int p_startY);
    bool FindOverlapPixels(const QImage& p_image, const int p_expectedPixels[OverlapPixels][OverlapPixels]);
    bool HasSameGrayValue(int p_firstValue, int p_secondValue);

    int FindCorrectOneOfOverlapCandidates(const QImage& p_finalImage, const QImage& p_extensionImage);
    int FindCorrectOneOfOverlapCandidatesX(const QImage& p_finalImage, const QImage& p_extensionImage);
    int FindCorrectOneOfOverlapCandidatesY(const QImage& p_finalImage, const QImage& p_extensionImage);
};

