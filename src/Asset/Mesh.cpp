#include "Mesh.h"

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


     mesh.normals.resize(mesh.faces.size());
     for (int i = 0; i < (int)mesh.faces.size(); i++)
     {
         const Maths::Vec3f& v0 = mesh.vertices[mesh.faces[i].v.x];
         const Maths::Vec3f& v1 = mesh.vertices[mesh.faces[i].v.y];
         const Maths::Vec3f& v2 = mesh.vertices[mesh.faces[i].v.z];
         mesh.normals[i] = Maths::Normalize(Maths::Cross(v1 - v0, v2 - v0));
     }
     return mesh;
}
