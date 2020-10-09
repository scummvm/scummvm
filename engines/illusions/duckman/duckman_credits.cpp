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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_credits.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/resources/fontresource.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/textdrawer.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "engines/util.h"

namespace Illusions {

// Duckman_SpecialCode

DuckmanCredits::DuckmanCredits(IllusionsEngine_Duckman *vm)
	: _vm(vm) {

}

DuckmanCredits::~DuckmanCredits() {
}

void DuckmanCredits::start() {
	static const struct { uint32 objectId; int scrollPosY; } kCreditsItems[] = {
		{0x40136,   0}, {0x40137,  16}, {0x40138,  32}, {0x40139,  48},
		{0x4013A,  64}, {0x4013B,  80}, {0x4013C,  96}, {0x4013D, 112}
	};
	_currText = (char*)_vm->_resSys->getResource(0x190052)->_data;
	_creditsItems.clear();
	for (uint i = 0; i < ARRAYSIZE(kCreditsItems);  ++i) {
		DCreditsItem creditsItem;
		creditsItem.objectId = kCreditsItems[i].objectId;
		creditsItem.scrollPosY = kCreditsItems[i].scrollPosY;
		creditsItem.scrollPosIndex = 0;
		creditsItem.active = false;
		_creditsItems.push_back(creditsItem);
	}
	uint32 currSceneId = _vm->getCurrentScene();
	_vm->_updateFunctions->add(0, currSceneId, new Common::Functor1Mem<uint, int, DuckmanCredits>(this, &DuckmanCredits::update));
	_nextUpdateTicks = getCurrentTime();
	_lastUpdateTicks = _nextUpdateTicks - 4;
}

int DuckmanCredits::update(uint flags) {

	if (_vm->_pauseCtr > 0) {
		_nextUpdateTicks = getCurrentTime() + 4;
		return 1;
	}

	if (flags & 1) {
		_vm->_scriptResource->_properties.set(0x000E0096, true);
		_lastItemIndex = -1;
		_endReached = false;
		return 2;
	}

	if (!isTimerExpired(_lastUpdateTicks, _nextUpdateTicks)) {
		return 1;
	}

	bool creditsRunning = false;
	int index = 0;
	for (DCreditsItems::iterator it = _creditsItems.begin(); it != _creditsItems.end(); ++it, ++index) {
		DCreditsItem &creditsItem = *it;
		Control *control = _vm->getObjectControl(creditsItem.objectId);
		if (!creditsItem.active && creditsItem.scrollPosY == 0 && !_endReached) {
			creditsItem.active = true;
			creditsItem.scrollPosIndex = 0;
			control->fillActor(0);
			char *text = readNextLine();
			if (!strncmp(text, "&&&END", 6)) {
				creditsItem.active = false;
				_endReached = true;
			} else {
				uint16 wtext[128];
				charToWChar(text, wtext, ARRAYSIZE(wtext));

				FontResource *font = _vm->_dict->findFont(0x120001);
				TextDrawer textDrawer;
				WidthHeight dimensions;
				uint16 *outText;
				control->getActorFrameDimensions(dimensions);
				textDrawer.wrapText(font, wtext, &dimensions, Common::Point(0, 0), 2, outText);
				textDrawer.drawText(_vm->_screen, control->_actor->_surface, 0, 0);
				control->_actor->_flags |= Illusions::ACTOR_FLAG_4000;

				_lastItemIndex = index;
			}
		}
		if (creditsItem.active) {
			if (_endReached && _creditsItems[_lastItemIndex].scrollPosIndex > 53) {
				creditsItem.active = false;
				creditsItem.scrollPosY = -1;
			} else {
				creditsRunning = true;
				control->_actor->_position = getItemPosition(creditsItem.scrollPosIndex);
				++creditsItem.scrollPosIndex;
				if (getItemPosition(creditsItem.scrollPosIndex).x < 0)
					creditsItem.active = false;
			}
		}
		if (creditsItem.scrollPosY > 0)
			--creditsItem.scrollPosY;
	}
	_lastUpdateTicks = _nextUpdateTicks;
	_nextUpdateTicks = getCurrentTime() + 4;

	if (!creditsRunning) {
		_vm->_scriptResource->_properties.set(0x000E0096, true);
		_lastItemIndex = -1;
		_endReached = false;
		return 2;
	}

	return 1;
}

char *DuckmanCredits::readNextLine() {
	static char line[256];
	char *dest = line;
	char *src = _currText;
	do {
		if (*src == 10 || *src == 13) {
			src += 2;
			*dest = 0;
			break;
		}
		*dest++ = *src++;
	} while (1);
	_currText = src;
	return line;
}

Common::Point DuckmanCredits::getItemPosition(int index) {
	static const struct { int16 x, y; } kCreditsItemsPoints[] = {
		{159, 200}, {158, 195}, {157, 190}, {156, 185}, {156, 180}, {157, 176},
		{158, 172}, {159, 168}, {161, 164}, {162, 161}, {163, 158}, {163, 155},
		{162, 152}, {161, 149}, {159, 147}, {158, 144}, {157, 142}, {156, 140},
		{156, 138}, {157, 136}, {158, 134}, {159, 132}, {161, 130}, {162, 128},
		{163, 127}, {163, 126}, {162, 125}, {161, 124}, {159, 123}, {158, 122},
		{157, 121}, {156, 120}, {156, 119}, {157, 118}, {158, 117}, {159, 116},
		{161, 115}, {162, 114}, {163, 113}, {163, 112}, {162, 111}, {161, 110},
		{159, 109}, {158, 108}, {157, 107}, {156, 106}, {156, 105}, {157, 104},
		{158, 103}, {159, 102}, {161, 101}, {162, 100}, {163,  99}, {163,  98},
		{162,  97}, {161,  96}, {159,  95}, {158,  94}, {157,  93}, {156,  92},
		{156,  91}, {157,  90}, {158,  89}, {159,  88}, {161,  87}, {162,  86},
		{163,  85}, {163,  84}, {162,  83}, {161,  82}, {159,  81}, {158,  80},
		{157,  79}, {156,  78}, {156,  77}, {157,  76}, {158,  75}, {159,  74},
		{161,  73}, {162,  72}, {163,  71}, {163,  70}, {162,  69}, {161,  68},
		{159,  67}, {158,  66}, {157,  64}, {156,  62}, {156,  60}, {157,  58},
		{158,  56}, {159,  54}, {161,  52}, {162,  50}, {163,  40}, {163,  40},
		{162,  40}, {161,  40}, {159,  40}, {158,  40}, {157,  40}, {156,  40},
		{156,  40}, {157,  40}, {158,  40}, {159,  40}, {161,  40}, {162,  40},
		{163,  40}, {163,  40}, {162,  40}, {161,  40}, {159,  40}, {158,  40},
		{157,  40}, {156,  40}, {156,  40}, {157,  40}, {158,  40}, {159,  40},
		{161,  40}, {162,  40}, {163,  40}, {163,  40}, {162,  40}, {161,  40},
		{159,  40}, {158,  40}, { -1,  -1}
	};

	if (index < 0 || index >= ARRAYSIZE(kCreditsItemsPoints))
		return Common::Point(-1, -1);
	return Common::Point(kCreditsItemsPoints[index].x, kCreditsItemsPoints[index].y);
}

void DuckmanCredits::charToWChar(char *text, uint16 *wtext, uint size) {
	while (*text != 0 && size > 1) {
		*wtext++ = (byte)*text++;
		--size;
	}
	*wtext++ = 0;
}

} // End of namespace Illusions
