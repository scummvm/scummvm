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

#include "bolt/bolt.h"

namespace Bolt {

BarkerTable *BoltEngine::createBarker(int16 minIndex, int16 maxIndex) {
	bool success = false;

	BarkerTable *table = new BarkerTable();
	if (table) {
		table->minIndex = minIndex;
		table->maxIndex = maxIndex;
		table->count = maxIndex - minIndex + 1;

		table->handlers = new SideShowHandler[table->count];
		if (!table->handlers) {
			freeBarker(table);
		} else {
			success = true;
			for (int i = 0; i < table->count; i++) {
				table->handlers[i] = nullptr;
			}
		}
	}

	if (success) {
		g_curErrorCode = 0;
		return table;
	} else {
		g_curErrorCode = 1;
		return nullptr;
	}
}

void BoltEngine::freeBarker(BarkerTable *table) {
	if (!table)
		return;

	if (table->handlers)
		delete[] table->handlers;

	delete table;
}

bool BoltEngine::registerSideShow(BarkerTable *table, SideShowHandler handler, int16 boothId) {
	if (table->minIndex > boothId || table->maxIndex < boothId) {
		g_curErrorCode = 5;
		return false;
	}

	table->handlers[boothId - table->minIndex] = handler;

	g_curErrorCode = 0;
	return true;
}

int16 BoltEngine::barker(BarkerTable *table, int16 startBooth) {
	int16 prevBooth = 0;
	int16 currentBooth = startBooth;

	while (true) {
		SideShowHandler handler = table->handlers[currentBooth - table->minIndex];

		int16 nextBooth = (this->*handler)(prevBooth);

		if (nextBooth == 0)
			break;

		prevBooth = currentBooth;
		currentBooth = nextBooth;
	}

	return g_currentBoothId;
}

bool BoltEngine::checkError() {
	return false;
}

} // End of namespace Bolt
