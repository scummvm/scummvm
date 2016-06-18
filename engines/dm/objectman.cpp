#include "objectman.h"
#include "dungeonman.h"


namespace DM {

ObjectMan::ObjectMan(DMEngine *vm): _vm(vm) {}

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


}