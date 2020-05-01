#pragma once

#include <memory>
#include <vector>

class Mesh;

class VertexAdjuster
{
public:
    VertexAdjuster() = default;
    virtual ~VertexAdjuster() = default;
    VertexAdjuster(const VertexAdjuster&) = delete;
    VertexAdjuster& operator=(const VertexAdjuster&) = delete;

    void NormVertices(std::vector<std::unique_ptr<Mesh>>& p_meshes, int p_originalImageWidth, int p_originalImageHeight);
};

