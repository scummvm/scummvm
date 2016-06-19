#include "inventory.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"



namespace DM {

Box gBoxFloppyZzzCross = Box(174, 218, 2, 12); // @ G0041_s_Graphic562_Box_ViewportFloppyZzzCross

InventoryMan::InventoryMan(DMEngine *vm): _vm(vm) {}

void InventoryMan::toggleInventory(ChampionIndex championIndex) {
	ChampionMan &cm = *_vm->_championMan;
	EventManager &em = *_vm->_eventMan;
	DisplayMan &dm = *_vm->_displayMan;

	if ((championIndex == kChmpionCloseInventory) && !cm._champions[championIndex]._currHealth)
		return;
	if (_vm->_pressingEye || _vm->_pressingMouth)
		return;
	_vm->_stopWaitingForPlayerInput = true;
	int16 invChampOrdinal = _inventoryChampionOrdinal; // copy, as the original will be edited
	if (indexToOrdinal(championIndex) == invChampOrdinal) {
		championIndex = kChmpionCloseInventory;
	}

	Champion *champion;
	if (invChampOrdinal) {
		_inventoryChampionOrdinal = indexToOrdinal(kChampionNone);
		warning("MISSING CODE: F0334_INVENTORY_CloseChest");
		champion = &cm._champions[ordinalToIndex(kChampionNone)];
		if (champion->_currHealth && !cm._candidateChampionOrdinal) {
			champion->setAttributeFlag(kChampionAttributeStatusBox, true);
			warning("MISSING CODE: F0292_CHAMPION_DrawState");
		}
		if (cm._partyIsSleeping) {
			return;
		}
		if (championIndex == kChmpionCloseInventory) {
			em._refreshMousePointerInMainLoop = true;
			_vm->_menuMan->drawMovementArrows();
			em._secondaryMouseInput = gSecondaryMouseInput_Movement;
			warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
			warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
			return;
		}
	}

	dm._useByteBoxCoordinates = false;
	_inventoryChampionOrdinal = indexToOrdinal(championIndex);
	if (!invChampOrdinal) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}

	champion = &cm._champions[championIndex];
	int16 w = dm.getWidth(kInventoryGraphicIndice);
	int16 h = dm.getHeight(kInventoryGraphicIndice);
	dm.blitToScreen(dm.getBitmap(kInventoryGraphicIndice), w, 0, 0, 0, w, 0, h, kColorNoTransparency, gDungeonViewport);
	if (cm._candidateChampionOrdinal) {
		dm.clearScreenBox(kColorDarkestGray, gBoxFloppyZzzCross, gDungeonViewport);
	} else {
		static Box gBoxFloppy = Box(174, 182, 2, 12); // @ K0300_s_Box_ViewportFloppy
		dm.clearScreenBox(kColorDarkestGray, gBoxFloppy, gDungeonViewport);
	}
	warning("MISSING CODE: F0052_TEXT_PrintToViewport -> HEALTH");
	warning("MISSING CODE: F0052_TEXT_PrintToViewport -> STAMINA");
	warning("MISSING CODE: F0052_TEXT_PrintToViewport -> MANA");

	warning("MISSING CODE: F0291_CHAMPION_DrawSlot in LOOOOOOOOOOOOP");

	champion->setAttributeFlag(kChampionAttributeViewport, true);
	champion->setAttributeFlag(kChampionAttributeStatusBox, true);
	champion->setAttributeFlag(kChampionAttributePanel, true);
	champion->setAttributeFlag(kChampionAttributeLoad, true);
	champion->setAttributeFlag(kChampionAttributeStatistics, true);
	champion->setAttributeFlag(kChampionAttributeNameTitle, true);

	warning("MISSING CODE: F0292_CHAMPION_DrawState");
	em._mousePointerBitmapUpdated = true;
	em._secondaryMouseInput = gSecondaryMouseInput_ChampionInventory;
	warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
	warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
}


}