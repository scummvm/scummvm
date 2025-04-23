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

#ifndef LASTEXPRESS_FIGHT_H
#define LASTEXPRESS_FIGHT_H

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class LastExpressEngine;
class CFighter;

struct Seq;
struct Event;

class CFight {
public:
	CFight(LastExpressEngine *engine, int fightId);
	~CFight();

	int process();
	void setOutcome(int outcome);
	void endFight(int outcome);
	void timer(Event *event, bool isProcessing);
	void mouse(Event *event);

	void setFightHappening(bool happening) { _fightIsHappening = happening; }
	bool fightHappening() { return _fightIsHappening; }

private:
	LastExpressEngine *_engine = nullptr;

	CFighter *_cath = nullptr;
	CFighter *_opponent = nullptr;
	int _outcome = 0;
	int _currentSeqIdx = 0;
	Common::String _seqNames[20];
	bool _fightIsHappening = false;

	void (LastExpressEngine::*_savedTimerEventHandle)(Event *) = nullptr;
	void (LastExpressEngine::*_savedMouseEventHandle)(Event *) = nullptr;

	bool _lowIconToggle = false;
	int _eggIconBrightness = 0;
	int _eggIconBrightnessStep = 1;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_FIGHT_H
