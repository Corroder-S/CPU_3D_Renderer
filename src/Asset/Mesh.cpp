#include "Mesh.h"

#include <map>

#include "Parsing/ObjLoader.h"

Mesh LoadMesh(const char* path, Material& material)
{
    Mesh mesh;

    std::string p = path;
    std::string ext = p.substr(p.rfind('.'));
    // ext = ".obj"
    if (ext == ".obj")
    {
        LoadObj(path, mesh.vertices, mesh.uvs, mesh.faces, material);
    }


    mesh.faceNormals.resize(mesh.faces.size());
    mesh.computeNormals(70.0f);
    return mesh;
}

Mesh LoadMesh(const char* path)
{
    Material dummyMaterial;
    return LoadMesh(path, dummyMaterial);
}