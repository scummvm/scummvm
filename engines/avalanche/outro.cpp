#include "graphics/cursorman.h"
#include "common/system.h"

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"
#include "avalanche/enums.h"
#include "avalanche/outro.h"

namespace Avalanche {

Outro::Outro(AvalancheEngine *vm) : _vm(vm) {
}

void Outro::run() {
    CursorMan.showMouse(false);

    Common::File file;
	if (!file.open("avalot.fnt"))
		error("AVALANCHE: Scrolls: File not found: avalot.fnt");
	for (int16 i = 0; i < 256; i++)
		file.read(_vm->_font[i], 16);
	file.close();

    // Black background
    _vm->_graphics->blackOutScreen();

    // Copyright line - top right, small gray text
    _vm->_graphics->drawNormalText("(c) 1995, Mike, Mark and Thomas Thurman.",
        _vm->_font, 16, 300, 5, kColorLightgray);

    // "hanks" - large dark red
    _vm->_graphics->helpDrawBigTextOutro("Thanks", 80, 20, kColorRed);

    // "for" - large brown/orange
    _vm->_graphics->helpDrawBigTextOutro("for", 150, 55, kColorBrown);

    // "playing" - large brown/orange
    _vm->_graphics->helpDrawBigTextOutro("playing", 50, 85, kColorBrown);

    // "Avalot." - large yellow
    _vm->_graphics->helpDrawBigTextOutro("Avalot.", 80, 125, kColorYellow);

    // "* Goodbye! *" - small cyan, middle right
    _vm->_graphics->drawNormalText("* Goodbye! *",
        _vm->_font, 16, 400, 75, kColorLightcyan);

    // Bottom text - small red
    _vm->_graphics->drawNormalText(
        "If you'd like to see yet more of these games, then don't forget to",
        _vm->_font, 16, 80, 160, kColorRed);
    _vm->_graphics->drawNormalText(
        "register, or your abacus will tickle you",
        _vm->_font, 16, 80, 170, kColorRed);
    _vm->_graphics->drawNormalText(
        "for the rest of your life!             (Only joking!)",
        _vm->_font, 16, 80, 180, kColorRed);

    _vm->_graphics->refreshScreen();

    // Wait 5 seconds or keypress/click
    uint32 startTime = g_system->getMillis();
    Common::Event event;
    while (!_vm->shouldQuit()) {
        if (g_system->getMillis() - startTime >= 5000)
            return;
        while (_vm->getEvent(event)) {
            switch (event.type) {
            case Common::EVENT_KEYDOWN:
            case Common::EVENT_LBUTTONDOWN:
                CursorMan.showMouse(true);
                return;
            default:
                break;
            }
        }
        g_system->delayMillis(10);
    }

    CursorMan.showMouse(true);
}

} // End of namespace Avalanche