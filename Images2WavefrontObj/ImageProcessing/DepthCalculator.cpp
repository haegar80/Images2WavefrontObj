#include "DepthCalculator.h"
#include "../WavefrontObject/Mesh.h"

void DepthCalculator::CalculateDepths(int p_imageWidth, int p_imageHeight, std::vector<std::unique_ptr<Mesh>>& p_meshes)
{
    for (std::unique_ptr<Mesh>& mesh : p_meshes)
    {
        if (mesh)
        {
            std::vector<ObjVertexCoords> vertices = mesh.get()->GetVertices();
            for(int i = 0; i < vertices.size(); i++)
            {
                int pixelX = vertices.at(i).X;
                int pixelY = vertices.at(i).Y;
                int pixelZ = CalculateDepth(p_imageWidth, p_imageHeight, pixelX, pixelY);
                mesh.get()->UpdateVertex(i, pixelX, pixelY, pixelZ);
            }
        }
    }
}

int DepthCalculator::CalculateDepth(int p_imageWidth, int p_imageHeight, int p_pixelX, int p_pixelY)
{
    int depthValue = -1;

    // Binocular disparity: We use the same image twice, on the left and on the right. Then we see the difference in X-axis.
    // The bigger the difference, the nearer a object will be the shown on the screen
    int x1 = p_pixelX;
    int x2 = p_imageWidth - p_pixelX;
 
    int disparity = x1 - x2;
    if (0 == disparity)
    {
        disparity = 1;
    }
    constexpr int minYOffset = 1; // Avoid 0
    depthValue = static_cast<int>((static_cast<double>(p_imageWidth) / disparity)*(p_imageHeight - p_pixelY + minYOffset));
    
    if (depthValue > 0)
    {
        depthValue *= -1;
    }
    else if (0 == depthValue)
    {
        depthValue = NearestZPixel;
    }

    return depthValue;
}

/**
int DepthCalculator::GetZPixel(int p_imageWidth, int p_imageHeight, int p_depth)
{
    constexpr int MinDepth = 0;
    int maxDepth = CalculateDepth(p_imageWidth, p_imageHeight, (p_imageWidth / 2), 0);
  
    int maxDepthDifference = maxDepth - MinDepth;
    float scaleFactor = static_cast<float>(p_depth) / maxDepthDifference;

    int maxZPixelDifference = FarestZPixel - NearestZPixel;
    int pixelZ = NearestZPixel + (static_cast<int>(scaleFactor) * maxZPixelDifference);
    
    return p_depth;
}
*/