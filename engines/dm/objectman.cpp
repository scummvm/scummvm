#include "objectman.h"
#include "dungeonman.h"


namespace DM {

int16 gIconGraphicHeight[7] = {32, 32, 32, 32, 32, 32, 32}; // @ K0077_ai_IconGraphicHeight

int16 gIconGraphicFirstIndex[7] = { // G0026_ai_Graphic562_IconGraphicFirstIconIndex
	0,     /* First icon index in graphic #42 */
	32,    /* First icon index in graphic #43 */
	64,    /* First icon index in graphic #44 */
	96,    /* First icon index in graphic #45 */
	128,   /* First icon index in graphic #46 */
	160,   /* First icon index in graphic #47 */
	192}; /* First icon index in graphic #48 */

ObjectMan::ObjectMan(DMEngine *vm) : _vm(vm) {}

IconIndice ObjectMan::getObjectType(Thing thing) {
	if (thing == Thing::_thingNone)
		return kIconIndiceNone;

	int16 objectInfoIndex = _vm->_dungeonMan->getObjectInfoIndex(thing);
	if (objectInfoIndex != -1) {
		objectInfoIndex = gObjectInfo[objectInfoIndex]._type;
	}
	return (IconIndice)objectInfoIndex;
}

byte gChargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}; // @ G0029_auc_Graphic562_ChargeCountToTorchType

IconIndice ObjectMan::getIconIndex(Thing thing) {
	IconIndice iconIndex = getObjectType(thing);

	if ((iconIndex != kIconIndiceNone) &&
		((iconIndex < kIconIndiceWeaponDagger) &&(iconIndex >= kIconIndiceJunkCompassNorth)) || // < instead of <= is no error
		((iconIndex >= kIconIndicePotionMaPotionMonPotion) && (iconIndex <= kIconIndicePotionWaterFlask)) ||
		(iconIndex == kIconIndicePotionEmptyFlask)
		) {
		uint16 *rawType = _vm->_dungeonMan->getThingData(thing);
		switch (iconIndex) {
		case kIconIndiceJunkCompassNorth:
			iconIndex = (IconIndice)(iconIndex + _vm->_dungeonMan->_currMap._partyDir);
			break;
		case kIconIndiceWeaponTorchUnlit: {
			Weapon weapon(rawType);
			if (weapon.isLit()) {
				iconIndex = (IconIndice)(iconIndex + gChargeCountToTorchType[weapon.getChargeCount()]);
			}
			break;
		}
		case kIconIndiceScrollOpen:
			if (Scroll(rawType).getClosed()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		case kIconIndiceJunkWater:
		case kIconIndiceJunkIllumuletUnequipped:
		case kIconIndiceJunkJewelSymalUnequipped:
			if (Junk(rawType).getChargeCount()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		case kIconIndiceWeaponBoltBladeStormEmpty:
		case kIconIndiceWeaponFlamittEmpty:
		case kIconIndiceWeaponStormringEmpty:
		case kIconIndiceWeaponFuryRaBladeEmpty:
		case kIconIndiceWeaponEyeOfTimeEmpty:
		case kIconIndiceWeaponStaffOfClawsEmpty:
			if (Weapon(rawType).getChargeCount()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		}
	}

	return iconIndex;
}

void ObjectMan::extractIconFromBitmap(uint16 iconIndex, byte *destBitmap) {
	int16 i;
	for (i = 0; i < 7; ++i)
		if (gIconGraphicFirstIndex[i] > iconIndex)
			break;
	--i;
	byte *srcBitmap = _vm->_displayMan->getBitmap(kObjectIcons_000_TO_031 + i);
	iconIndex -= gIconGraphicFirstIndex[i];
	_vm->_displayMan->_useByteBoxCoordinates = true;
	_vm->_displayMan->blitToBitmap(srcBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, destBitmap, 16, 0, 16, 0, 16, kColorNoTransparency);

}

}