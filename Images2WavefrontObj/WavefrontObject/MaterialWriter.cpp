#include "MaterialWriter.h"
#include <iostream>
#include <fstream>
#include <sstream>

MaterialWriter::MaterialWriter()
{
}

void MaterialWriter::WriteMaterials(const std::string& p_dirPath, const std::string& p_fileName, const std::vector<Material*> p_materials)
{
    std::ofstream mtlFile;

    std::string filePath((p_dirPath + p_fileName + ".mtl"));
    mtlFile.open(filePath.c_str());
    if (!mtlFile) {
        std::cout << "File " << filePath.c_str() << " could not be opened!!" << std::endl;
        return;
    }

    for (Material* material : p_materials)
    {
        WriteMaterial(mtlFile, material);
    }

    mtlFile.close();
}

void MaterialWriter::WriteMaterial(std::ofstream& p_mtlFile, const Material* p_material)
{
    std::string& materialName(p_material->getName());
    if (0 != materialName.compare("default"))
    {
        p_mtlFile << "newmtl " << materialName.c_str() << "\n";
        p_mtlFile << "Ns " << p_material->getSpecularExponent();

        MaterialRGBValue rgbValue = p_material->getAmbientColor();
        p_mtlFile << "Ka " << rgbValue.R << " " << rgbValue.G << " " << rgbValue.B << "\n";

        rgbValue = p_material->getDiffuseColor();
        p_mtlFile << "Kd " << rgbValue.R << " " << rgbValue.G << " " << rgbValue.B << "\n";

        rgbValue = p_material->getSpecularColor();
        p_mtlFile << "Ks " << rgbValue.R << " " << rgbValue.G << " " << rgbValue.B << "\n";

        p_mtlFile << "map_Ka" << p_material->getAmbientTexturePath().c_str() << "\n";
        p_mtlFile << "map_Kd" << p_material->getDiffuseTexturePath().c_str() << "\n";
        p_mtlFile << "map_Ks" << p_material->getSpecularTexturePath().c_str() << "\n";
    }
}