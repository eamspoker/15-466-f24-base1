#include "load_save_png.hpp"
#include <iostream> 
#include <glm/glm.hpp>
#include "data_path.hpp"

int main()
{
    glm::uvec2 size = {16, 48};
    write_asset(data_path("dist/assets/placeholder.png"), data_path("dist/assets/palette1.png"), &size);
    glm::uvec2 ground_size = {8,64};
    write_asset(data_path("dist/assets/ground.png"), data_path("dist/assets/ground_palette.png"), &ground_size);
    glm::uvec2 sky_size = {8,128};
    write_asset(data_path("dist/assets/sky.png"), data_path("dist/assets/sky_palette.png"), &sky_size);
    glm::uvec2 pole_size = {16,64};
    write_asset(data_path("dist/assets/pole.png"), data_path("dist/assets/pole_palette.png"), &pole_size);
    glm::uvec2 person_size = {8,8};
    write_asset(data_path("dist/assets/person.png"), data_path("dist/assets/person_palette.png"), &person_size);
    glm::uvec2 checkbox_size = {16,32};
    write_asset(data_path("dist/assets/checkbox.png"), data_path("dist/assets/checkbox_palette.png"), &checkbox_size);
    glm::uvec2 winlose_size = {8,24};
    write_asset(data_path("dist/assets/winlose.png"), data_path("dist/assets/ground_palette.png"), &winlose_size);
    return 0;
}