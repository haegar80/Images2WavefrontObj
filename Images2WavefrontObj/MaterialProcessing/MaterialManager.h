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
    void CreateMaterialsBasedOnTextures(std::map<FaceKey, std::string>& p_texturePaths);
    void UpdateMaterialsInMesh(Mesh* p_mesh);
    std::vector<Material*>& GetMaterials()
    {
        return m_materials;
    }

private:
    std::vector<Material*> m_materials{};
    std::map<FaceKey, Material*> m_pendingUpdateMaterialsInMesh{};
    
    int GetVertexIndex(Mesh* p_mesh, int p_submeshVectorIndex, int p_faceVectorIndex);
    bool HasMoreFacesWithSameVertexIndex(Mesh* p_mesh, int p_submeshVectorIndex, int p_faceVectorIndex, int p_vertexIndex);
};

