#include "WavefrontObjectWriter.h"
#include "MaterialWriter.h"
#include <iostream>
#include <fstream>
#include <sstream>

WavefrontObjectWriter::WavefrontObjectWriter(const std::string& p_dirPath, const std::string& p_fileName)
{
    m_currentDirPath = p_dirPath;
    m_currentFileName = p_fileName;
}

void WavefrontObjectWriter::WriteWavefrontObject(const std::vector<Mesh*>& p_meshes, const std::vector<Material*>& p_materials)
{
    std::ofstream objFile;
    std::string filePath((m_currentDirPath + m_currentFileName + ".obj"));
    objFile.open(filePath.c_str());
    if (!objFile) {
        std::cout << "File " << filePath.c_str() << " could not be opened!!" << std::endl;
        return;
    }

    WriteMaterials(objFile, p_materials);
    
    for (const Mesh* mesh : p_meshes)
    {
        WriteMesh(objFile, mesh);
    }
  
    objFile.close();
}

void WavefrontObjectWriter::WriteMaterials(std::ofstream& p_objFile, const std::vector<Material*>& p_materials)
{
    MaterialWriter materialWriter;
    materialWriter.WriteMaterials(m_currentDirPath, m_currentFileName, p_materials);
    p_objFile << "mtllib " << m_currentFileName << ".mtl\n";
}

void WavefrontObjectWriter::WriteMesh(std::ofstream& p_objFile, const Mesh* p_mesh)
{
    p_objFile << "o " << p_mesh->getName().c_str() << "\n";

    m_hasTextures = false;
    m_hasNormals = false;

    WriteObjVertexCoords(p_objFile, p_mesh->GetVertices());
    WriteObjTextureCoords(p_objFile, p_mesh->GetTextures());
    WriteObjNormalCoords(p_objFile, p_mesh->GetNormals());

    const std::vector<SubMesh*> subMeshes = p_mesh->GetSubmeshes();
    for (SubMesh* subMesh : subMeshes)
    {
        std::string& materialName(subMesh->GetMaterial()->getName());
        if (0 != materialName.compare("default"))
        {
            p_objFile << "usemtl " << materialName.c_str() << "\n";
        }
        if (m_hasTextures && m_hasNormals)
        {
            WriteObjFaceWithNormalsAndTexture(p_objFile, subMesh->GetFaces());
        }
        else if (m_hasTextures)
        {
            WriteObjFaceWithTexture(p_objFile, subMesh->GetFaces());
        }
        else if (m_hasNormals)
        {
            WriteObjFaceWithNormals(p_objFile, subMesh->GetFaces());
        }
        else
        {
            WriteObjFaceVertexOnly(p_objFile, subMesh->GetFaces());
        }
    }
}

void WavefrontObjectWriter::WriteObjVertexCoords(std::ofstream& p_objFile, const std::vector<ObjVertexCoords>& p_vertices)
{
    for (ObjVertexCoords vertex : p_vertices)
    {
        p_objFile << "v " << vertex.X << " " << vertex.Y << " " << vertex.Z << "\n";
    }
}

void WavefrontObjectWriter::WriteObjTextureCoords(std::ofstream& p_objFile, const std::vector<ObjTextureCoords>& p_textures)
{
    if (p_textures.size() > 0)
    {
        m_hasTextures = true;
        for (ObjTextureCoords texture : p_textures)
        {
            p_objFile << "vt " << texture.U << " " << texture.U << "\n";
        }
    }
}

void WavefrontObjectWriter::WriteObjNormalCoords(std::ofstream& p_objFile, const std::vector<ObjVertexCoords>& p_normals)
{
    if (p_normals.size() > 0)
    {
        m_hasNormals = true;
        for (ObjVertexCoords normal : p_normals)
        {
            p_objFile << "vn " << normal.X << " " << normal.Y << " " << normal.Z << "\n";
        }
    }
}

void WavefrontObjectWriter::WriteObjFaceWithNormalsAndTexture(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces)
{
    for (ObjFace face : p_faces)
    {
        p_objFile << "f";
        for (ObjFaceIndices faceIndices : face.Indices)
        {
            p_objFile << " " << faceIndices.VertexIndex << "/" << faceIndices.TextureIndex << "/" << faceIndices.NormalIndex;
        }
        p_objFile << "\n";
    }
}

void WavefrontObjectWriter::WriteObjFaceWithNormals(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces)
{
    for (ObjFace face : p_faces)
    {
        p_objFile << "f";
        for (ObjFaceIndices faceIndices : face.Indices)
        {
            p_objFile << " " << faceIndices.VertexIndex << "//" << faceIndices.NormalIndex;
        }
        p_objFile << "\n";
    }
}

void WavefrontObjectWriter::WriteObjFaceWithTexture(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces)
{
    for (ObjFace face : p_faces)
    {
        p_objFile << "f";
        for (ObjFaceIndices faceIndices : face.Indices)
        {
            p_objFile << " " << faceIndices.VertexIndex << "/" << faceIndices.TextureIndex;
        }
        p_objFile << "\n";
    }
}

void WavefrontObjectWriter::WriteObjFaceVertexOnly(std::ofstream& p_objFile, const std::vector<ObjFace>& p_faces)
{
    for (ObjFace face : p_faces)
    {
        p_objFile << "f";
        for (ObjFaceIndices faceIndices : face.Indices)
        {
            p_objFile << " " << faceIndices.VertexIndex;
        }
        p_objFile << "\n";
    }
}