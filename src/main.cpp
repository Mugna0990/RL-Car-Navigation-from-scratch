#include "Game.h"
#include "MapEditor.h"

int main() {
    
    Game game;
    MapEditor editor(80, 80, 10);
    editor.run();
    game.run();
    return 0;
}
