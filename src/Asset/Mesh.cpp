#include "Mesh.h"

#include <map>

#include "Parsing/ObjLoader.h"

Mesh LoadMesh(const char* path)
{
    Mesh mesh;

    std::string p = path;
    std::string ext = p.substr(p.rfind('.'));
    // ext = ".obj"
    if (ext == ".obj")
    {
        LoadObj(path, mesh.vertices, mesh.uvs, mesh.faces);
    }


    mesh.faceNormals.resize(mesh.faces.size());
    mesh.vertexNormals.resize(mesh.vertices.size());
    for (int i = 0; i < (int)mesh.faces.size(); i++)
    {
        const Maths::Vec3f& v0 = mesh.vertices[mesh.faces[i].v.x];
        const Maths::Vec3f& v1 = mesh.vertices[mesh.faces[i].v.y];
        const Maths::Vec3f& v2 = mesh.vertices[mesh.faces[i].v.z];
        mesh.faceNormals[i] = Maths::Normalize(Maths::Cross(v2 - v0, v1 - v0));
        mesh.vertexNormals[mesh.faces[i].v.x] += mesh.faceNormals[i];
        mesh.vertexNormals[mesh.faces[i].v.y] += mesh.faceNormals[i];
        mesh.vertexNormals[mesh.faces[i].v.z] += mesh.faceNormals[i];
    }



    std::map<std::tuple<float,float,float>, Maths::Vec3f> posNormals;
    for (int i = 0; i < (int)mesh.faces.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int vi = (j==0) ? mesh.faces[i].v.x : (j==1) ? mesh.faces[i].v.y : mesh.faces[i].v.z;
            const Maths::Vec3f& pos = mesh.vertices[vi];
            auto key = std::make_tuple(pos.x, pos.y, pos.z);
            posNormals[key] += mesh.faceNormals[i];
        }
    }


    mesh.vertexNormals.resize(mesh.vertices.size());
    for (int i = 0; i < (int)mesh.vertices.size(); i++)
    {
        const Maths::Vec3f& pos = mesh.vertices[i];
        auto key = std::make_tuple(pos.x, pos.y, pos.z);
        mesh.vertexNormals[i] = Maths::Normalize(posNormals[key]);
    }

    return mesh;
}