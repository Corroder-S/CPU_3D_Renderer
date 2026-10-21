#pragma once

#include <iostream>
#include <string>
#include <cstdint>

#include "Color.h"
#include "Texture.h"

struct Material {
	std::string name = "Default";

	Color ambient = Color::White;
	Color diffuse = Color::White;
	Color secular = Color::White;

	float shininess = 32.0f;
	float dissolve = 1.0f;

	int illum = 1;

	std::shared_ptr<Texture> texture = nullptr;

};