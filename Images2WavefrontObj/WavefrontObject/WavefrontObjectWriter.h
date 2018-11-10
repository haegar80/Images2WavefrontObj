#pragma once

#include "MaterialWriter.h"
#include "Mesh.h"
#include "Material.h"
#include <vector>
#include <string>

class WavefrontObjectWriter
{
public:
    WavefrontObjectWriter(const std::string& p_dirPath, const std::string& p_fileName);
    virtual ~WavefrontObjectWriter() = default;

    void WriteWavefrontObject(const std::vector<Mesh*>& p_meshes, const std::vector<Material*>& p_materials);

private:
    std::string m_currentDirPath{ "" };
    std::string m_currentFileName{ "" };
    Material* m_currentMaterial{ nullptr };
    bool m_hasNormals{ false };
    bool m_hasTextures{ false };

    void WriteMaterials(std::ofstream& p_objFile, const std::vector<Material*>& p_materials);
    void WriteMesh(std::ofstream& p_objFile, const Mesh* p_mesh);
    void WriteObjVertexCoords(std::ofstream& p_objFile, const std::vector<ObjVertexCoords>& p_vertices);
    void WriteObjTextureCoords(std::ofstream& p_objFile, const std::vector<ObjTextureCoords>& p_textures);
    void WriteObjNormalCoords(std::ofstream& p_objFile, const std::vector<ObjVertexCoords>& p_normals);
    void WriteObjFaceWithNormalsAndTexture(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces);
    void WriteObjFaceWithNormals(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces);
    void WriteObjFaceWithTexture(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces);
    void WriteObjFaceVertexOnly(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces);
};

