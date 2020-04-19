#include "VertexAdjuster.h"
#include "DepthCalculator.h"
#include "../WavefrontObject/Mesh.h"

void VertexAdjuster::NormVertices(std::vector<std::unique_ptr<Mesh>>& p_meshes, int p_originalImageWidth, int p_p_originalImageHeight)
{
    for (std::unique_ptr<Mesh>& mesh : p_meshes)
    {
        if (mesh)
        {
            std::vector<ObjVertexCoords> vertices = mesh->GetVertices();
            for (int i = 0; i < vertices.size(); i++)
            {
                ObjVertexCoords& vertex = vertices.at(i);
                float normedX = vertex.X / p_originalImageWidth;
                float normedY = vertex.Y / p_p_originalImageHeight;
                float normedZ = vertex.Z / (DepthCalculator::GetZPixelFarest() - DepthCalculator::GetZPixelNearest()) * -1.0f;
                mesh->UpdateVertex(i, normedX, normedY, normedZ);
            }
        }
    }
}