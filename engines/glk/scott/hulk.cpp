#include "hulk.h"
#include "globals.h"
#include "scott.h"
#include "definitions.h"

namespace Glk {
namespace Scott {

void hulkShowImageOnExamine(int noun) {
	int image = 0;
	switch (noun) {
	case 55: // Dome
		if (_G(_items)[11]._location == MY_LOC)
			image = 28;
		break;
	case 108: // Natter energy egg
		if (_G(_items)[17]._location == MY_LOC || _G(_items)[17]._location == CARRIED)
			image = 30;
		break;
	case 124: // Bio-Gem
	case 41:
		if (_G(_items)[18]._location == MY_LOC || _G(_items)[18]._location == CARRIED)
			image = 29;
		break;
	case 21: // Killer Bees
		if (_G(_items)[24]._location == MY_LOC)
			image = 31;
		break;
	case 83: // Iron ring
		if (_G(_items)[33]._location == MY_LOC)
			image = 32;
		break;
	case 121: // Cage
		if (_G(_items)[47]._location == MY_LOC)
			image = 33;
		break;
	default:
		break;
	}
	if (image) {
		g_vm->drawImage(image);
		g_vm->output(_G(_sys)[HIT_ENTER]);
		g_vm->hitEnter();
	}
}

void hulkLook() {
	g_vm->drawImage(_G(_rooms)[MY_LOC]._image);
	for (int ct = 0; ct <= _G(_gameHeader)._numItems; ct++) {
		int image = _G(_items)[ct]._image;
		if (_G(_items)[ct]._location == MY_LOC && image != 255) {
			/* Don't draw bio gem in fuzzy area */
			if ((ct == 18 && MY_LOC != 15) ||
				/* Don't draw Dr. Strange until outlet is plugged */
				(ct == 26 && _G(_items)[28]._location != MY_LOC))
				continue;
			g_vm->drawImage(image);
		}
	}
}

void drawHulkImage(int p) {
	int image = 0;
	switch (p) {
	case 85:
		image = 34;
		break;
	case 86:
		image = 35;
		break;
	case 83:
		image = 36;
		break;
	case 84:
		image = 37;
		break;
	case 87:
		image = 38;
		break;
	case 88:
		image = 39;
		break;
	case 89:
		image = 40;
		break;
	case 82:
		image = 41;
		break;
	case 81:
		image = 42;
		break;
	default:
		error("Unhandled image number %d!\n", p);
		break;
	}

	if (image != 0) {
		g_vm->drawImage(image);
		g_vm->output(_G(_sys)[HIT_ENTER]);
		g_vm->hitEnter();
	}
}

} // End of namespace Scott
} // End of namespace Glk
