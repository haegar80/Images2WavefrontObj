#pragma once

#include "Material.h"
#include <string>
#include <vector>

class MaterialWriter
{
public:
    MaterialWriter(const std::string& p_dirPath, const std::string& p_fileName);
    virtual ~MaterialWriter() = default;

    void WriteMaterials(const std::vector<Material*> p_materials);

private:
    std::string m_currentDirPath{ "" };
    std::string m_currentFileName{ "" };
    std::vector<Material*> m_materials;

    void WriteMaterial(std::ofstream& p_mtlFile, const Material* p_material);
};

