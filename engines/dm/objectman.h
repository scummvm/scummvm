#include "dm.h"
#include "champion.h"


namespace DM {

class ObjectMan {
	DMEngine *_vm;
public:
	ObjectMan(DMEngine *vm);
	IconIndice getObjectType(Thing thing); // @ F0032_OBJECT_GetType
	IconIndice getIconIndex(Thing thing); // @ F0033_OBJECT_GetIconIndex
	void extractIconFromBitmap(uint16 iconIndex, byte *srcBitmap); // F0036_OBJECT_ExtractIconFromBitmap
};

}
