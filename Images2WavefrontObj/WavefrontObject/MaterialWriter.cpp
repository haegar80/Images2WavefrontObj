#include "MaterialWriter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <QFileInfo>
#include <QDir>

MaterialWriter::MaterialWriter(const std::string& p_dirPath, const std::string& p_fileName) :
    m_currentDirPath(p_dirPath),
    m_currentFileName(p_fileName)
{
}

void MaterialWriter::WriteMaterials(const std::vector<Material*> p_materials)
{
    std::ofstream mtlFile;

    std::string fileNameTempString(m_currentFileName + ".mtl");
    QString currentFileName(fileNameTempString.c_str());

    QDir currentDirPath(QString(m_currentDirPath.c_str()));
    if (!currentDirPath.exists())
    {
        currentDirPath.mkpath(".");
    }

    QFileInfo file(currentDirPath, currentFileName);
    QString filePath = file.absoluteFilePath();
    mtlFile.open(filePath.toUtf8().constData());
    if (!mtlFile) {
        std::cout << "File " << filePath.toUtf8().constData() << " could not be opened!!" << std::endl;
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