#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <map>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f, 4.0f);

	bool spawned_npc = false;


	// how many draw cycles has the npc overlapped with the buss
	uint32_t npc_overlapping_time = 0;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	std::map<std::string, glm::u32vec2> t_indices;

	// number of npcs picked up
	size_t num_picked_up = 0;

	float total_elapsed = 0.0f;
	float percent_done = 0.0f;
	glm::uvec4 day_sky = {36, 178, 255, 255};
	glm::uvec4 night_sky = {18, 1, 1, 255};

	bool done = false;
	bool win = false;

	

};
