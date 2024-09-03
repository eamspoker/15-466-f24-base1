#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <stdint.h>
#include "PPU466.hpp"


/*
 * Load and save PNG files.
 */

enum OriginLocation {
	LowerLeftOrigin,
	UpperLeftOrigin,
};

//NOTE: load_png will throw on error
void load_png(std::string filename, glm::uvec2 *size, std::vector< glm::u8vec4 > *data, OriginLocation origin);
void save_png(std::string filename, glm::uvec2 size, glm::u8vec4 const *data, OriginLocation origin);
void write_asset(std::string filename, std::string palette_filename, glm::uvec2 *size);
void read_tilemap(std::string filename, std::vector<PPU466::Tile> *tiles, std::vector<glm::u8vec4> *palette);
