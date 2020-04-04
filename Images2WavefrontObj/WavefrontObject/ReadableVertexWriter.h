#pragma once

#include "MaterialWriter.h"
#include "Mesh.h"
#include "Material.h"
#include <vector>
#include <string>

class ReadableVertexWriter
{
public:
    ReadableVertexWriter(const std::string& p_dirPath, const std::string& p_fileName);
    virtual ~ReadableVertexWriter() = default;
    ReadableVertexWriter(const ReadableVertexWriter&) = delete;
    ReadableVertexWriter& operator=(const ReadableVertexWriter&) = delete;

    void WriteVertices(const std::vector<Mesh*>& p_meshes, int p_origImageWidth, int p_origImageHeight);

private:
    std::string m_currentDirPath{ "" };
    std::string m_currentFileName{ "" };

    void WriteMesh(std::ofstream& p_vertexFile, const Mesh* p_mesh, int p_origImageWidth, int p_origImageHeight);
    void WriteObjVertexCoords(std::ofstream& p_vertexFile, const std::vector<ObjVertexCoords>& p_vertices, int p_origImageWidth, int p_origImageHeight);
    void WriteObjFaceVertexOnly(std::ofstream& p_vertexFile, const std::vector<ObjFace>& p_faces);
};

