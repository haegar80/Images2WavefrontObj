#include "MaterialManager.h"
#include "../WavefrontObject/Material.h"
#include "../WavefrontObject/Mesh.h"
#include <sstream>

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
    for (Material* material : m_materials)
    {
        delete material;
        material = nullptr;
    }
}

void MaterialManager::CreateDefaultMaterial()
{
    Material* material = new Material("DummyMaterial");
    MaterialRGBValue rgbValue;
    rgbValue.R = 1.0;
    rgbValue.G = 1.0;
    rgbValue.B = 1.0;
    material->setDiffuseColor(rgbValue);
    m_materials.push_back(material);
}

void MaterialManager::CreateMaterialsBasedOnTextures(std::map<std::string, std::vector<FaceKey>>& p_texturePaths)
{
    int materialNumber = m_materials.size();
    for (std::pair<std::string, std::vector<FaceKey>> texturePath : p_texturePaths)
    {
        std::stringstream materialNameString;
        materialNameString << "material_" << materialNumber;
        Material* material = new Material(materialNameString.str().c_str());
        MaterialRGBValue rgbValue;
        rgbValue.R = 1.0;
        rgbValue.G = 1.0;
        rgbValue.B = 1.0;
        material->setDiffuseColor(rgbValue);
        material->setDiffuseTexturePath(texturePath.first);

        m_materials.push_back(material);
        m_pendingUpdateMaterialsInMesh.insert(std::make_pair(material, texturePath.second));
        materialNumber++;
    }
}

void MaterialManager::UpdateMaterialsInMesh(Mesh* p_mesh)
{
    for (std::pair<Material*, std::vector<FaceKey>> materialToUpdate : m_pendingUpdateMaterialsInMesh)
    {
        for (FaceKey& faceKey : materialToUpdate.second)
        {
            p_mesh->MoveFace(faceKey.first, faceKey.second, materialToUpdate.first, false);
        }
    }

    // Assumption: For each mesh we had only one submesh with default material before calling this function
    constexpr int SubmeshIndex = 0;

    std::vector<SubMesh*>& submeshes = p_mesh->GetSubmeshes();
    std::vector<ObjFace> faces = submeshes.at(SubmeshIndex)->GetFaces();
    for (int faceIndex = faces.size() - 1; faceIndex >= 0; faceIndex--)
    {
        bool hasUpdatedSubmeshAndFace = HasUpdatedSubmeshAndFace(SubmeshIndex, faceIndex);
        if (hasUpdatedSubmeshAndFace)
        {
            SubMesh* subMesh = p_mesh->GetSubmeshes().at(SubmeshIndex);
            subMesh->DeleteFace(faceIndex);
        }
    }

    p_mesh->DeleteEmptySubmesh(SubmeshIndex);

    m_pendingUpdateMaterialsInMesh.clear();
}

bool MaterialManager::HasUpdatedSubmeshAndFace(int p_submeshVectorIndex, int p_faceVectorIndex)
{
    bool hasUpdatedSubmeshAndFace = false;

    for (std::pair<Material*, std::vector<FaceKey>> materialToUpdate : m_pendingUpdateMaterialsInMesh)
    {
        std::vector<FaceKey>& faceKeys = materialToUpdate.second;

        auto findFaceKey = std::find_if(faceKeys.begin(), faceKeys.end(), [p_submeshVectorIndex, p_faceVectorIndex](const FaceKey& faceKey) {return ((faceKey.first == p_submeshVectorIndex) && (faceKey.second == p_faceVectorIndex)); });
        if (faceKeys.end() != findFaceKey)
        {
            hasUpdatedSubmeshAndFace = true;
        }
    }

    return hasUpdatedSubmeshAndFace;
}