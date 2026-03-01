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

/**
 * Wally Beben custom music engine player.
 *
 * Used in the Amiga version of Dark Side (Incentive Software, 1988).
 * HDSMUSIC.AM is an embedded GEMDOS executable containing a 4-channel
 * music engine with its own byte-stream pattern format, ~57KB of PCM
 * samples, and ADSR volume envelopes.
 *
 * Assembly reference addresses (TEXT-relative, HDSMUSIC.AM):
 *   $0004  Tick entry point (called at 50 Hz from VBI)
 *   $0012  Command mailbox: 0=stop, 1/2=play song, $FF=playing
 *   $010C  Main sequencer body (readPatternCommands equivalent)
 *   $0268  Pattern command dispatcher ($E0=inst, $C0=env, $80=dur, etc.)
 *   $030C  Note-on handler (triggerNote equivalent)
 *   $068C  Envelope processing (processEnvelope equivalent)
 *   $0AAE  Period table (48 x uint16 BE)
 *   $0C42  Sample pointer table (16 x uint32 BE, TEXT-relative)
 *   $0C82  Instrument table (16 x 8 bytes)
 *   $0D02  Arpeggio interval lookup (8 bytes)
 *   $0D0A  Envelope table (10 x 8 bytes)
 *   $0DBA  Song table (2 songs x 4 channel pointers)
 *   $0DCA  Pattern pointer table (up to ~71 x uint32 BE)
 *   $14B3  PCM sample data start (~57KB through $F4BB)
 *
 */

#include "freescape/wb.h"
#include "freescape/freescape.h"
#include "audio/mods/paula.h"
#include "common/endian.h"
#include "common/debug.h"
#include "common/util.h"

namespace Freescape {

namespace WBCommon {

int8 decodeOrderTranspose(byte cmd) {
	return (int8)((cmd + 0x20) & 0xFF);
}

byte decodeTickSpeed(byte cmd) {
	byte speed = cmd & 0x0F;
	return speed == 0 ? 1 : speed;
}

byte decodeDuration(byte cmd) {
	byte duration = cmd & 0x3F;
	return duration == 0 ? 1 : duration;
}

byte buildArpeggioTable(const byte intervals[8], byte mask, byte *outTable, byte maxLen, bool includeBase) {
	byte len = 0;
	if (includeBase && maxLen > 0)
		outTable[len++] = 0;

	for (int i = 0; i < 8 && len < maxLen; i++) {
		if (mask & (1 << i))
			outTable[len++] = intervals[i];
	}

	if (includeBase && len <= 1)
		return 0;
	return len;
}

} // End of namespace WBCommon

// TEXT-relative offsets for data tables within HDSMUSIC.AM
// All addresses verified against disassembly of the 68000 code.
static const uint32 kPeriodTableOffset    = 0x0AAE; // 48 x uint16 BE (note 0=silence, 1-47=C-1..B-3)
static const uint32 kSamplePtrTableOffset = 0x0C42; // 16 x uint32 BE (TEXT-relative PCM offsets)
static const uint32 kInstrumentTableOffset = 0x0C82; // 16 x 8 bytes (sample#, loopFlag, len, loopOff, loopLen)
static const uint32 kArpeggioIntervalsOffset = 0x0D02; // 8 bytes (semitone offsets for arpeggio bitmask)
static const uint32 kEnvelopeTableOffset  = 0x0D0A; // 10 x 8 bytes (atk, dec, fadeRate, rel, mod, vib, arp, flags)
static const uint32 kSongTableOffset      = 0x0DBA; // 2 songs x 4 channels x uint32 BE order-list pointers
static const uint32 kPatternPtrTableOffset = 0x0DCA; // up to 128 x uint32 BE (overlaps Song 2 order ptrs)
static const uint32 kMaxPatternEntries    = 128;

class WallyBebenStream : public Audio::Paula {
public:
	WallyBebenStream(const byte *data, uint32 dataSize, int songNum, int rate, bool stereo);
	~WallyBebenStream() override;

private:
	void interrupt() override;

	// --- Data tables (parsed from HDSMUSIC.AM TEXT segment) ---

	const byte *_data;
	uint32 _dataSize;

	uint16 _periods[48];

	struct InstrumentDesc {
		byte sampleIndex;
		byte loopFlag;
		uint16 totalLength;
		uint16 loopOffset;
		uint16 loopLength;
	};
	InstrumentDesc _instruments[16];

	struct EnvelopeDesc {
		byte attackLevel;
		byte decayTarget;
		byte sustainLevel;
		byte releaseRate;
		byte modDepth;
		byte vibratoWave;
		byte arpeggioMask;
		byte flags;
	};
	EnvelopeDesc _envelopes[10];

