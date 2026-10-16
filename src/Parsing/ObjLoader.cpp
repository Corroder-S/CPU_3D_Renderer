#include "ObjLoader.h"



bool LoadObj(const char *path, std::vector<Maths::Vec3f> &outVertices, std::vector<Maths::Vec3i> &outFaces)
{
    MappedFile file(path);
    if (!file.IsOpen()) return false;
    const char* data = file.Data();
    const char* end  = data + file.Size();

    while (data < end)
    {
        if (data[0] == 'v' && data[1] == ' ')
        {
            const char* p = data + 2;
            float x = std::strtof(p, (char**)&p);
            float y = std::strtof(p, (char**)&p);
            float z = std::strtof(p, (char**)&p);
            outVertices.push_back(Maths::Vec3f(x, y, z));
        }
        else if (data[0] == 'f' && data[1] == ' ')
        {
            const char* p = data + 2;
            int v1 = std::strtol(p, (char**)&p, 10) - 1;
            while (*p && *p != ' ' && *p != '\n') p++;
            int v2 = std::strtol(p, (char**)&p, 10) - 1;
            while (*p && *p != ' ' && *p != '\n') p++;
            int v3 = std::strtol(p, (char**)&p, 10) - 1;
            while (*p && *p != ' ' && *p != '\n') p++;
            outFaces.push_back(Maths::Vec3i(v1, v2, v3));
        }
        while (data < end && *data != '\n') data++;
        data++;
    }

    return true;

}
