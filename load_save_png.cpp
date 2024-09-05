#include "load_save_png.hpp"

#include <png.h>

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include "PPU466.hpp"

#define LOG_ERROR( X ) std::cerr << X << std::endl

using std::vector;

bool load_png(std::istream &from, unsigned int *width, unsigned int *height, vector< glm::u8vec4 > *data, OriginLocation origin);
void save_png(std::ostream &to, unsigned int width, unsigned int height, glm::u8vec4 const *data, OriginLocation origin);

void load_png(std::string filename, glm::uvec2 *size, std::vector< glm::u8vec4 > *data, OriginLocation origin) {
	assert(size);

	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open PNG image file '" + filename + "'.");
	}
	if (!load_png(file, &size->x, &size->y, data, origin)) {
		throw std::runtime_error("Failed to read PNG image from '" + filename + "'.");
	}
}

void save_png(std::string filename, glm::uvec2 size, glm::u8vec4 const *data, OriginLocation origin) {
	std::ofstream file(filename.c_str(), std::ios::binary);
	save_png(file, size.x, size.y, data, origin);
}


static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	std::istream *from = reinterpret_cast< std::istream * >(png_get_io_ptr(png_ptr));
	assert(from);
	if (!from->read(reinterpret_cast< char * >(data), length)) {
		png_error(png_ptr, "Error reading.");
	}
}

static void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	std::ostream *to = reinterpret_cast< std::ostream * >(png_get_io_ptr(png_ptr));
	assert(to);
	if (!to->write(reinterpret_cast< char * >(data), length)) {
		png_error(png_ptr, "Error writing.");
	}
}

static void user_flush_data(png_structp png_ptr) {
	std::ostream *to = reinterpret_cast< std::ostream * >(png_get_io_ptr(png_ptr));
	assert(to);
	if (!to->flush()) {
		png_error(png_ptr, "Error flushing.");
	}
}


bool load_png(std::istream &from, unsigned int *width, unsigned int *height, vector< glm::u8vec4 > *data, OriginLocation origin) {
	assert(data);
	uint32_t local_width, local_height;
	if (width == nullptr) width = &local_width;
	if (height == nullptr) height = &local_height;
	*width = *height = 0;
	data->clear();
	//..... load file ......
	//Load a png file, as per the libpng docs:
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);

	png_set_read_fn(png, &from, user_read_data);

	if (!png) {
		LOG_ERROR("  cannot alloc read struct.");
		return false;
	}
	png_infop info = png_create_info_struct(png);
	if (!info) {
		LOG_ERROR("  cannot alloc info struct.");
		png_destroy_read_struct(&png, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}
	png_bytep *row_pointers = NULL;
	if (setjmp(png_jmpbuf(png))) {
		LOG_ERROR("  png interal error.");
		png_destroy_read_struct(&png, &info, (png_infopp)NULL);
		if (row_pointers != NULL) delete[] row_pointers;
		data->clear();
		return false;
	}
	//not needed with custom read/write functions: png_init_io(png, NULL);
	png_read_info(png, info);
	unsigned int w = png_get_image_width(png, info);
	unsigned int h = png_get_image_height(png, info);
	if (png_get_color_type(png, info) == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);
	if (png_get_color_type(png, info) == PNG_COLOR_TYPE_GRAY || png_get_color_type(png, info) == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);
	if (!(png_get_color_type(png, info) & PNG_COLOR_MASK_ALPHA))
		png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);
	if (png_get_bit_depth(png, info) < 8)
		png_set_packing(png);
	if (png_get_bit_depth(png,info) == 16)
		png_set_strip_16(png);
	//Ok, should be 32-bit RGBA now.

	png_read_update_info(png, info);
	size_t rowbytes = png_get_rowbytes(png, info);
	//Make sure it's the format we think it is...
	assert(rowbytes == w*sizeof(uint32_t));

	data->resize(w*h);
	row_pointers = new png_bytep[h];
	for (unsigned int r = 0; r < h; ++r) {
		if (origin == LowerLeftOrigin) {
			row_pointers[h-1-r] = (png_bytep)(&(*data)[r*w]);
		} else {
			row_pointers[r] = (png_bytep)(&(*data)[r*w]);
		}
	}
	png_read_image(png, row_pointers);
	png_destroy_read_struct(&png, &info, NULL);
	delete[] row_pointers;

	*width = w;
	*height = h;
	return true;
}


