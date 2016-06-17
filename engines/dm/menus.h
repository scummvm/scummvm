#ifndef DM_MENUS_H
#define DM_MENUS_H

#include "dm.h"

namespace DM {

class MenuMan {
	DMEngine *_vm;
public:
	MenuMan(DMEngine *vm);

	void drawMovementArrows();
};

}

#endif // !DM_MENUS_H
