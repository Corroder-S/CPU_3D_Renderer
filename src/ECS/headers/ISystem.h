#pragma once


struct ISystem
{
    virtual void update(float deltaTime) = 0;
    virtual ~ISystem() = default;
};