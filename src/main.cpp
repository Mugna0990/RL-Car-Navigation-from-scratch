#include "Game.h"
#include "MapEditor.h"

int main() {
    
    
    MapEditor editor(120, 120, 5);
    editor.run();
    Game game;
    game.run();
    return 0;
}
