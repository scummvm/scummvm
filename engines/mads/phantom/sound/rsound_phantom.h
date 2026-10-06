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

#ifndef MADS_PHANTOM_SOUND_RSOUND_PHANTOM_H
#define MADS_PHANTOM_SOUND_RSOUND_PHANTOM_H

#include "mads/phantom/sound/rsound.h"

namespace MADS {
namespace Phantom {
namespace Sound {

/**
 * RSound1 (rsound.ph1)
 *
 * Dispatch table layout: a 5-bucket sparse dispatch, mirroring the
 * sibling ASound1's identical structure. There is no command 17 -
 * bucket 2's dispatch bounds check only ever admits index 16
 * (confirmed: the upper bound is 16, equal to the bucket's own lower
 * bound).
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

	// Avoids picking the same alternative twice in a row.
	int _lastRandomAmbianceIndex = -1;

	void checkRandomAmbianceTrigger() override;

	int command16();

	// Overrides confirming the isSoundActive(0x3D98) gate the base
	// class's pure-virtual command4()/command5() require - see
	// RSound::command4/5.
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

/**
 * RSound2 (rsound.ph2)
 *
 * Dispatch table layout (funcs_10964/1097A/10990/109A6/109BC in the
 * disassembly - same 5-bucket sparse-dispatch shape as RSound1, but here
 * represented as a single flat array, matching the Rex Nebular RSound
 * convention, since the bucket boundaries don't otherwise affect behavior):
 *   commands  0-8   (base class, except command4/5 - see RSound::command4/5)
 *   command   16    (this class - a plain gated 3-channel load, NOT a
 *                    random picker like RSound1's command16)
 *   commands 24-27  (this class)
 *   commands 32-35  (this class)
 *   commands 64-72  (this class)
 * The native bucket limits are 8, 16, 27, 35, and 72. A trailing table
 * word points to nullsub_1, but command 73 is above the native limit and
 * is rejected before dispatch. Its C++ slot remains an equivalent no-op.
 */
class RSound2 : public RSound {
private:
	typedef int (RSound2:: *CommandPtr)();
	static const CommandPtr _commandList[74];

	// Overrides confirming the isSoundActive(0x3D98) gate the base
	// class's pure-virtual command4()/command5() require - see
	// RSound::command4/5. Same offset NUMBER as RSound1's, but not
	// assumed to be the same underlying data.
	int command4() override;
	int command5() override;

	int command16();
	int command24();
	int command25();
	int command26();
	int command27();
	int command32();
	int command33();
	int command34();
	int command35();
	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();

public:
	RSound2(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound3 (rsound.ph3)
 *
 * Dispatch table layout (funcs_10A0C/10A22/10A38/10A4E/10A64 in the
 * disassembly - same 5-bucket sparse-dispatch shape as RSound1/RSound2,
 * represented here as a single flat array):
 *   commands  0-8   (base class, except command4/5 - see RSound::command4/5;
 *                    index 3 aliases directly to command1's full handler -
 *                    see below)
 *   command   16    (this class - a plain gated 4-channel load)
 *   commands 24-27  (this class)
 *   commands 32-36  (this class); 37 is confirmed nullsub_1
 *   commands 64-75  (this class); 76 is confirmed nullsub_1
 *
 * command0/1/2/3/6/7/8 all confirmed to match the shared RSound base
 * exactly (no overrides needed) - including a notable dispatch-table
 * detail: command index 3 in this driver's dispatch table is a plain
 * jmp straight into command1's full body (lower channels 1-4,9 AND
 * upper channels 5-8), NOT the "inner" lower-only logic that the base
 * class's own command3() implements - so the command list must map
 * index 3 to &RSound::command1, not &RSound::command3. command0's
 * disassembly is confirmed to genuinely omit the _isDisabled
 * save/restore wrap that RSound::command0() has, but with no
 * observable behavioral difference, so no override is needed there.
 *
 * command4()/command5() are also a genuinely different shape from
 * RSound1/RSound2: both gate on TWO chained isSoundActive() checks
 * (0x2AA6 then 0x1E30, each an independent early-out per the
 * pop-return-address mechanic) rather than one, and command4()'s tail
 * resets only channels 5-9 (indices 4-8) - NOT channels
 * 4-9 (indices 3-8) like the shared resetAndGmResetUpperChannels() -
 * so it can't reuse that helper and calls resetChannelRange()
 * directly instead.
 */
class RSound3 : public RSound {
private:
	typedef int (RSound3:: *CommandPtr)();
	static const CommandPtr _commandList[77];

