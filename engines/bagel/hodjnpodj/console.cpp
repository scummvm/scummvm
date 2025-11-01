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

} // namespace HodjNPodj
} // namespace Bagel
