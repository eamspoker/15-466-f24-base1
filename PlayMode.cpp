#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "Load.hpp"

#include "load_save_png.hpp"
#include "data_path.hpp"

#include <map>
#include <chrono>

// a set of tiles that share a palette
struct TileMapInfo {
	std::vector<PPU466::Tile> tiles;
	PPU466::Palette palette;
	
	TileMapInfo(std::vector<PPU466::Tile> tiles_,
	std::vector<glm::u8vec4> palette_){
		tiles = tiles_;
		palette = {palette_[0], palette_[1],palette_[2],palette_[3]};
	}
	~TileMapInfo();

};

Load<TileMapInfo> player_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/placeholder.png.tile"), &tiles, &palette_data);

	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});

Load<TileMapInfo> ground_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/ground.png.tile"), &tiles, &palette_data);

	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});

Load<TileMapInfo> sky_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/sky.png.tile"), &tiles, &palette_data);


	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});

Load<TileMapInfo> pole_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/pole.png.tile"), &tiles, &palette_data);


	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});
Load<TileMapInfo> npc_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/person.png.tile"), &tiles, &palette_data);


	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});


Load<TileMapInfo> checkbox_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/checkbox.png.tile"), &tiles, &palette_data);


	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});

Load<TileMapInfo> winlose_info(LoadTagDefault, [](){

	std::vector<glm::u8vec4> palette_data;
	std::vector<PPU466::Tile> tiles;
	read_tilemap(data_path("assets/winlose.png.tile"), &tiles, &palette_data);


	TileMapInfo *s = new TileMapInfo(tiles, palette_data);
	return s;

});


