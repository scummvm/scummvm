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

#include "freescape/freescape.h"

namespace Freescape {

static const entrancesTableEntry rawEntranceTable[] = {
	{183, {36, 137, 13}}, // Correct?
	{184, {36, 137, 13}}, // TODO
	{185, {36, 137, 13}}, // TODO
	{186, {36, 137, 13}}, // TODO
	{187, {36, 137, 13}}, // TODO
	{188, {36, 137, 13}}, // TODO
	{190, {36, 137, 13}}, // TODO
	{191, {36, 137, 13}}, // TODO
	{192, {36, 137, 13}}, // TODO
	{193, {36, 137, 13}}, // TODO
	{194, {36, 137, 13}}, // TODO
	{195, {36, 137, 13}}, // TODO
	{196, {36, 137, 13}}, // TODO
	{197, {203, 0, 31}},  // TODO
	{198, {36, 137, 13}}, // TODO
	{199, {36, 137, 13}}, // TODO
	{200, {36, 137, 13}}, // TODO
	{201, {36, 137, 13}}, // TODO
	{202, {360, 0, 373}}, // TODO
	{203, {207, 0, 384}},
	{204, {207, 0, 372}},
	{206, {36, 137, 13}}, // TODO
	{0, {0, 0, 0}},        // NULL
};

EclipseEngine::EclipseEngine(OSystem *syst) : FreescapeEngine(syst) {
	const entrancesTableEntry *entry = rawEntranceTable;
	while (entry->id) {
		_entranceTable[entry->id] = entry;
		entry++;
	}
}

} // End of namespace Freescape