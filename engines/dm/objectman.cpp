#include "objectman.h"
#include "dungeonman.h"


namespace DM {

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

int16 ObjectMan::getIconIndex(Thing thing) {
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

}