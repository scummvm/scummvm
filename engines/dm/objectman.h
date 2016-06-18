#include "dm.h"
#include "champion.h"


namespace DM {

class ObjectMan {
	DMEngine *_vm;
public:
	ObjectMan(DMEngine *vm);
	IconIndice getObjectType(Thing thing); // @ F0032_OBJECT_GetType

};

}
