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
 */

#ifndef SCUMM_INSANE_REBEL2_SHARED_H
#define SCUMM_INSANE_REBEL2_SHARED_H

#include "common/keyboard.h"

namespace Common {
class RandomSource;
}

namespace Scumm {

enum Rebel2MenuCommand {
	kRebel2MenuCommandNone,
	kRebel2MenuCommandUp,
	kRebel2MenuCommandDown,
	kRebel2MenuCommandAccept,
	kRebel2MenuCommandCancel
};

enum {
	kRebel2MenuResultCancel = -2,
	kRebel2MenuResultNone = -1
};

class Rebel2Level1Handler {
public:
	enum Result {
		kQuit,
		kComplete,
		kDeath,
		kGameOver,
		kError
	};

	virtual ~Rebel2Level1Handler() {}
	virtual bool shouldQuit() const = 0;
	virtual Result playAttempt(int &lives) = 0;
	virtual bool playComplete() = 0;
	virtual bool playDeath() = 0;
	virtual bool playRetry(int lives) = 0;
	virtual bool playGameOver(int lives) = 0;
};

Rebel2Level1Handler::Result runRebel2Level1(Rebel2Level1Handler &handler, int lives);

// Level 2 - and the chapters built on the same cover shooter - runs three phases of waves
// against a budget. The flow below is the level's design rather than any one release's, so
// the handler only has to play what the runner asks for and report the state it keeps.
class Rebel2Level2Handler {
public:
	enum Result {
		kQuit,
		kComplete,
		kGameOver,
		kError
	};

	// What a finished wave credited toward picking the next one, and whether the phase
	// should stop regardless.
	struct WaveCredit {
		WaveCredit() : bits(0), stop(false) {}

		uint16 bits;
		bool stop;
	};

	virtual ~Rebel2Level2Handler() {}
	virtual bool shouldQuit() const = 0;

	virtual bool playOpening() = 0;
	virtual void beginAttempt() = 0;
	virtual void beginPhase(int phase, bool clearEnemies) = 0;
	virtual int16 waveBudget(int phase) = 0;
	virtual bool playBackgroundWave(int phase) = 0;
	virtual bool playWave(int phase, uint16 selection) = 0;
	virtual WaveCredit creditWave(int16 mask, int16 *budget, int16 threshold) = 0;
	virtual bool playPhaseEnd(int phase) = 0;

	virtual uint16 phaseState() const = 0;
	virtual bool playerDead() const = 0;
	virtual void accumulateKills() = 0;
	virtual void accumulateMisses() = 0;

	// Returns true when the player still has a life and the attempt should restart.
	virtual bool handleDeath(int phase, Result &result) = 0;
	virtual void playComplete(int bonusCount) = 0;
};

Rebel2Level2Handler::Result runRebel2Level2(Rebel2Level2Handler &handler,
		Common::RandomSource &random);

inline Rebel2MenuCommand getRebel2MenuCommand(const Common::KeyState &key) {
	switch (key.keycode) {
	case Common::KEYCODE_UP:
		return kRebel2MenuCommandUp;
	case Common::KEYCODE_DOWN:
		return kRebel2MenuCommandDown;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		return kRebel2MenuCommandAccept;
	case Common::KEYCODE_ESCAPE:
		return kRebel2MenuCommandCancel;
	default:
		return kRebel2MenuCommandNone;
	}
}

inline int applyRebel2MenuCommand(Rebel2MenuCommand command, int itemCount,
		int &selection) {
	if (itemCount <= 0)
		return kRebel2MenuResultNone;

	switch (command) {
	case kRebel2MenuCommandUp:
		if (--selection < 0)
			selection = itemCount - 1;
		break;
	case kRebel2MenuCommandDown:
		if (++selection >= itemCount)
			selection = 0;
		break;
	case kRebel2MenuCommandAccept:
		return selection >= 0 && selection < itemCount ? selection : kRebel2MenuResultNone;
	case kRebel2MenuCommandCancel:
		return kRebel2MenuResultCancel;
	default:
		break;
	}
	return kRebel2MenuResultNone;
}

inline bool updateRebel2Fire(bool pressed, bool wasPressed,
		bool rapidFire, bool autoFire, int16 &rapidFireCounter) {
	const bool pressedEdge = pressed && !wasPressed;
	if (pressedEdge)
		rapidFireCounter = 0;

	bool rapidFireShot = false;
	if (rapidFire) {
		rapidFireShot = pressed && rapidFireCounter % 5 == 0;
		rapidFireCounter = (rapidFireCounter + 1) % 5;
	}
	return pressedEdge || rapidFireShot || autoFire;
}

} // End of namespace Scumm

#endif
