#include "menus.h"
#include "gfx.h"
#include "champion.h"
#include "dungeonman.h"


namespace DM {

byte gPalChangesActionAreaObjectIcon[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0}; // @ G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon

MenuMan::MenuMan(DMEngine *vm): _vm(vm) {}

void MenuMan::drawMovementArrows() {
	DisplayMan &disp = *_vm->_displayMan;
	byte *arrowsBitmap = disp.getBitmap(kMovementArrowsIndice);
	Box &dest = gBoxMovementArrows;
	uint16 w = disp.getWidth(kMovementArrowsIndice);

	disp.blitToScreen(arrowsBitmap, w, 0, 0, dest._x1, dest._x2, dest._y1, dest._y2, kColorNoTransparency);
}
void MenuMan::clearActingChampion() {
	ChampionMan &cm = *_vm->_championMan;
	if (cm._actingChampionOrdinal) {
		cm._actingChampionOrdinal--;
		cm._champions[cm._actingChampionOrdinal].setAttributeFlag(kChampionAttributeActionHand, true);
		warning("MISSING CODE: F0292_CHAMPION_DrawState");
		cm._actingChampionOrdinal = indexToOrdinal(kChampionNone);
		_shouldRefreshActionArea = true;
	}
}

}