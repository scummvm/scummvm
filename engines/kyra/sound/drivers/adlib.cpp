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
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

 // Basic AdLib Programming:
 // https://web.archive.org/web/20050322080425/http://www.gamedev.net/reference/articles/article446.asp

#include "kyra/sound/drivers/pc_base.h"
#include "audio/fmopl.h"
#include "common/mutex.h"

#define CALLBACKS_PER_SECOND 72

namespace Kyra {

class AdLibDriver : public PCSoundDriver {
public:
	AdLibDriver(Audio::Mixer *mixer, int version);
	~AdLibDriver() override;

	void initDriver() override;
	void setSoundData(uint8 *data, uint32 size) override;
	void startSound(int track, int volume) override;
	bool isChannelPlaying(int channel) const override;
	void stopAllChannels() override;
	int getSoundTrigger() const override { return _soundTrigger; }
	void resetSoundTrigger() override { _soundTrigger = 0; }

	void callback();

	void setSyncJumpMask(uint16 mask) override { _syncJumpMask = mask; }

	void setMusicVolume(uint8 volume) override;
	void setSfxVolume(uint8 volume) override;

private:
	// These variables have not yet been named, but some of them are partly
	// known nevertheless:
	//
	// unk39 - Currently unused, except for updateCallback56()
	// unk40 - Currently unused, except for updateCallback56()

	struct Channel {
		bool lock;	// New to ScummVM
		uint8 opExtraLevel2;
		const uint8 *dataptr;
		uint8 duration;
		uint8 repeatCounter;
		int8 baseOctave;
		uint8 priority;
		uint8 dataptrStackPos;
		const uint8 *dataptrStack[4];
		int8 baseNote;
		uint8 slideTempo;
		uint8 slideTimer;
		int16 slideStep;
		int16 vibratoStep;
		uint8 vibratoStepRange;
		uint8 vibratoStepsCountdown;
		uint8 vibratoNumSteps;
		uint8 vibratoDelay;
		uint8 vibratoTempo;
		uint8 vibratoTimer;
		uint8 vibratoDelayCountdown;
		uint8 opExtraLevel1;
		uint8 spacing2;
		uint8 baseFreq;
		uint8 tempo;
		uint8 position;
		uint8 regAx;
		uint8 regBx;
		typedef void (AdLibDriver::*Callback)(Channel&);
		Callback primaryEffect;
		Callback secondaryEffect;
		uint8 fractionalSpacing;
		uint8 opLevel1;
		uint8 opLevel2;
		uint8 opExtraLevel3;
		uint8 twoChan;
		uint8 unk39;
		uint8 unk40;
		uint8 spacing1;
		uint8 durationRandomness;
		uint8 secondaryEffectTempo;
		uint8 secondaryEffectTimer;
		int8 secondaryEffectSize;
		int8 secondaryEffectPos;
		uint8 secondaryEffectRegbase;
		uint16 secondaryEffectData;
		uint8 tempoReset;
		uint8 rawNote;
		int8 pitchBend;
		uint8 volumeModifier;
	};

	void primaryEffectSlide(Channel &channel);
	void primaryEffectVibrato(Channel &channel);
	void secondaryEffect1(Channel &channel);

	void resetAdLibState();
	void writeOPL(byte reg, byte val);
	void initChannel(Channel &channel);
	void noteOff(Channel &channel);
	void initAdlibChannel(uint8 num);

	uint16 getRandomNr();
	void setupDuration(uint8 duration, Channel &channel);

	void setupNote(uint8 rawNote, Channel &channel, bool flag = false);
	void setupInstrument(uint8 regOffset, const uint8 *dataptr, Channel &channel);
	void noteOn(Channel &channel);

	void adjustVolume(Channel &channel);

	uint8 calculateOpLevel1(Channel &channel);
	uint8 calculateOpLevel2(Channel &channel);

	uint16 checkValue(int16 val) { return CLIP<int16>(val, 0, 0x3F); }

	// The sound data has at least two lookup tables:
	//
	// * One for programs, starting at offset 0.
	// * One for instruments, starting at offset 500.

	const uint8 *getInstrument(int instrumentId) {
		return getProgram(_numPrograms + instrumentId);
	}

	void setupPrograms();
	void executePrograms();

	struct ParserOpcode {
		typedef int (AdLibDriver::*POpcode)(const uint8 *&dataptr, Channel &channel, uint8 value);
		POpcode function;
		const char *name;
		int values;
	};

	static const ParserOpcode _parserOpcodeTable[];
	static const int _parserOpcodeTableSize;

