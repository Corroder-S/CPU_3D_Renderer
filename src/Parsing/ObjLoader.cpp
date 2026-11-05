#include "ObjLoader.h"
#include "Platform/MemoryMappedFile.h"
#include "MtlLoader.h"

#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

namespace {

    void ParseFaceVertex(const char*& p, int& vIdx, int& tIdx, size_t vertCount, size_t uvCount)
    {
        vIdx = -1;
        tIdx = -1;

        if ((*p >= '0' && *p <= '9') || *p == '-') {
            int rawV = std::strtol(p, (char**)&p, 10);
            vIdx = (rawV < 0) ? static_cast<int>(vertCount) + rawV : rawV - 1;
        }

        if (*p == '/') {
            p++;
            if (*p != '/' && ((*p >= '0' && *p <= '9') || *p == '-')) {
                int rawT = std::strtol(p, (char**)&p, 10);
                tIdx = (rawT < 0) ? static_cast<int>(uvCount) + rawT : rawT - 1;
            }
            if (*p == '/') {
                p++;
                std::strtol(p, (char**)&p, 10);
            }
        }

        while (*p && *p != ' ' && *p != '\r' && *p != '\n') {
            p++;
        }
    }
}

bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec2f>& outuvs, std::vector<Face>& outFaces)
{
    Material dummyMaterial;
    return LoadObj(path, outVertices, outuvs, outFaces, dummyMaterial);
}

bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec2f>& outuvs, std::vector<Face>& outFaces, Material& outMaterial)
{
    MappedFile file(path);
    if (!file.isOpen()) return false;

    const char* data = file.data();
    const char* end = data + file.size();

    std::unordered_map<std::string, Material> loadedMaterials;
    std::string currentMtlName;

    if (outuvs.empty()) {
        outuvs.push_back(Maths::Vec2f(0.0f, 0.0f));
    }

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
            outVertices.push_back(Maths::Vec3f(x, y, z));
        }
        else if (data[0] == 'v' && data[1] == 't' && data[2] == ' ')
        {
            const char* p = data + 3;
            float u = std::strtof(p, (char**)&p);
            float v = std::strtof(p, (char**)&p);
            outuvs.push_back(Maths::Vec2f(u, v));
        }
        else if (data[0] == 'f' && data[1] == ' ')
        {
            const char* p = data + 2;

            int v1 = std::strtol(p, (char**)&p, 10) - 1;
            int t1 = -1;
            if (*p == '/') {
                p++;
                if (*p != '/') t1 = std::strtol(p, (char**)&p, 10) - 1;
            }
            while (*p && *p != ' ' && *p != '\n') p++;
            while (*p == ' ') p++;

            int v2 = std::strtol(p, (char**)&p, 10) - 1;
            int t2 = -1;
            if (*p == '/') {
                p++;
                if (*p != '/') t2 = std::strtol(p, (char**)&p, 10) - 1;
            }
            while (*p && *p != ' ' && *p != '\n') p++;
            while (*p == ' ') p++;

            int v3 = std::strtol(p, (char**)&p, 10) - 1;
            int t3 = -1;
            if (*p == '/') {
                p++;
                if (*p != '/') t3 = std::strtol(p, (char**)&p, 10) - 1;
            }

            int safeT1 = (t1 < 0 || t1 >= (int)outuvs.size()) ? 0 : t1;
            int safeT2 = (t2 < 0 || t2 >= (int)outuvs.size()) ? 0 : t2;
            int safeT3 = (t3 < 0 || t3 >= (int)outuvs.size()) ? 0 : t3;

            outFaces.push_back(Face(Maths::Vec3i(v1, v2, v3), Maths::Vec3i(safeT1, safeT2, safeT3)));
        }
        else if (std::strncmp(data, "mtllib ", 7) == 0)
        {
            const char* p = data + 7;
            while (*p == ' ' || *p == '\t') p++;

            const char* startFilename = p;
            while (p < end && *p != '\r' && *p != '\n') p++;

            const char* endFilename = p;
            while (endFilename > startFilename && (*(endFilename - 1) == ' ' || *(endFilename - 1) == '\r')) {
                endFilename--;
            }

            std::string mtlFilename(startFilename, endFilename - startFilename);
            std::string objPathStr(path);
            size_t lastSlash = objPathStr.find_last_of("/\\");
            std::string baseDir = (lastSlash != std::string::npos) ? objPathStr.substr(0, lastSlash) : "";
            std::string mtlPath = baseDir.empty() ? mtlFilename : baseDir + "/" + mtlFilename;

            loadedMaterials = MtlLoader::load(mtlPath, baseDir);
            if (!loadedMaterials.empty()) {
                outMaterial = loadedMaterials.begin()->second;
            }
        }
        else if (std::strncmp(data, "usemtl ", 7) == 0)
        {
            const char* p = data + 7;
            while (*p == ' ' || *p == '\t') p++;
            const char* startMtl = p;
            while (p < end && *p != '\r' && *p != '\n' && *p != ' ') p++;

            std::string mtlName(startMtl, p - startMtl);
            auto it = loadedMaterials.find(mtlName);
            if (it != loadedMaterials.end()) {
                outMaterial = it->second;
            }
        }

        while (data < end && *data != '\n') data++;
        data++;
    }

    return true;
}