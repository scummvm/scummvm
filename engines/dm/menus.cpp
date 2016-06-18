#include "menus.h"
#include "gfx.h"


namespace DM {

MenuMan::MenuMan(DMEngine *vm): _vm(vm) {}

void MenuMan::drawMovementArrows() {
	DisplayMan &disp = *_vm->_displayMan;
	byte *arrowsBitmap = disp.getBitmap(kMovementArrowsIndice);
	Box &dest = gBoxMovementArrows;
	uint16 w = disp.getWidth(kMovementArrowsIndice);

	disp.blitToScreen(arrowsBitmap, w, 0, 0, dest._x1, dest._x2, dest._y1, dest._y2, kColorNoTransparency);
}

}