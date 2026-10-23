#pragma once
#include <string>
#include <unordered_map>

#include "Asset/Material.h"

namespace MtlLoader {
	std::unordered_map<std::string, Material> load(const std::string& filepath, const std::string& baseDir = "");
}