/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_DUCKMAN_CREDITS_H
#define ILLUSIONS_DUCKMAN_CREDITS_H

#include "illusions/illusions.h"

namespace Illusions {

class IllusionsEngine_Duckman;

struct DCreditsItem {
	uint32 objectId;
	bool active;
	int16 scrollPosIndex;
	int16 scrollPosY;
};

class DuckmanCredits {
public:
	DuckmanCredits(IllusionsEngine_Duckman *vm);
	~DuckmanCredits();
	void start();
public:
	typedef Common::Array<DCreditsItem> DCreditsItems;
	IllusionsEngine_Duckman *_vm;
	uint32 _lastUpdateTicks;
	uint32 _nextUpdateTicks;
	int _lastItemIndex;
	bool _endReached;
	DCreditsItems _creditsItems;
	char *_currText;
	int update(uint flags);
	char *readNextLine();
	Common::Point getItemPosition(int index);
	void charToWChar(char *text, uint16 *wtext, uint size);

};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
