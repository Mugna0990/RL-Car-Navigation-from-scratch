#include "./game/Game.h"
#include "./UI/MapEditor.h"

int main() {
    
    
    MapEditor editor(80, 80, 10);
    editor.run();
    Game game;
    game.run();
    return 0;
}
