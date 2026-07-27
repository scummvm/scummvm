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

#ifndef MADS_PHANTOM_RSOUND_PHANTOM_H
#define MADS_PHANTOM_RSOUND_PHANTOM_H

#include "mads/phantom/rsound.h"

namespace MADS {
namespace Phantom {

/**
 * RSound1 (rsound.ph1)
 *
 * Dispatch table layout (funcs_10960/10976/1098C/109A2/109B8 in the
 * disassembly - a 5-bucket sparse dispatch, mirroring the sibling ASound1's
 * identical structure). There is no command 17 - bucket 2's dispatch
 * bounds check only ever admits index 16 (confirmed: word_13047 = 16, an
 * inclusive upper bound equal to the bucket's own lower bound).
 *   commands  0-8   (base class, except command4/5 - see RSound::command4/5)
 *   command   16    (random-ambiance picker, this class)
 *   commands 24-27  (this class)
 *   commands 32-39  (this class)
 *   commands 64-76  (this class - unlike ASound1's equivalent range, this
 *                    one WAS reconstructable from the disassembly)
 *
 * command16 picks one of five alternatives at random (avoiding immediate
 * repeats via _lastRandomAmbianceIndex), after first checking whether one
 * of five known pieces is already playing via isSoundActive() gates.
 * sound1() doesn't exist as a separate function in the disassembly -
 * index 0 of funcs_122AD dispatches directly to command38 (also
 * independently reachable as a direct command) - but is declared here as
 * a thin wrapper for consistency with sound2()-sound5().
 */
class RSound1 : public RSound {
private:
	typedef int (RSound1:: *CommandPtr)();

	// Mirrors word_1225D: avoids picking the same alternative twice in a row.
	int _lastRandomAmbianceIndex = -1;

	void checkRandomAmbianceTrigger() override;

	int command16();

	// Overrides confirming the isSoundActive(0x3D98) gate the base
	// class's ungated command4()/command5() lack - see RSound::command4/5.
	int command4() override;
	int command5() override;

	int command24();
	int command25();
	int command26();
	int command27();

	int command32();
	// Targets of command16's random picker (funcs_122AD in the
	// disassembly). sound1() is not a real disassembly function - see
	// the class comment.
	int command38();
	int sound1();
	int sound2();
	int sound3();
	int sound4();
	int sound5();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command39();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();
	int command73();
	int command74();
	int command75();
	int command76();

public:
	RSound1(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

} // namespace Phantom
} // namespace MADS

#endif