	// Sample offsets within the data buffer
	uint32 _sampleOffsets[16];

	// Song order list pointers (TEXT-relative)
	uint32 _songOrderPtrs[2][4];

	// Pattern pointer table (TEXT+$DCA, up to 128 entries)
	uint32 _patternPtrs[kMaxPatternEntries];
	uint32 _numPatterns; // actual number of valid entries loaded
	uint32 _firstSampleOffset; // lowest sample data offset — upper bound for pattern pointers

	// Arpeggio interval lookup
	byte _arpeggioIntervals[8];

	// --- Per-channel state ---

	struct ChannelState {
		// Order list
		uint32 orderListOffset; // TEXT-relative offset to order list
		int orderListPos;       // Current byte position in order list
		int8 transpose;         // Semitone transpose from order list

		// Current pattern
		uint32 patternOffset;   // TEXT-relative offset to current pattern
		int patternPos;         // Current byte position in pattern

		// Note state
		byte note;              // Current note (0=rest, 1-47)
		byte prevNote;          // Previous note (for portamento)
		byte duration;          // Note duration in sequencer steps
		int durationCounter;    // Remaining steps for current note

		// Instrument / envelope selection
		byte instrumentIdx;     // 0-15
		byte envelopeIdx;       // 0-9

		// Volume envelope
		byte volume;            // Current output volume (0-64)
		byte attackLevel;
		byte decayTarget;
		byte sustainLevel;
		byte releaseRate;
		byte envelopePhase;     // 0=attack, 1=decay, 2=sustain, 3=release
		byte modDepth;

		// Effects
		byte effectMode;        // 0=none, 1=porta/arpeggio, 2=envelope vibrato
		bool portaUp;
		bool portaDown;
		int16 portaStep;
		int16 portaTarget;
		byte arpeggioMask;
		byte arpeggioPos;
		byte arpeggioTable[16];
		byte arpeggioTableLen;
		byte effect7BBaseNote;
		byte effect7BRate;
		byte effect7BCounter;
		bool effect7BUseBase;
		bool effect7BActive;
		byte vibratoPos;

		// Period
		int16 basePeriod;       // From note lookup
		int16 outputPeriod;     // After effects

		// Delay
		byte delay;
		byte delayCounter;
		bool pendingNoteOn;

		bool active;
	};
	ChannelState _channels[4];

	// --- Global state ---

	bool _musicActive;
	byte _tickSpeed;    // Ticks between sequencer steps
	byte _tickCounter;  // Current tick count (counts up to _tickSpeed)
	int _pendingSongCommand; // -1=no pending, else mailbox command from $DD

	// --- Methods ---

	void loadTables();
	void startSong(int songNum);
	void initChannel(int ch);
	void readOrderList(int ch);
	void readPatternCommands(int ch);
	void triggerNote(int ch);
	void processEffects(int ch);
	void processEnvelope(int ch);
	void buildArpeggioTable(int ch, byte mask);
	uint16 getPeriod(int note) const;

	byte readDataByte(uint32 offset) const {
		if (offset < _dataSize)
			return _data[offset];
		return 0;
	}

	uint16 readDataWord(uint32 offset) const {
		if (offset + 1 < _dataSize)
			return READ_BE_UINT16(_data + offset);
		return 0;
	}

