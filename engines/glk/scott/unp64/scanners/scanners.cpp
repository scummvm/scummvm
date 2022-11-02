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

#include "glk/scott/unp64/unp64.h"

namespace Glk {
namespace Scott {

void scnECA(UnpStr *unp);
void scnExpert(UnpStr *unp);
void scnCruel(UnpStr *unp);
void scnPuCrunch(UnpStr *unp);
void scnByteBoiler(UnpStr *unp);
void scnMasterCompressor(UnpStr *unp);
void scnTCScrunch(UnpStr *unp);
void scnTBCMultiComp(UnpStr *unp);
void scnXTC(UnpStr *unp);
void scnCCS(UnpStr *unp);
void scnMegabyte(UnpStr *unp);
void scnSection8(UnpStr *unp);
void scnCaution(UnpStr *unp);
void scnActionPacker(UnpStr *unp);
void scnExomizer(UnpStr *unp);

Scnptr g_scanFunc[] = {
	scnECA,
	scnExpert,
	scnCruel,
	scnPuCrunch,
	scnByteBoiler,
	scnMasterCompressor,
	scnTCScrunch,
	scnTBCMultiComp,
	scnXTC,
	scnCCS,
	scnMegabyte,
	scnSection8,
	scnCaution,
	scnActionPacker,
	scnExomizer
};

void scanners(UnpStr* unp) {
	int x, y;
	y = sizeof(g_scanFunc) / sizeof(*g_scanFunc);
	for (x = 0; x < y; x++) {
		(g_scanFunc[x])(unp);
		if (unp->_idFlag)
			break;
	}
}

} // End of namespace Scott
} // End of namespace Glk