void save_png(std::ostream &to, unsigned int width, unsigned int height, glm::u8vec4 const *data, OriginLocation origin) {
//After the libpng example.c
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	png_set_write_fn(png_ptr, &to, user_write_data, user_flush_data);

	if (png_ptr == NULL) {
		LOG_ERROR("Can't create write struct.");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, NULL);
		LOG_ERROR("Can't craete info pointer");
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		LOG_ERROR("Error writing png.");
		return;
	}

	//Not needed with custom read/write functions: png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);
	//png_set_swap_alpha(png_ptr) // might need?
	vector< png_bytep > row_pointers(height);
	for (unsigned int i = 0; i < height; ++i) {
		if (origin == UpperLeftOrigin) {
			row_pointers[i] = (png_bytep)&(data[i * width]);
		} else {
			row_pointers[i] = (png_bytep)&(data[(height - 1 - i) * width]);
		}
	}
	png_write_image(png_ptr, &(row_pointers[0]));

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	return;
}


// chunk helpers from assets 2 prereading
template< typename T >
void write_chunk(std::string const &magic, std::vector< T > const &from, std::ostream *to_) {
//!todo{
	assert(magic.size() == 4);
	assert(to_);
	auto &to = *to_;

	//write magic value:
	to.write(magic.c_str(), 4);

	//write size (in bytes):
	uint32_t size = from.size() * sizeof(T);
	to.write(reinterpret_cast< char const * >(&size), 4);

	//write data:
	// from: https://stackoverflow.com/questions/4254615/how-to-cast-vectorunsigned-char-to-char
	to.write(reinterpret_cast<char const*>(from.data()), size);
}

template< typename T >
void read_chunk(std::string const &magic, std::istream &from, std::vector< T > *to_) {
	assert(to_);
	auto &to = *to_;

	struct ChunkHeader {
		char magic[4] = {'\0', '\0', '\0', '\0'};
		uint32_t size = 0;
	};
	static_assert(sizeof(ChunkHeader) == 8, "header is packed");

	ChunkHeader header;
	if (!from.read(reinterpret_cast< char * >(&header), sizeof(header))) {
		throw std::runtime_error("Failed to read chunk header");
	}
	if (std::string(header.magic,4) != magic) {
		throw std::runtime_error("Unexpected magic number in chunk");
	}

	if (header.size % sizeof(T) != 0) {
		throw std::runtime_error("Size of chunk not divisible by element size");
	}

	to.resize(header.size / sizeof(T));
	if (!from.read(reinterpret_cast< char * >(&to[0]), to.size() * sizeof(T))) {
		throw std::runtime_error("Failed to read chunk data.");
	}
}