	int command4() override;
	int command5() override;

	int command16();

	int command24();
	int command25();
	int command26();
	int command27();

	int command32();
	int command33();
	int command34();
	int command35();
	int command36();

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

public:
	RSound3(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound4 (rsound.ph4)
 *
 * Dispatch table layout (funcs_109CE/109E4/109FA/10A10/10A26 in the
 * disassembly - same 5-bucket sparse-dispatch shape as RSound1-3,
 * represented here as a single flat array):
 *   commands  0-8   (base class, except command4/5 - see RSound::command4/5)
 *   command   16    (this class)
 *   commands 24-27  (this class)
 *   command   32    (confirmed nullsub_1 - this driver has no command32-39
 *                    at all, unlike RSound1-3, which all had a populated
 *                    32+ bucket)
 *   commands 64-70  (this class); 71 is confirmed nullsub_1
 *
 * command0/1/2/3/6/7/8 confirmed to match the shared RSound base exactly
 * (no overrides needed) - including command3, which (unlike RSound3) is
 * NOT aliased to command1's dispatch slot; this driver's dispatch table
 * has a genuine, separate rsound_command3 entry matching the base
 * class's command3() exactly.
 *
 * command4() is a genuinely new shape: unlike every driver confirmed so
 * far, it has NO isSoundActive() gate at all - it unconditionally resets
 * channels 6-9 (0-based indices 5-8, confirmed via explicit
 * Channel._activeCount/_volumeFadeStep-labeled writes) via
 * resetChannelRange(), then sends a full sendGmReset(9). A third distinct
 * channel range for this reset (RSound1/RSound2 used 4-9, RSound3 used
 * 5-9), reinforcing that this range is always driver-specific.
 *
 * command1/command3 confirmed to match the shared RSound base exactly
 * (no overrides needed) - the helper called at the top of both is
 * confirmed to be exactly "_fadeCheckPeriod = 1", and command5's four
 * channel-enable targets are confirmed to be channels 5-8, matching
 * enableUpperChannels() exactly.
 */
class RSound4 : public RSound {
private:
	typedef int (RSound4:: *CommandPtr)();
	static const CommandPtr _commandList[72];
	bool callFunction(uint16 targetOffset) override;

	int command4() override;
	int command5() override;

	int command16();

	int command24();
	int command25();
	int command26();
	int command27();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();

public:
	RSound4(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound5 (rsound.ph5)
 *
 * Dispatch table layout (funcs_109F0/10A06/10A1C/10A32/10A48 in the
 * disassembly - same 5-bucket sparse-dispatch shape as RSound1-4):
 *   commands  0-8   (base class, except command1/3/4/5 - see below)
 *   command   16    (this class)
 *   commands 24-27  (this class)
 *   commands 32-39  (this class); 32 is confirmed nullsub_2
 *   commands 64-78  (this class); 79 is confirmed nullsub_1
 *
 * command0/2/6/7/8 confirmed to match the shared RSound base exactly (no
 * overrides needed).
 *
 * command1/command3 are a genuinely different shape from every prior
 * driver: command3's "lower" enable group is channels 1-5 AND 9 (SIX
 * channels), not the base class's 1-4,9 (five channels) - channel 5 has
 * moved from the "upper" group into the "lower" one. Since command1/2/3
 * are not virtual in the base class, RSound::command1() would still call
 * the base's own command3() even with a derived override present, so
 * command1() needs its own override here too (calling this driver's own
 * command3()) rather than relying on the inherited one. command4/5's
 * upper group is correspondingly narrowed to 3 channels (6,7,8) instead
 * of the usual 4 (5,6,7,8) - both confirmed directly from disassembly.
 *
 * command39 is notable: its gate leads to this driver's own command3()
 * (the 6-channel lower-group enable), NOT command1() like every other
 * gated multi-load/multi-play command in this driver - confirmed
 * directly from the disassembly ("call rsound_command3"), not a typo.
 */
class RSound5 : public RSound {
private:
	typedef int (RSound5:: *CommandPtr)();
	static const CommandPtr _commandList[80];

	int command1();
	int command3();
	int command4() override;
	int command5() override;

	/**
	 * Shared tail of command1() and command5() in this driver: enables
	 * channels 6,7,8 (three channels - this driver's narrowed "upper"
	 * group, see class comment). Matches rsound_command5's body.
	 */
	void enableChannels678();

	int command16();

	int command24();
	int command25();
	int command26();
	int command27();

	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
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
	int command77();
	int command78();

public:
	RSound5(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound9 (rsound.ph9)
 *
 * Dispatch table layout (funcs_109F5/10A0B/10A37/10A4D in the
 * disassembly - same sparse-dispatch shape as RSound1-5, represented
 * here as a single flat array):
 *   commands  0-8   (base class, except command4/5 - see RSound::command4/5)
 *   commands 16,24  (SHARED handler - both indices point to the exact
 *                    same function in the dispatch table)
 *   commands 25-27  (this class)
 *   commands 32-39  (this class); 33 is confirmed nullsub_1. IMPORTANT:
 *                    the disassembly's own symbol names for 34-39 are
 *                    each off by one from their true dispatch index
 *                    (e.g. the function IDA calls "rsound_command33" is
 *                    actually invoked at index 34, "rsound_command34" at
 *                    35, etc., through "rsound_command38" at 39) -
 *                    confirmed directly from the funcs_10A37 table
 *                    layout, not a transcription error. The C++ method
 *                    names below use the TRUE (table-position) command
 *                    ID, not the stale IDA symbol suffix.
 *   commands 64-71  (this class); 65 and 66 SHARE a handler (both point
 *                    to the same function); 72 is confirmed nullsub_1
 *
 * command0/1/2/3/6/7/8 confirmed to match the shared RSound base exactly
 * (no overrides needed, including command1/command3, unlike RSound5).
 * command5 also matches the base's enableUpperChannels() exactly
 * (channels 5-8), so its override just delegates to that helper.
 */
class RSound9 : public RSound {
private:
	typedef int (RSound9:: *CommandPtr)();
	static const CommandPtr _commandList[73];

	int command4() override;
	int command5() override;

	// Shared by dispatch indices 16 AND 24 (identical handler).
	int command16Or24();

	int command25();
	int command26();
	int command27();

	int command32();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();

	int command64();
	// Shared by dispatch indices 65 AND 66 (identical handler).
	int command65Or66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();

public:
	RSound9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/** Shared control-command behavior of the Phantom demo RSOUND overlay. */
class RSoundDemo : public RSound {
protected:
	RSoundDemo(Audio::Mixer *mixer, const Common::Path &filename,
			   int dataOffset, int dataSize, int sysExOffset);

	int dispatchCommonCommand(int commandId);
	int command4() override;
	int command5() override;
};

/** Controller for the exact RSOUND.PHA overlay shipped with the demo. */
class RSoundDemoPHA final : public RSoundDemo {
private:
	bool callFunction(uint16 targetOffset) override;
	void writeRandomizedPair(uint16 firstLowOffset, uint16 secondLowOffset,
							 uint16 firstHighOffset, uint16 secondHighOffset,
							 byte firstLow, byte secondLow, byte firstHigh, byte secondHigh);

public:
	explicit RSoundDemoPHA(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
	static bool validate(Common::String *reason = nullptr);
};

} // namespace Sound
} // namespace Phantom
} // namespace MADS

#endif
