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

#ifndef LASTEXPRESS_FIGHTER_H
#define LASTEXPRESS_FIGHTER_H

#include "lastexpress/fight/fight.h"

namespace LastExpress {

class LastExpressEngine;
class CFight;

struct Seq;
struct Sprite;

class CFighter {

public:
	CFighter(LastExpressEngine *engine, CFight *fight);
	virtual ~CFighter();

	virtual void timer();
	void doAction(int sequenceIndex, int action);
	void newSeq();
	bool init(CFighter *opponent);
	virtual bool actionAvailable(int action);
	int getAction();
	int getDodges();
	int getHitPoints();
	void setHitPoints(int hitPoints);
	bool isDead();
	virtual void send(int action);

protected:
	LastExpressEngine *_engine = nullptr;

	CFight *_fight = nullptr;
	CFighter *_opponent = nullptr;
	Seq *_seqs[10];
	int _numSeqs = 0;
	int _currentActionIdx = 0;
	Seq *_currentSeq = nullptr;
	Sprite *_currentSprite = nullptr;
	int _currentSpriteIdx = 0;
	int _unusedFlag = 0;
	int _nextMessage = 0;
	int _nextSequenceIdx = 0;
	int _hitPoints = 0;
	int _dodges = 0;
	int _timer = 0;
};

// Generic fighters

class CCath : public CFighter {
public:
	CCath(LastExpressEngine *engine, CFight *fight) : CFighter(engine, fight) {}

	virtual void timer() override;
};

class COpponent : public CFighter {
public:
	COpponent(LastExpressEngine *engine, CFight *fight) : CFighter(engine, fight) {}

	virtual void timer() override;
};


// First fight: Cath vs Milos

class CCath1 : public CCath {
public:
	CCath1(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	bool actionAvailable(int action) override;
	void send(int action) override;
};

class COpponent1 : public COpponent {
public:
	COpponent1(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	void send(int action) override;
};


// Second fight: Cath vs Vesna (when saving Anna)

class CCath2 : public CCath {
public:
	CCath2(LastExpressEngine *engine, CFight *fight);

	void send(int action) override;
};

class COpponent2 : public COpponent {
public:
	COpponent2(LastExpressEngine *engine, CFight *fight);

	void timer() override;
};


// Third fight: Cath vs Ivo

class CCath3 : public CCath {
public:
	CCath3(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	bool actionAvailable(int action) override;
	void send(int action) override;
};

class COpponent3 : public COpponent {
public:
	COpponent3(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	void send(int action) override;
};


// Fourth fight: Cath vs Salko

class CCath4 : public CCath {
public:
	CCath4(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	bool actionAvailable(int action) override;
	void send(int action) override;
};

class COpponent4 : public COpponent {
public:
	COpponent4(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	void send(int action) override;
};


// Fifth fight: Cath vs Vesna (final fight)

class CCath5 : public CCath {
public:
	CCath5(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	bool actionAvailable(int action) override;
	void send(int action) override;
};

class COpponent5 : public COpponent {
public:
	COpponent5(LastExpressEngine *engine, CFight *fight);

	void timer() override;
	void send(int action) override;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_FIGHTER_H
