#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "Sprite.h"
#include <vector>
#include <string>
#include <fstream>

void WriteTileData(const std::string& outputName, std::ofstream& sourceFile, const std::vector<RawSprite>& sprites);