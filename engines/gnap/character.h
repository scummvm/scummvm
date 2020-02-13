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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
	kDirIdleLeft = 0,
	kDirBottomRight = 1,
	kDirBottomLeft = 3,
	kDirIdleRight = 4,
	kDirUpLeft = 5,
	kDirUpRight = 7
};

struct GridStruct {
	int _deltaX, _deltaY;
	int _gridX1, _gridY1;
	int _sequenceId;
	int _id;
};

const int kMaxGridStructs = 30;

class Character {
public:
	Character(GnapEngine *vm);
	virtual ~Character();

	void walkStep();

	virtual int getSequenceId(int kind, Common::Point gridPos) = 0;
	virtual void playSequence(int sequenceId) = 0;
	virtual void updateIdleSequence() = 0;
	virtual void updateIdleSequence2() = 0;
	virtual void initPos(int gridX, int gridY, Facing facing) = 0;
	virtual int getWalkSequenceId(int deltaX, int deltaY) = 0;
	virtual bool walkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) = 0;

	Common::Point _pos;
	Facing _idleFacing;
	int _actionStatus;
	int _sequenceId;
	int _sequenceDatNum;
	int _id;
	int _gridX;
	int _gridY;
	int _walkNodesCount;
	GridStruct _walkNodes[kMaxGridStructs];
	int _walkDestX, _walkDestY;
	int _walkDeltaX, _walkDeltaY, _walkDirX, _walkDirY, _walkDirXIncr, _walkDirYIncr;

protected:
	GnapEngine *_vm;
};

class PlayerGnap : public Character {
public:
	PlayerGnap(GnapEngine *vm);
	int getSequenceId(int kind, Common::Point gridPos) override;
	void initPos(int gridX, int gridY, Facing facing) override;
	void playSequence(int sequenceId) override;
	void updateIdleSequence() override;
	void updateIdleSequence2() override;
	int getWalkSequenceId(int deltaX, int deltaY) override;
	bool walkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) override;

	void actionIdle(int sequenceId);
	bool doPlatypusAction(int gridX, int gridY, int platSequenceId, int callback);
	int getShowSequenceId(int index, int gridX, int gridY);
	Facing getWalkFacing(int deltaX, int deltaY);
	int getWalkStopSequenceId(int deltaX, int deltaY);
	void idle();
	void initBrainPulseRndValue();
	void kissPlatypus(int callback);
	void playBrainPulsating(Common::Point gridPos = Common::Point(0, 0));
	void playIdle(Common::Point gridPos = Common::Point(0, 0));
	void playImpossible(Common::Point gridPos = Common::Point(0, 0));
	void playMoan1(Common::Point gridPos = Common::Point(0, 0));
	void playMoan2(Common::Point gridPos = Common::Point(0, 0));
	void playPullOutDevice(Common::Point gridPos = Common::Point(0, 0));
	void playPullOutDeviceNonWorking(Common::Point gridPos = Common::Point(0, 0));
	void playScratchingHead(Common::Point gridPos = Common::Point(0, 0));
	void playShowCurrItem(Common::Point destPos, int gridLookX, int gridLookY);
	void playShowItem(int itemIndex, int gridLookX, int gridLookY);
	void playUseDevice(Common::Point gridPos = Common::Point(0, 0));
	void useDeviceOnPlatypus();
	void useDisguiseOnPlatypus();
	void useJointOnPlatypus();

	int _brainPulseNum;
	int _brainPulseRndValue;

private:
	bool findPath1(int gridX, int gridY, int index);
	bool findPath2(int gridX, int gridY, int index);
	bool findPath3(int gridX, int gridY);
	bool findPath4(int gridX, int gridY);
};

class PlayerPlat : public Character {
public:
	PlayerPlat(GnapEngine *vm);
	~PlayerPlat() override {}
	int getSequenceId(int kind = 0, Common::Point gridPos = Common::Point(0, 0)) override;
	void initPos(int gridX, int gridY, Facing facing) override;
	void playSequence(int sequenceId) override;
	void updateIdleSequence() override;
	void updateIdleSequence2() override;
	int getWalkSequenceId(int deltaX, int deltaY) override;
	bool walkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) override;

	void makeRoom();

private:
	bool findPath1(int gridX, int gridY, int index);
	bool findPath2(int gridX, int gridY, int index);
	bool findPath3(int gridX, int gridY);
	bool findPath4(int gridX, int gridY);
};
} // End of namespace Gnap

#endif // GNAP_CHARACTER_H
