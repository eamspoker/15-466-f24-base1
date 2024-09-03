#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "Load.hpp"

#include "load_save_png.hpp"
#include "data_path.hpp"

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


PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	{ //use tiles 0-16 as some weird dot pattern thing:
		std::array< uint8_t, 8*8 > distance;
		for (uint32_t y = 0; y < 8; ++y) {
			for (uint32_t x = 0; x < 8; ++x) {
				float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
				d /= glm::length(glm::vec2(4.0f, 4.0f));
				distance[x+8*y] = uint8_t(std::max(0,std::min(255,int32_t( 255.0f * d ))));
			}
		}
		for (uint32_t index = 0; index < 16; ++index) {
			PPU466::Tile tile;
			uint8_t t = uint8_t((255 * index) / 16);
			for (uint32_t y = 0; y < 8; ++y) {
				uint8_t bit0 = 0;
				uint8_t bit1 = 0;
				for (uint32_t x = 0; x < 8; ++x) {
					uint8_t d = distance[x+8*y];
					if (d > t) {
						bit0 |= (1 << x);
					} else {
						bit1 |= (1 << x);
					}
				}
				tile.bit0[y] = bit0;
				tile.bit1[y] = bit1;
			}
			ppu.tile_table[index] = tile;
		}
	}

	//use sprites 31,32,33 as a "player":
	ppu.tile_table[31].bit0 = player_info->tiles[0].bit0;
	ppu.tile_table[31].bit1 = player_info->tiles[0].bit1;

	ppu.tile_table[32].bit0 = player_info->tiles[1].bit0;
	ppu.tile_table[32].bit1 = player_info->tiles[1].bit1;

	ppu.tile_table[33].bit0 = player_info->tiles[2].bit0;
	ppu.tile_table[33].bit1 = player_info->tiles[2].bit1;
	
	
	//makes the outside of tiles 0-16 solid:
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//makes the center of tiles 0-16 solid:
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//used for the player:
	ppu.palette_table[7] = player_info->palette;

	//used for the misc other sprites:
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x88, 0x88, 0xff, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};

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

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade = player_at.x/PPU466::BackgroundWidth;

	constexpr float PlayerSpeed = 30.0f;
	player_at.x += PlayerSpeed * elapsed;
	if (up.pressed) player_at.x += 0.75*(PlayerSpeed * elapsed);
	if (down.pressed) player_at.x -= 0.75*(PlayerSpeed * elapsed);

	// ckpt_time += elapsed;

	// if (ckpt_time > 30.0f)
	// {
	// 	ckpt_time = 0.0f;
	// }
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	// //background color will be some hsv-like fade:
	// ppu.background_color = glm::u8vec4(255,
	// 	255,
	// 	255,
	// 	255
	// );

	// //tilemap gets recomputed every frame as some weird plasma thing:
	// //NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
			
				ppu.background[x+PPU466::BackgroundWidth*y] = 0.0f;

		}
	}

	//background scroll:
	ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprites:
	// back of bus:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 31;
	ppu.sprites[0].attributes = 7;

	// front of bus:
	ppu.sprites[1].x = int8_t(player_at.x+8);
	ppu.sprites[1].y = int8_t(player_at.y);
	ppu.sprites[1].index = 32;
	ppu.sprites[1].attributes = 7;

	ppu.sprites[2].x = int8_t(player_at.x+16);
	ppu.sprites[2].y = int8_t(player_at.y);
	ppu.sprites[2].index = 33;
	ppu.sprites[2].attributes = 7;
	


	//timer sprites:
	// for (uint32_t i = 1; i < 60; ++i) {
	// 	ppu.sprites[i].x = int8_t(i*(PPU466::ScreenWidth/80.0f));
	// 	ppu.sprites[i].y = int8_t(0.52f*PPU466::ScreenHeight);
	// 	ppu.sprites[i].index = 32;
	// 	ppu.sprites[i].attributes = 6;
	// 	if (i > (uint32_t)(ckpt_time*2)) 
	// 		ppu.sprites[i].attributes = 5;

	// }

	//--- actually draw ---
	ppu.draw(drawable_size);
}