	uint32 readDataLong(uint32 offset) const {
		if (offset + 3 < _dataSize)
			return READ_BE_UINT32(_data + offset);
		return 0;
	}
};

// ---------------------------------------------------------------------------
// Construction / data loading
// ---------------------------------------------------------------------------

WallyBebenStream::WallyBebenStream(const byte *data, uint32 dataSize,
                                   int songNum, int rate, bool stereo)
	: Paula(stereo, rate, rate / 50), // 50 Hz PAL VBI interrupt rate
	  _data(data), _dataSize(dataSize),
	  _musicActive(false), _tickSpeed(6), _tickCounter(0),
	  _pendingSongCommand(-1), _numPatterns(0), _firstSampleOffset(0) {

	memset(_periods, 0, sizeof(_periods));
	memset(_instruments, 0, sizeof(_instruments));
	memset(_envelopes, 0, sizeof(_envelopes));
	memset(_sampleOffsets, 0, sizeof(_sampleOffsets));
	memset(_songOrderPtrs, 0, sizeof(_songOrderPtrs));
	memset(_patternPtrs, 0, sizeof(_patternPtrs));
	memset(_arpeggioIntervals, 0, sizeof(_arpeggioIntervals));
	memset(_channels, 0, sizeof(_channels));

	// Standard Amiga panning: channels 0,3 left — channels 1,2 right
	setChannelPanning(0, PANNING_LEFT);
	setChannelPanning(1, PANNING_RIGHT);
	setChannelPanning(2, PANNING_RIGHT);
	setChannelPanning(3, PANNING_LEFT);

	loadTables();
	startSong(songNum);
}

WallyBebenStream::~WallyBebenStream() {
}

void WallyBebenStream::loadTables() {
	// Period table: 48 x uint16 BE at TEXT+$AAE
	for (int i = 0; i < 48; i++) {
		_periods[i] = readDataWord(kPeriodTableOffset + i * 2);
	}

	// Sample pointer table: 16 x uint32 BE at TEXT+$C42
	// These are TEXT-relative offsets to PCM sample data
	for (int i = 0; i < 16; i++) {
		_sampleOffsets[i] = readDataLong(kSamplePtrTableOffset + i * 4);
	}

	// Instrument table: 16 x 8 bytes at TEXT+$C82
	for (int i = 0; i < 16; i++) {
		uint32 off = kInstrumentTableOffset + i * 8;
		_instruments[i].sampleIndex  = readDataByte(off + 0);
		_instruments[i].loopFlag     = readDataByte(off + 1);
		_instruments[i].totalLength  = readDataWord(off + 2);
		_instruments[i].loopOffset   = readDataWord(off + 4);
		_instruments[i].loopLength   = readDataWord(off + 6);
	}

	// Arpeggio interval table: 8 bytes at TEXT+$D02
	for (int i = 0; i < 8; i++) {
		_arpeggioIntervals[i] = readDataByte(kArpeggioIntervalsOffset + i);
	}

	// Envelope table: 10 x 8 bytes at TEXT+$D0A
	for (int i = 0; i < 10; i++) {
		uint32 off = kEnvelopeTableOffset + i * 8;
		_envelopes[i].attackLevel  = readDataByte(off + 0);
		_envelopes[i].decayTarget  = readDataByte(off + 1);
		_envelopes[i].sustainLevel = readDataByte(off + 2);
		_envelopes[i].releaseRate  = readDataByte(off + 3);
		_envelopes[i].modDepth     = readDataByte(off + 4);
		_envelopes[i].vibratoWave  = readDataByte(off + 5);
		_envelopes[i].arpeggioMask = readDataByte(off + 6);
		_envelopes[i].flags        = readDataByte(off + 7);
	}

	// Song table: 2 songs x 4 channels x uint32 BE at TEXT+$DBA
	for (int s = 0; s < 2; s++) {
		for (int ch = 0; ch < 4; ch++) {
			_songOrderPtrs[s][ch] = readDataLong(kSongTableOffset + s * 16 + ch * 4);
		}
	}

	// Compute the first sample offset — this is the upper bound for valid
	// pattern/order-list data. Anything at or above this is PCM sample data.
	_firstSampleOffset = _dataSize;
	for (int i = 0; i < 16; i++) {
		if (_sampleOffsets[i] > 0 && _sampleOffsets[i] < _firstSampleOffset)
			_firstSampleOffset = _sampleOffsets[i];
	}

	// Pattern pointer table at TEXT+$DCA.
	// Valid pattern pointers must be > 0 and < _firstSampleOffset (i.e., they
	// point into the song data area between the tables and the PCM samples).
	// Entries at or beyond _firstSampleOffset are stale data, not real patterns.
	_numPatterns = 0;
	for (uint32 i = 0; i < kMaxPatternEntries; i++) {
		uint32 ptr = readDataLong(kPatternPtrTableOffset + i * 4);
		_patternPtrs[i] = ptr;
		if (ptr > 0 && ptr < _firstSampleOffset)
			_numPatterns = i + 1;
	}

	// Debug: dump loaded data tables for verification
	debug(3, "WB: Data loaded from HDSMUSIC.AM TEXT segment (%u bytes)", _dataSize);

	debug(3, "WB: Period table (first 12): %d %d %d %d %d %d %d %d %d %d %d %d",
		_periods[0], _periods[1], _periods[2], _periods[3],
		_periods[4], _periods[5], _periods[6], _periods[7],
		_periods[8], _periods[9], _periods[10], _periods[11]);

	for (int i = 0; i < 16; i++) {
		if (_sampleOffsets[i])
			debug(3, "WB: Sample %d: offset=$%X", i, _sampleOffsets[i]);
	}

	for (int i = 0; i < 16; i++) {
		const InstrumentDesc &inst = _instruments[i];
		if (inst.totalLength > 0)
			debug(3, "WB: Inst %d: sample=%d loop=%d len=%d loopOff=%d loopLen=%d",
				i, inst.sampleIndex, inst.loopFlag, inst.totalLength,
				inst.loopOffset, inst.loopLength);
	}

	for (int i = 0; i < 10; i++) {
		const EnvelopeDesc &env = _envelopes[i];
		debug(3, "WB: Env %d: atk=%d dec=%d sus=%d rel=%d mod=%d arp=$%02X",
			i, env.attackLevel, env.decayTarget, env.sustainLevel,
			env.releaseRate, env.modDepth, env.arpeggioMask);
	}

	debug(3, "WB: Song 1 order ptrs: $%X $%X $%X $%X",
		_songOrderPtrs[0][0], _songOrderPtrs[0][1],
		_songOrderPtrs[0][2], _songOrderPtrs[0][3]);
	debug(3, "WB: Song 2 order ptrs: $%X $%X $%X $%X",
		_songOrderPtrs[1][0], _songOrderPtrs[1][1],
		_songOrderPtrs[1][2], _songOrderPtrs[1][3]);

	debug(3, "WB: %d valid patterns (firstSampleOffset=$%X)", _numPatterns, _firstSampleOffset);
	for (uint32 i = 0; i < _numPatterns; i++) {
		if (_patternPtrs[i])
			debug(3, "WB: Pattern %d: offset=$%X", i, _patternPtrs[i]);
	}
}

// ---------------------------------------------------------------------------
// Song init
// ---------------------------------------------------------------------------

void WallyBebenStream::startSong(int songNum) {
	_musicActive = false;

	if (songNum < 1 || songNum > 2)
		return;

	int songIdx = songNum - 1;
	_tickSpeed = 6;
	_tickCounter = 0;
	_pendingSongCommand = -1;

	// Silence all Paula channels
	for (int ch = 0; ch < NUM_VOICES; ch++) {
		clearVoice(ch);
	}

	// Initialize each channel from the song table
	for (int ch = 3; ch >= 0; ch--) {
		initChannel(ch);
		_channels[ch].orderListOffset = _songOrderPtrs[songIdx][ch];
		_channels[ch].orderListPos = 0;
		_channels[ch].active = true;

		// Read first order list entry to get the initial pattern
		readOrderList(ch);
	}

	_musicActive = true;
	startPaula();

	debug(3, "WB: Song %d started, tickSpeed=%d", songNum, _tickSpeed);
	for (int ch = 3; ch >= 0; ch--) {
		debug(3, "WB: ch%d orderList=$%X pattern=$%X",
			ch, _channels[ch].orderListOffset, _channels[ch].patternOffset);
	}
}

void WallyBebenStream::initChannel(int ch) {
	ChannelState &c = _channels[ch];
	memset(&c, 0, sizeof(ChannelState));
	c.duration = 1;
	c.durationCounter = 0; // Will trigger readPatternCommands on first tick
	c.envelopePhase = 3;   // Start in release (silent) until note-on

	// Default envelope params: full volume sustain, so notes before any
	// $C0 envelope command still produce sound (Env 0 has all zeros = silence)
	c.attackLevel = 64;
	c.decayTarget = 64;
	c.sustainLevel = 64;
	c.releaseRate = 0;
}

// ---------------------------------------------------------------------------
// Order list reader — advances to the next pattern for a channel
// Asm ref: TEXT+$01A0 (order list processing)
// Order list format: $00-$C0=pattern#, $C1-$FE=transpose, $FF=loop
// ---------------------------------------------------------------------------

void WallyBebenStream::readOrderList(int ch) {
	ChannelState &c = _channels[ch];

	for (int safety = 0; safety < 256; safety++) {
		if (c.orderListOffset + c.orderListPos >= _dataSize)
			break;

		byte cmd = readDataByte(c.orderListOffset + c.orderListPos);
		c.orderListPos++;

		if (cmd == 0xFF) {
			// Loop song: reset order list to beginning
			c.orderListPos = 0;
			continue;
		}

		if (cmd > 0xC0) {
			// Transpose command: transpose = (cmd + 0x20) & 0xFF
			// Stored as signed offset
			c.transpose = WBCommon::decodeOrderTranspose(cmd);
			continue;
		}

		// Pattern index (0 to $C0) — look up in pattern pointer table
		// Validate: must be within table AND point to real pattern data (< sample area)
		if (cmd < kMaxPatternEntries && _patternPtrs[cmd] > 0 && _patternPtrs[cmd] < _firstSampleOffset) {
			c.patternOffset = _patternPtrs[cmd];
			c.patternPos = 0;
			debugC(3, kFreescapeDebugParser, "WB: ch%d order -> pattern %d (offset $%04X)", ch, cmd, c.patternOffset);
		} else {
			warning("WallyBeben: ch%d pattern index %d invalid (ptr=$%X, sampleStart=$%X)",
				ch, cmd, (cmd < kMaxPatternEntries) ? _patternPtrs[cmd] : 0, _firstSampleOffset);
			c.patternOffset = _patternPtrs[0];
			c.patternPos = 0;
		}
		return;
	}

	warning("WallyBeben: ch%d order list safety limit hit", ch);
}

// ---------------------------------------------------------------------------
// Pattern command reader — reads byte stream until a note is found
// Asm ref: TEXT+$0268 (command dispatcher)
// Pattern format: $FF=end-pattern, $FE=end-song, $F0-$FD=speed,
//   $E0-$EF=instrument, $C0-$DF=envelope, $80-$BF=duration,
//   $7F/$7E=portamento, $7D/$7C=vibrato, $7B=arpeggio,
//   $7A=delay, $00-$5F=note (0=rest, 1-47=C-1..B-3)
// ---------------------------------------------------------------------------

void WallyBebenStream::readPatternCommands(int ch) {
	ChannelState &c = _channels[ch];

	for (int safety = 0; safety < 256; safety++) {
		if (c.patternOffset + c.patternPos >= _dataSize)
			break;

		byte cmd = readDataByte(c.patternOffset + c.patternPos);
		c.patternPos++;

		if (cmd == 0xFF) {
			// End of pattern — advance order list
			readOrderList(ch);
			continue;
		}

		if (cmd == 0xFE) {
			// End of song
			_musicActive = false;
			return;
		}

		if (cmd == 0xDD) {
			// Song change command: hand off through mailbox-equivalent path.
			byte param = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			_pendingSongCommand = param;
			return;
		}

		if (cmd >= 0xF0) {
			// Set speed: low nibble
			_tickSpeed = WBCommon::decodeTickSpeed(cmd);
			continue;
		}

		if (cmd >= 0xE0) {
			// Set instrument: low nibble (0-15)
			c.instrumentIdx = cmd & 0x0F;
			continue;
		}

		if (cmd >= 0xC0) {
			// Set envelope: low 5 bits (0-31, but only 1-9 valid)
			// Asm ref: TEXT+$2C8 — envelope command handler
			// $C0 (index 0) is a no-op: Env 0 is all zeros (sentinel entry).
			// The original engine treats index 0 as "no envelope change".
			byte envIdx = cmd & 0x1F;
			if (envIdx == 0 || envIdx > 9) {
				// Index 0 or out-of-range: skip, keep current envelope params
				continue;
			}

			c.envelopeIdx = envIdx;

			// Copy envelope parameters into channel state immediately
			// (original engine loads params on $C0 command, not on note-on)
			const EnvelopeDesc &env = _envelopes[c.envelopeIdx];
			c.attackLevel  = MIN(env.attackLevel,  (byte)64);
			c.decayTarget  = MIN(env.decayTarget,  (byte)64);
			c.sustainLevel = MIN(env.sustainLevel, (byte)64);
			c.releaseRate  = env.releaseRate;
			c.modDepth     = env.modDepth;

			// Envelope-triggered arpeggio/vibrato from envelope table
			if (env.arpeggioMask != 0) {
				c.effectMode = 2;
				c.arpeggioMask = env.arpeggioMask;
				buildArpeggioTable(ch, env.arpeggioMask);
			}
			continue;
		}

		if (cmd >= 0x80) {
			// Set duration: low 6 bits (0-63)
			c.duration = WBCommon::decodeDuration(cmd);
			continue;
		}

		if (cmd == 0x7F) {
			// Portamento up
			c.portaUp = true;
			c.portaDown = false;
			c.effectMode = 1;
			c.effect7BActive = false;
			continue;
		}

		if (cmd == 0x7E) {
			// Portamento down
			c.portaDown = true;
			c.portaUp = false;
			c.effectMode = 1;
			c.effect7BActive = false;
			continue;
		}

		if (cmd == 0x7D) {
			// Vibrato type 1
			byte param = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.effectMode = 1;
			c.effect7BActive = false;
			c.arpeggioMask = param;
			buildArpeggioTable(ch, param);
			continue;
		}

		if (cmd == 0x7C) {
			// Vibrato type 2
			byte param = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.effectMode = 2;
			c.effect7BActive = false;
			c.arpeggioMask = param;
			buildArpeggioTable(ch, param);
			continue;
		}

		if (cmd == 0x7B) {
			// Dedicated arpeggio/slide mode:
			//  xx + transpose -> base note, yy -> rate.
			byte base = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			byte rate = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.effectMode = 1;
			c.arpeggioMask = 0;
			c.arpeggioPos = 0;
			c.arpeggioTableLen = 0;
			c.effect7BActive = true;
			c.effect7BCounter = 0;
			c.effect7BUseBase = true;

			int baseNote = base + c.transpose;
			if (baseNote < 1)
				baseNote = 1;
			if (baseNote > 47)
				baseNote = 47;
			c.effect7BBaseNote = (byte)baseNote;
			c.effect7BRate = rate;
			continue;
		}

		if (cmd == 0x7A) {
			// Set delay
			byte param = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.delay = param;
			continue;
		}

		if (cmd > 0x5F) {
			// Original command parser accepts notes in range 0x00-0x5F.
			warning("WallyBeben: ch%d unknown pattern command $%02X", ch, cmd);
			continue;
		}

		// Note value (0x00-0x5F)
		c.prevNote = c.note;
		c.note = cmd;
		if (c.note != 0 && c.delay > 0) {
			// Delay note-on by c.delay frames.
			c.delayCounter = c.delay;
			c.pendingNoteOn = true;
		} else {
			c.durationCounter = c.duration;
			c.pendingNoteOn = false;
			triggerNote(ch);
		}
		return; // One note per sequencer step
	}

	warning("WallyBeben: ch%d pattern read safety limit hit", ch);
}

// ---------------------------------------------------------------------------
// Note trigger — load instrument, set up Paula channel, reset envelope
// Asm ref: TEXT+$030C (note-on handler)
// ---------------------------------------------------------------------------

void WallyBebenStream::triggerNote(int ch) {
	ChannelState &c = _channels[ch];
	c.pendingNoteOn = false;

	if (c.note == 0) {
		// Rest — silence channel
		c.outputPeriod = 0;
		c.volume = 0;
		c.envelopePhase = 3;
		c.effect7BActive = false;
		setChannelVolume(ch, 0);
		return;
	}

	// Apply transpose and clamp
	int note = c.note + c.transpose;
	if (note < 1) note = 1;
	if (note > 47) note = 47;

	// Look up period
	c.basePeriod = getPeriod(note);
	c.outputPeriod = c.basePeriod;

	if (c.effect7BActive) {
		c.effect7BCounter = 0;
		c.effect7BUseBase = true;
	}

	if (c.basePeriod == 0) {
		warning("WallyBeben: ch%d note %d (raw %d + transpose %d) has period 0",
			ch, note, c.note, c.transpose);
		return;
	}

	// Load instrument
	const InstrumentDesc &inst = _instruments[c.instrumentIdx];
	byte sampleIdx = inst.sampleIndex;
	if (sampleIdx >= 16)
		sampleIdx = 0;

	uint32 sampleOffset = _sampleOffsets[sampleIdx];
	if (sampleOffset >= _dataSize) {
		warning("WallyBeben: ch%d sample %d offset $%X out of range (dataSize=$%X)",
			ch, sampleIdx, sampleOffset, _dataSize);
		return;
	}

	const int8 *sampleData = (const int8 *)(_data + sampleOffset);
	uint32 maxLen = _dataSize - sampleOffset;

	// Instrument lengths are in bytes
	uint32 totalLen = MIN((uint32)inst.totalLength, maxLen);
	uint32 loopOff = MIN((uint32)inst.loopOffset, totalLen);
	uint32 loopLen = inst.loopLength;

	debug(5, "WB: ch%d note=%d period=%d inst=%d sample=%d offset=$%X len=%d loop=%d/%d",
		ch, note, c.basePeriod, c.instrumentIdx, sampleIdx, sampleOffset,
		totalLen, loopOff, loopLen);

	if (totalLen < 4) {
		warning("WallyBeben: ch%d sample too short (%d bytes)", ch, totalLen);
		return;
	}

	// setChannelData calls enableChannel internally — do NOT call enableChannel again!
	if (inst.loopFlag && loopLen > 2 && loopOff + loopLen <= totalLen) {
		uint32 initialLen = loopOff + loopLen;
		// Looped sample: initial play through loop end, then loop region
		setChannelData(ch,
			sampleData,                // initial data pointer
			sampleData + loopOff,      // loop start pointer
			initialLen,                // initial length in bytes
			loopLen);                  // loop length in bytes
	} else {
		// One-shot: play once, then 1-byte silence loop
		setChannelData(ch,
			sampleData,
			sampleData + totalLen - 2,
			totalLen,                  // length in bytes
			1);
	}

	setChannelPeriod(ch, c.outputPeriod);

	// Reset envelope phase — params were already loaded by $C0 command
	// (or default to full volume from initChannel)
	// Asm ref: TEXT+$30C — note-on envelope reset
	c.envelopePhase = 0; // Start at attack
	c.volume = c.attackLevel;

	setChannelVolume(ch, c.volume);

	debugC(3, kFreescapeDebugParser, "WB: ch%d NOTE note=%d(+%d) period=%d inst=%d env=%d vol=%d",
		ch, c.note, c.transpose, c.basePeriod, c.instrumentIdx, c.envelopeIdx, c.volume);

	// Set up portamento if active
	if (c.portaUp || c.portaDown) {
		int16 prevPeriod = (c.prevNote > 0 && c.prevNote <= 47) ? getPeriod(c.prevNote + c.transpose) : c.basePeriod;
		int16 delta = ABS(c.basePeriod - prevPeriod);
		int steps = (_tickSpeed > 0) ? _tickSpeed : 1;
		c.portaStep = delta / steps;
		if (c.portaStep == 0)
			c.portaStep = 1;
		c.portaTarget = c.basePeriod;
		c.basePeriod = prevPeriod; // Start from previous note
		c.outputPeriod = prevPeriod;
	}
}

// ---------------------------------------------------------------------------
// Effects processing — runs every frame (50Hz)
// Asm ref: TEXT+$05A0 (portamento), TEXT+$0620 (arpeggio/vibrato)
// ---------------------------------------------------------------------------

void WallyBebenStream::processEffects(int ch) {
	ChannelState &c = _channels[ch];

	if (c.pendingNoteOn)
		return;

	if (c.effectMode == 0) {
		c.outputPeriod = c.basePeriod;
		return;
	}

	// Portamento
	if (c.portaUp) {
		// Sliding toward lower period (higher pitch)
		c.basePeriod -= c.portaStep;
		if (c.basePeriod <= c.portaTarget) {
			c.basePeriod = c.portaTarget;
			c.portaUp = false;
			c.effectMode = 0;
		}
		c.outputPeriod = c.basePeriod;
		return;
	}

	if (c.portaDown) {
		// Sliding toward higher period (lower pitch)
		c.basePeriod += c.portaStep;
		if (c.basePeriod >= c.portaTarget) {
			c.basePeriod = c.portaTarget;
			c.portaDown = false;
			c.effectMode = 0;
		}
		c.outputPeriod = c.basePeriod;
		return;
	}

	// Dedicated $7B effect: cycle between base note and current note.
	if (c.effect7BActive) {
		int rate = c.effect7BRate;
		if (rate <= 0)
			rate = 1;

		c.effect7BCounter++;
		if (c.effect7BCounter >= rate) {
			c.effect7BCounter = 0;
			c.effect7BUseBase = !c.effect7BUseBase;
		}

		int note = c.effect7BUseBase ? c.effect7BBaseNote : (c.note + c.transpose);
		if (note < 1)
			note = 1;
		if (note > 47)
			note = 47;
		c.outputPeriod = getPeriod(note);
		return;
	}

	// Arpeggio effect (effectMode 1 or 2)
	if (c.arpeggioTableLen > 0) {
		int note = c.note + c.transpose;
		int offset = c.arpeggioTable[c.arpeggioPos % c.arpeggioTableLen];
		note += offset;
		if (note < 1) note = 1;
		if (note > 47) note = 47;
		c.outputPeriod = getPeriod(note);
		c.arpeggioPos++;
		if (c.arpeggioPos >= c.arpeggioTableLen)
			c.arpeggioPos = 0;
	} else {
		c.outputPeriod = c.basePeriod;
	}
}

// ---------------------------------------------------------------------------
// Volume envelope — runs every frame (50Hz)
// Asm ref: TEXT+$068C (envelope processing)
//
// Envelope table bytes (per entry, 8 bytes at TEXT+$D0A):
//   byte 0 (attackLevel):  initial volume on note-on
//   byte 1 (decayTarget):  target volume to fade toward and hold
//   byte 2 (sustainLevel): sustain volume while note is active
//   byte 3 (releaseRate):  volume decrease per tick on note-off
//   byte 4 (modDepth):     modulation depth
//   byte 5 (vibratoWave):  vibrato waveform selector
//   byte 6 (arpeggioMask): bitmask into arpeggio interval table at TEXT+$D02
//   byte 7 (flags):        misc flags
// ---------------------------------------------------------------------------

void WallyBebenStream::processEnvelope(int ch) {
	ChannelState &c = _channels[ch];

	switch (c.envelopePhase) {
	case 0: // Attack — start from attack level, then enter decay.
		c.volume = MIN(c.attackLevel, (byte)64);
		c.envelopePhase = 1;
		break;

	case 1: // Decay — decrease toward decay target.
		if (c.volume > c.decayTarget) {
			c.volume--;
		} else {
			c.volume = c.sustainLevel;
			c.envelopePhase = 2;
		}
		break;

	case 2: // Sustain — hold sustain level until note-off.
		c.volume = c.sustainLevel;
		break;

	case 3: // Release — decrease volume on note-off
		if (c.releaseRate > 0) {
			if (c.volume > c.releaseRate) {
				c.volume -= c.releaseRate;
			} else {
				c.volume = 0;
			}
		} else {
			c.volume = 0; // Rate 0 = instant off
		}
		break;
	}

	// Clamp to Paula range
	if (c.volume > 64)
		c.volume = 64;
}

// ---------------------------------------------------------------------------
// Arpeggio table builder — unpacks a bitmask into interval offsets
// ---------------------------------------------------------------------------

void WallyBebenStream::buildArpeggioTable(int ch, byte mask) {
	ChannelState &c = _channels[ch];
	c.arpeggioTableLen = WBCommon::buildArpeggioTable(_arpeggioIntervals, mask, c.arpeggioTable, 16, true);
	c.arpeggioPos = 0;
}

// ---------------------------------------------------------------------------
// Period lookup
// ---------------------------------------------------------------------------

uint16 WallyBebenStream::getPeriod(int note) const {
	if (note < 0 || note > 47)
		return 0;
	return _periods[note];
}

// ---------------------------------------------------------------------------
// Main interrupt — called at 50Hz by Paula
// Asm ref: TEXT+$0004 (tick entry), TEXT+$010C (sequencer body)
// ---------------------------------------------------------------------------

void WallyBebenStream::interrupt() {
	if (!_musicActive)
		return;

	// Sequencer step: when tick counter reaches 0
	if (_tickCounter == 0) {
		for (int ch = 3; ch >= 0; ch--) {
			if (!_channels[ch].active)
				continue;
			if (_channels[ch].pendingNoteOn)
				continue;

			if (_channels[ch].durationCounter > 0) {
				_channels[ch].durationCounter--;
			}

			if (_channels[ch].durationCounter == 0) {
				// Note-off: enter release phase
				if (_channels[ch].envelopePhase < 3)
					_channels[ch].envelopePhase = 3;

				// Read next commands
				readPatternCommands(ch);
			}
		}
	}

	// Apply pending song command ($DD xx) after parser step.
	if (_pendingSongCommand != -1) {
		byte command = (byte)_pendingSongCommand;
		_pendingSongCommand = -1;

		if (command == 0) {
			_musicActive = false;
			for (int ch = 0; ch < NUM_VOICES; ch++)
				clearVoice(ch);
			return;
		}
		if (command >= 1 && command <= 2) {
			startSong(command);
			return;
		}
	}

	// Delay-before-note handling ($7A): countdown in frames, then trigger note-on.
	for (int ch = 3; ch >= 0; ch--) {
		ChannelState &c = _channels[ch];
		if (!c.active || !c.pendingNoteOn)
			continue;

		if (c.delayCounter > 0)
			c.delayCounter--;

		if (c.delayCounter == 0) {
			c.pendingNoteOn = false;
			c.durationCounter = c.duration;
			triggerNote(ch);
		}
	}

	// Every frame: process effects and envelope, update Paula
	for (int ch = 3; ch >= 0; ch--) {
		if (!_channels[ch].active)
			continue;

		processEffects(ch);
		processEnvelope(ch);

		setChannelPeriod(ch, _channels[ch].outputPeriod);
		setChannelVolume(ch, _channels[ch].volume);
	}

	// Advance tick counter
	_tickCounter++;
	if (_tickCounter >= _tickSpeed) {
		_tickCounter = 0;
	}
}

// ---------------------------------------------------------------------------
// Factory function
// ---------------------------------------------------------------------------

Audio::AudioStream *makeWallyBebenStream(const byte *data, uint32 dataSize,
                                         int songNum, int rate, bool stereo) {
	if (!data || dataSize < 0xF000) {
		warning("WallyBeben: invalid data (size %u)", dataSize);
		return nullptr;
	}

	return new WallyBebenStream(data, dataSize, songNum, rate, stereo);
}

} // End of namespace Freescape
