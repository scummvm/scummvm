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

#ifndef WORLD_ACTORS_ATTACKPROCESS_H
#define WORLD_ACTORS_ATTACKPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/direction.h"

#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

class Actor;
class CombatDat;

/**
 * The NPC attack process used in Crusader games.  This is more advanced than the Ultima
 * CombatProcess, and contains a small language to implement the AI, which is specified in
 * the combat.dat file (see CombatDat class)
 */
class AttackProcess : public Process {
public:
	AttackProcess();
	AttackProcess(Actor *actor);

	virtual ~AttackProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	void terminate() override;

	void dumpInfo() const override;

	void setIsActivityAOrB() {
		_isActivityAorB = true;
	}
	void setIsActivity9OrB() {
		_isActivity9orB = true;
	}
	void setField97() {
		_field97 = true;
	}

	void setTimer3();

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

private:
	/** Set the current tactic in use from the combat.dat file.  If 0,
	 * will use the genericAttack function. */
	void setTacticNo(int block);
	/** Set the sub-tactic block - should be 0 or 1 (although 0-3 are
	 * supported in the dat file format, only 0/1 are ever used) */
	void setBlockNo(int block);

	/// Read the next word and return the value without using array
	uint16 readNextWordRaw();
	/** Read the next word and pull from the data array if its value
	 * is over the magic number*/
	uint16 readNextWordWithData();

	/// set data in the array - offset includes the magic number
	void setAttackData(uint16 offset, uint16 val);
	/// get data from the array - offset includes the magic number
	uint16 getAttackData(uint16 offset) const;

	/// This is the equivalent of run() when a tactic hasn't been selected yet.
	void genericAttack();

	/// Sleep the process for the given number of ticks
	void sleep(int ticks);

	/// Check the sound timer and return if we are ready for a new sound
	bool readyForNextSound(int now);

	bool checkTimer2PlusDelayElapsed(int now);
	void pathfindToItemInNPCData();
	bool timer4and5Update(int now);
	void timeNowToTimerVal2(int now);
	bool checkReady(int now, Direction targetdir);

	/** Check if it's time to make a sound and if so start one - for most NPCs
	 * that's on startup, but some make regular sounds (see readyForNextSound) */
	void checkRandomAttackSound(int now, uint32 shapeno);

	uint16 _target; // TODO: this is stored in NPC in game, does it matter?
	uint16 _tactic;
	uint16 _block;
	uint16 _tacticDatStartOffset;
	const CombatDat *_tacticDat;
	Common::MemoryReadStream *_tacticDatReadStream;

	int16 _soundNo;
	bool _playedStartSound;

	Direction _npcInitialDir;

	// Unknown fields..
	int16 _field57;
	uint16 _field59;
	//uint16 _field53; // Never really used?
	bool _field7f;
	bool _field96;
	bool _field97;

	bool _isActivity9orB;
	bool _isActivityAorB;
	bool _timer2set;
	bool _timer3set;
	bool _doubleDelay;

	uint16 _wpnField8;

	/// an array used to hold data for the combat lang
	uint16 _dataArray[10];

	int32 _wpnBasedTimeout;
	int32 _difficultyBasedTimeout;

	int32 _timer2; // 0x73/0x75 in orig
	int32 _timer3; // 0x77/0x79 in orig
	int32 _timer4; // 0x6f/0x71 in orig
	int32 _timer5; // 0x8a/0x8c in orig

	int32 _soundTimestamp; /// 0x84/0x86 in orig - time a sound was last played
	int32 _fireTimestamp; /// 0x90/0x92 in orig - time NPC last fired

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
