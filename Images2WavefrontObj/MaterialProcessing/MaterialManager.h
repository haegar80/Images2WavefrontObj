#pragma once

#include <string>
#include <vector>
#include <map>

class Material;
class Mesh;

typedef std::pair<int, int> FaceKey;

class MaterialManager
{
public:
    MaterialManager();
    virtual ~MaterialManager();

    void CreateDefaultMaterial();
    void CreateMaterialsBasedOnTextures(std::map<std::string, std::vector<FaceKey>>& p_texturePaths);
    void UpdateMaterialsInMesh(Mesh* p_mesh);
    std::vector<Material*>& GetMaterials()
    {
        return m_materials;
    }

private:
    std::vector<Material*> m_materials{};
    std::map<Material*, std::vector<FaceKey>> m_pendingUpdateMaterialsInMesh{};
    
    bool HasUpdatedSubmeshAndFace(int p_submeshVectorIndex, int p_faceVectorIndex);
};

