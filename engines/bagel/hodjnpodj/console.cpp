/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/hodjnpodj/console.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {

Console::Console() : GUI::Debugger() {
	registerCmd("dumpres", WRAP_METHOD(Console, cmdDumpRes));
#ifndef RELEASE_BUILD
	registerCmd("clicks", WRAP_METHOD(Console, cmdClicks));
#endif
}

Console::~Console() {
}

bool Console::cmdDumpRes(int argc, const char **argv) {
	if (argc == 2) {
		int num = atoi(argv[1]);
		Common::String name = Common::String::format("#%d", num);
		HINSTANCE hInst = nullptr;
		uint dwBytes;
		HRSRC hRsc;
		HGLOBAL hGbl;
		byte *pData;

		hRsc = FindResource(hInst, name.c_str(), RT_BITMAP);
		if (hRsc != nullptr) {
			dwBytes = (size_t)SizeofResource(hInst, hRsc);
			hGbl = LoadResource(hInst, hRsc);
			if ((dwBytes != 0) && (hGbl != nullptr)) {
				pData = (byte *)LockResource(hGbl);
				Common::MemoryReadStream rs(pData, dwBytes);
				Common::DumpFile df;
				if (df.open("dump.bin"))
					df.writeStream(&rs);

				UnlockResource(hGbl);
				FreeResource(hGbl);

				if (df.isOpen())
					debugPrintf("Created dump.bin\n");
				else
					debugPrintf("Could not create dump.bin\n");
			} else {
				debugPrintf("Could not find resource\n");
			}
		} else {
			debugPrintf("Could not find resource\n");
		}
	} else {
		debugPrintf("dumpres [num]\n");
	}

	return true;
}

#ifndef RELEASE_BUILD
bool Console::cmdClicks(int argc, const char **argv) {
	debugPrintf("Initial seed: %u\n", g_engine->getRandomSeed());

	auto &clicks = g_engine->_metagameClicks;
	if (clicks.empty()) {
		debugPrintf("No boardgame clicks yet.\n");
	} else {
		debugPrintf("Boardgame clicks:\n");
		for (const Common::Point &pt : clicks)
			debugPrintf("(%d, %d)\n", pt.x, pt.y);
	}

	return true;
}
#endif

} // namespace HodjNPodj
} // namespace Bagel
