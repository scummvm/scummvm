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

#ifndef MADS_DRAGONSPHERE_RSOUND_DRAGONSPHERE_H
#define MADS_DRAGONSPHERE_RSOUND_DRAGONSPHERE_H

#include "mads/dragonsphere/rsound.h"

namespace MADS {
namespace Dragonsphere {

/**
 * RSound1 (rsound.dr1)
 *
 * Dispatch table layout (funcs_10936/1094C/10962/1097C/10992 in the
 * disassembly - same 5-bucket sparse dispatch as the Phantom RSound
 * family, represented here as a single flat array):
 *   commands  0-8   (base class)
 *   commands 16-18  (this class - command16 is a gated deferred music
 *                    loader, command17 an ungated-scheduling immediate
 *                    loader, command18 the re-entrant launcher that
 *                    re-dispatches into bucket 2 or bucket 4 by
 *                    _musicIndex)
 *   commands 24-31  (this class); 29 is confirmed nullsub_2
 *   commands 32-48  (this class - music-piece loaders, mostly the
 *                    "immediate load, or defer until the lower/music
 *                    channels free up" idiom via isMusicChannelsActive()
 *                    + scheduleCallback())
 *   commands 64-101 (this class); 72, 92, 98 confirmed nullsub_2
 *
 * command43/command48 share a tail (a variant byte written into the
 * loaded sound data before playing, matching the sibling ASound1
 * driver's command43/48 idiom) - see command43_48Tail().
 */
class RSound1 : public RSound {
private:
	typedef int (RSound1:: *CommandPtr)();
	static const CommandPtr _commandList[102];

	int command16();
	void loadCommand16();

	int command17();

	int command18();

	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command30();
	int command31();

	int command32();
	void loadCommand32();
	int command33();
	void loadCommand33();
	int command34();
	int command35();
	void loadCommand35();
	int command36();
	void loadCommand36();
	int command37();
	void loadCommand37();
	int command38();
	void loadCommand38();
	int command39();
	void loadCommand39();
	int command40();
	void loadCommand40();
	int command41();
	void loadCommand41();
	int command42();
	void loadCommand42();

	/**
	 * Shared tail of command43()/command48(): writes a variant byte
	 * (0x5D for command43, 0x31 for command48) into the loaded sound
	 * data at offset 0x2539 (one byte into the block that command2's
	 * load targets) before the gated load, matching the sibling ASound1
	 * driver's command43/48 idiom (see CLAUDE.md).
	 */
	void command43_48Tail(byte variant);
	int command43();
	int command48();
	void loadCommand43_48();

