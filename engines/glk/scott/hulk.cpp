#include "hulk.h"
#include "globals.h"
#include "scott.h"

namespace Glk {
namespace Scott {

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

} // End of namespace Scott
} // End of namespace Glk
