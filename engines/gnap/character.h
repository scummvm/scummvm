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

#ifndef GNAP_CHARACTER_H
#define GNAP_CHARACTER_H

namespace Gnap {

class GnapEngine;

enum Facing {
	kDirNone = 0,
	kDirBottomRight = 1,
	kDirBottomLeft = 3,
	kDirUnk4 = 4,
	kDirUpLeft = 5,
	kDirUpRight = 7
};

class Character {
public:
	Character(GnapEngine *vm);
	~Character();

	virtual int getSequenceId(int kind, int gridX, int gridY) = 0;
	virtual void playSequence(int sequenceId) = 0;
	virtual void updateIdleSequence() = 0;
	virtual void updateIdleSequence2() = 0;

	Common::Point _pos;
	Facing _idleFacing;
	int _actionStatus;
	int _sequenceId;
	int _sequenceDatNum;
	int _id;

protected:
	GnapEngine *_vm;
};

class PlayerGnap : public Character {
public:
	PlayerGnap(GnapEngine *vm);
	virtual int getSequenceId(int kind, int gridX, int gridY);
	virtual void playSequence(int sequenceId);
	virtual void updateIdleSequence();
	virtual void updateIdleSequence2();

	void initBrainPulseRndValue();
	void kissPlatypus(int callback);
	void useDeviceOnPlatypus();
	void useJointOnPlatypus();

	int _brainPulseNum, _brainPulseRndValue;
};

class PlayerPlat : public Character {
public:
	PlayerPlat(GnapEngine *vm);
	virtual int getSequenceId(int kind = 0, int gridX = 0, int gridY = 0);
	virtual void playSequence(int sequenceId);
	virtual void updateIdleSequence();
	virtual void updateIdleSequence2();
};
} // End of namespace Gnap

#endif // GNAP_CHARACTER_H
