#include "engines/stark/ui/menu/mainmenu.h"

namespace Stark {

MainMenuScreen::MainMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
        StaticLocationScreen(gfx, cursor, "MainMenuLocation", Screen::kScreenMainMenu) {
}

MainMenuScreen::~MainMenuScreen() {
}

void MainMenuScreen::open() {
    StaticLocationScreen::open();

    //TODO: push back widgets
}

}