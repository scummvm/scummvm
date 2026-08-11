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

#ifndef MADS_DRAGONSPHERE_SOUND_RSOUND_DRAGONSPHERE_H
#define MADS_DRAGONSPHERE_SOUND_RSOUND_DRAGONSPHERE_H

#include "mads/dragonsphere/sound/rsound.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

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
	int loadCommand16();

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
	int loadCommand32();
	int command33();
	int loadCommand33();
	int command34();
	int command35();
	int loadCommand35();
	int command36();
	int loadCommand36();
	int command37();
	int loadCommand37();
	int command38();
	int loadCommand38();
	int command39();
	int loadCommand39();
	int command40();
	int loadCommand40();
	int command41();
	int loadCommand41();
	int command42();
	int loadCommand42();

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
	int loadCommand43_48();

	int command44();
	int loadCommand44();
	int command45();
	int loadCommand45();
	int command46();
	int loadCommand46();
	int command47();
	int loadCommand47();

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
 * command2 calls resetChannels1to6() + sendGmReset(4)
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
	int loadCommand16();
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
	int loadCommand32();
	int command33();
	int loadCommand33();
	int command34();
	int loadCommand34();

	int command35();
	int loadCommand35();

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
 * including command4 (whose reset helper matches the base
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
	int loadCommand16();
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
	 * Shared tail of command31()/command64(): writes
	 * a variant byte (0x5A for command31, 0x78 for command64) into the
	 * sound data at offset 0x1402 before playing 0x13FF.
	 */
	void command31_64Tail(byte variant);
	int command64();

	int command32();
	int loadCommand32();
	int command33();
	int loadCommand33();

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
 *   - They use the base class's disableChannelTo()
 *     instead of Channel::enable().
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
	int loadCommand16();

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
	int loadCommand32();
	int command33();
	int loadCommand33();

	/** Calls command1() (not command3(), unlike every other bucket-4 command here) before loading a single channel. */
	int command35();
	int loadCommand35();

	int command36();
	int loadCommand36();
	int command37();
	int loadCommand37();
	int command38();
	int loadCommand38();
	int command39();
	int loadCommand39();
	int command40();
	int loadCommand40();

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

/**
 * RSound5 (rsound.dr5)
 *
 * command0/command3/command4/command6/command7/command8 all confirmed
 * to match the shared RSound base exactly (no overrides needed),
 * including command4 (whose resetChannels6to8 matches the base class's
 * unconditional 3-channel version exactly, unlike RSound4's conditional
 * one) and command3 (matches the base's default 1-5,9 lower-group
 * enable exactly).
 *
 * command5 uses the base class's disableChannelTo()
 * instead of Channel::enable(), for channels 6,7,8 (three
 * channels, matching the base's default upper group range, just via a
 * different mechanic).
 *
 * command1/2/3 are not virtual in the base class, so this driver's own
 * command1() must be overridden too (calling THIS class's command5()) -
 * same pitfall as every other Dragonsphere RSound driver so far.
 */
class RSound5 : public RSound {
private:
	typedef int (RSound5:: *CommandPtr)();
	static const CommandPtr _commandList[79];

	int command1();
	int command5();

	/**
	 * False the first time command16 is ever called,
	 * true forever after (there is no code path that resets it back to
	 * false) - selects between two near-identical variants (A the first
	 * time, B every time after).
	 */
	bool _command16Played = false;

	int command16();
	int loadCommand16A();
	int loadCommand16B();

	int command17();
	int command18();

	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();

	/**
	 * Shared tail of command31()/command78(): writes
	 * a variant byte (0x5A for command31, 0x78 for command78) into the
	 * sound data at offset 0x2301 (3 bytes into the block about to be
	 * played) before playing 0x22FE.
	 */
	void command31_78Tail(byte variant);
	int command31();

	int command32();
	int loadCommand32();
	int command33();
	int loadCommand33();
	int command34();
	int loadCommand34();
	int command35();
	int loadCommand35();
	int command36();

	/** No gate at all (unlike every other bucket-4 command here): just clears _callbackFnPtr, calls command1(), then 4x playSoundChannels1To5(). */
	int command37();

	int command38();
	int loadCommand38();

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

	/** Uses _commandParam: if 0, conditionally redirects channel 8's inner loop pointer; otherwise writes a clamped 7-bit value into the sound data at offset 0x20D6 (11 bytes into the block about to be played) and gate-loads channel 8. */
	int command77();
	int command78();

public:
	RSound5(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound6 (rsound.dr6)
 *
 * command0/command6/command7/command8 all confirmed to match the shared
 * RSound base exactly (no overrides needed).
 *
 * command2/command3 use a genuinely wider group than the base class's
 * default: channels 1-6 AND 9 (seven channels, confirmed via symbolic
 * names) rather than 1-5,9 (six). command5 enables only channels 7,8
 * (two channels, confirmed via address arithmetic against command3's
 * range - channel spacing and the absence of any other channel
 * unaccounted for).
 *
 * command4 uses a custom inline GM-reset loop bounded 8 down to 6
 * (matches the new sendGmResetRange() helper) rather than the shared
 * sendGmReset(count) (which always counts from count down to 1) -
 * confirmed via resetChannels6to8(), which matches the base class
 * exactly (channels 6,7,8).
 *
 * command1/2/3 are not virtual in the base class, so command1() must be
 * overridden too (calling THIS class's command3()) - same pitfall as
 * every other Dragonsphere RSound driver so far. isMusicChannelsActive()
 * is likewise overridden (hiding the base's private method) to match
 * command3()'s wider 7-channel group.
 *
 * Dispatch table layout (funcs_10A43/10A59/10A6F/10A89/10A9F):
 *   commands  0-8   (base class, except command1-5 - see above)
 *   commands 16-18  (this class)
 *   commands 24-31  (this class); 29 confirmed nullsub_1
 *   commands 32-46  (this class); 41,42,43,46 confirmed nullsub_1
 *   commands 64-98  (this class, all populated)
 */
class RSound6 : public RSound {
private:
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();

	/**
	 * Hides the base class's private method (inaccessible from here
	 * otherwise). Confirmed to match the base class's exact shape
	 * (channels 6,7,8 unconditionally).
	 */
	void resetChannels6to8();

	/**
	 * Hides the base class's method: channels 1-6
	 * AND 9 (seven channels), matching command3()'s own wider group,
	 * not the base class's default (1-5,9).
	 */
	int isMusicChannelsActive();

	/**
	 * Plays the specified sound, using any free channel from 1 to 6.
	 * Matches the disassembly's own playSoundChannesl1to6 (sic - a typo
	 * in the disassembly's own symbol name, fixed here): symmetric
	 * free/fallback scan.
	 */
	Channel *playSoundChannels1To6(int offset);

	/**
	 * Plays the specified sound, using any free channel from 1 to 8
	 * (everything except channel 9). Matches this driver's own
	 * playSoundAny exactly - a genuinely different (fuller) range than
	 * RSound1's identically-named playSoundAny (renamed there to
	 * playSoundChannels1To5 since it only reached channels 1-5).
	 */
	Channel *playSoundAny(int offset);

	typedef int (RSound6:: *CommandPtr)();
	static const CommandPtr _commandList[99];

	int command16();
	int loadCommand16();
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
	 * command32/command33 share an intricate tail: command33's entry
	 * gate falls through into command32's own shared-load logic (and,
	 * in one branch, directly into command32's "load channel 4" tail,
	 * skipping the main load) - see the .cpp for the full breakdown.
	 */
	int command32();
	int command33();
	int command32_33Load();
	void command32_33LoadCh4();
	byte _command33Flag = 0;

	int command34();
	int command34LoadCh1AndRest();
	void command34LoadRestOnly();

	int command35();

	int command36();
	int command37();
	int command38();
	int command39();
	int command40();

	int command44();
	int loadCommand44();
	int command45();

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
	int command92();
	int command93();
	int command94();
	int command95();
	int command96();
	int command97();
	int command98();

public:
	RSound6(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

/**
 * RSound9 (rsound.dr9)
 *
 * command0/command2/command4/command6/command7/command8 all confirmed to
 * match the shared RSound base exactly (no overrides needed) - including
 * command2's resetChannels1to5 (channels 1-5 AND 9, six channels) and
 * command4's resetChannels6to8 (channels 6,7,8), both confirmed directly
 * from the disassembly.
 *
 * command3/command5 are a genuinely different split from the base
 * class's default (1-5,9 lower / 6,7,8 upper): here command3 enables
 * only channels 1,2,3,4,9 (five channels - channel 5 is NOT included),
 * and command5 enables channels 5,6,7,8 (four channels) - channel 5 has
 * moved from the lower group into the upper group compared to the base.
 *
 * command1/2/3 are not virtual in the base class, so this driver's own
 * command1() must be overridden too (calling THIS class's command3()/
 * command5()) - same pitfall as every other Dragonsphere RSound driver
 * so far.
 *
 * Dispatch table layout (funcs_10936/1094C/10962/10978):
 *   commands  0-8   (base class, except command1/3/5 - see above)
 *   commands 16-31  entirely unreachable (both bucket tables are 100%
 *                    nullsub_2, regardless of exactly where the
 *                    internal split between them falls)
 *   commands 32-63  (this class); 44,56 confirmed nullsub_2. Two shared
 *                    handlers: commands 33 and 47 point to the exact
 *                    same function (command33Or47()); commands 34 and
 *                    54 likewise (command34Or54()). command53's body is
 *                    an unlabeled function in the disassembly (no proc
 *                    name/index shown) - assigned to index 53 by
 *                    elimination (the only index in this range with no
 *                    other confirmed body), not from an explicit label;
 *                    flag if that's wrong.
 *   commands 64-95  CONFIRMED by the user to be an exact duplicate of
 *                    the 32-63 table (index 64+N behaves identically to
 *                    index 32+N) - the flat _commandList[] below just
 *                    reuses the same function pointers for that range,
 *                    no separate implementation needed.
 */
class RSound9 : public RSound {
private:
	int command1();
	int command3();
	int command5();

	int command32();

	/** Writes a "variant A" set of bytes into the shared command33Or47()/command60() sound block, at the same 8 offsets touched by variantSetupB(). Called only by command60(). */
	void variantSetupA();

	/** Same 8 offsets as variantSetupA(), with different "variant B" byte values. Called only by command33Or47(). */
	void variantSetupB();

	/**
	 * Shared tail of command60()/command33Or47() -
	 * the two commands are otherwise identical (same gate, same
	 * counter=98/period=84 timer) and differ only in which variant setup
	 * helper runs first.
	 */
	int command33_47_60Load();
	int command60();

	/** Dispatch table entries 33 AND 47 both point to this single function (confirmed identical symbol at both table slots). */
	int command33Or47();

	/** Dispatch table entries 34 AND 54 both point to this single function (confirmed identical symbol at both table slots). */
	int command34Or54();
	int loadCommand34Or54();

	int command35();
	int loadCommand35();
	int command36();
	int loadCommand36();
	int command37();
	int loadCommand37();
	int command38();
	int loadCommand38();
	int command39();
	int loadCommand39();
	int command40();
	int loadCommand40();
	int command41();
	int loadCommand41();

	/**
	 * Matches an unlabeled function immediately following command41() in
	 * the disassembly - see class comment re: its index being inferred
	 * by elimination.
	 */
	int command53();
	int loadCommand53();

	int command42();
	int loadCommand42();

	int command43();

	int command45();
	int command46();

	int command48();
	int command49();
	int command50();

	int command51();
	int loadCommand51();

	/**
	 * Matches rsound_command52: the deferred-schedule branch loads bx
	 * with the callback target and immediately calls isSoundActive, but
	 * cx (isSoundActive's data-offset parameter) is never set beforehand
	 * on this path, and its result isn't tested by any branch either -
	 * confirmed dead/vestigial code in the original, intentionally
	 * omitted here.
	 */
	int command52();
	int loadCommand52();

	int command55();
	int loadCommand55();
	int command57();
	int loadCommand57();

	/** Calls the full command0() reset (not command1(), unlike every other command in this batch) before loading. */
	int command58();
	int loadCommand58();

	int command59();

	int command61();
	int command62();
	int command63();

	typedef int (RSound9:: *CommandPtr)();
	static const CommandPtr _commandList[96];

protected:
	/**
	 * Calls a function at a fixed offset within the sound driver.
	 * @param offset		Offset of the function
	 */
	void callFunction(uint16 offset) override;

public:
	RSound9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS

#endif