PlayMode::PlayMode() {
	// maps name of tileset to [start, end) vector in memory

	auto get_indices = [&](size_t previous_end, size_t current_size) {
		return glm::uvec2{previous_end, current_size + previous_end};
	};

	t_indices["ground"] = get_indices(0, ground_info->tiles.size());
	t_indices["sky"] = get_indices(t_indices["ground"].y,sky_info->tiles.size());
	t_indices["checkbox"] = get_indices(t_indices["sky"].y,checkbox_info->tiles.size());

	assert(t_indices["checkbox"].y == 32);
	t_indices["player"] = {32, 32+player_info->tiles.size()};
	t_indices["pole"] = {32+player_info->tiles.size(), 32+player_info->tiles.size()+pole_info->tiles.size()};
	t_indices["npc"] = {
		32+player_info->tiles.size()+pole_info->tiles.size(),
		32+player_info->tiles.size()+pole_info->tiles.size()
		+ npc_info->tiles.size()};

	t_indices["winlose"] = {
		t_indices["npc"].y,
		t_indices["npc"].y+winlose_info->tiles.size()};

	// fill in all tiles
	for (size_t j = 0; j < ppu.tile_table.size(); j++)
	{
		ppu.tile_table[j].bit0 = player_info->tiles[0].bit0;
		ppu.tile_table[j].bit1 = player_info->tiles[0].bit1;
	}


	for (size_t i = t_indices["ground"].x; i < t_indices["ground"].y; i++)
	{
		ppu.tile_table[i].bit0 = ground_info->tiles[i].bit0;
		ppu.tile_table[i].bit1 = ground_info->tiles[i].bit1;
	}

	for (size_t i = t_indices["sky"].x; i < t_indices["sky"].y; i++)
	{
		ppu.tile_table[i].bit0 = sky_info->tiles[i-ground_info->tiles.size()].bit0;
		ppu.tile_table[i].bit1 = sky_info->tiles[i-ground_info->tiles.size()].bit1;

	}

	for (size_t i = t_indices["checkbox"].x; i < t_indices["checkbox"].y; i++)
	{
		ppu.tile_table[i].bit0 = checkbox_info->tiles[i-t_indices["checkbox"].x].bit0;
		ppu.tile_table[i].bit1 = checkbox_info->tiles[i-t_indices["checkbox"].x].bit1;

	}
	//use sprites 31,32 as a "player":

	for (size_t i = t_indices["player"].x; i < t_indices["player"].y; i++)
	{
		ppu.tile_table[i].bit0 = player_info->tiles[i-32].bit0;
		ppu.tile_table[i].bit1 = player_info->tiles[i-32].bit1;

	}

	for (size_t i = t_indices["pole"].x; i < t_indices["pole"].y; i++)
	{
		ppu.tile_table[i].bit0 = pole_info->tiles[i-(32+player_info->tiles.size())].bit0;
		ppu.tile_table[i].bit1 = pole_info->tiles[i-(32+player_info->tiles.size())].bit1;

	}
	for (size_t i = t_indices["npc"].x; i < t_indices["npc"].y; i++)
	{
		ppu.tile_table[i].bit0 = npc_info->tiles[i-t_indices["npc"].x].bit0;
		ppu.tile_table[i].bit1 = npc_info->tiles[i-t_indices["npc"].x].bit1;

	}
	for (size_t i = t_indices["winlose"].x; i < t_indices["winlose"].y; i++)
	{
		ppu.tile_table[i].bit0 = winlose_info->tiles[i-t_indices["winlose"].x].bit0;
		ppu.tile_table[i].bit1 = winlose_info->tiles[i-t_indices["winlose"].x].bit1;

	}

	ppu.palette_table[2] =  checkbox_info->palette;
	ppu.palette_table[3] =  sky_info->palette;
	ppu.palette_table[4] =  pole_info->palette;
	ppu.palette_table[5] =  npc_info->palette;
	ppu.palette_table[6] =  ground_info->palette;



	//used for the player:
	ppu.palette_table[7] = player_info->palette;

	// helper function for generating background tiles
	auto make_bg_tile = [&](size_t index, size_t palette)
	{
		return index | (palette << 8);
	};

	// create random tiling for sky, poles
	// 0.25 probability of returning a random tile between [start, end)
	// 0.75 probability of returning a default tile
	auto get_random = [&](size_t start, size_t end, size_t dtile, size_t palette) {

		// from https://cplusplus.com/reference/random/mersenne_twister_engine/
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 generator (seed);

		// generator generates int, need to cast to float for proportion
		uint_fast32_t generated = generator();

		// if even number, return the sky tile (last in tileset)
		// otherwise, use generated random number to pick a sky tile
		size_t index = 
		generated % 4 == 0 ? 
			start + ((float)generated/(float)generator.max())*(end-start)
			: dtile;
		
		return make_bg_tile(index, palette);
	};

	// sky color
	ppu.background_color = ppu.palette_table[3][3];

	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			if (y == 0)
			{ 
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					make_bg_tile(x%(t_indices["ground"].y - t_indices["ground"].x), 6);
			} else if (y == 2)
			{
				// tiles 9-14 in pole map are the random wires
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					make_bg_tile(t_indices["pole"].x+9+(x%6), 4); 
				


			} else if (y== 1)
			{
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					make_bg_tile(t_indices["pole"].x+1+(x%6), 4); 
			}
			 else if (y > 10 && y < 20)
			{

				// random clouds
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					get_random(t_indices["sky"].x, t_indices["sky"].y, t_indices["sky"].y-1, 3); 
			} else
			{
				// clear sky tile is last tile
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					make_bg_tile(t_indices["sky"].y-1, 3);
			}
		}
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	if (!done)
	{
		constexpr float PlayerSpeed = 30.0f;
		player_at.x += PlayerSpeed * elapsed;
		if (up.pressed) player_at.x += 0.75*(PlayerSpeed * elapsed);
		if (down.pressed) player_at.x -= 0.75*(PlayerSpeed * elapsed);

		total_elapsed += elapsed;
		percent_done = (total_elapsed/60);
		

		if (abs(percent_done-1.0f) < 0.001 && num_picked_up >= 3)
		{
			done = true;
			win = true;
		} else if (num_picked_up >= 3)
		{
			done = true;
			win = true;
		} else if (abs(percent_done-1.0f) < 0.001)
		{
			done = true;
			win = false;
		}


		left.downs = 0;
		right.downs = 0;
		up.downs = 0;
		down.downs = 0;
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---
	
	if (!done)
	{
		ppu.background_color = {(1-percent_done)*day_sky.r+ (percent_done)*night_sky.r,
									(1-percent_done)*day_sky.g+ (percent_done)*night_sky.g,
									(1-percent_done)*day_sky.b+ (percent_done)*night_sky.b};
		
		//background scroll:
		ppu.background_position.x = int32_t(-0.5f * player_at.x);


		// bus sprites (bus is 6 x 2 tiles)
		// player_at.x determines the x of the back of the bus
		for (size_t i = 0; i < player_info->tiles.size(); i++)
		{

			ppu.sprites[i].x = int8_t(player_at.x+8*(i%6));
			ppu.sprites[i].y = i > 5 ? int8_t(player_at.y+8) : int8_t(player_at.y);
			ppu.sprites[i].index = 32+i;
			ppu.sprites[i].attributes = 7;

		}

		// drawing powerlines
		size_t pole_index = player_info->tiles.size(); 

		// left side pole
		for (size_t i = 0; i < 3; i++)
		{
			ppu.sprites[pole_index+i].x = 0;
			ppu.sprites[pole_index+i].y = (i)*8;
			ppu.sprites[pole_index+i].index = i > 1 ? pole_index + 32 + 8 : pole_index + 32;
			ppu.sprites[pole_index+i].attributes = 4;

		}

		// right side pole
		for (size_t i = 6; i < 9; i++)
		{
			ppu.sprites[pole_index+i].x = 248;
			ppu.sprites[pole_index+i].y = (i-6)*8;
			ppu.sprites[pole_index+i].index = i > 7 ? pole_index + 32 + 15 : pole_index + 32 + 7;
			ppu.sprites[pole_index+i].attributes = 4;

		}

		size_t checkbox_index = pole_index + 9;

		for (size_t num = 0; num < 3; num++)
		{
	
			for (size_t i = 0; i < 4; i++)
			{

				ppu.sprites[checkbox_index+i+(num*4)].x = int8_t(10+(20*num)+8*(i%2));
				ppu.sprites[checkbox_index+i+(num*4)].y = i > 1 ? int8_t(210+8) : int8_t(210);

				if (num >= num_picked_up)
				{
					ppu.sprites[checkbox_index+i+(num*4)].index = i > 1 ? t_indices["checkbox"].x+i+2 :
																	t_indices["checkbox"].x+i;
				} else {
						ppu.sprites[checkbox_index+i+(num*4)].index = i > 1 ? t_indices["checkbox"].x+i+4 :
																	t_indices["checkbox"].x+i+2;
				}
				ppu.sprites[checkbox_index+i+(num*4)].attributes = 2;

			}
		}


		// npc

		size_t npc_index = checkbox_index + 24;

		auto spawn_npc = [&](size_t npc_index)
			{// from https://cplusplus.com/reference/random/mersenne_twister_engine/
				unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
				std::mt19937 generator (seed);

				// generator generates int, need to cast to float for proportion
				uint_fast32_t generated = generator();

				if (generated%2 == 0)
				{

					// create sprite
					ppu.sprites[npc_index].x = 100 + (generated % 100);
					ppu.sprites[npc_index].y = 0;
					ppu.sprites[npc_index].index =  
						32+player_info->tiles.size()+pole_info->tiles.size();
						
					ppu.sprites[npc_index].attributes = 5;

					// TODO: create bus stop
				}

			};

		auto despawn_npc = [&](size_t npc_index)
			{// from https://cplusplus.com/reference/random/mersenne_twister_engine/
				
						
					ppu.sprites[npc_index].attributes = 5 | (1 << 7);
			};


		// randomly spawn/despawn npcs at bus stops
		
		if ((uint32_t)(player_at.x)%256 < 5 && !spawned_npc)
		{
			spawn_npc(npc_index);
			spawned_npc = true;
			npc_overlapping_time = 0;

		} else if ((uint32_t)(player_at.x)%256 > 202 && spawned_npc)
		{
			despawn_npc(npc_index);
			spawned_npc = false;
			npc_overlapping_time = 0;
		}

		if (spawned_npc)
		{
			if (abs(ppu.sprites[npc_index].x - ppu.sprites[0].x) <= 48
					&& npc_overlapping_time < 300)
				{
					npc_overlapping_time++;
				} else if (npc_overlapping_time >= 300)
				{
					despawn_npc(npc_index);
					spawned_npc = false;
					npc_overlapping_time = 0;
					num_picked_up++;

				}
		}
	} else
	{

		for (size_t i = 0; i < 64; i++)
		{
			ppu.sprites[i].attributes = 5 | (1 << 7);
		}

		// helper function for generating background tiles
		auto make_bg_tile = [&](size_t index, size_t palette)
		{
			return index | (palette << 8);
		};

		for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {

			if (x % 4 == 0)
			{
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					make_bg_tile(t_indices["winlose"].x + win, 6);
			} else {
				ppu.background[x+y*PPU466::BackgroundWidth] = 
					make_bg_tile(t_indices["winlose"].x + 2, 6);
			}
		}
		}
	}







	

	//--- actually draw ---
	ppu.draw(drawable_size);
}
