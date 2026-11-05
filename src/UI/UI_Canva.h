#pragma once
#include <cstdint>

#include "Asset/Font.h"

struct Texture;


struct Image
{
    uint32_t x, y, w, h;
    const Texture* texture;
    uint32_t zOrder;
};

struct Text
{
    uint32_t x, y;
    Font* font;
    Color color;
    std::string text;
    uint32_t zOrder;
};

struct Canva {
    uint32_t x, y, w, h;
    uint32_t color;
    uint32_t zOrder;
};

class UI_Canva
{
public:

    uint32_t m_width;
    uint32_t m_height;
    void drawRectangle(int x, int y, int width, int height, std::uint32_t color, uint32_t* framebuffer);
    void drawImage(int x, int y, int width, int height, const Texture* texture, uint32_t* framebuffer);
    void drawText(const Font& font,const std::string& text,uint32_t posX, uint32_t posY,const Color& color,uint32_t* framebuffer,uint32_t fbWidth, uint32_t fbHeight);




};
