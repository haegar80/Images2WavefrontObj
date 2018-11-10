#pragma once

#include "Material.h"
#include <string>
#include <vector>

class MaterialWriter
{
public:
    MaterialWriter();
    virtual ~MaterialWriter() = default;

    void WriteMaterials(const std::string& p_dirPath, const std::string& p_fileName, const std::vector<Material*> p_materials);

private:
    std::vector<Material*> m_materials;

    void WriteMaterial(std::ofstream& p_mtlFile, const Material* p_material);
};

