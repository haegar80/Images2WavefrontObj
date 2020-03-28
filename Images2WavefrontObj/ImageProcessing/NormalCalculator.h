#pragma once

#include "../WavefrontObject/Mesh.h"
#include <memory>
#include <vector>

class QVector3D;

class NormalCalculator
{
public:
    NormalCalculator() = default;
    virtual ~NormalCalculator() = default;
    NormalCalculator(const NormalCalculator&) = delete;
    NormalCalculator& operator=(const NormalCalculator&) = delete;

    void CalculateNormals(std::vector<std::unique_ptr<Mesh>>& p_meshes);

private:
    void CalculateAndAddNormalVector(Mesh* p_mesh, SubMesh* p_submesh);
};

