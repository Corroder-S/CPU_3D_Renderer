#define STB_TRUETYPE_IMPLEMENTATION
#include "Font.h"
#include <fstream>
#include <iostream>

bool Font::load(const std::string& filepath, float fontSize) {
    m_fontSize = fontSize;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Font : Impossible d'ouvrir " << filepath << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> ttfBuffer(size);
    if (!file.read(reinterpret_cast<char*>(ttfBuffer.data()), size)) {
        return false;
    }

    m_atlasAlpha.resize(m_atlasWidth * m_atlasHeight);

    int res = stbtt_BakeFontBitmap(
        ttfBuffer.data(), 0,
        m_fontSize,
        m_atlasAlpha.data(), m_atlasWidth, m_atlasHeight,
        32, 96,
        m_cdata
    );

    if (res <= 0) {
        std::cerr << "Font : Atlas trop petit" << std::endl;
        return false;
    }

    return true;
}

void Font::getBakedQuad(char c, float& x, float& y, GlyphQuad& outQuad) const {
    if (c < 32 || c > 126) c = '?';

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(m_cdata, m_atlasWidth, m_atlasHeight, c - 32, &x, &y, &q, 1);

    outQuad.x0 = static_cast<int>(q.x0);
    outQuad.y0 = static_cast<int>(q.y0);
    outQuad.x1 = static_cast<int>(q.x1);
    outQuad.y1 = static_cast<int>(q.y1);
    outQuad.s0 = q.s0;
    outQuad.t0 = q.t0;
    outQuad.s1 = q.s1;
    outQuad.t1 = q.t1;
}