	int command44();
	void loadCommand44();
	int command45();
	void loadCommand45();
	int command46();
	void loadCommand46();
	int command47();
	void loadCommand47();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command73();
	int command74();
	int command75();
	int command76();
	int command77();
	int command78();
	int command79();
	int command80();
	int command81();
	int command82();
	int command83();
	int command84();
	int command85();
	int command86();
	int command87();
	int command88();
	int command89();
	int command90();
	int command91();
	int command93();
	int command94();
	int command95();
	int command96();
	int command97();
	int command99();
	int command100();
	int command101();

public:
	RSound1(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound2 (rsound.dr2)
 *
 * command0/reset()/command4/command6/command7/command8 all confirmed to
 * match the shared RSound base exactly (no overrides needed).
 *
 * command3/command5 are a genuinely different channel split from the
 * base class's default (1-5,9 lower / 6,7,8 upper): here command3
 * enables channels 1-6 (six channels, confirmed via symbolic names -
 * channel 9 is untouched), and command5 enables channels 7,8 (confirmed
 * via raw offsets 0x19CA/0x19F2, spaced by the channel stride).
 *
 * command2 calls resetChannels1to6() (matches sub_1079A) + sendGmReset(4)
 * - a genuinely different 6-channel range from the base class's
 * resetChannels1to5(), matching command3's own shifted grouping.
 *
 * command1/2/3 are not virtual in the base class, so this driver's own
 * command1() must be overridden too (calling THIS class's command3()) -
 * otherwise the inherited RSound::command1() would still call the base
 * class's command3() rather than the driver-specific one, same pitfall
 * as Phantom's RSound5.
 */
class RSound2 : public RSound {
private:
	typedef int (RSound2:: *CommandPtr)();
	static const CommandPtr _commandList[73];

	int command1();
	int command2();
	int command3();
	int command5();

	/** Matches resetChannels1to6: zeroes channels 1-6 (0-based 0-5), plus the held-notes table. */
	void resetChannels1to6();

	int command16();
	void loadCommand16();
	int command17();
	int command18();

	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command30();
	int command31();

	/**
	 * Plays the specified sound, using any free channel from 7 to 8.
	 * Matches playSoundChannels7to8: symmetric free/fallback scan (free
	 * scan ch7 then ch8; fallback scan ch8 then ch7).
	 */
	Channel *playSoundChannels7to8(int offset);

	int command32();
	void loadCommand32();
	int command33();
	void loadCommand33();
	int command34();
	void loadCommand34();

	int command35();
	void loadCommand35();

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
 * RSound3 (rsound.dr3)
 *
 * command0/command2/command4/command6/command7/command8 all confirmed to
 * match the shared RSound base exactly (no overrides needed) -
 * including command4 (whose reset helper sub_107E7 matches the base
 * class's own resetChannels6to8() exactly) and command2 (whose
 * resetChannels1to5 matches the base class's own method exactly,
 * including the channel-9 fix).
 *
 * command3/command5 are a genuinely new, wider split with actual
 * overlap: command3 enables channels 1-6 AND 9 (seven channels, via
 * explicit calls through channel 6, falling through to channel 9), and
 * command5 enables channels 5,6,7,8 (four channels) - so channels 5 and
 * 6 are enabled by BOTH commands, confirmed directly from the
 * disassembly.
 *
 * command1/2/3 are not virtual in the base class, so this driver's own
 * command1() must be overridden too (calling THIS class's command3()) -
 * same pitfall as Phantom's RSound5 and this game's RSound2.
 *
 * Dispatch table layout (funcs_109BB/109D1/109E7/10A01/10A17):
 *   commands  0-8   (base class, except command1/3/5 - see above)
 *   commands 16-18  (this class - same isSoundActive+isMusicChannelsActive
 *                    deferred-load idiom as RSound1/2; the disassembly's
 *                    gate uses the name "isSoundPlaying", confirmed
 *                    equivalent to isSoundActive())
 *   commands 24-31  (this class); 29 confirmed nullsub_1
 *   commands 32-34  (this class); 34 confirmed nullsub_1
 *   commands 64-73  (this class); 66 confirmed nullsub_1. command68 was
 *                    initially suspected to be unreferenced dead code
 *                    (an earlier dispatch table read appeared to skip
 *                    straight from 67 to 69) - confirmed by the user to
 *                    be a real, live dispatch entry after correcting the
 *                    function names in the IDB; no index shift needed.
 *                    There is no command74 - 73 is the last real entry.
 */
class RSound3 : public RSound {
private:
	typedef int (RSound3:: *CommandPtr)();
	static const CommandPtr _commandList[74];

	int command1();
	int command3();
	int command5();

	int command16();
	void loadCommand16();
	int command17();
	int command18();

	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command30();
	int command31();

	/**
	 * Shared tail of command31()/command64() (matches loc_124D8): writes
	 * a variant byte (0x5A for command31, 0x78 for command64) into the
	 * sound data at offset 0x1402 before playing 0x13FF.
	 */
	void command31_64Tail(byte variant);
	int command64();

	int command32();
	void loadCommand32();
	int command33();
	void loadCommand33();

	int command65();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();
	int command73();

public:
	RSound3(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound4 (rsound.dr4)
 *
 * command0/command6/command7/command8 all confirmed to match the shared
 * RSound base exactly (no overrides needed).
 *
 * There is no separate command5 here - that dispatch slot is nullsub_1;
 * its role is folded directly into command1().
 *
 * command1/command2/command3/command4 are all genuinely different from
 * the base class, in two ways:
 *   - They use a NEW driver-specific channel-disable mechanic
 *     (disableChannel(), matching sub_1092A) instead of Channel::enable() -
 *     it redirects _soundData (and, if the channel is about to expire
 *     this tick, _pSrc too) to loadData(0x1F8B) instead of nullptr.
 *   - Channel 6 (the boundary between the "lower" 1-5,9 group and
 *     "upper" 6,7,8 group) is conditionally included/excluded based on
 *     isSoundActive(loadData(0x1F4F)) in command1/command3, and based on
 *     a direct _activeCount/_soundData check against the same offset in
 *     resetChannels6to8(). resetChannels1to5() here also always resets
 *     channel 6 in addition to 1-5,9 (seven channels total) - one more
 *     than the base class's six.
 *
 * command1/2/3 are not virtual in the base class, so all four overrides
 * are needed to ensure they call each other correctly (same pitfall as
 * every other Dragonsphere RSound driver so far) - resetChannels1to5()
 * and resetChannels6to8() are likewise redeclared here (matching the
 * disassembly's own names) to hide the base class's versions for calls
 * made from this class's own command2()/command4().
 */
class RSound4 : public RSound {
private:
	/**
	 * Matches sub_1092A: a driver-specific variant of Channel::enable()
	 * that redirects _soundData (and _pSrc, if the channel is about to
	 * expire this tick) to loadData(0x1F8B) instead of nullptr.
	 */
	void disableChannel(int channelIndex, byte flag);

	int command1();
	int command2();
	int command3();
	int command4();

	/** Hides the base class's method - see class comment. Resets channels 1-6 AND 9 (seven channels). */
	void resetChannels1to5();

	/** Hides the base class's method - see class comment. Channels 7,8 unconditionally; channel 6 only if it's playing loadData(0x1F4F). */
	void resetChannels6to8();

	typedef int (RSound4:: *CommandPtr)();
	static const CommandPtr _commandList[79];

	/**
	 * Genuinely different gate order from every other bucket-4/16-18
	 * command in this driver: checks isMusicChannelsActive() FIRST
	 * (unconditionally, no isSoundActive() gate at all on the immediate
	 * path); only if music channels ARE active does it then check
	 * isSoundActive(0x7C4) as a secondary gate before scheduling the
	 * deferred callback.
	 */
	int command16();
	void loadCommand16();

	int command17();
	int command18();

	int command24();
	int command25();
	int command26();
	int command27();
	int command28();

	/** Matches rsound_command29 - reuses offset 0x1F4F both as the isSoundActive gate AND the play target. */
	int command29();
	int command30();
	int command31();

	int command32();
	void loadCommand32();
	int command33();
	void loadCommand33();

	/** Calls command1() (not command3(), unlike every other bucket-4 command here) before loading a single channel. */
	int command35();
	void loadCommand35();

	int command36();
	void loadCommand36();
	int command37();
	void loadCommand37();
	int command38();
	void loadCommand38();
	int command39();
	void loadCommand39();
	int command40();
	void loadCommand40();

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
	RSound4(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

} // namespace Dragonsphere
} // namespace MADS

#endif
