#include "graphics/cursorman.h"
#include "common/system.h"

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"
#include "avalanche/enums.h"
#include "avalanche/outro.h"

namespace Avalanche {

Outro::Outro(AvalancheEngine *vm) : _vm(vm) {
}

void Outro::run () {
    warning(">>> OUTRO: STARTING RUN <<<");
    
    // Disable Mouse and restore the screen size.
    CursorMan.showMouse(false);
    
    // Load image file
    Common::File file;
    if(!file.open("finale.avd")){
        warning(">>> OUTRO: gameover.avd NOT FOUND! Returning. <<<");
        return;
    }
    
    warning(">>> OUTRO: File opened, loading picture... <<<");
    Graphics::Surface img = _vm->_graphics->loadPictureRaw(file, kScreenWidth, kScreenHeight);
    file.close();

    warning(">>> OUTRO: Picture loaded, doing copyFrom... <<<");
    _vm->_graphics->getSurface().copyFrom(img);
    img.free();
    
    warning(">>> OUTRO: Copy done, refreshing screen... <<<");
    _vm->_graphics->refreshScreen();

    warning(">>> OUTRO: Refresh done, delaying for 5 seconds... <<<");
    // Wait 5 seconds
    _vm->_system->delayMillis(5000);
    
    warning(">>> OUTRO: DONE <<<");
}

} // End of namespace Avalanche