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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/xlibs/xutil/xglobal.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#define CPUINFODLL "cpuinf32.dll"

unsigned int xt_processor_id = 0;
unsigned int xt_processor_type = 0;
unsigned int xt_processor_family = 0;
unsigned int xt_processor_model = 0;
unsigned int xt_processor_revision = 0;
unsigned int xt_processor_features = 0;

int xt_mmxUse;

unsigned int xt_get_cpuid() {
	unsigned int type, family, model, revision, id, clone;
	type = family = model = revision = id = clone = 0;

	HINSTANCE hLibrary = LoadLibrary(CPUINFODLL);
	if (!hLibrary) {
		/*
		        char buf[256];
		        sprintf(buf, "Error loading library: %s\n", CPUINFODLL);
		        ErrH.Abort(buf);
		*/
		return 0;
	}
	typedef WORD(*WORDvoid)();
	typedef DWORD(*DWORDvoid)();

	WORD(*lpfnwincpuidsupport)();
	WORD(*lpfnwincpuidext)();
	DWORD(*lpfnwincpufeatures)();

	lpfnwincpuidsupport = (WORDvoid)GetProcAddress(hLibrary, "wincpuidsupport");
	lpfnwincpuidext = (WORDvoid)GetProcAddress(hLibrary, "wincpuidext");
	lpfnwincpufeatures = (DWORDvoid)GetProcAddress(hLibrary, "wincpufeatures");

	if (lpfnwincpuidsupport()) {
		id = lpfnwincpuidext();
		xt_processor_type = (id & ((1 << 16)) - 1) >> 12;
		xt_processor_family = (id & ((1 << 12)) - 1) >> 8;
		xt_processor_model = (id & ((1 << 8)) - 1) >> 4;
		xt_processor_revision = id & ((1 << 4)) - 1;
		xt_processor_features = lpfnwincpufeatures() & 0xFFFF0000;
		return 1;
	} else return 0;
}

char* xt_getMMXstatus() {
	static char message[32];
	strcpy(message, "MMX ");

	if (xt_get_cpuid())
		if (xt_processor_features & INTEL_MMX) {
			xt_mmxUse = 1;
			strcat(message, "detected...");
		}

	if (!xt_mmxUse) strcat(message, "is absent...");

	return message;
}

} // namespace QDEngine
