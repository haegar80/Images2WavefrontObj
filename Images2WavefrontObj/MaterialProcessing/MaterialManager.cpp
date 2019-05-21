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

void MaterialManager::CreateMaterialsBasedOnTextures(std::map<FaceKey, std::string>& p_texturePaths)
{
    int materialNumber = m_materials.size();
    for (std::pair<FaceKey, std::string> texturePath : p_texturePaths)
    {
        std::stringstream materialNameString;
        materialNameString << "material_" << materialNumber;
        Material* material = new Material(materialNameString.str().c_str());
        MaterialRGBValue rgbValue;
        rgbValue.R = 1.0;
        rgbValue.G = 1.0;
        rgbValue.B = 1.0;
        material->setDiffuseColor(rgbValue);
        material->setDiffuseTexturePath(texturePath.second);

        m_materials.push_back(material);
        m_pendingUpdateMaterialsInMesh.insert(std::make_pair(texturePath.first, material));
        materialNumber++;
    }
}

void MaterialManager::UpdateMaterialsInMesh(Mesh* p_mesh)
{
    int lastCheckedSubmeshVectorIndex = 0;
    int numberOfMovedFaces = 0;
    for (std::pair<FaceKey, Material*> materialToUpdate : m_pendingUpdateMaterialsInMesh)
    {
        int submeshVectorIndex = materialToUpdate.first.first;
        if (lastCheckedSubmeshVectorIndex != submeshVectorIndex)
        {
            lastCheckedSubmeshVectorIndex = submeshVectorIndex;
            numberOfMovedFaces = 0;
        }
        int faceVectorIndex = materialToUpdate.first.second - numberOfMovedFaces;

        int vertexIndex = GetVertexIndex(p_mesh, submeshVectorIndex, faceVectorIndex);
        p_mesh->MoveFace(submeshVectorIndex, faceVectorIndex, materialToUpdate.second);
        numberOfMovedFaces++;

        // Check if further faces has same vertex index
        while (HasMoreFacesWithSameVertexIndex(p_mesh, submeshVectorIndex, faceVectorIndex, vertexIndex))
        {
            p_mesh->MoveFace(submeshVectorIndex, faceVectorIndex, materialToUpdate.second);
            numberOfMovedFaces++;
        }

        p_mesh->DeleteEmptySubmesh(submeshVectorIndex);
    }
    m_pendingUpdateMaterialsInMesh.clear();
}

int MaterialManager::GetVertexIndex(Mesh* p_mesh, int p_submeshVectorIndex, int p_faceVectorIndex)
{
    SubMesh* submesh = p_mesh->GetSubmeshes().at(p_submeshVectorIndex);
    std::vector<ObjFace> faces = submesh->GetFaces();

    int vertexIndex = 0;
    if (faces.size() > p_faceVectorIndex)
    {
        ObjFace face = faces.at(p_faceVectorIndex);
        vertexIndex = face.Indices.at(0).VertexIndex;
    }
    
    return vertexIndex;
}

bool MaterialManager::HasMoreFacesWithSameVertexIndex(Mesh* p_mesh, int p_submeshVectorIndex, int p_faceVectorIndex, int p_vertexIndex)
{
    bool hasMoreFaces = false;

    int vertexIndex = GetVertexIndex(p_mesh, p_submeshVectorIndex, p_faceVectorIndex);
    if ((vertexIndex > 0) && (vertexIndex == p_vertexIndex))
    {
        hasMoreFaces = true;
    }
    return hasMoreFaces;
}