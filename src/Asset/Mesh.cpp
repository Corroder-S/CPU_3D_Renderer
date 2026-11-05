#include "Parsing/ObjLoader.h"
#include "Platform/MemoryMappedFile.h"
#include "Parsing/MtlLoader.h"

#include <cstring>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>

bool LoadObj(const char* path, Mesh& outMesh)
{
    MappedFile file(path);
    if (!file.IsOpen()) return false;

    const char* data = file.Data();
    const char* end = data + file.Size();

    std::unordered_map<std::string, Material> loadedMaterials;

    outMesh.uvs.push_back(Maths::Vec2f(0.0f, 0.0f));

    SubMesh currentSubMesh;
    currentSubMesh.materialName = "Default";

    while (data < end)
    {
        while (data < end && (*data == ' ' || *data == '\t')) data++;
        if (data >= end) break;

        if (data[0] == 'v' && data[1] == ' ')
        {
            const char* p = data + 2;
            float x = std::strtof(p, (char**)&p);
            float y = std::strtof(p, (char**)&p);
            float z = std::strtof(p, (char**)&p);
            outMesh.vertices.push_back(Maths::Vec3f(x, y, z));
        }
        else if (data[0] == 'v' && data[1] == 't' && data[2] == ' ')
        {
            const char* p = data + 3;
            float u = std::strtof(p, (char**)&p);
            float v = std::strtof(p, (char**)&p);
            outMesh.uvs.push_back(Maths::Vec2f(u, v));
        }
        else if (data[0] == 'f' && data[1] == ' ')
        {
            const char* p = data + 2;

            int v1 = std::strtol(p, (char**)&p, 10) - 1;
            int t1 = -1;
            if (*p == '/') { p++; if (*p != '/') t1 = std::strtol(p, (char**)&p, 10) - 1; }
            while (*p && *p != ' ' && *p != '\n') p++;
            while (*p == ' ') p++;

            int v2 = std::strtol(p, (char**)&p, 10) - 1;
            int t2 = -1;
            if (*p == '/') { p++; if (*p != '/') t2 = std::strtol(p, (char**)&p, 10) - 1; }
            while (*p && *p != ' ' && *p != '\n') p++;
            while (*p == ' ') p++;

            int v3 = std::strtol(p, (char**)&p, 10) - 1;
            int t3 = -1;
            if (*p == '/') { p++; if (*p != '/') t3 = std::strtol(p, (char**)&p, 10) - 1; }

            int safeT1 = (t1 < 0 || t1 >= (int)outMesh.uvs.size()) ? 0 : t1;
            int safeT2 = (t2 < 0 || t2 >= (int)outMesh.uvs.size()) ? 0 : t2;
            int safeT3 = (t3 < 0 || t3 >= (int)outMesh.uvs.size()) ? 0 : t3;

            currentSubMesh.faces.push_back(face(Maths::Vec3i(v1, v2, v3), Maths::Vec3i(safeT1, safeT2, safeT3)));
        }
        else if (std::strncmp(data, "mtllib ", 7) == 0)
        {
            const char* p = data + 7;
            while (*p == ' ' || *p == '\t') p++;
            const char* startFilename = p;
            while (p < end && *p != '\r' && *p != '\n') p++;

            std::string mtlFilename(startFilename, p - startFilename);
            std::string objPathStr(path);
            size_t lastSlash = objPathStr.find_last_of("/\\");
            std::string baseDir = (lastSlash != std::string::npos) ? objPathStr.substr(0, lastSlash) : "";
            std::string mtlPath = baseDir.empty() ? mtlFilename : baseDir + "/" + mtlFilename;

            loadedMaterials = MtlLoader::load(mtlPath, baseDir);
        }
        else if (std::strncmp(data, "usemtl ", 7) == 0)
        {
            if (!currentSubMesh.faces.empty()) {
                outMesh.submeshes.push_back(currentSubMesh);
                currentSubMesh.faces.clear();
            }

            const char* p = data + 7;
            while (*p == ' ' || *p == '\t') p++;
            const char* startMtl = p;
            while (p < end && *p != '\r' && *p != '\n' && *p != ' ') p++;

            std::string mtlName(startMtl, p - startMtl);
            currentSubMesh.materialName = mtlName;
            auto it = loadedMaterials.find(mtlName);
            if (it != loadedMaterials.end()) {
                currentSubMesh.material = it->second;
            }
        }

        while (data < end && *data != '\n') data++;
        data++;
    }

    if (!currentSubMesh.faces.empty()) {
        outMesh.submeshes.push_back(currentSubMesh);
    }

    return true;
}

Mesh LoadMesh(const char* path)
{
    Mesh mesh;

    std::string p = path;
    size_t dotPos = p.rfind('.');
    if (dotPos != std::string::npos)
    {
        std::string ext = p.substr(dotPos);
        if (ext == ".obj")
        {
            LoadObj(path, mesh);
        }
    }

    mesh.computeNormals(70.0f);
    return mesh;
}

Mesh LoadMesh(const char* path, Material& material)
{
    Mesh mesh = LoadMesh(path);
    if (!mesh.submeshes.empty())
    {
        material = mesh.submeshes[0].material;
    }
    return mesh;
}