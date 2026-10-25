//
// Created by leoam on 23/10/2026.
//

#include "Particle.h"

void Particle::reset()
{
    position = { 0.0f, 0.0f, 0.0f };
    velocity = { 0.0f, 1.0f, 0.0f };
    life = 0.0f;
    startLife = 0.0f;
    size = 0.0f;
    startSize = 0.0f;
    endSize = 0.0f;
    startColor = 0xFFFFFFFF;
    endColor = 0x00FFFFFF;
}