	int update_setRepeat(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_checkRepeat(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupProgram(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setNoteSpacing(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_jump(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_jumpToSubroutine(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_returnFromSubroutine(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseOctave(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_stopChannel(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playRest(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_writeAdLib(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupNoteAndDuration(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseNote(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupSecondaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_stopOtherChannel(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_waitForEndOfProgram(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupInstrument(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupPrimaryEffectSlide(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removePrimaryEffectSlide(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseFreq(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupPrimaryEffectVibrato(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setPriority(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBeat(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_waitForNextBeat(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupDuration(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playNote(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setFractionalNoteSpacing(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removeSecondaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setChannelTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel3(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeExtraLevel2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setAMDepth(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setVibratoDepth(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeExtraLevel1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_clearChannel(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeNoteRandomly(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removePrimaryEffectVibrato(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_pitchBend(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_resetToGlobalTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_nop(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setDurationRandomness(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeChannelTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback46(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removeRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setRhythmLevel2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeRhythmLevel1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setRhythmLevel1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setSoundTrigger(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setTempoReset(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback56(const uint8 *&dataptr, Channel &channel, uint8 value);
private:
	// These variables have not yet been named, but some of them are partly
	// known nevertheless:
	//
	// _unkTable2[]    - Unknown. Currently only used by updateCallback46()
	// _unkTable2_1[]  - One of the tables in _unkTable2[]
	// _unkTable2_2[]  - One of the tables in _unkTable2[]
	// _unkTable2_3[]  - One of the tables in _unkTable2[]

	int _curChannel;
	uint8 _soundTrigger;

	uint16 _rnd;

	uint8 _beatDivider;
	uint8 _beatDivCnt;
	uint8 _callbackTimer;
	uint8 _beatCounter;
	uint8 _beatWaiting;
	uint8 _opLevelBD;
	uint8 _opLevelHH;
	uint8 _opLevelSD;
	uint8 _opLevelTT;
	uint8 _opLevelCY;
	uint8 _opExtraLevel1HH;
	uint8 _opExtraLevel2HH;
	uint8 _opExtraLevel1CY;
	uint8 _opExtraLevel2CY;
	uint8 _opExtraLevel2TT;
	uint8 _opExtraLevel1TT;
	uint8 _opExtraLevel1SD;
	uint8 _opExtraLevel2SD;
	uint8 _opExtraLevel1BD;
	uint8 _opExtraLevel2BD;

	OPL::OPL *_adlib;

	struct QueueEntry {
		QueueEntry() : data(0), id(0), volume(0) {}
		QueueEntry(uint8 *ptr, uint8 track, uint8 vol) : data(ptr), id(track), volume(vol) {}
		uint8 *data;
		uint8 id;
		uint8 volume;
	};

	QueueEntry _programQueue[16];
	int _programStartTimeout;
	int _programQueueStart, _programQueueEnd;
	bool _retrySounds;

	void adjustSfxData(uint8 *data, int volume);
	uint8 *_sfxPointer;
	int _sfxPriority;
	int _sfxVelocity;

	Channel _channels[10];

	uint8 _vibratoAndAMDepthBits;
	uint8 _rhythmSectionBits;

	uint8 _curRegOffset;
	uint8 _tempo;

	const uint8 *_tablePtr1;
	const uint8 *_tablePtr2;

	static const uint8 _regOffset[];
	static const uint16 _freqTable[];
	static const uint8 *const _unkTable2[];
	static const int _unkTable2Size;
	static const uint8 _unkTable2_1[];
	static const uint8 _unkTable2_2[];
	static const uint8 _unkTable2_3[];
	static const uint8 _pitchBendTables[][32];

	uint16 _syncJumpMask;

	Common::Mutex _mutex;
	Audio::Mixer *_mixer;

	uint8 _musicVolume, _sfxVolume;

	int _numPrograms;
	int _version;
};

AdLibDriver::AdLibDriver(Audio::Mixer *mixer, int version) : PCSoundDriver() {
	_version = version;
	_numPrograms = (_version == 1) ? 150 : ((_version == 4) ? 500 : 250);

	_mixer = mixer;

	_adlib = OPL::Config::create();
	if (!_adlib || !_adlib->init())
		error("Failed to create OPL");

	memset(_channels, 0, sizeof(_channels));

	_vibratoAndAMDepthBits = _curRegOffset = 0;

	_curChannel = _rhythmSectionBits = 0;
	_rnd = 0x1234;

	_tempo = 0;
	_soundTrigger = 0;
	_programStartTimeout = 0;

	_callbackTimer = 0xFF;
	_beatDivider = _beatDivCnt = _beatCounter = _beatWaiting = 0;
	_opLevelBD = _opLevelHH = _opLevelSD = _opLevelTT = _opLevelCY = 0;
	_opExtraLevel1HH = _opExtraLevel2HH =
	_opExtraLevel1CY = _opExtraLevel2CY =
	_opExtraLevel2TT = _opExtraLevel1TT =
	_opExtraLevel1SD = _opExtraLevel2SD =
	_opExtraLevel1BD = _opExtraLevel2BD = 0;

	_tablePtr1 = _tablePtr2 = nullptr;

	_syncJumpMask = 0;

	_musicVolume = 0;
	_sfxVolume = 0;

	_sfxPointer = nullptr;

	_programQueueStart = _programQueueEnd = 0;
	_retrySounds = false;

	_adlib->start(new Common::Functor0Mem<void, AdLibDriver>(this, &AdLibDriver::callback), CALLBACKS_PER_SECOND);
}

AdLibDriver::~AdLibDriver() {
	delete _adlib;
	_adlib = nullptr;
}

void AdLibDriver::setMusicVolume(uint8 volume) {
	Common::StackLock lock(_mutex);

	_musicVolume = volume;

	for (uint i = 0; i < 6; ++i) {
		Channel &chan = _channels[i];
		chan.volumeModifier = volume;

		const uint8 regOffset = _regOffset[i];

		// Level Key Scaling / Total Level
		writeOPL(0x40 + regOffset, calculateOpLevel1(chan));
		writeOPL(0x43 + regOffset, calculateOpLevel2(chan));
	}

	// For now we use the music volume for both sfx and music in Kyra1 and EoB
	if (_version < 4) {
		_sfxVolume = volume;

		for (uint i = 6; i < 9; ++i) {
			Channel &chan = _channels[i];
			chan.volumeModifier = volume;

			const uint8 regOffset = _regOffset[i];

			// Level Key Scaling / Total Level
			writeOPL(0x40 + regOffset, calculateOpLevel1(chan));
			writeOPL(0x43 + regOffset, calculateOpLevel2(chan));
		}
	}
}

void AdLibDriver::setSfxVolume(uint8 volume) {
	// We only support sfx volume in version 4 games.
	if (_version < 4)
		return;

	Common::StackLock lock(_mutex);

	_sfxVolume = volume;

	for (uint i = 6; i < 9; ++i) {
		Channel &chan = _channels[i];
		chan.volumeModifier = volume;

		const uint8 regOffset = _regOffset[i];

		// Level Key Scaling / Total Level
		writeOPL(0x40 + regOffset, calculateOpLevel1(chan));
		writeOPL(0x43 + regOffset, calculateOpLevel2(chan));
	}
}

void AdLibDriver::initDriver() {
	Common::StackLock lock(_mutex);
	resetAdLibState();
}

void AdLibDriver::setSoundData(uint8 *data, uint32 size) {
	Common::StackLock lock(_mutex);

	// Drop all tracks that are still queued. These would point to the old
	// sound data.
	_programQueueStart = _programQueueEnd = 0;
	_programQueue[0] = QueueEntry();

	_sfxPointer = nullptr;

	delete[] _soundData;
	_soundData = data;
	_soundDataSize = size;
}

void AdLibDriver::startSound(int track, int volume) {
	Common::StackLock lock(_mutex);

	uint8 *trackData = getProgram(track);
	if (!trackData)
		return;

	if (_programQueueEnd == _programQueueStart && _programQueue[_programQueueEnd].data != 0) {
		// Don't warn when dropping tracks in EoB. The queue is always full there if a couple of monsters are around.
		if (_version >= 3)
			warning("AdLibDriver: Program queue full, dropping track %d", track);
		return;
	}

	_programQueue[_programQueueEnd] = QueueEntry(trackData, track, volume);
	++_programQueueEnd &= 15;
}

bool AdLibDriver::isChannelPlaying(int channel) const {
	Common::StackLock lock(_mutex);

	assert(channel >= 0 && channel <= 9);
	return (_channels[channel].dataptr != 0);
}

void AdLibDriver::stopAllChannels() {
	Common::StackLock lock(_mutex);

	for (int channel = 0; channel <= 9; ++channel) {
		_curChannel = channel;

		Channel &chan = _channels[_curChannel];
		chan.priority = 0;
		chan.dataptr = 0;

		if (channel != 9)
			noteOff(chan);
	}
	_retrySounds = false;

	_programQueueStart = _programQueueEnd = 0;
	_programQueue[0] = QueueEntry();
	_programStartTimeout = 0;
}

// timer callback
//
// Starts and executes programs and maintains a global beat that channels
// can synchronize on.

void AdLibDriver::callback() {
	Common::StackLock lock(_mutex);
	if (_programStartTimeout)
		--_programStartTimeout;
	else
		setupPrograms();
	executePrograms();

	uint8 temp = _callbackTimer;
	_callbackTimer += _tempo;
	if (_callbackTimer < temp) {
		if (!(--_beatDivCnt)) {
			_beatDivCnt = _beatDivider;
			++_beatCounter;
		}
	}
}

void AdLibDriver::setupPrograms() {
	QueueEntry &entry = _programQueue[_programQueueStart];
	uint8 *ptr = entry.data;

	// If there is no program queued, we skip this.
	if (_programQueueStart == _programQueueEnd && !ptr)
		return;

	// The AdLib driver (in its old versions used for EOB) is not suitable for modern (fast) CPUs.
	// The stop sound track (track 0 which has a priority of 50) will often still be busy when the
	// next sound (with a lower priority) starts which will cause that sound to be skipped. We simply
	// restart incoming sounds during stop sound execution.
	// UPDATE: This still applies after introduction of the _programQueue.
	// UPDATE: This can also happen with the HOF main menu, so I commented out the version < 3 limitation.
	QueueEntry retrySound;
	if (/*_version < 3 &&*/ entry.id == 0)
		_retrySounds = true;
	else if (_retrySounds)
		retrySound = entry;

	// Clear the queue entry
	entry.data = nullptr;
	++_programQueueStart &= 15;

	// Safety check: 2 bytes (channel, priority) are required for each
	// program, plus 2 more bytes (opcode, _sfxVelocity) for sound effects.
	// More data is needed, but executePrograms() checks for that.
	// Also ignore request for invalid channel number.
	if (!ptr || ptr - _soundData + 2 > _soundDataSize)
		return;

	const int chan = *ptr;
	if (chan > 9 || (chan < 9 && ptr - _soundData + 4 > _soundDataSize))
		return;

	Channel &channel = _channels[chan];

	// Adjust data in case we hit a sound effect.
	adjustSfxData(ptr++, entry.volume);

	const int priority = *ptr++;

	// Only start this sound if its priority is higher than the one
	// already playing.

	if (priority >= channel.priority) {
		initChannel(channel);
		channel.priority = priority;
		channel.dataptr = ptr;
		channel.tempo = 0xFF;
		channel.position = 0xFF;
		channel.duration = 1;

		if (chan <= 5)
			channel.volumeModifier = _musicVolume;
		else
			channel.volumeModifier = _sfxVolume;

		initAdlibChannel(chan);

		// We need to wait two callback calls till we can start another track.
		// This is (probably) required to assure that the sfx are started with
		// the correct priority and velocity.
		_programStartTimeout = 2;

		retrySound = QueueEntry();
	}

	if (retrySound.data) {
		debugC(9, kDebugLevelSound, "AdLibDriver::setupPrograms(): WORKAROUND - Restarting skipped sound %d)", retrySound.id);
		startSound(retrySound.id, retrySound.volume);
	}
}

void AdLibDriver::adjustSfxData(uint8 *ptr, int volume) {
	// Check whether we need to reset the data of an old sfx which has been
	// started.
	if (_sfxPointer) {
		_sfxPointer[1] = _sfxPriority;
		_sfxPointer[3] = _sfxVelocity;
		_sfxPointer = nullptr;
	}

	// Only music tracks are started on channel 9, thus we need to make sure
	// we do not have a music track here.
	if (*ptr == 9)
		return;

	// Store the pointer so we can reset the data when a new program is started.
	_sfxPointer = ptr;

	// Store the old values.
	_sfxPriority = ptr[1];
	_sfxVelocity = ptr[3];

	// Adjust the values.
	if (volume != 0xFF) {
		if (_version >= 3) {
			int newVal = ((((ptr[3]) + 63) * volume) >> 8) & 0xFF;
			ptr[3] = -newVal + 63;
			ptr[1] = ((ptr[1] * volume) >> 8) & 0xFF;
		} else {
			int newVal = ((_sfxVelocity << 2) ^ 0xFF) * volume;
			ptr[3] = (newVal >> 10) ^ 0x3F;
			ptr[1] = newVal >> 11;
		}
	}
}

// A few words on opcode parsing and timing:
//
// First of all, We simulate a timer callback 72 times per second. Each timeout
// we update each channel that has something to play.
//
// Each channel has its own individual tempo, which is added to its position.
// This will frequently cause the position to "wrap around" but that is
// intentional. In fact, it's the signal to go ahead and do more stuff with
// that channel.
//
// Each channel also has a duration, indicating how much time is left on the
// its current task. This duration is decreased by one. As long as it still has
// not reached zero, the only thing that can happen is that the note is turned
// off depending on manual or automatic note spacing. Once the duration reaches
// zero, a new set of musical opcodes are executed.
//
// An opcode is one byte, followed by a variable number of parameters. Since
// most opcodes have at least one one-byte parameter, we read that as well. Any
// opcode that doesn't have that one parameter is responsible for moving the
// data pointer back again.
//
// If the most significant bit of the opcode is 1, it's a function; call it.
// The opcode functions return either 0 (continue), 1 (stop) or 2 (stop, and do
// not run the effects callbacks).
//
// If the most significant bit of the opcode is 0, it's a note, and the first
// parameter is its duration. (There are cases where the duration is modified
// but that's an exception.) The note opcode is assumed to return 1, and is the
// last opcode unless its duration is zero.
//
// Finally, most of the times that the callback is called, it will invoke the
// effects callbacks. The final opcode in a set can prevent this, if it's a
// function and it returns anything other than 1.

void AdLibDriver::executePrograms() {
	// Each channel runs its own program. There are ten channels: One for
	// each AdLib channel (0-8), plus one "control channel" (9) which is
	// the one that tells the other channels what to do.

	// This is where we ensure that channels that are made to jump "in
	// sync" do so.

	if (_syncJumpMask) {
		bool forceUnlock = true;

		for (_curChannel = 9; _curChannel >= 0; --_curChannel) {
			if ((_syncJumpMask & (1 << _curChannel)) == 0)
				continue;

			if (_channels[_curChannel].dataptr && !_channels[_curChannel].lock)
				forceUnlock = false;
		}

		if (forceUnlock) {
			for (_curChannel = 9; _curChannel >= 0; --_curChannel)
				if (_syncJumpMask & (1 << _curChannel))
					_channels[_curChannel].lock = false;
		}
	}

	for (_curChannel = 9; _curChannel >= 0; --_curChannel) {
		Channel &channel = _channels[_curChannel];
		int result = 1;

		if (!channel.dataptr)
			continue;

		if (channel.lock && (_syncJumpMask & (1 << _curChannel)))
			continue;

		if (_curChannel == 9)
			_curRegOffset = 0;
		else
			_curRegOffset = _regOffset[_curChannel];

		if (channel.tempoReset)
			channel.tempo = _tempo;

		uint8 backup = channel.position;
		channel.position += channel.tempo;
		if (channel.position < backup) {
			if (--channel.duration) {
				if (channel.duration == channel.spacing2)
					noteOff(channel);
				if (channel.duration == channel.spacing1 && _curChannel != 9)
					noteOff(channel);
			} else {
				// An opcode is not allowed to modify its own
				// data pointer except through the 'dataptr'
				// parameter. To enforce that, we have to work
				// on a copy of the data pointer.
				//
				// This fixes a subtle music bug where the
				// wrong music would play when getting the
				// quill in Kyra 1.
				const uint8 *dataptr = channel.dataptr;
				while (dataptr) {
					uint8 opcode, param;
					// Safety check to avoid illegal access.
					// Stop channel if not enough data.
					if (dataptr - _soundData < _soundDataSize)
						opcode = *dataptr++;
					else
						opcode = 0xFF;
					if (opcode < 0x80 && dataptr - _soundData == _soundDataSize)
						opcode = 0xFF;

					if (opcode & 0x80) {
						opcode &= 0x7F;
						if (opcode >= _parserOpcodeTableSize)
							opcode = _parserOpcodeTableSize - 1;
						// Safety check for end of data.
						if (dataptr - _soundData + _parserOpcodeTable[opcode].values > _soundDataSize)
							opcode = _parserOpcodeTableSize - 1;

						const ParserOpcode &op = _parserOpcodeTable[opcode];
						param = op.values ? *dataptr : 0;
						dataptr++;

						debugC(9, kDebugLevelSound, "Calling opcode '%s' (%d) (channel: %d)", op.name, opcode, _curChannel);
						result = (this->*(op.function))(dataptr, channel, param);
						channel.dataptr = dataptr;
						if (result)
							break;
					} else {
						param = *dataptr++;
						debugC(9, kDebugLevelSound, "Note on opcode 0x%02X (duration: %d) (channel: %d)", opcode, param, _curChannel);
						setupNote(opcode, channel);
						noteOn(channel);
						setupDuration(param, channel);
						if (param) {
							// We need to make sure we are always running the
							// effects after this. Otherwise some sounds are
							// wrong. Like the sfx when bumping into a wall in
							// LoL.
							result = 1;
							channel.dataptr = dataptr;
							break;
						}
					}
				}
			}
		}

		if (result == 1) {
			if (channel.primaryEffect)
				(this->*(channel.primaryEffect))(channel);
			if (channel.secondaryEffect)
				(this->*(channel.secondaryEffect))(channel);
		}
	}
}

//

void AdLibDriver::resetAdLibState() {
	debugC(9, kDebugLevelSound, "resetAdLibState()");
	_rnd = 0x1234;

	// Authorize the control of the waveforms
	writeOPL(0x01, 0x20);

	// Select FM music mode
	writeOPL(0x08, 0x00);

	// I would guess the main purpose of this is to turn off the rhythm,
	// thus allowing us to use 9 melodic voices instead of 6.
	writeOPL(0xBD, 0x00);

	initChannel(_channels[9]);
	for (int loop = 8; loop >= 0; loop--) {
		// Silence the channel
		writeOPL(0x40 + _regOffset[loop], 0x3F);
		writeOPL(0x43 + _regOffset[loop], 0x3F);
		initChannel(_channels[loop]);
	}
}

// Old calling style: output0x388(0xABCD)
// New calling style: writeOPL(0xAB, 0xCD)

void AdLibDriver::writeOPL(byte reg, byte val) {
	_adlib->writeReg(reg, val);
}

void AdLibDriver::initChannel(Channel &channel) {
	debugC(9, kDebugLevelSound, "initChannel(%lu)", (long)(&channel - _channels));
	memset(&channel.dataptr, 0, sizeof(Channel) - ((char *)&channel.dataptr - (char *)&channel));

	channel.tempo = 0xFF;
	channel.priority = 0;
	// normally here are nullfuncs but we set nullptr for now
	channel.primaryEffect = nullptr;
	channel.secondaryEffect = nullptr;
	channel.spacing1 = 1;
	channel.lock = false;
}

void AdLibDriver::noteOff(Channel &channel) {
	debugC(9, kDebugLevelSound, "noteOff(%lu)", (long)(&channel - _channels));

	// The control channel has no corresponding AdLib channel

	if (_curChannel >= 9)
		return;

	// When the rhythm section is enabled, channels 6, 7 and 8 are special.

	if (_rhythmSectionBits && _curChannel >= 6)
		return;

	// This means the "Key On" bit will always be 0
	channel.regBx &= 0xDF;

	// Octave / F-Number / Key-On
	writeOPL(0xB0 + _curChannel, channel.regBx);
}

void AdLibDriver::initAdlibChannel(uint8 chan) {
	debugC(9, kDebugLevelSound, "initAdlibChannel(%d)", chan);

	// The control channel has no corresponding AdLib channel

	if (chan >= 9)
		return;

	// I believe this has to do with channels 6, 7, and 8 being special
	// when AdLib's rhythm section is enabled.

	if (_rhythmSectionBits && chan >= 6)
		return;

	uint8 offset = _regOffset[chan];

	// The channel is cleared: First the attack/delay rate, then the
	// sustain level/release rate, and finally the note is turned off.

	writeOPL(0x60 + offset, 0xFF);
	writeOPL(0x63 + offset, 0xFF);

	writeOPL(0x80 + offset, 0xFF);
	writeOPL(0x83 + offset, 0xFF);

	writeOPL(0xB0 + chan, 0x00);

	// ...and then the note is turned on again, with whatever value is
	// still lurking in the A0 + chan register, but everything else -
	// including the two most significant frequency bit, and the octave -
	// set to zero.
	//
	// This is very strange behavior, and causes problems with the ancient
	// FMOPL code we borrowed from AdPlug. I've added a workaround. See
	// audio/softsynth/opl/mame.cpp for more details.
	//
	// Fortunately, the more modern DOSBox FMOPL code does not seem to have
	// any trouble with this.

	writeOPL(0xB0 + chan, 0x20);
}

// I believe this is a random number generator. It actually does seem to
// generate an even distribution of almost all numbers from 0 through 65535,
// though in my tests some numbers were never generated.

uint16 AdLibDriver::getRandomNr() {
	_rnd += 0x9248;
	uint16 lowBits = _rnd & 7;
	_rnd >>= 3;
	_rnd |= (lowBits << 13);
	return _rnd;
}

void AdLibDriver::setupDuration(uint8 duration, Channel &channel) {
	debugC(9, kDebugLevelSound, "setupDuration(%d, %lu)", duration, (long)(&channel - _channels));
	if (channel.durationRandomness) {
		channel.duration = duration + (getRandomNr() & channel.durationRandomness);
		return;
	}
	if (channel.fractionalSpacing)
		channel.spacing2 = (duration >> 3) * channel.fractionalSpacing;
	channel.duration = duration;
}

// This function may or may not play the note. It's usually followed by a call
// to noteOn(), which will always play the current note.

void AdLibDriver::setupNote(uint8 rawNote, Channel &channel, bool flag) {
	debugC(9, kDebugLevelSound, "setupNote(%d, %lu)", rawNote, (long)(&channel - _channels));

	if (_curChannel >= 9)
		return;

	channel.rawNote = rawNote;

	int8 note = (rawNote & 0x0F) + channel.baseNote;
	int8 octave = ((rawNote + channel.baseOctave) >> 4) & 0x0F;

	// There are only twelve notes. If we go outside that, we have to
	// adjust the note and octave.

	if (note >= 12) {
		octave += note / 12;
		note %= 12;
	} else if (note < 0) {
		int8 octaves = -(note + 1) / 12 + 1;
		octave -= octaves;
		note += 12 * octaves;
	}

	// The calculation of frequency looks quite different from the original
	// disassembly at a first glance, but when you consider that the
	// largest possible value would be 0x0246 + 0xFF + 0x47 (and that's if
	// baseFreq is unsigned), freq is still a 10-bit value, just as it
	// should be to fit in the Ax and Bx registers.
	//
	// If it were larger than that, it could have overflowed into the
	// octave bits, and that could possibly have been used in some sound.
	// But as it is now, I can't see any way it would happen.

	uint16 freq = _freqTable[note] + channel.baseFreq;

	// When called from callback 41, the behavior is slightly different:
	// We adjust the frequency, even when channel.pitchBend is 0.

	if (channel.pitchBend || flag) {
		const uint8 *table;
		// For safety, limit the values used to index the tables.
		uint8 indexNote = CLIP(rawNote & 0x0F, 0, 11);

		if (channel.pitchBend >= 0) {
			table = _pitchBendTables[indexNote + 2];
			freq += table[CLIP(+channel.pitchBend, 0, 31)];
		} else {
			table = _pitchBendTables[indexNote];
			freq -= table[CLIP(-channel.pitchBend, 0, 31)];
		}
	}

	// Shift octave to correct bit position and limit to valid range.
	octave = CLIP<int8>(octave, 0, 7) << 2;

	// Update octave & frequency, but keep on/off state.
	channel.regAx = freq & 0xFF;
	channel.regBx = (channel.regBx & 0x20) | octave | ((freq >> 8) & 0x03);

	writeOPL(0xA0 + _curChannel, channel.regAx);
	writeOPL(0xB0 + _curChannel, channel.regBx);
}

void AdLibDriver::setupInstrument(uint8 regOffset, const uint8 *dataptr, Channel &channel) {
	debugC(9, kDebugLevelSound, "setupInstrument(%d, %p, %lu)", regOffset, (const void *)dataptr, (long)(&channel - _channels));

	if (_curChannel >= 9)
		return;

	// Safety check: need 11 bytes of data.
	if (dataptr - _soundData + 11 > _soundDataSize)
		return;

	// Amplitude Modulation / Vibrato / Envelope Generator Type /
	// Keyboard Scaling Rate / Modulator Frequency Multiple
	writeOPL(0x20 + regOffset, *dataptr++);
	writeOPL(0x23 + regOffset, *dataptr++);

	uint8 temp = *dataptr++;

	// Feedback / Algorithm

	// It is very likely that _curChannel really does refer to the same
	// channel as regOffset, but there's only one Cx register per channel.

	writeOPL(0xC0 + _curChannel, temp);

	// The algorithm bit. I don't pretend to understand this fully, but
	// "If set to 0, operator 1 modulates operator 2. In this case,
	// operator 2 is the only one producing sound. If set to 1, both
	// operators produce sound directly. Complex sounds are more easily
	// created if the algorithm is set to 0."

	channel.twoChan = temp & 1;

	// Waveform Select
	writeOPL(0xE0 + regOffset, *dataptr++);
	writeOPL(0xE3 + regOffset, *dataptr++);

	channel.opLevel1 = *dataptr++;
	channel.opLevel2 = *dataptr++;

	// Level Key Scaling / Total Level
	writeOPL(0x40 + regOffset, calculateOpLevel1(channel));
	writeOPL(0x43 + regOffset, calculateOpLevel2(channel));

	// Attack Rate / Decay Rate
	writeOPL(0x60 + regOffset, *dataptr++);
	writeOPL(0x63 + regOffset, *dataptr++);

	// Sustain Level / Release Rate
	writeOPL(0x80 + regOffset, *dataptr++);
	writeOPL(0x83 + regOffset, *dataptr++);
}

// Apart from playing the note, this function also updates the variables for
// the vibrato primary effect.

void AdLibDriver::noteOn(Channel &channel) {
	debugC(9, kDebugLevelSound, "noteOn(%lu)", (long)(&channel - _channels));

	// The "note on" bit is set, and the current note is played.

	if (_curChannel >= 9)
		return;

	channel.regBx |= 0x20;
	writeOPL(0xB0 + _curChannel, channel.regBx);

	// Update vibrato effect variables: vibratoStep is set to a
	// vibratoStepRange+1-bit value proportional to the note's f-number.
	// Reinitalize delay countdown; vibratoStepsCountdown reinitialization omitted.
	int8 shift = 9 - CLIP<int8>(channel.vibratoStepRange, 0, 9);
	uint16 freq = ((channel.regBx << 8) | channel.regAx) & 0x3FF;
	channel.vibratoStep = (freq >> shift) & 0xFF;
	channel.vibratoDelayCountdown = channel.vibratoDelay;
}

void AdLibDriver::adjustVolume(Channel &channel) {
	debugC(9, kDebugLevelSound, "adjustVolume(%lu)", (long)(&channel - _channels));

	if (_curChannel >= 9)
		return;

	// Level Key Scaling / Total Level

	writeOPL(0x43 + _regOffset[_curChannel], calculateOpLevel2(channel));
	if (channel.twoChan)
		writeOPL(0x40 + _regOffset[_curChannel], calculateOpLevel1(channel));
}

// This is presumably only used for some sound effects, e.g. Malcolm blowing up
// the trees in the intro (but not the effect where he "booby-traps" the big
// tree) and turning Kallak to stone. Related functions and variables:
//
// update_setupPrimaryEffectSlide()
//    - Initializes slideTempo, slideStep and slideTimer
//    - slideTempo is not further modified
//    - slideStep is not further modified, except by update_removePrimaryEffectSlide()
//
// update_removePrimaryEffectSlide()
//    - Deinitializes slideStep
//
// slideTempo - determines how often the frequency is updated
// slideStep  - amount the frequency changes each update
// slideTimer - keeps track of time

void AdLibDriver::primaryEffectSlide(Channel &channel) {
	debugC(9, kDebugLevelSound, "Calling primaryEffectSlide (channel: %d)", _curChannel);

	if (_curChannel >= 9)
		return;

	// Next update is due when slideTimer overflows.
	uint8 temp = channel.slideTimer;
	channel.slideTimer += channel.slideTempo;
	if (channel.slideTimer >= temp)
		return;

	// Extract current frequency, (shifted) octave, and "note on" bit into
	// separate variable so calculations can't overflow into other fields.
	int16 freq = ((channel.regBx & 0x03) << 8) | channel.regAx;
	uint8 octave = channel.regBx & 0x1C;
	uint8 note_on = channel.regBx & 0x20;

	// Limit slideStep to prevent integer overflow.
	freq += CLIP<int16>(channel.slideStep, -0x3FF, 0x3FF);

	if (channel.slideStep >= 0 && freq >= 734) {
		// The new frequency is too high. Shift it down and go
		// up one octave.
		freq >>= 1;
		if (!(freq & 0x3FF))
			++freq;
		octave += 4;
	} else if (channel.slideStep < 0 && freq < 388) {
		// Safety check: a negative frequency triggers undefined
		// behavior for the left shift operator below.
		if (freq < 0)
			freq = 0;

		// The new frequency is too low. Shift it up and go
		// down one octave.
		freq <<= 1;
		if (!(freq & 0x3FF))
			--freq;
		octave -= 4;
	}

	// Set new frequency and octave.
	channel.regAx = freq & 0xFF;
	channel.regBx = note_on | (octave & 0x1C) | ((freq >> 8) & 0x03);

	writeOPL(0xA0 + _curChannel, channel.regAx);
	writeOPL(0xB0 + _curChannel, channel.regBx);
}

// This is presumably only used for some sound effects, e.g. Malcolm entering
// and leaving Kallak's hut. Related functions and variables:
//
// update_setupPrimaryEffectVibrato()
//    - Initializes vibratoTempo, vibratoStepRange, vibratoStepsCountdown,
//      vibratoNumSteps, and vibratoDelay
//    - vibratoTempo is not further modified
//    - vibratoStepRange is not further modified
//    - vibratoStepsCountdown is a countdown that gets reinitialized to
//      vibratoNumSteps on zero, but is initially only half as much
//    - vibratoNumSteps is not further modified
//    - vibratoDelay is not further modified
//
// noteOn()
//    - Plays the current note
//    - Sets vibratoStep depending on vibratoStepRange and the note's f-number
//    - Initializes vibratoDelayCountdown with vibratoDelay
//
// vibratoTempo          - determines how often the frequency is updated
// vibratoStepRange      - determines frequency step size depending on f-number
// vibratoStepsCountdown - reverses slide direction on zero
// vibratoNumSteps       - initializer for vibratoStepsCountdown countdown
// vibratoDelay          - initializer for vibratoDelayCountdown
// vibratoStep           - amount the frequency changes each update
// vibratoDelayCountdown - effect starts when it reaches zero
// vibratoTimer          - keeps track of time
//
// Note that vibratoTimer is never initialized. Not that it should matter much,
// but it is a bit sloppy. Also vibratoStepsCountdown should be reset to its
// initial value in noteOn() but isn't.

void AdLibDriver::primaryEffectVibrato(Channel &channel) {
	debugC(9, kDebugLevelSound, "Calling primaryEffectVibrato (channel: %d)", _curChannel);

	if (_curChannel >= 9)
		return;

	// When a new note is played the effect doesn't start immediately.
	if (channel.vibratoDelayCountdown) {
		--channel.vibratoDelayCountdown;
		return;
	}

	// Next update is due when vibratoTimer overflows.
	uint8 temp = channel.vibratoTimer;
	channel.vibratoTimer += channel.vibratoTempo;
	if (channel.vibratoTimer < temp) {
		// Reverse direction every vibratoNumSteps updates
		if (!(--channel.vibratoStepsCountdown)) {
			channel.vibratoStep = -channel.vibratoStep;
			channel.vibratoStepsCountdown = channel.vibratoNumSteps;
		}

		// Update frequency.
		uint16 freq = ((channel.regBx << 8) | channel.regAx) & 0x3FF;
		freq += channel.vibratoStep;

		channel.regAx = freq & 0xFF;
		channel.regBx = (channel.regBx & 0xFC) | (freq >> 8);

		// Octave / F-Number / Key-On
		writeOPL(0xA0 + _curChannel, channel.regAx);
		writeOPL(0xB0 + _curChannel, channel.regBx);
	}
}

// I don't know where this is used. An OPL register is regularly updated
// with data from a chunk of the _soundData[] buffer, i.e., one instrument
// parameter register is modulated with data from the chunk. The data is
// reused repeatedly starting from the end of the chunk.
//
// Since we use _curRegOffset to specify the final register, it's quite
// unlikely that this function is ever used to play notes. It's probably only
// used to modify the sound. Another thing that supports this idea is that it
// can be combined with any of the effects callbacks above.
//
// Related functions and variables:
//
// update_setupSecondaryEffect1()
//    - Initialies secondaryEffectTimer, secondaryEffectTempo,
//      secondaryEffectSize, secondaryEffectPos, secondaryEffectRegbase,
//      and secondaryEffectData
//    - secondaryEffectTempo is not further modified
//    - secondaryEffectSize is not further modified
//    - secondaryEffectRegbase is not further modified
//    - secondaryEffectData is not further modified
//
// secondaryEffectTimer   - keeps track of time
// secondaryEffectTempo   - determines how often the operation is performed
// secondaryEffectSize    - the size of the data chunk
// secondaryEffectPos     - the current index into the data chunk
// secondaryEffectRegbase - the operation to perform
// secondaryEffectData    - the offset of the data chunk

void AdLibDriver::secondaryEffect1(Channel &channel) {
	debugC(9, kDebugLevelSound, "Calling secondaryEffect1 (channel: %d)", _curChannel);

	if (_curChannel >= 9)
		return;

	uint8 temp = channel.secondaryEffectTimer;
	channel.secondaryEffectTimer += channel.secondaryEffectTempo;
	if (channel.secondaryEffectTimer < temp) {
		if (--channel.secondaryEffectPos < 0)
			channel.secondaryEffectPos = channel.secondaryEffectSize;
		writeOPL(channel.secondaryEffectRegbase + _curRegOffset,
			_soundData[channel.secondaryEffectData + channel.secondaryEffectPos]);
	}
}

uint8 AdLibDriver::calculateOpLevel1(Channel &channel) {
	uint8 value = channel.opLevel1 & 0x3F;

	if (channel.twoChan) {
		value += channel.opExtraLevel1;
		value += channel.opExtraLevel2;

		uint16 level3 = (channel.opExtraLevel3 ^ 0x3F) * channel.volumeModifier;
		if (level3) {
			level3 += 0x3F;
			level3 >>= 8;
		}

		value += level3 ^ 0x3F;
	}

	// The clipping as signed instead of unsigned caused very ugly noises in LOK when the music
	// was fading out in certain situations (bug #11303). The bug seems to come to surface only
	// when the volume is not set to the maximum.
	// I have confirmed that the noise bug also appears in LOL floppy (Westwood logo sound). It has
	// been reported to be present in EOB 1 (intro music), but I haven't been able to confirm it.
	// The original AdLib drivers all do the same wrong clipping. At least in the original EOB and
	// LOK games this wouldn't cause issues, since the original drivers (and games) do not have
	// volume settings and use a simpler calculation of the total level (just adding the three
	// opExtraLevels to the opLevel).
	// The later (HOF/LOL) original drivers do the same wrong clipping, too. But original LOL floppy
	// doesn't have volume settings either. And with max volume the logo sound is okay...
	if (value & 0x80)
		debugC(3, kDebugLevelSound, "AdLibDriver::calculateOpLevel1(): WORKAROUND - total level clipping uint/int bug encountered");
	value = CLIP<uint8>(value, 0, 0x3F);

	if (!channel.volumeModifier)
		value = 0x3F;

	// Preserve the scaling level bits from opLevel1
	return value | (channel.opLevel1 & 0xC0);
}

uint8 AdLibDriver::calculateOpLevel2(Channel &channel) {
	uint8 value = channel.opLevel2 & 0x3F;

	value += channel.opExtraLevel1;
	value += channel.opExtraLevel2;

	uint16 level3 = (channel.opExtraLevel3 ^ 0x3F) * channel.volumeModifier;
	if (level3) {
		level3 += 0x3F;
		level3 >>= 8;
	}

	value += level3 ^ 0x3F;

	// See comment in calculateOpLevel1()
	if (value & 0x80)
		debugC(3, kDebugLevelSound, "AdLibDriver::calculateOpLevel2(): WORKAROUND - total level clipping uint/int bug encountered");
	value = CLIP<uint8>(value, 0, 0x3F);

	if (!channel.volumeModifier)
		value = 0x3F;

	// Preserve the scaling level bits from opLevel2
	return value | (channel.opLevel2 & 0xC0);
}

// parser opcodes

int AdLibDriver::update_setRepeat(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.repeatCounter = value;
	return 0;
}

int AdLibDriver::update_checkRepeat(const uint8 *&dataptr, Channel &channel, uint8 value) {
	++dataptr;
	if (--channel.repeatCounter) {
		int16 add = READ_LE_UINT16(dataptr - 2);

		// Safety check: ignore jump to invalid address
		if (add < -(dataptr - _soundData) || _soundData + _soundDataSize - dataptr <= add)
			warning("AdlibDriver::update_checkRepeat: Ignoring invalid offset %i", add);
		else
			dataptr += add;
	}
	return 0;
}

int AdLibDriver::update_setupProgram(const uint8 *&dataptr, Channel &channel, uint8 value) {
	if (value == 0xFF)
		return 0;

	const uint8 *ptr = getProgram(value);

	// In case we encounter an invalid program we simply ignore it and do
	// nothing instead. The original did not care about invalid programs and
	// simply tried to play them anyway... But to avoid crashes due we ingore
	// them.
	// This, for example, happens in the Lands of Lore intro when Scotia gets
	// the ring in the intro.
	if (!ptr || ptr - _soundData + 2 > _soundDataSize) {
		debugC(3, kDebugLevelSound, "AdLibDriver::update_setupProgram: Invalid program %d specified", value);
		return 0;
	}

	uint8 chan = *ptr++;
	uint8 priority = *ptr++;

	// Safety check: ignore programs with invalid channel number.
	if (chan > 9) {
		warning("AdLibDriver::update_setupProgram: Invalid channel %d", chan);
		return 0;
	}

	Channel &channel2 = _channels[chan];

	if (priority >= channel2.priority) {
		// We keep new tracks from being started for two further iterations of
		// the callback. This assures the correct velocity is used for this
		// program.
		_programStartTimeout = 2;
		initChannel(channel2);
		channel2.priority = priority;
		channel2.dataptr = ptr;
		channel2.tempo = 0xFF;
		channel2.position = 0xFF;
		channel2.duration = 1;

		if (chan <= 5)
			channel2.volumeModifier = _musicVolume;
		else
			channel2.volumeModifier = _sfxVolume;

		initAdlibChannel(chan);
	}

	return 0;
}

int AdLibDriver::update_setNoteSpacing(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.spacing1 = value;
	return 0;
}

int AdLibDriver::update_jump(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	int16 add = READ_LE_UINT16(dataptr); dataptr += 2;
	if (_version == 1) {
		// Safety check: ignore jump to invalid address
		if (add < 191 || add - 191 >= (int32)_soundDataSize)
			dataptr = nullptr;
		else
			dataptr = _soundData + add - 191;
	} else {
		if (add < -(dataptr - _soundData) || _soundData + _soundDataSize - dataptr <= add)
			dataptr = nullptr;
		else
			dataptr += add;
	}
	if (!dataptr) {
		warning("AdlibDriver::update_jump: Invalid offset %i, stopping channel", add);
		return update_stopChannel(dataptr, channel, 0);
	}
	if (_syncJumpMask & (1 << (&channel - _channels)))
		channel.lock = true;
	return 0;
}

int AdLibDriver::update_jumpToSubroutine(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	int16 add = READ_LE_UINT16(dataptr); dataptr += 2;

	// Safety checks: ignore jumps when stack is full or address is invalid.
	if (channel.dataptrStackPos >= ARRAYSIZE(channel.dataptrStack)) {
		warning("AdLibDriver::update_jumpToSubroutine: Stack overlow");
		return 0;
	}
	channel.dataptrStack[channel.dataptrStackPos++] = dataptr;
	if (_version < 3) {
		if (add < 191 || add - 191 >= (int32)_soundDataSize)
			dataptr = nullptr;
		else
			dataptr = _soundData + add - 191;
	} else {
		if (add < -(dataptr - _soundData) || _soundData + _soundDataSize - dataptr <= add)
			dataptr = nullptr;
		else
			dataptr += add;
	}
	if (!dataptr)
		dataptr = channel.dataptrStack[--channel.dataptrStackPos];
	return 0;
}

int AdLibDriver::update_returnFromSubroutine(const uint8 *&dataptr, Channel &channel, uint8 value) {
	// Safety check: stop track when stack is empty.
	if (!channel.dataptrStackPos) {
		warning("AdLibDriver::update_returnFromSubroutine: Stack underflow");
		return update_stopChannel(dataptr, channel, 0);
	}
	dataptr = channel.dataptrStack[--channel.dataptrStackPos];
	return 0;
}

int AdLibDriver::update_setBaseOctave(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.baseOctave = value;
	return 0;
}

int AdLibDriver::update_stopChannel(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.priority = 0;
	if (_curChannel != 9)
		noteOff(channel);
	dataptr = nullptr;
	return 2;
}

int AdLibDriver::update_playRest(const uint8 *&dataptr, Channel &channel, uint8 value) {
	setupDuration(value, channel);
	noteOff(channel);
	return (value != 0);
}

int AdLibDriver::update_writeAdLib(const uint8 *&dataptr, Channel &channel, uint8 value) {
	writeOPL(value, *dataptr++);
	return 0;
}

int AdLibDriver::update_setupNoteAndDuration(const uint8 *&dataptr, Channel &channel, uint8 value) {
	setupNote(value, channel);
	value = *dataptr++;
	setupDuration(value, channel);
	return (value != 0);
}

int AdLibDriver::update_setBaseNote(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.baseNote = value;
	return 0;
}

int AdLibDriver::update_setupSecondaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.secondaryEffectTimer = value;
	channel.secondaryEffectTempo = value;
	channel.secondaryEffectSize = channel.secondaryEffectPos = *dataptr++;
	channel.secondaryEffectRegbase = *dataptr++;
	// WORKAROUND: The original code reads a true offset which later gets translated via xlat (in
	// the current segment). This means that the outcome depends on the sound data offset.
	// Unfortunately this offset is different in most implementations of the audio driver and
	// probably also different from the offset assumed by the sequencer.
	// It seems that the driver assumes an offset of 191 which is wrong for all the game driver
	// implementations.
	// This bug has probably not been noticed, since the effect is hardly used and the sounds are
	// not necessarily worse. I noticed the difference between ScummVM and DOSBox for the EOB II
	// teleporter sound. I also found the location of the table which is supposed to be used here
	// (simple enough: it is located at the end of the track after the 0x88 ending opcode).
	// Teleporters in EOB I and II now sound exactly the same which I am sure was the intended way,
	// since the sound data is exactly the same.
	// In DOSBox the teleporters will sound different in EOB I and II, due to different sound
	// data offsets.
	channel.secondaryEffectData = READ_LE_UINT16(dataptr) - 191;
	dataptr += 2;
	channel.secondaryEffect = &AdLibDriver::secondaryEffect1;

	// Safety check: don't enable effect when table location is invalid.
	int start = channel.secondaryEffectData + channel.secondaryEffectSize;
	if (start < 0 || start >= (int)_soundDataSize) {
		warning("AdLibDriver::update_setupSecondaryEffect1: Ignoring due to invalid table location");
		channel.secondaryEffect = nullptr;
	}
	return 0;
}

int AdLibDriver::update_stopOtherChannel(const uint8 *&dataptr, Channel &channel, uint8 value) {
	// Safety check
	if (value > 9) {
		warning("AdLibDriver::update_stopOtherChannel: Ignoring invalid channel %d", value);
		return 0;
	}

	Channel &channel2 = _channels[value];
	channel2.duration = 0;
	channel2.priority = 0;
	channel2.dataptr = nullptr;
	return 0;
}

int AdLibDriver::update_waitForEndOfProgram(const uint8 *&dataptr, Channel &channel, uint8 value) {
	const uint8 *ptr = getProgram(value);

	// Safety check in case an invalid program is specified. This would make
	// getProgram return a nullptr and thus cause invalid memory reads.
	if (!ptr) {
		debugC(3, kDebugLevelSound, "AdLibDriver::update_waitForEndOfProgram: Invalid program %d specified", value);
		return 0;
	}

	uint8 chan = *ptr;

	if (chan > 9 || !_channels[chan].dataptr)
		return 0;

	dataptr -= 2;
	return 2;
}

int AdLibDriver::update_setupInstrument(const uint8 *&dataptr, Channel &channel, uint8 value) {
	const uint8 *instrument = getInstrument(value);

	// We add a safety check to avoid setting up invalid instruments. This is
	// not done in the original. However, to avoid crashes due to invalid
	// memory reads we simply ignore the request.
	// This happens, for example, in Hand of Fate when using the swampsnake
	// potion on Zanthia to scare off the rat in the cave in the first chapter
	// of the game.
	if (!instrument) {
		debugC(3, kDebugLevelSound, "AdLibDriver::update_setupInstrument: Invalid instrument %d specified", value);
		return 0;
	}

	setupInstrument(_curRegOffset, instrument, channel);
	return 0;
}

int AdLibDriver::update_setupPrimaryEffectSlide(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.slideTempo = value;
	channel.slideStep = READ_BE_UINT16(dataptr);
	dataptr += 2;
	channel.primaryEffect = &AdLibDriver::primaryEffectSlide;
	channel.slideTimer = 0xFF;
	return 0;
}

int AdLibDriver::update_removePrimaryEffectSlide(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.primaryEffect = nullptr;
	channel.slideStep = 0;
	return 0;
}

int AdLibDriver::update_setBaseFreq(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.baseFreq = value;
	return 0;
}

int AdLibDriver::update_setupPrimaryEffectVibrato(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.vibratoTempo = value;
	channel.vibratoStepRange = *dataptr++;
	uint8 temp = *dataptr++;
	channel.vibratoStepsCountdown = temp + 1;
	channel.vibratoNumSteps = temp << 1;
	channel.vibratoDelay = *dataptr++;
	channel.primaryEffect = &AdLibDriver::primaryEffectVibrato;
	return 0;
}

int AdLibDriver::update_setPriority(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.priority = value;
	return 0;
}

// This provides a way to synchronize channels with a global beat:
//
// update_setBeat()
//    - Initializes _beatDivider, _beatDivCnt, _beatCounter, and _beatWaiting;
//      resets _callbackTimer
//    - _beatDivider is not further modified
//
// callback()
//    - _beatDivCnt is a countdown, gets reinitialized to _beatDivider on zero 
//    - _beatCounter is incremented when _beatDivCnt is reset, i.e., it's a
//      counter which updates with the global _tempo divided by _beatDivider.
//
// update_waitForNextBeat()
//    - _beatWaiting is updated if some bits are 0 in _beatCounter (off beat)
//    - the program is stopped until some of the masked bits in _beatCounter
//      become 1 and _beatWaiting is non-zero (on beat), then _beatWaiting is
//      cleared
//
// _beatDivider - determines how fast _beatCounter is incremented
// _beatDivCnt - countdown for the divider
// _beatCounter - counter updated with global _tempo divided by _beatDivider
// _beatWaiting - flags that waiting started before watched counter bit got 1
//
// Note that in theory _beatWaiting could wrap around to zero while waiting,
// then the rising edge wouldn't trigger. That's probably not a big issue
// in practice sice it can only happen for long delays (big _beatDivider and
// waiting on one of the higher bits) but could have been prevented easily.

int AdLibDriver::update_setBeat(const uint8 *&dataptr, Channel &channel, uint8 value) {
	value >>= 1;
	_beatDivider = _beatDivCnt = value;
	_callbackTimer = 0xFF;
	_beatCounter = _beatWaiting = 0;
	return 0;
}

int AdLibDriver::update_waitForNextBeat(const uint8 *&dataptr, Channel &channel, uint8 value) {
	if ((_beatCounter & value) && _beatWaiting) {
		_beatWaiting = 0;
		return 0;
	}

	if (!(_beatCounter & value))
		++_beatWaiting;

	dataptr -= 2;
	channel.duration = 1;
	return 2;
}

int AdLibDriver::update_setExtraLevel1(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.opExtraLevel1 = value;
	adjustVolume(channel);
	return 0;
}

int AdLibDriver::update_setupDuration(const uint8 *&dataptr, Channel &channel, uint8 value) {
	setupDuration(value, channel);
	return (value != 0);
}

int AdLibDriver::update_playNote(const uint8 *&dataptr, Channel &channel, uint8 value) {
	setupDuration(value, channel);
	noteOn(channel);
	return (value != 0);
}

int AdLibDriver::update_setFractionalNoteSpacing(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.fractionalSpacing = value & 7;
	return 0;
}

int AdLibDriver::update_setTempo(const uint8 *&dataptr, Channel &channel, uint8 value) {
	_tempo = value;
	return 0;
}

int AdLibDriver::update_removeSecondaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.secondaryEffect = nullptr;
	return 0;
}

int AdLibDriver::update_setChannelTempo(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.tempo = value;
	return 0;
}

int AdLibDriver::update_setExtraLevel3(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.opExtraLevel3 = value;
	return 0;
}

int AdLibDriver::update_setExtraLevel2(const uint8 *&dataptr, Channel &channel, uint8 value) {
	// Safety check
	if (value > 9) {
		warning("AdLibDriver::update_setExtraLevel2: Ignore invalid channel %d", value);
		dataptr++;
		return 0;
	}

	int channelBackUp = _curChannel;

	_curChannel = value;
	Channel &channel2 = _channels[value];
	channel2.opExtraLevel2 = *dataptr++;
	adjustVolume(channel2);

	_curChannel = channelBackUp;
	return 0;
}

int AdLibDriver::update_changeExtraLevel2(const uint8 *&dataptr, Channel &channel, uint8 value) {
	// Safety check
	if (value > 9) {
		warning("AdLibDriver::update_changeExtraLevel2: Ignore invalid channel %d", value);
		dataptr++;
		return 0;
	}

	int channelBackUp = _curChannel;

	_curChannel = value;
	Channel &channel2 = _channels[value];
	channel2.opExtraLevel2 += *dataptr++;
	adjustVolume(channel2);

	_curChannel = channelBackUp;
	return 0;
}

// Apart from initializing to zero, these two functions are the only ones that
// modify _vibratoAndAMDepthBits.

int AdLibDriver::update_setAMDepth(const uint8 *&dataptr, Channel &channel, uint8 value) {
	if (value & 1)
		_vibratoAndAMDepthBits |= 0x80;
	else
		_vibratoAndAMDepthBits &= 0x7F;

	writeOPL(0xBD, _vibratoAndAMDepthBits);
	return 0;
}

int AdLibDriver::update_setVibratoDepth(const uint8 *&dataptr, Channel &channel, uint8 value) {
	if (value & 1)
		_vibratoAndAMDepthBits |= 0x40;
	else
		_vibratoAndAMDepthBits &= 0xBF;

	writeOPL(0xBD, _vibratoAndAMDepthBits);
	return 0;
}

int AdLibDriver::update_changeExtraLevel1(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.opExtraLevel1 += value;
	adjustVolume(channel);
	return 0;
}

int AdLibDriver::update_clearChannel(const uint8 *&dataptr, Channel &channel, uint8 value) {
	// Safety check
	if (value > 9) {
		warning("AdLibDriver::update_clearChannel: Ignore invalid channel %d", value);
		return 0;
	}

	int channelBackUp = _curChannel;
	_curChannel = value;

	// Stop channel
	Channel &channel2 = _channels[value];
	channel2.duration = channel2.priority = 0;
	channel2.dataptr = 0;
	channel2.opExtraLevel2 = 0;

	if (value != 9) {
		// Silence channel
		uint8 regOff = _regOffset[value];

		// Feedback strength / Connection type
		writeOPL(0xC0 + _curChannel, 0x00);

		// Key scaling level / Operator output level
		writeOPL(0x43 + regOff, 0x3F);

		// Sustain Level / Release Rate
		writeOPL(0x83 + regOff, 0xFF);

		// Key On / Octave / Frequency
		writeOPL(0xB0 + _curChannel, 0x00);
	}

	_curChannel = channelBackUp;
	return 0;
}

int AdLibDriver::update_changeNoteRandomly(const uint8 *&dataptr, Channel &channel, uint8 value) {
	if (_curChannel >= 9)
		return 0;

	uint16 mask = READ_BE_UINT16(++dataptr - 2);

	uint16 note = ((channel.regBx & 0x1F) << 8) | channel.regAx;

	note += mask & getRandomNr();
	note |= ((channel.regBx & 0x20) << 8);

	// Frequency
	writeOPL(0xA0 + _curChannel, note & 0xFF);

	// Key On / Octave / Frequency
	writeOPL(0xB0 + _curChannel, (note & 0xFF00) >> 8);

	return 0;
}

int AdLibDriver::update_removePrimaryEffectVibrato(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.primaryEffect = nullptr;
	return 0;
}

int AdLibDriver::update_pitchBend(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.pitchBend = (int8)value;
	setupNote(channel.rawNote, channel, true);
	return 0;
}

int AdLibDriver::update_resetToGlobalTempo(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.tempo = _tempo;
	return 0;
}

int AdLibDriver::update_nop(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	return 0;
}

int AdLibDriver::update_setDurationRandomness(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.durationRandomness = value;
	return 0;
}

int AdLibDriver::update_changeChannelTempo(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.tempo = CLIP(channel.tempo + (int8)value, 1, 255);
	return 0;
}

int AdLibDriver::updateCallback46(const uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 entry = *dataptr++;

	// Safety check: prevent illegal table access
	if (entry + 2 > _unkTable2Size)
		return 0;

	_tablePtr1 = _unkTable2[entry++];
	_tablePtr2 = _unkTable2[entry];
	if (value == 2) {
		// Frequency
		writeOPL(0xA0, _tablePtr2[0]);
	}
	return 0;
}

int AdLibDriver::update_setupRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value) {
	int channelBackUp = _curChannel;
	int regOffsetBackUp = _curRegOffset;

	_curChannel = 6;
	_curRegOffset = _regOffset[6];

	const uint8 *instrument;
	instrument = getInstrument(value);
	if (instrument) {
		setupInstrument(_curRegOffset, instrument, channel);
	} else {
		debugC(3, kDebugLevelSound, "AdLibDriver::update_setupRhythmSection: Invalid instrument %d for channel 6 specified", value);
	}
	_opLevelBD = channel.opLevel2;

	_curChannel = 7;
	_curRegOffset = _regOffset[7];

	instrument = getInstrument(value = *dataptr++);
	if (instrument) {
		setupInstrument(_curRegOffset, instrument, channel);
	} else {
		debugC(3, kDebugLevelSound, "AdLibDriver::update_setupRhythmSection: Invalid instrument %d for channel 7 specified", value);
	}
	_opLevelHH = channel.opLevel1;
	_opLevelSD = channel.opLevel2;

	_curChannel = 8;
	_curRegOffset = _regOffset[8];

	instrument = getInstrument(value = *dataptr++);
	if (instrument) {
		setupInstrument(_curRegOffset, instrument, channel);
	} else {
		debugC(3, kDebugLevelSound, "AdLibDriver::update_setupRhythmSection: Invalid instrument %d for channel 8 specified", value);
	}
	_opLevelTT = channel.opLevel1;
	_opLevelCY = channel.opLevel2;

	// Octave / F-Number / Key-On for channels 6, 7 and 8

	_channels[6].regBx = *dataptr++ & 0x2F;
	writeOPL(0xB6, _channels[6].regBx);
	writeOPL(0xA6, *dataptr++);

	_channels[7].regBx = *dataptr++ & 0x2F;
	writeOPL(0xB7, _channels[7].regBx);
	writeOPL(0xA7, *dataptr++);

	_channels[8].regBx = *dataptr++ & 0x2F;
	writeOPL(0xB8, _channels[8].regBx);
	writeOPL(0xA8, *dataptr++);

	_rhythmSectionBits = 0x20;

	_curRegOffset = regOffsetBackUp;
	_curChannel = channelBackUp;
	return 0;
}

int AdLibDriver::update_playRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value) {
	// Any instrument that we want to play, and which was already playing,
	// is temporarily keyed off. Instruments that were off already, or
	// which we don't want to play, retain their old on/off status. This is
	// probably so that the instrument's envelope is played from its
	// beginning again...

	writeOPL(0xBD, (_rhythmSectionBits & ~(value & 0x1F)) | 0x20);

	// ...but since we only set the rhythm instrument bits, and never clear
	// them (until the entire rhythm section is disabled), I'm not sure how
	// useful the cleverness above is. We could perhaps simply turn off all
	// the rhythm instruments instead.

	_rhythmSectionBits |= value;

	writeOPL(0xBD, _vibratoAndAMDepthBits | 0x20 | _rhythmSectionBits);
	return 0;
}

int AdLibDriver::update_removeRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	_rhythmSectionBits = 0;

	// All the rhythm bits are cleared. The AM and Vibrato depth bits
	// remain unchanged.

	writeOPL(0xBD, _vibratoAndAMDepthBits);
	return 0;
}

int AdLibDriver::update_setRhythmLevel2(const uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 value2 = *dataptr++;

	if (value & 1) {
		_opExtraLevel2HH = value2;

		// Channel 7, op1: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(value2 + _opLevelHH + _opExtraLevel1HH + _opExtraLevel2HH));
	}

	if (value & 2) {
		_opExtraLevel2CY = value2;

		// Channel 8, op2: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(value2 + _opLevelCY + _opExtraLevel1CY + _opExtraLevel2CY));
	}

	if (value & 4) {
		_opExtraLevel2TT = value2;

		// Channel 8, op1: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(value2 + _opLevelTT + _opExtraLevel1TT + _opExtraLevel2TT));
	}

	if (value & 8) {
		_opExtraLevel2SD = value2;

		// Channel 7, op2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(value2 + _opLevelSD + _opExtraLevel1SD + _opExtraLevel2SD));
	}

	if (value & 16) {
		_opExtraLevel2BD = value2;

		// Channel 6, op2: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(value2 + _opLevelBD + _opExtraLevel1BD + _opExtraLevel2BD));
	}

	return 0;
}

int AdLibDriver::update_changeRhythmLevel1(const uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 value2 = *dataptr++;

	if (value & 1) {
		_opExtraLevel1HH = checkValue(value2 + _opLevelHH + _opExtraLevel1HH + _opExtraLevel2HH);

		// Channel 7, op1: Level Key Scaling / Total Level
		writeOPL(0x51, _opExtraLevel1HH);
	}

	if (value & 2) {
		_opExtraLevel1CY = checkValue(value2 + _opLevelCY + _opExtraLevel1CY + _opExtraLevel2CY);

		// Channel 8, op2: Level Key Scaling / Total Level
		writeOPL(0x55, _opExtraLevel1CY);
	}

	if (value & 4) {
		_opExtraLevel1TT = checkValue(value2 + _opLevelTT + _opExtraLevel1TT + _opExtraLevel2TT);

		// Channel 8, op1: Level Key Scaling / Total Level
		writeOPL(0x52, _opExtraLevel1TT);
	}

	if (value & 8) {
		_opExtraLevel1SD = checkValue(value2 + _opLevelSD + _opExtraLevel1SD + _opExtraLevel2SD);

		// Channel 7, op2: Level Key Scaling / Total Level
		writeOPL(0x54, _opExtraLevel1SD);
	}

	if (value & 16) {
		_opExtraLevel1BD = checkValue(value2 + _opLevelBD + _opExtraLevel1BD + _opExtraLevel2BD);

		// Channel 6, op2: Level Key Scaling / Total Level
		writeOPL(0x53, _opExtraLevel1BD);
	}

	return 0;
}

int AdLibDriver::update_setRhythmLevel1(const uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 value2 = *dataptr++;

	if (value & 1) {
		_opExtraLevel1HH = value2;

		// Channel 7, op1: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(value2 + _opLevelHH + _opExtraLevel2HH));
	}

	if (value & 2) {
		_opExtraLevel1CY = value2;

		// Channel 8, op2: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(value2 + _opLevelCY + _opExtraLevel2CY));
	}

	if (value & 4) {
		_opExtraLevel1TT = value2;

		// Channel 8, op1: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(value2 + _opLevelTT + _opExtraLevel2TT));
	}

	if (value & 8) {
		_opExtraLevel1SD = value2;

		// Channel 7, op2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(value2 + _opLevelSD + _opExtraLevel2SD));
	}

	if (value & 16) {
		_opExtraLevel1BD = value2;

		// Channel 6, op2: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(value2 + _opLevelBD + _opExtraLevel2BD));
	}

	return 0;
}

int AdLibDriver::update_setSoundTrigger(const uint8 *&dataptr, Channel &channel, uint8 value) {
	_soundTrigger = value;
	return 0;
}

int AdLibDriver::update_setTempoReset(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.tempoReset = value;
	return 0;
}

int AdLibDriver::updateCallback56(const uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.unk39 = value;
	channel.unk40 = *dataptr++;
	return 0;
}

// static res

#define COMMAND(x, n) { &AdLibDriver::x, #x, n }

const AdLibDriver::ParserOpcode AdLibDriver::_parserOpcodeTable[] = {
	// 0
	COMMAND(update_setRepeat, 1),
	COMMAND(update_checkRepeat, 2),
	COMMAND(update_setupProgram, 1),
	COMMAND(update_setNoteSpacing, 1),

	// 4
	COMMAND(update_jump, 2),
	COMMAND(update_jumpToSubroutine, 2),
	COMMAND(update_returnFromSubroutine, 0),
	COMMAND(update_setBaseOctave, 1),

	// 8
	COMMAND(update_stopChannel, 0),
	COMMAND(update_playRest, 1),
	COMMAND(update_writeAdLib, 2),
	COMMAND(update_setupNoteAndDuration, 2),

	// 12
	COMMAND(update_setBaseNote, 1),
	COMMAND(update_setupSecondaryEffect1, 5),
	COMMAND(update_stopOtherChannel, 1),
	COMMAND(update_waitForEndOfProgram, 1),

	// 16
	COMMAND(update_setupInstrument, 1),
	COMMAND(update_setupPrimaryEffectSlide, 3),
	COMMAND(update_removePrimaryEffectSlide, 0),
	COMMAND(update_setBaseFreq, 1),

	// 20
	COMMAND(update_stopChannel, 0),
	COMMAND(update_setupPrimaryEffectVibrato, 4),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_stopChannel, 0),

	// 24
	COMMAND(update_stopChannel, 0),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_setPriority, 1),
	COMMAND(update_stopChannel, 0),

	// 28
	COMMAND(update_setBeat, 1),
	COMMAND(update_waitForNextBeat, 1),
	COMMAND(update_setExtraLevel1, 1),
	COMMAND(update_stopChannel, 0),

	// 32
	COMMAND(update_setupDuration, 1),
	COMMAND(update_playNote, 1),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_stopChannel, 0),

	// 36
	COMMAND(update_setFractionalNoteSpacing, 1),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_setTempo, 1),
	COMMAND(update_removeSecondaryEffect1, 0),

	// 40
	COMMAND(update_stopChannel, 0),
	COMMAND(update_setChannelTempo, 1),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_setExtraLevel3, 1),

	// 44
	COMMAND(update_setExtraLevel2, 2),
	COMMAND(update_changeExtraLevel2, 2),
	COMMAND(update_setAMDepth, 1),
	COMMAND(update_setVibratoDepth, 1),

	// 48
	COMMAND(update_changeExtraLevel1, 1),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_stopChannel, 0),
	COMMAND(update_clearChannel, 1),

	// 52
	COMMAND(update_stopChannel, 0),
	COMMAND(update_changeNoteRandomly, 2),
	COMMAND(update_removePrimaryEffectVibrato, 0),
	COMMAND(update_stopChannel, 0),

	// 56
	COMMAND(update_stopChannel, 0),
	COMMAND(update_pitchBend, 1),
	COMMAND(update_resetToGlobalTempo, 0),
	COMMAND(update_nop, 0),

	// 60
	COMMAND(update_setDurationRandomness, 1),
	COMMAND(update_changeChannelTempo, 1),
	COMMAND(update_stopChannel, 0),
	COMMAND(updateCallback46, 2),

	// 64
	COMMAND(update_nop, 0),
	COMMAND(update_setupRhythmSection, 9),
	COMMAND(update_playRhythmSection, 1),
	COMMAND(update_removeRhythmSection, 0),

	// 68
	COMMAND(update_setRhythmLevel2, 2),
	COMMAND(update_changeRhythmLevel1, 2),
	COMMAND(update_setRhythmLevel1, 2),
	COMMAND(update_setSoundTrigger, 1),

	// 72
	COMMAND(update_setTempoReset, 1),
	COMMAND(updateCallback56, 2),
	COMMAND(update_stopChannel, 0)
};

#undef COMMAND

const int AdLibDriver::_parserOpcodeTableSize = ARRAYSIZE(AdLibDriver::_parserOpcodeTable);

// This table holds the register offset for operator 1 for each of the nine
// channels. To get the register offset for operator 2, simply add 3.

const uint8 AdLibDriver::_regOffset[] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11,
	0x12
};

// These are the F-Numbers (10 bits) for the notes of the 12-tone scale.
// However, it does not match the table in the AdLib documentation I've seen.

const uint16 AdLibDriver::_freqTable[] = {
	0x0134, 0x0147, 0x015A, 0x016F, 0x0184, 0x019C, 0x01B4, 0x01CE, 0x01E9,
	0x0207, 0x0225, 0x0246
};

// These tables are currently only used by updateCallback46(), which only ever
// uses the first element of one of the sub-tables.

const uint8 *const AdLibDriver::_unkTable2[] = {
	AdLibDriver::_unkTable2_1,
	AdLibDriver::_unkTable2_2,
	AdLibDriver::_unkTable2_1,
	AdLibDriver::_unkTable2_2,
	AdLibDriver::_unkTable2_3,
	AdLibDriver::_unkTable2_2
};

const int AdLibDriver::_unkTable2Size = ARRAYSIZE(AdLibDriver::_unkTable2);

const uint8 AdLibDriver::_unkTable2_1[] = {
	0x50, 0x50, 0x4F, 0x4F, 0x4E, 0x4E, 0x4D, 0x4D,
	0x4C, 0x4C, 0x4B, 0x4B, 0x4A, 0x4A, 0x49, 0x49,
	0x48, 0x48, 0x47, 0x47, 0x46, 0x46, 0x45, 0x45,
	0x44, 0x44, 0x43, 0x43, 0x42, 0x42, 0x41, 0x41,
	0x40, 0x40, 0x3F, 0x3F, 0x3E, 0x3E, 0x3D, 0x3D,
	0x3C, 0x3C, 0x3B, 0x3B, 0x3A, 0x3A, 0x39, 0x39,
	0x38, 0x38, 0x37, 0x37, 0x36, 0x36, 0x35, 0x35,
	0x34, 0x34, 0x33, 0x33, 0x32, 0x32, 0x31, 0x31,
	0x30, 0x30, 0x2F, 0x2F, 0x2E, 0x2E, 0x2D, 0x2D,
	0x2C, 0x2C, 0x2B, 0x2B, 0x2A, 0x2A, 0x29, 0x29,
	0x28, 0x28, 0x27, 0x27, 0x26, 0x26, 0x25, 0x25,
	0x24, 0x24, 0x23, 0x23, 0x22, 0x22, 0x21, 0x21,
	0x20, 0x20, 0x1F, 0x1F, 0x1E, 0x1E, 0x1D, 0x1D,
	0x1C, 0x1C, 0x1B, 0x1B, 0x1A, 0x1A, 0x19, 0x19,
	0x18, 0x18, 0x17, 0x17, 0x16, 0x16, 0x15, 0x15,
	0x14, 0x14, 0x13, 0x13, 0x12, 0x12, 0x11, 0x11,
	0x10, 0x10
};

// no don't ask me WHY this table exsits!
const uint8 AdLibDriver::_unkTable2_2[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x6F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F
};

const uint8 AdLibDriver::_unkTable2_3[] = {
	0x40, 0x40, 0x40, 0x3F, 0x3F, 0x3F, 0x3E, 0x3E,
	0x3E, 0x3D, 0x3D, 0x3D, 0x3C, 0x3C, 0x3C, 0x3B,
	0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x39, 0x39, 0x39,
	0x38, 0x38, 0x38, 0x37, 0x37, 0x37, 0x36, 0x36,
	0x36, 0x35, 0x35, 0x35, 0x34, 0x34, 0x34, 0x33,
	0x33, 0x33, 0x32, 0x32, 0x32, 0x31, 0x31, 0x31,
	0x30, 0x30, 0x30, 0x2F, 0x2F, 0x2F, 0x2E, 0x2E,
	0x2E, 0x2D, 0x2D, 0x2D, 0x2C, 0x2C, 0x2C, 0x2B,
	0x2B, 0x2B, 0x2A, 0x2A, 0x2A, 0x29, 0x29, 0x29,
	0x28, 0x28, 0x28, 0x27, 0x27, 0x27, 0x26, 0x26,
	0x26, 0x25, 0x25, 0x25, 0x24, 0x24, 0x24, 0x23,
	0x23, 0x23, 0x22, 0x22, 0x22, 0x21, 0x21, 0x21,
	0x20, 0x20, 0x20, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E,
	0x1E, 0x1D, 0x1D, 0x1D, 0x1C, 0x1C, 0x1C, 0x1B,
	0x1B, 0x1B, 0x1A, 0x1A, 0x1A, 0x19, 0x19, 0x19,
	0x18, 0x18, 0x18, 0x17, 0x17, 0x17, 0x16, 0x16,
	0x16, 0x15
};

// This table is used to modify the frequency of the notes, depending on the
// note value and the pitch bend value. In theory, we could very well try to
// access memory outside this table, but in reality that probably won't happen.
//

const uint8 AdLibDriver::_pitchBendTables[][32] = {
	// 0
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
	  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
	  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x19,
	  0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21 },
	// 1
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x09,
	  0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
	  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1A,
	  0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24 },
	// 2
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x09,
	  0x0A, 0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13,
	  0x14, 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D,
	  0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26 },
	// 3
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A,
	  0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13,
	  0x14, 0x15, 0x16, 0x17, 0x18, 0x1A, 0x1C, 0x1D,
	  0x1E, 0x1F, 0x20, 0x21, 0x23, 0x25, 0x27, 0x28 },
	// 4
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A,
	  0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x13, 0x15,
	  0x16, 0x17, 0x18, 0x19, 0x1B, 0x1D, 0x1F, 0x20,
	  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x2A },
	// 5
	{ 0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B,
	  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, 0x15,
	  0x16, 0x17, 0x18, 0x19, 0x1B, 0x1D, 0x1F, 0x20,
	  0x21, 0x22, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D },
	// 6
	{ 0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B,
	  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, 0x15,
	  0x16, 0x17, 0x18, 0x1A, 0x1C, 0x1E, 0x21, 0x24,
	  0x25, 0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x30 },
	// 7
	{ 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C,
	  0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, 0x15, 0x18,
	  0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x25,
	  0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x30, 0x32 },
	// 8
	{ 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0D,
	  0x0E, 0x0F, 0x10, 0x11, 0x12, 0x14, 0x17, 0x1A,
	  0x19, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x25, 0x28,
	  0x29, 0x2A, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0x35 },
	// 9
	{ 0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0E,
	  0x0F, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1B,
	  0x1C, 0x1D, 0x1E, 0x20, 0x22, 0x24, 0x26, 0x29,
	  0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x39 },
	// 10
	{ 0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0E,
	  0x0F, 0x10, 0x12, 0x14, 0x16, 0x19, 0x1B, 0x1E,
	  0x1F, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D,
	  0x2E, 0x2F, 0x31, 0x32, 0x34, 0x36, 0x39, 0x3C },
	// 11
	{ 0x00, 0x01, 0x03, 0x05, 0x07, 0x0A, 0x0C, 0x0F,
	  0x10, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1E,
	  0x1F, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2B, 0x2E,
	  0x2F, 0x30, 0x32, 0x34, 0x36, 0x39, 0x3C, 0x3F },
	// 12
	{ 0x00, 0x02, 0x04, 0x06, 0x08, 0x0B, 0x0D, 0x10,
	  0x11, 0x12, 0x14, 0x16, 0x18, 0x1B, 0x1E, 0x21,
	  0x22, 0x23, 0x25, 0x27, 0x29, 0x2C, 0x2F, 0x32,
	  0x33, 0x34, 0x36, 0x38, 0x3B, 0x34, 0x41, 0x44 },
	// 13
	{ 0x00, 0x02, 0x04, 0x06, 0x08, 0x0B, 0x0D, 0x11,
	  0x12, 0x13, 0x15, 0x17, 0x1A, 0x1D, 0x20, 0x23,
	  0x24, 0x25, 0x27, 0x29, 0x2C, 0x2F, 0x32, 0x35,
	  0x36, 0x37, 0x39, 0x3B, 0x3E, 0x41, 0x44, 0x47 }
};

PCSoundDriver *PCSoundDriver::createAdLib(Audio::Mixer *mixer, int version) {
	return new AdLibDriver(mixer, version);
}

} // End of namespace Kyra

#undef CALLBACKS_PER_SECOND
