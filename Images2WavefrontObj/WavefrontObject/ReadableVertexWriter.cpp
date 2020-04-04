#include "ReadableVertexWriter.h"
#include "../ImageProcessing/DepthCalculator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <QFileInfo>
#include <QDir>

ReadableVertexWriter::ReadableVertexWriter(const std::string& p_dirPath, const std::string& p_fileName) :
    m_currentDirPath(p_dirPath),
    m_currentFileName(p_fileName)
{
}

void ReadableVertexWriter::WriteVertices(const std::vector<Mesh*>& p_meshes, int p_origImageWidth, int p_origImageHeight)
{
    std::ofstream vertexFile;

    std::string fileNameTempString(m_currentFileName + ".vertex");
    QString currentFileName(fileNameTempString.c_str());

    QDir currentDirPath(QString(m_currentDirPath.c_str()));
    if (!currentDirPath.exists())
    {
        currentDirPath.mkpath(".");
    }

    QFileInfo file(currentDirPath, currentFileName);
    QString filePath = file.absoluteFilePath();
    vertexFile.open(filePath.toUtf8().constData());
    if (!vertexFile) {
        std::cout << "File " << filePath.toUtf8().constData() << " could not be opened!!" << std::endl;
        return;
    }

    for (const Mesh* mesh : p_meshes)
    {
        WriteMesh(vertexFile, mesh, p_origImageWidth, p_origImageHeight);
    }

    vertexFile.close();
}

void ReadableVertexWriter::WriteMesh(std::ofstream& p_vertexFile, const Mesh* p_mesh, int p_origImageWidth, int p_origImageHeight)
{
    p_vertexFile << "o " << p_mesh->getName().c_str() << "\n";

    WriteObjVertexCoords(p_vertexFile, p_mesh->GetVertices(), p_origImageWidth, p_origImageHeight);

    const std::vector<SubMesh*> subMeshes = p_mesh->GetSubmeshes();
    for (SubMesh* subMesh : subMeshes)
    {
        WriteObjFaceVertexOnly(p_vertexFile, subMesh->GetFaces());
    }
}

void ReadableVertexWriter::WriteObjVertexCoords(std::ofstream& p_vertexFile, const std::vector<ObjVertexCoords>& p_vertices, int p_origImageWidth, int p_origImageHeight)
{
    for (ObjVertexCoords vertex : p_vertices)
    {
        int pixelX = static_cast<int>(vertex.X * p_origImageWidth);
        int pixelY = static_cast<int>(vertex.Y * p_origImageHeight);
        int pixelZ = static_cast<int>(vertex.Z * (DepthCalculator::GetZPixelFarest() - DepthCalculator::GetZPixelNearest()) * -1);
        p_vertexFile << "v " << pixelX << " " << pixelY << " " << pixelZ << "\n";
    }
}

void ReadableVertexWriter::WriteObjFaceVertexOnly(std::ofstream& p_vertexFile, const std::vector<ObjFace>& p_faces)
{
    for (ObjFace face : p_faces)
    {
        p_vertexFile << "f";
        for (ObjFaceIndices faceIndices : face.Indices)
        {
            p_vertexFile << " " << faceIndices.VertexIndex;
        }
        p_vertexFile << "\n";
    }
}