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

#ifndef MADS_NEBULAR_SOUND_RSOUND_NEBULAR_H
#define MADS_NEBULAR_SOUND_RSOUND_NEBULAR_H

#include "mads/nebular/sound/rsound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

class RSound1 : public RSound {
private:
	typedef int (RSound1:: *CommandPtr)();
	static const CommandPtr _commandList[42];

	/**
	 * Shared loader for command11/12/13 - matches method1 in the
	 * disassembly (isSoundPlaying-gated command1() + 4-channel load).
	 */
	void playCommand11_12_13SharedChannels();

	/**
	 * Clamp helper: param > 0x40 ? param - 0x40 : 0. Matches the same
	 * shape seen repeatedly across the Adlib drivers (e.g. ASound1's
	 * command2627293032()).
	 */
	int clampParam();

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
public:
	RSound1(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

/** Demo RSOUND.001: `RLND AGAdemo 6-11-92`; 41 commands. */
class RSoundDemo1 : public RSound {
private:
	typedef int (RSoundDemo1::*CommandPtr)();
	static const CommandPtr _commandList[41];

	bool _command23Toggle;

	byte adjustedCommandParam() const;
	void playCommand11_12_13CommonChannels();
	int patchAndPlaySound(int offset);

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();

public:
	RSoundDemo1(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound2 : public RSound {
private:
	typedef int (RSound2:: *CommandPtr)();
	static const CommandPtr _commandList[44];

	/**
	 * Random-pick table used by command18 (TABLE1 in the disassembly).
	 * A further two rows of data follow TABLE1 in seg001 but were never
	 * listed as reachable by any command in this batch, and their values
	 * look like they may be text/other data rather than further table
	 * entries - excluded until a command turns up that actually reaches
	 * them.
	 */
	static const uint16 _command18RandomSfx[16];

	/**
	 * Persistent counter (initial value
	 * 0x2F/47). Incremented by 16 (wrapping as a byte) each time
	 * command12 runs; the low 7 bits are written into the sound data's
	 * pitch/note byte before playback. command5 resets it back to 47.
	 */
	byte _volumeCycleCounter = 0x2F;

	int command5();

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();
public:
	RSound2(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound3 : public RSound {
private:
	typedef int (RSound3:: *CommandPtr)();
	static const CommandPtr _commandList[61];

	/**
	 * Toggle used by command16 (initially
	 * 0). Flips every call; alternates between two completely different
	 * 4-channel music loads (one with a command1() fade first, one
	 * without) - preserved exactly despite the asymmetry looking odd.
	 */
	bool _command16AltFlag = false;

	/**
	 * Toggle used by command39/40 (initially 0). Shared between both commands: flips bit 2 (^= 4) on
	 * every call to either one, and the post-toggle value + 0x28 is
	 * written into the same sound data's byte 6, regardless of which of
	 * the two commands triggered the toggle. This alternates the two
	 * command's played note value between 0x28 and 0x2C.
	 */
	byte _command39_40NoteToggle = 0;

	/**
	 * Shared helper: pData[5] = value, then plays pData. Command 25
	 * calls it for both native sequence offsets. "method1" in
	 * disassembly.
	 */
	Channel *patchAndPlaySound(int offset, byte value);

	/**
	 * Matches the disassembly's OTHER "method1" (a same-named but
	 * unrelated function at a different address, seg000:0947) - writes
	 * the same byte into offset 1 of four fixed sound-data blocks
	 * (0x204A/0x229C/0x2748/0x2C56), which are exactly the offsets
	 * command11 loads into channels 4-7 right afterward. Renamed to
	 * avoid colliding with the unrelated method1() above.
	 */
	void setVariantByte(byte value);

	/**
	 * Matches the disassembly's driver-specific "sendVolume" (distinct
	 * from - and not related by inheritance to - the shared base
	 * RSound::sendVolume(); renamed here to avoid the collision). Caches
	 * the value into _channels[0]._volume (offset 0xC in the confirmed
	 * Channel struct layout - see rsound.h), then sends it via the base
	 * sendVolume() on MIDI channels 1 and 2 both. The double write to
	 * _channels[0]._volume before each send is a preserved quirk from
	 * the original (redundant but harmless).
	 */
	void sendDualVolume(byte volume);

	int command9();
	int command10();
	int command11();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27_42();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command43();
	int command44();
	int command45();
	int command46();
	int command47_49();
	int command48();
	int command50();
	int command51();
	int command57();
	int command59();
	int command60();
public:
	RSound3(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound4 : public RSound {
private:
	/**
	 * Deferred callback state (checkCallback() in the disassembly,
	 * called from this driver's own rsound_update() override) - confirmed
	 * identical in shape to RSound9's mechanism: the reload period,
	 * the countdown, and _soundPtr the pointer
	 * invoked (without self-clearing - each loadCommandNN() body clears
	 * it itself, same as RSound9's loaders) once the countdown reaches 0.
	 */
	typedef void (RSound4:: *CallbackFunction)();
	CallbackFunction _callbackFnPtr = nullptr;
	int _callbackCounter = 0;
	int _callbackPeriod = 0;

	typedef int (RSound4:: *CommandPtr)();
	static const CommandPtr _commandList[60];

	/**
	 * Writes the same variant byte into
	 * offset 1 of the five sound blocks command12 (re)loads.
	 * Value is computed from command param.
	 */
	void setCommand12Variant();

	/**
	 * Shared tail of both command10 and command58, loading channels 1-3.
	 */
	void playCommand10_58CommonChannels();

	void tickCallback() override;

	void command54Callback();
	void command55Callback();
	void command56Callback();

	/**
	 * Loads a null stream (containing event 00 00) into the specified
	 * channel, which stops playback on that channel.
	 */
	void stopChannel(byte channel);

	int command9();
	int command10();
	int command12();
	int command19();
	int command20();
	int command21();
	int command27();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command52();
	int command53();
	int command54();
	int command55();
	int command56();
	int command57();
	int command58();
	int command59();
public:
	RSound4(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound5 : public RSound {
private:
	typedef int (RSound5:: *CommandPtr)();
	static const CommandPtr _commandList[42];

	/**
	 * Shared tail of command29 and command38, loading channels 4 and 9.
	 */
	void playCommand29_38CommonChannels();

	/**
	 * Loads a null stream (containing event 00 00) into the specified
	 * channel, which stops playback on that channel.
	 */
	void stopChannel(byte channel);

	int command9();
	int command10();
	int command11_24();
	int command12_25();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
public:
	RSound5(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound6 : public RSound {
private:
	typedef int (RSound6:: *CommandPtr)();
	static const CommandPtr _commandList[30];

	/**
	 * Confirmed via rsound_update's own body (its checkCallback-equivalent
	 * is inlined directly rather than factored into a separate function
	 * like RSound4's checkCallback()): the reload period,
	 * the countdown, and a genuine CODE pointer
	 * ("call bx" - not sound data) invoked once the countdown reaches 0.
	 * Matches RSound4's mechanism exactly in shape.
	 */
	typedef void (RSound6:: *CallbackFunction)();
	int _callbackCounter = 0;
	int _callbackPeriod = 0;
	CallbackFunction _callbackFnPtr = nullptr;

	void tickCallback() override;

	/**
	 * command24/command29's
	 * own full-reload bodies. When channel 1 is currently playing the
	 * OTHER command's theme, that command doesn't interrupt it
	 * immediately - it just points _callbackFnPtr at this same reload
	 * logic so the switch happens on the next callback tick instead.
	 */
	void command24Callback();
	void command29Callback();

	int command9();
	int command10();
	int command11();
	int command12();
	int command13_14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command29();
public:
	RSound6(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound7 : public RSound {
private:
	typedef int (RSound7:: *CommandPtr)();
	static const CommandPtr _commandList[38];

	int command9();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command27();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
public:
	RSound7(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound8 : public RSound {
private:
	typedef int (RSound8:: *CommandPtr)();
	static const CommandPtr _commandList[38];

	/**
	 * Shared tail of command14/command15 -
	 * mutates three bytes of the shared sound data then plays it 4 times.
	 */
	void playCommand14_15Variant(byte v1, byte v2);

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
public:
	RSound8(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

class RSound9 : public RSound {
private:
	/**
	 * Deferred sound-loader callback state (g_callbackCounter/
	 * g_callbackPeriod/_soundPtr in the original disassembly). Mirrors
	 * ASound9's identical mechanism: arms a recurring timer that
	 * re-invokes a scheduled loader function every _callbackPeriod
	 * ticks, without ever clearing the pointer itself (the loader body
	 * clears it if it wants the recurrence to stop).
	 */
	typedef void (RSound9:: *CallbackFunction)();
	int _callbackCounter;
	int _callbackPeriod;
	CallbackFunction _callbackFnPtr;

	typedef int (RSound9:: *CommandPtr)();
	static const CommandPtr _commandList[52];

	void tickCallback() override;
	int command0() override;

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();
	int command44_46();
	int command45();
	int command47();
	int command48();
	int command49();
	int command50();
	int command51();

	// Deferred callbacks, scheduled via _callbackFnPtr by the commands above
	void command38Callback();
	void command39Callback();
	void command40Callback();
	void command41Callback();
	void command42Callback();
	void command44_46Callback();
	void command45Callback();
	void command47Callback();
	void command50Callback();
public:
	RSound9(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

/** Demo RSOUND.009: `RLND AGAdemo 6-25-92`; 40 commands. */
class RSoundDemo9 : public RSound {
private:
	typedef int (RSoundDemo9::*CommandPtr)();
	static const CommandPtr _commandList[40];

	int patchAndPlaySound(byte param);

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34_39();
	int command35();
	int command36();
	int command37();
	int command38();

public:
	RSoundDemo9(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver);

	int command(int commandId, int param) override;
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
