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


}