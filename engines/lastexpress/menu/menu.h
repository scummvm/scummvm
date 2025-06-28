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

#ifndef LASTEXPRESS_MENU_H
#define LASTEXPRESS_MENU_H

#include "lastexpress/lastexpress.h"
#include "lastexpress/shared.h"

namespace LastExpress {

class LastExpressEngine;

struct Sprite;
struct Seq;
struct Event;
struct Link;

class Menu {
public:
	Menu(LastExpressEngine *engine);
	~Menu();

	void doEgg(bool doSaveGame, int type, int32 time);
	void endEgg();
	void eggFree();
	void eggMouse(Event *event);
	void eggTimer(Event *event);
	void clearSprites();
	void updateEgg();
	bool eggCursorAction(int8 action, int8 flags);
	void setSprite(int sequenceType, int index, bool redrawFlag);
	void setCity(int cityIndex);
	void switchEggs(int whichEgg);
	bool isShowingMenu();

	void setEggTimerDelta(int delta);
	int getEggTimerDelta();

protected:
	LastExpressEngine *_engine = nullptr;
	bool _isShowingMenu = false;
	bool _hasShownIntro = false;
	bool _hasShownStartScreen = false;
	Link *_currentHotspotLink = nullptr;
	bool _moveClockHandsFlag = false;
	int _eggTimerDelta = 15;
	int32 _eggCurrentMouseFlags = 0;
	int32 _eggCreditsIndex = 0;

	Seq *_menuSeqs[8] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	Sprite *_startMenuFrames[8] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	char _eggButtonsSeqNames[8][13] = {
		"helpnewr.seq",
		"buttns.seq",
		"quit.seq",
		"bogus!.seq",
		"jlinetl.seq",
		"jlinecen.seq",
		"jlinebr.seq",
		"credits.seq"
	};

	uint8 _cityIndexes[8] = { 64, 128, 129, 130, 131, 132, 192, 0 };
	bool _gameInNotStartedInFile = false;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_MENU_H
