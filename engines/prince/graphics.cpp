#include "prince/graphics.h"

#include "prince/prince.h"

#include "graphics/palette.h"

namespace Prince {

GraphicsMan::GraphicsMan(PrinceEngine *vm) 
    : _vm(vm), _changed(false) {
	initGraphics(640, 480, true);
}

void GraphicsMan::update() {
    if (_changed) {
        _vm->_system->copyRectToScreen((byte*)_roomBackground->getBasePtr(0,0), 640, 0, 0, 640, 480);

        _vm->_system->updateScreen();
    }
}

void GraphicsMan::setPalette(const byte *palette) {
    _vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void GraphicsMan::change() {
    _changed = true;
}

}
