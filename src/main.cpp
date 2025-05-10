#include "Game.h"
#include "MapEditor.h"
#include "util.h"

int main() {
    
    
    MapEditor editor(MAP_WIDTH, MAP_HEIGHT, 3);
    editor.run();
    Game game;
    game.run();
    return 0;
}
