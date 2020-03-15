#pragma once

#include <memory>
#include <vector>

class Mesh;

class DepthCalculator
{
public:
    DepthCalculator() = default;
    virtual ~DepthCalculator() = default;
    DepthCalculator(const DepthCalculator&) = delete;
    DepthCalculator& operator=(const DepthCalculator&) = delete;
    DepthCalculator(DepthCalculator&&) = default;
    DepthCalculator& operator=(DepthCalculator&&) = default;

    void CalculateDepths(int p_imageWidth, int p_imageHeight, std::vector<std::unique_ptr<Mesh>>& p_meshes);

private:
    static constexpr int NearestZPixel = -1;
    static constexpr int FarestZPixel = -100;

    int CalculateDepth(int p_imageWidth, int p_imageHeight, int p_pixelX, int p_pixelY);
    int GetZPixel(int p_imageWidth, int p_imageHeight, int p_depth);
};

