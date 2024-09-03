#include "load_save_png.hpp"
#include <iostream> 
#include <glm/glm.hpp>
#include "data_path.hpp"

int main()
{
    glm::uvec2 size = {8,24};
    write_asset(data_path("dist/assets/placeholder.png"), data_path("dist/assets/palette1.png"), &size);
    return 0;
}