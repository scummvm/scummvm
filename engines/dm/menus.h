#ifndef DM_MENUS_H
#define DM_MENUS_H

#include "dm.h"

namespace DM {

class MenuMan {
	DMEngine *_vm;
public:
	bool _shouldRefreshActionArea; // @ G0508_B_RefreshActionArea
	MenuMan(DMEngine *vm);

	void drawMovementArrows();
};

}

#endif // !DM_MENUS_H
