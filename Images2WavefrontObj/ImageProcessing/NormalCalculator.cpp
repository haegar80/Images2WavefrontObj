#include "NormalCalculator.h"
#include <QVector3D>

void NormalCalculator::CalculateNormals(std::vector<std::unique_ptr<Mesh>>& p_meshes)
{
    for (std::unique_ptr<Mesh>& mesh : p_meshes)
    {
        if (mesh)
        {
            std::vector<SubMesh*> submeshes = mesh->GetSubmeshes();
            for (int submeshIndex = 0; submeshIndex < submeshes.size(); submeshIndex++)
            {
                SubMesh* submesh = submeshes.at(submeshIndex);
                CalculateAndAddNormalVector(mesh.get(), submesh);
            }
        }
    }
}

void NormalCalculator::CalculateAndAddNormalVector(Mesh* p_mesh, SubMesh* p_submesh)
{
    std::vector<ObjVertexCoords> vertices = p_mesh->GetVertices();
    std::vector<ObjFace>& faces = p_submesh->GetFaces();
    for (int faceVectorIndex = 0; faceVectorIndex < faces.size(); faceVectorIndex++)
    {
        ObjFace& face = faces.at(faceVectorIndex);
        ObjFaceIndices faceIndices1 = face.Indices.at(0);
        ObjVertexCoords vertex1 = vertices.at(faceIndices1.VertexIndex - 1);
        const QVector3D vector1(static_cast<float>(vertex1.X), static_cast<float>(vertex1.Y), static_cast<float>(vertex1.Z));

        ObjFaceIndices faceIndices2 = face.Indices.at(1);
        ObjVertexCoords vertex2 = vertices.at(faceIndices2.VertexIndex - 1);
        const QVector3D vector2(static_cast<float>(vertex2.X), static_cast<float>(vertex2.Y), static_cast<float>(vertex2.Z));

        QVector3D normalVector = QVector3D::crossProduct(vector1, vector2);

        p_mesh->AddNormal(static_cast<int>(normalVector.x()), static_cast<int>(normalVector.y()), static_cast<int>(normalVector.z()));

        for (ObjFaceIndices& faceIndex : face.Indices)
        {
            faceIndex.NormalIndex = p_mesh->GetNormals().size();
            p_submesh->UpdateExistingFace(faceVectorIndex, face);
        }
    }
}