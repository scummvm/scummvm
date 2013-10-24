#include "prince/graphics.h"

#include "prince/prince.h"

#include "graphics/palette.h"

namespace Prince {

GraphicsMan::GraphicsMan(PrinceEngine *vm) 
    : _vm(vm), _changed(false) {
	initGraphics(640, 480, true);
    _frontScreen = new Graphics::Surface();
    _frontScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
}

void GraphicsMan::update() {
    if (_changed) {
        _vm->_system->copyRectToScreen((byte*)_frontScreen->getBasePtr(0,0), 640, 0, 0, 640, 480);

        _vm->_system->updateScreen();
    }
}

void GraphicsMan::setPalette(const byte *palette) {
    _vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void GraphicsMan::change() {
    _changed = true;
}

void GraphicsMan::draw(const Graphics::Surface *s)
{
   for (uint y = 0; y < 480; y++)
       memcpy((byte*)_frontScreen->getBasePtr(0, y), (byte*)s->getBasePtr(0, y), 640);
   change();
}

void GraphicsMan::drawTransparent(const Graphics::Surface *s)
{
    for (uint y = 0; y < 480; ++y) {
        for (uint x = 0; x < 640; ++x) {
            byte pixel = *((byte*)s->getBasePtr(x,y));
            if (pixel != 255) {
                *((byte*)_frontScreen->getBasePtr(x, y)) = pixel;
            }
        }
    }
   change();
}

}