void write_asset(std::string filename, std::string palette_filename, glm::uvec2 *size)
{
	// get the palette and tile pixels in vector form
	std::vector< glm::u8vec4> palette_data;
	glm::uvec2 palette_size = {2,2};
	load_png(palette_filename, &palette_size, &palette_data, LowerLeftOrigin);

	std::vector< glm::u8vec4> tile_data;

	assert(size != nullptr);
	load_png(filename, size, &tile_data, LowerLeftOrigin);
	assert(tile_data.size() % 64 == 0);

	// convert from rgba to palette indices
	std::vector<uint8_t> tile_map;
	for (glm::u8vec4 pixel : tile_data)
	{
		uint8_t index = 0;
		for (glm::u8vec4 color : palette_data)
		{
			if (pixel.r == color.r &&
				pixel.g == color.g &&
				pixel.b == color.b &&
				pixel.a == color.a)
			{
				tile_map.emplace_back(index);
				break;
			}
			index++;
		}

		

		// no palette match, throw error
		if (index > 3)
		{
			throw std::runtime_error("Failed to create tile file '" + 
				filename + "'. Pixel with no palette match found. Color: "
				+ std::to_string(pixel.r) + ", "
				+ std::to_string(pixel.g) + ", "
				+ std::to_string(pixel.b) + ", "
				+ std::to_string(pixel.a));
		}

		
	}

	// ofstream code adapted from save png above
	std::ofstream tilefile((filename+".tile").c_str(), std::ios::binary);

	// write the number of tiles in the file
	size_t n_tiles = tile_data.size()/64;
	std::vector<size_t> num_tiles;
	num_tiles.emplace_back(n_tiles);
	write_chunk<size_t>("numt", num_tiles, &tilefile);
	

	// write chunks to tile file for each 8x8 block

	// size of the image in tiles
	size_t tiles_width = (size->x)/8;

	size_t tile_index = 0;
	for (size_t t_row = 0; t_row < (size->y); t_row += 8)
	{
		for (size_t t_col = 0; t_col < (size->x); t_col += 8)
		{
			std::vector<uint8_t> tile;
			// create a smaller 8x8 tile
			for (size_t row = t_row; row < t_row+8; row++)
			{
				for (size_t col = t_col; col < t_col+8; col++)
				{

					tile.emplace_back(tile_map[col + row*tiles_width*8]);

				}
			}

			std::string tile_id = std::to_string(tile_index);
			tile_id =  (tile_index < 100) ? "0" + tile_id : tile_id;
			tile_id =  (tile_index < 10) ? "0" + tile_id : tile_id;

			tile_index++;
			
			// read tile into bit vectors
			write_chunk<uint8_t>("t"+tile_id, tile, &tilefile); 
			
		}

	}
	
	// write palette, tile chunks
	write_chunk<glm::u8vec4>("plet", palette_data, &tilefile);
	
}

void read_tilemap(std::string filename, std::vector<PPU466::Tile> *tiles, std::vector<glm::u8vec4> *palette)
{
	// ifstream code from load png above
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open tile asset file '" + filename + "'.");
	}



	// first get the number of tiles in the file...
	std::vector<size_t> num_tiles;
	read_chunk<size_t>("numt", file, &num_tiles); 
	if (num_tiles.size() != 1)
	{
		throw std::runtime_error("Failed to open tile asset file '" + filename + "', multiple num tiles.");
	}






	// .. then create a PPU466 object for each tile to add to tiles
	for (size_t i = 0; i < num_tiles[0]; i++)
	{
		// get the tile id
		std::string tile_id = std::to_string(i);
		tile_id =  (i < 100) ? "0" + tile_id : tile_id;
		tile_id =  (i < 10) ? "0" + tile_id : tile_id;

		// read tile into bit vectors
		std::vector<uint8_t> tile_map;
		read_chunk<uint8_t>("t"+tile_id, file, &tile_map); 
		assert(tile_map.size() == 64);
		
		std::vector<uint8_t> bit0;
		std::vector<uint8_t> bit1;

		// convert palette indices into bit0 and bit1 format
		for (size_t row = 0; row < 8; row++)
		{
			uint8_t curr_row_bit0 = 0;
			uint8_t curr_row_bit1 = 0;

			for (size_t col = 0; col < 8; col++)
			{
				uint8_t pixel = tile_map[(row*8) + col];
				curr_row_bit0 = curr_row_bit0 | ((pixel & 1) << col);
				curr_row_bit1 = curr_row_bit1 | (((pixel >> 1) & 1) << col);

				// check that the index can be correctly reconstructed
				// formula from PPU466.hpp
				uint8_t bit0_check = (curr_row_bit0 >> col) & 1;
				uint8_t bit1_check = (curr_row_bit1 >> col) & 1;
				uint8_t color_index = (bit1_check << 1) | bit0_check;
				assert(color_index == pixel);
			}

			bit0.emplace_back(curr_row_bit0);
			bit1.emplace_back(curr_row_bit1);
		}

		// create tile
		PPU466::Tile t;
		t.bit0 = {
			bit0[0], bit0[1], bit0[2], bit0[3], 
			bit0[4], bit0[5], bit0[6], bit0[7],
		};
		t.bit1 = {
			bit1[0], bit1[1], bit1[2], bit1[3],
			bit1[4], bit1[5], bit1[6], bit1[7],
		};

		tiles->emplace_back(t);
	}

	// read the palette
	read_chunk<glm::u8vec4>("plet", file, palette);


}