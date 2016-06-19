#include "menus.h"
#include "gfx.h"
#include "champion.h"
#include "dungeonman.h"
#include "objectman.h"


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

void MenuMan::drawActionIcon(ChampionIndex championIndex) {
	if (!_actionAreaContainsIcons)
		return;
	DisplayMan &dm = *_vm->_displayMan;
	Champion &champion = _vm->_championMan->_champions[championIndex];

	Box box;
	box._x1 = championIndex * 22 + 233;
	box._x2 = box._x1 + 19 + 1;
	box._y1 = 86;
	box._y2 = 120 + 1;
	dm._useByteBoxCoordinates = false;
	if (!champion._currHealth) {
		dm.clearScreenBox(kColorBlack, box);
		return;
	}
	byte *bitmapIcon = dm._tmpBitmap;
	Thing thing = champion.getSlot(kChampionSlotActionHand);
	IconIndice iconIndex;
	if (thing == Thing::_thingNone) {
		iconIndex = kIconIndiceActionEmptyHand;
	} else if (gObjectInfo[_vm->_dungeonMan->getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->getIconIndex(thing);
	} else {
		dm.clearBitmap(bitmapIcon, 16, 16, kColorCyan);
		goto T0386006;
	}
	_vm->_objectMan->extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.blitToBitmapShrinkWithPalChange(bitmapIcon, 16, 16, bitmapIcon, 16, 16, gPalChangesActionAreaObjectIcon);
T0386006:
	dm.clearScreenBox(kColorCyan, box);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2; // no need to add +1 for exclusive boundaries, box already has that
	box2._y1 = 95;
	box2._y2 = 110 + 1;
	dm.blitToScreen(bitmapIcon, 16, 0, 0, box2._x1, box2._x2, box2._y1, box2._y2);
	if (champion.getAttributes(kChampionAttributeDisableAction) || _vm->_championMan->_candidateChampionOrdinal || _vm->_championMan->_partyIsSleeping) {		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");	}}}