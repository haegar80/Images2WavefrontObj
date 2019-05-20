#include "MaterialManager.h"
#include "../WavefrontObject/Material.h"
#include "../WavefrontObject/Mesh.h"
#include <sstream>

MaterialManager::MaterialManager()
{
}

void MaterialManager::CreateDefaultMaterial()
{
    Material material("DummyMaterial");
    MaterialRGBValue rgbValue;
    rgbValue.R = 1.0;
    rgbValue.G = 1.0;
    rgbValue.B = 1.0;
    material.setDiffuseColor(rgbValue);
    m_materials.push_back(&material);
}

void MaterialManager::CreateMaterialsBasedOnTextures(std::map<FaceKey, std::string>& p_texturePaths)
{
    int texturePathNumber = 1;
    for (std::pair<FaceKey, std::string> texturePath : p_texturePaths)
    {
        std::stringstream materialNameString;
        materialNameString << "material_" << texturePathNumber;
        Material material(materialNameString.str().c_str());
        MaterialRGBValue rgbValue;
        rgbValue.R = 1.0;
        rgbValue.G = 1.0;
        rgbValue.B = 1.0;
        material.setDiffuseColor(rgbValue);
        material.setDiffuseTexturePath(texturePath.second);

        m_materials.push_back(&material);
        m_pendingUpdateMaterialsInMesh.insert(std::make_pair(texturePath.first, &material));
    }
    texturePathNumber++;
}

void MaterialManager::UpdateMaterialsInMesh(Mesh* p_mesh)
{
    for (std::pair<FaceKey, Material*> materialToUpdate : m_pendingUpdateMaterialsInMesh)
    {
        p_mesh->MoveFace(materialToUpdate.first.first, materialToUpdate.first.second, materialToUpdate.second);
    }
}