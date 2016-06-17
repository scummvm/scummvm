#include "menus.h"
#include "gfx.h"


namespace DM {

MenuMan::MenuMan(DMEngine *vm): _vm(vm) {}

void MenuMan::drawMovementArrows() {
	DisplayMan &disp = *_vm->_displayMan;
	byte *arrowsBitmap = disp.getBitmap(kMovementArrowsIndice);
	Box &dest = gBoxMovementArrows;
	uint16 w = disp.width(kMovementArrowsIndice);

	disp.blitToScreen(arrowsBitmap, w, 0, 0, dest.X1, dest.X2, dest.Y1, dest.Y2, kColorNoTransparency);
}

}