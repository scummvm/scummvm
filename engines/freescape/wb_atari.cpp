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
 * Atari ST player for the Wally Beben music engine (YM2149 PSG).
 *
 * Drives TEMUSIC.ST (Total Eclipse) and DSMUSIC2.ST (Dark Side), which are the
 * same engine at the same code addresses with their data tables at different
 * offsets. It shares the byte-stream pattern format with the Amiga flavour in
 * wb.cpp, but writes YM2149 registers instead of feeding Paula.
 */

#include "audio/ym2149.h"

#include "freescape/freescape.h"
#include "freescape/music.h"
#include "freescape/wb.h"

#include "common/endian.h"
#include "common/debug.h"
#include "common/util.h"

namespace Freescape {

const WBAtariTableOffsets kEclipseAtariOffsets = {
	0x0B24, // periodTable
	0x0CC8, // arpeggioIntervals
	0x0D60, // instrumentTable
	0x0DC0, // songTable
	0x0DCC, // patternPtrTable
	12,     // numInstruments
	0x20,   // noiseSeed, asm ref TEMUSIC.ST $09CC
	true,   // noiseFollowsInstrument, asm ref TEMUSIC.ST $0A02
	0,      // noiseDrift: the value lives in a corrupt word of the only dump
	0x64, 0x0FFF, 1 // sweep, asm ref TEMUSIC.ST $042A
};

const WBAtariTableOffsets kDarkSideAtariOffsets = {
	0x0AFA, // periodTable
	0x0C9E, // arpeggioIntervals
	0x0CA6, // instrumentTable
	0x0D46, // songTable
	0x0D52, // patternPtrTable
	20,     // numInstruments
	0x3C,   // noiseSeed, asm ref DSMUSIC2.ST $09B4
	false,  // noiseFollowsInstrument, asm ref DSMUSIC2.ST $09D8
	6,      // noiseDrift, asm ref DSMUSIC2.ST $0462
	0x96, 0, -8 // sweep, asm ref DSMUSIC2.ST $042A
};

const int kTENumChannels    = 3;
const int kTENumPeriods     = 96;
const int kTEMaxInstruments = 32; // the instrument command carries a 5-bit index
// Total Eclipse has 31 pattern pointers and Dark Side 32; both tables end where
// the first order list begins, and loadTables() drops entries that do not point
// into the module.
const int kTEMaxPatterns    = 32;

class WallyBebenAtariPlayer : public MusicPlayer {
public:
	WallyBebenAtariPlayer(const byte *data, uint32 dataSize,
	                      const WBAtariTableOffsets &offsets, int songNum);
	~WallyBebenAtariPlayer();

	void startMusic() override;
	void stopMusic() override;
	bool isPlaying() const override;

private:
	YM2149::YM2149 *_ym2149;

	// --- Data tables ---
	const byte *_data;
	uint32 _dataSize;
	WBAtariTableOffsets _offsets;

	uint16 _periods[kTENumPeriods];

	struct InstrumentDesc {
		byte volume;
		byte targetVol;
		byte attackRate;
		byte releaseRate;
		byte envFlags;     // Bit 7 selects the oscillating volume mode
		byte effectType;
		byte arpeggioData;
		byte flags;
	};
	InstrumentDesc _instruments[kTEMaxInstruments];

	// Song order list pointers (TEXT-relative)
	uint32 _songOrderPtrs[2][kTENumChannels];

	// Pattern pointer table
	uint32 _patternPtrs[kTEMaxPatterns];
	uint32 _numPatterns;

	// Arpeggio interval lookup
	byte _arpeggioIntervals[8];

	// --- Per-channel state ---
	struct ChannelState {
		// Order list
		uint32 orderListOffset;
		int orderListPos;
		int8 transpose;

		// Current pattern
		uint32 patternOffset;
		int patternPos;

		// Note state
		byte note;
		byte duration;
		int durationCounter;

		// Instrument
		byte instrumentIdx;

		// Volume envelope
		byte volume;       // 0-63 internal, written to the YM as >>2
		byte attackLevel;
		byte decayTarget;
		byte attackRate;
		byte releaseRate;
		byte envelopeFlags;
		byte envelopeToggle;
		bool envelopeDone;

		// Effects
		byte effectMode;   // 0=none, 1=pattern FX ($7D), 2=instrument FX ($7C)
		bool portaUp;
		bool portaDown;
		bool skipEffects;   // porta steps bypass the rest of the effects
		byte arpeggioMask;
		byte arpeggioPos;
		byte arpeggioTable[16];
		byte arpeggioTableLen;
		byte effect7BBaseNote;
		byte effect7BParam;
		bool effect7BActive;
		int16 effect7BPeriod;
		byte delay;
		byte delayCounter;

		// Noise
		bool noiseEnabled;
		bool toneEnabled;
		bool skipTranspose; // Noise-only mode bypasses the order-list transpose
		bool freqSweep;
		byte noisePeriod;
		byte noiseCounter;  // Countdown from the instrument flags high nibble

		// Instrument byte-5 period modulation. Asm ref: $048A
		byte modParam;
		byte modSpan;
		byte modPos;
		int8 modDir;
		int16 modStep;

		// Period
		int16 basePeriod;
		int16 outputPeriod;

		bool active;
	};
	ChannelState _channels[kTENumChannels];

	// --- Global state ---
	bool _musicActive;
	byte _tickSpeed;
	byte _tickCounter;
	int _songNum;

	// --- Methods ---
	void loadTables();
	void startSong(int songNum);
	void initChannel(int ch);
	void readOrderList(int ch);
	void readPatternCommands(int ch);
	void loadInstrument(int ch);
	void triggerNote(int ch);
	void processEffects(int ch);
	void processEnvelope(int ch);
	void buildArpeggioTable(ChannelState &c, byte mask);
	void tickUpdate();
	void writeYMRegisters();
	void setReg(int reg, byte value) { if (_ym2149) _ym2149->writeReg(reg, value); }

	uint16 getPeriod(int note) const {
		if (note < 0 || note >= kTENumPeriods)
			return 0;
		return _periods[note];
	}

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

WallyBebenAtariPlayer::WallyBebenAtariPlayer(const byte *data, uint32 dataSize,
                                             const WBAtariTableOffsets &offsets,
                                             int songNum)
	: _data(data), _dataSize(dataSize), _offsets(offsets),
	  _musicActive(false), _tickSpeed(6), _tickCounter(0),
	  _numPatterns(0), _songNum(songNum) {

	memset(_periods, 0, sizeof(_periods));
	memset(_instruments, 0, sizeof(_instruments));
	memset(_songOrderPtrs, 0, sizeof(_songOrderPtrs));
	memset(_patternPtrs, 0, sizeof(_patternPtrs));
	memset(_arpeggioIntervals, 0, sizeof(_arpeggioIntervals));
	memset(_channels, 0, sizeof(_channels));

	_ym2149 = YM2149::Config::create();
	if (!_ym2149 || !_ym2149->init()) {
		warning("WallyBebenAtariPlayer: Failed to create YM2149 emulator");
		delete _ym2149;
		_ym2149 = nullptr;
	}

	loadTables();
}

WallyBebenAtariPlayer::~WallyBebenAtariPlayer() {
	stopMusic();
	delete _ym2149;
}


// ============================================================================
// Public interface
// ============================================================================

void WallyBebenAtariPlayer::startMusic() {
	if (!_ym2149)
		return;
	stopMusic();
	_ym2149->start(new Common::Functor0Mem<void, WallyBebenAtariPlayer>(
		this, &WallyBebenAtariPlayer::tickUpdate), 50);
	startSong(_songNum);
}

void WallyBebenAtariPlayer::stopMusic() {
	_musicActive = false;
	if (_ym2149) {
		_ym2149->stop();
	}
}

bool WallyBebenAtariPlayer::isPlaying() const {
	return _musicActive;
}

void WallyBebenAtariPlayer::loadTables() {
	for (int i = 0; i < kTENumPeriods; i++) {
		_periods[i] = readDataWord(_offsets.periodTable + i * 2);
	}

	for (int i = 0; i < 8; i++) {
		_arpeggioIntervals[i] = readDataByte(_offsets.arpeggioIntervals + i);
	}

	for (int i = 0; i < _offsets.numInstruments && i < kTEMaxInstruments; i++) {
		uint32 off = _offsets.instrumentTable + i * 8;
		_instruments[i].volume      = readDataByte(off + 0);
		_instruments[i].targetVol   = readDataByte(off + 1);
		_instruments[i].attackRate  = readDataByte(off + 2);
		_instruments[i].releaseRate = readDataByte(off + 3);
		_instruments[i].envFlags    = readDataByte(off + 4);
		_instruments[i].effectType  = readDataByte(off + 5);
		_instruments[i].arpeggioData = readDataByte(off + 6);
		_instruments[i].flags       = readDataByte(off + 7);
	}

	// Song table: 2 songs x 3 channels x uint32 BE at TEXT+$0DC0
	for (int s = 0; s < 2; s++) {
		for (int ch = 0; ch < kTENumChannels; ch++) {
			_songOrderPtrs[s][ch] = readDataLong(_offsets.songTable + s * 12 + ch * 4);
		}
	}

	// Pattern pointer table at TEXT+$0DCC
	_numPatterns = 0;
	for (uint32 i = 0; i < kTEMaxPatterns; i++) {
		uint32 ptr = readDataLong(_offsets.patternPtrTable + i * 4);
		_patternPtrs[i] = ptr;
		if (ptr > 0 && ptr < _dataSize)
			_numPatterns = i + 1;
	}

	debug(3, "WB-Atari: Loaded music data (%u bytes)", _dataSize);
	debug(3, "WB-Atari: %d valid patterns", _numPatterns);

	for (int s = 0; s < 2; s++) {
		debug(3, "WB-Atari: Song %d order ptrs: $%X $%X $%X",
			s + 1, _songOrderPtrs[s][0], _songOrderPtrs[s][1], _songOrderPtrs[s][2]);
	}

	for (int i = 0; i < _offsets.numInstruments; i++) {
		const InstrumentDesc &inst = _instruments[i];
		if (inst.volume > 0 || inst.targetVol > 0)
			debug(3, "WB-Atari: Inst %d: vol=%d target=%d atk=%d rel=%d envFlags=$%02X effect=$%02X arp=$%02X flags=$%02X",
				i, inst.volume, inst.targetVol, inst.attackRate, inst.releaseRate,
				inst.envFlags, inst.effectType, inst.arpeggioData, inst.flags);
	}
}

// ---------------------------------------------------------------------------
// Song init
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::startSong(int songNum) {
	_musicActive = false;

	if (songNum < 1 || songNum > 2)
		return;

	int songIdx = songNum - 1;
	_tickSpeed = 6;
	_tickCounter = 0;

	// Silence all YM channels
	for (int r = 0; r < 14; r++)
		setReg(r, 0);
	setReg(7, 0x3F); // All disabled

	for (int ch = 0; ch < kTENumChannels; ch++) {
		initChannel(ch);
		_channels[ch].orderListOffset = _songOrderPtrs[songIdx][ch];
		_channels[ch].orderListPos = 0;
		_channels[ch].active = true;
		readOrderList(ch);
	}

	_musicActive = true;

	debug(3, "WB-Atari: Song %d started, tickSpeed=%d", songNum, _tickSpeed);
	for (int ch = 0; ch < kTENumChannels; ch++) {
		debug(3, "WB-Atari: ch%d orderList=$%X pattern=$%X",
			ch, _channels[ch].orderListOffset, _channels[ch].patternOffset);
	}
}

void WallyBebenAtariPlayer::initChannel(int ch) {
	ChannelState &c = _channels[ch];
	memset(&c, 0, sizeof(ChannelState));
	c.duration = 1;
	c.durationCounter = 0;
	c.toneEnabled = true;
	c.envelopeDone = true;
}

// ---------------------------------------------------------------------------
// Order list reader: $00-$C0 pattern, $C1-$FE transpose, $FF loop
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::readOrderList(int ch) {
	ChannelState &c = _channels[ch];

	for (int safety = 0; safety < 256; safety++) {
		if (c.orderListOffset + c.orderListPos >= _dataSize)
			break;

		byte cmd = readDataByte(c.orderListOffset + c.orderListPos);
		c.orderListPos++;

		if (cmd == 0xFF) {
			c.orderListPos = 0;
			continue;
		}

		if (cmd > 0xC0) {
			c.transpose = WBCommon::decodeOrderTranspose(cmd);
			continue;
		}

		if (cmd < _numPatterns && _patternPtrs[cmd] > 0 && _patternPtrs[cmd] < _dataSize) {
			c.patternOffset = _patternPtrs[cmd];
			c.patternPos = 0;
			debugC(3, kFreescapeDebugMedia, "WB-Atari: ch%d order -> pattern %d (offset $%04X)", ch, cmd, c.patternOffset);
		} else {
			// Invalid pattern index — skip it and try next order entry
			debugC(3, kFreescapeDebugMedia, "WB-Atari: ch%d skipping invalid pattern index %d", ch, cmd);
			continue;
		}
		return;
	}

	warning("WB-Atari: ch%d order list safety limit hit", ch);
}

// ---------------------------------------------------------------------------
// Pattern command reader
// Same format as wb.cpp: $FF=end-pattern, $FE=end-song, $F0+=speed,
//   $C0+=instrument, $80+=duration, $7F/$7E=portamento,
//   $7D/$7C=vibrato/arpeggio, $00-$5F=note
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::readPatternCommands(int ch) {
	ChannelState &c = _channels[ch];

	for (int safety = 0; safety < 256; safety++) {
		if (c.patternOffset + c.patternPos >= _dataSize)
			break;

		byte cmd = readDataByte(c.patternOffset + c.patternPos);
		c.patternPos++;

		if (cmd == 0xFF) {
			readOrderList(ch);
			continue;
		}

		if (cmd == 0xFE) {
			_musicActive = false;
			return;
		}

		if (cmd == 0xFC) {
			// Song jump, via the mailbox.
			byte command = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			if (command == 0) {
				_musicActive = false;
				for (int r = 0; r < 14; r++)
					setReg(r, 0);
				setReg(7, 0x3F);
			} else if (command >= 1 && command <= 2) {
				startSong(command);
			}
			return;
		}

		if (cmd >= 0xF0) {
			_tickSpeed = WBCommon::decodeTickSpeed(cmd);
			continue;
		}

		if (cmd >= 0xC0) {
			// Asm ref: $027C only records the table offset. The parameters are
			// copied into the channel at note-on, by loadInstrument().
			byte instIdx = cmd & 0x1F;
			if (instIdx < _offsets.numInstruments)
				c.instrumentIdx = instIdx;
			continue;
		}

		if (cmd >= 0x80) {
			c.duration = WBCommon::decodeDuration(cmd);
			continue;
		}

		if (cmd == 0x7F) {
			c.portaUp = true;
			c.portaDown = false;
			c.effectMode = 1;
			if (c.patternOffset + c.patternPos >= _dataSize)
				return;
			c.note = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.durationCounter = c.duration;
			triggerNote(ch);
			return;
		}

		if (cmd == 0x7E) {
			c.portaDown = true;
			c.portaUp = false;
			c.effectMode = 1;
			if (c.patternOffset + c.patternPos >= _dataSize)
				return;
			c.note = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.durationCounter = c.duration;
			triggerNote(ch);
			return;
		}

		if (cmd == 0x7D) {
			// Pattern effect 1: parameterized interval table (channel-local)
			byte param = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.effectMode = 1;
			c.arpeggioMask = param;
			c.arpeggioPos = 0;
			buildArpeggioTable(c, param);
			continue;
		}

		if (cmd == 0x7C) {
			// Asm ref: $031A loads mode 2 and jumps into the $7D handler, so
			// this consumes a parameter too. Mode 2 survives a step boundary.
			byte param = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			c.effectMode = 2;
			c.arpeggioMask = param;
			c.arpeggioPos = 0;
			buildArpeggioTable(c, param);
			continue;
		}

		if (cmd == 0x7B) {
			// Slide: byte 1 is the base note, byte 2 packs the start offset
			// and window length. Asm ref: $061A
			c.arpeggioTableLen = 0;
			c.arpeggioPos = 0;
			c.effectMode = 1;
			c.effect7BActive = false;

			if (c.patternOffset + c.patternPos < _dataSize) {
				byte base = readDataByte(c.patternOffset + c.patternPos);
				c.patternPos++;
				base = (byte)(base + c.transpose);
				c.effect7BBaseNote = base;

				int baseNote = c.effect7BBaseNote;
				if (baseNote < 1)
					baseNote = 1;
				if (baseNote >= kTENumPeriods)
					baseNote = kTENumPeriods - 1;
				c.effect7BPeriod = getPeriod(baseNote);
			}
			if (c.patternOffset + c.patternPos < _dataSize) {
				c.effect7BParam = readDataByte(c.patternOffset + c.patternPos);
				c.patternPos++;
			}
			c.effect7BActive = true;
			continue;
		}

		if (cmd == 0x7A) {
			// Delay command — consumed and copied to per-note delay counter.
			c.delay = readDataByte(c.patternOffset + c.patternPos);
			c.patternPos++;
			continue;
		}

		// Note value ($00-$5F)
		c.note = cmd;
		c.durationCounter = c.duration;
		triggerNote(ch);
		return;
	}

	warning("WB-Atari: ch%d pattern read safety limit hit", ch);
}

// ---------------------------------------------------------------------------
// Note trigger
// ---------------------------------------------------------------------------

// Asm ref: $0938, which note-on calls before it decides whether to transpose.
void WallyBebenAtariPlayer::loadInstrument(int ch) {
	ChannelState &c = _channels[ch];
	const InstrumentDesc &inst = _instruments[c.instrumentIdx];

	c.volume = inst.volume;
	c.attackLevel = inst.volume;
	c.decayTarget = inst.targetVol;
	c.attackRate = inst.attackRate;
	c.releaseRate = inst.releaseRate;
	c.envelopeFlags = inst.envFlags;

	c.modParam = inst.effectType;
	c.modSpan = (inst.effectType >> 4) & 0x0F;
	c.noisePeriod = inst.effectType & 0x3F;
	c.toneEnabled = true;
	c.noiseEnabled = false;
	c.skipTranspose = false;
	c.freqSweep = false;
	c.noiseCounter = inst.flags >> 4;

	// Byte 6 preloads the interval table. Its mode is $7C, not 2, so unlike a
	// pattern $7C it dies at the step boundary and is reapplied by each note.
	if (inst.arpeggioData != 0) {
		c.effectMode = 0x7C;
		c.arpeggioMask = inst.arpeggioData;
		buildArpeggioTable(c, inst.arpeggioData);
	}

	debugC(2, kFreescapeDebugMedia,
		"WB-Atari: ch%d inst %-2d vol=%d target=%d atk=%d rel=%d env=$%02X mod=$%02X arp=$%02X flags=$%02X",
		ch, c.instrumentIdx, inst.volume, inst.targetVol, inst.attackRate,
		inst.releaseRate, inst.envFlags, inst.effectType, inst.arpeggioData, inst.flags);

	// Byte 7: bit0/1 noise modes, bit2 frequency sweep, bit3 retrigger.
	if (inst.flags & 0x01) {
		c.noiseEnabled = true;
		c.noisePeriod = _offsets.noiseSeed;
	} else if (inst.flags & 0x02) {
		c.noiseEnabled = true;
		c.toneEnabled = false;
		c.skipTranspose = true;
		if (!_offsets.noiseFollowsInstrument)
			c.noisePeriod = _offsets.noiseSeed;
		if (c.noisePeriod == 0)
			c.noisePeriod = 1;
	} else if (inst.flags & 0x04) {
		c.noiseEnabled = true;
		c.freqSweep = true;
	}
}

void WallyBebenAtariPlayer::triggerNote(int ch) {
	ChannelState &c = _channels[ch];

	loadInstrument(ch);

	// Apply transpose and clamp
	int note = c.note;
	if (!c.skipTranspose)
		note += c.transpose;
	bool isRest = (note == 0);
	if (!isRest) {
		if (note < 1)
			note = 1;
		if (note >= kTENumPeriods)
			note = kTENumPeriods - 1;
	}

	c.basePeriod = isRest ? 0 : getPeriod(note);
	c.outputPeriod = c.basePeriod;

	if (!isRest && c.basePeriod == 0) {
		warning("WB-Atari: ch%d note %d has period 0", ch, note);
		return;
	}

	// Reset envelope
	c.envelopeToggle = 0;
	c.envelopeDone = false;
	c.delayCounter = c.delay;
	c.arpeggioPos = 0;
	c.modPos = 0;
	c.modDir = 1;
	c.modStep = 0;

	if (!isRest && c.modParam != 0 && note + 1 < kTENumPeriods) {
		int16 periodDelta = ABS((int16)getPeriod(note) - (int16)getPeriod(note + 1));
		byte depth = c.modParam & 0x0F;
		while (depth > 0) {
			periodDelta = (periodDelta >> 1) | 1;
			depth--;
		}
		c.modStep = periodDelta;
	}

	debugC(1, kFreescapeDebugMedia,
		"WB-Atari: ch%d %s %3d (%d%+d) %5dHz inst=%-2d dur=%d vol=%d%s%s%s",
		ch, isRest ? "rest" : "NOTE", note, c.note, c.transpose,
		c.basePeriod > 0 ? 125000 / c.basePeriod : 0,
		c.instrumentIdx, c.duration, c.volume,
		c.noiseEnabled ? (c.toneEnabled ? " [tone+noise]" : " [noise]") : "",
		c.arpeggioTableLen > 0 ? " [arpeggio]" : "",
		c.effect7BActive ? " [slide]" : "");

}

// ---------------------------------------------------------------------------
// Effects — run every tick
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::processEffects(int ch) {
	ChannelState &c = _channels[ch];

	// The instrument flags high nibble counts down to a noise cut-off.
	if (c.noiseEnabled) {
		if (c.noiseCounter > 0) {
			c.noiseCounter--;
			// Asm ref: $0436 — the period walks down while the counter runs.
			if (c.noiseCounter == 0)
				c.noiseEnabled = false;
			else if (_offsets.noiseDrift != 0)
				c.noisePeriod = (byte)((c.noisePeriod - _offsets.noiseDrift) & 0x3F);
		}
	}

	int16 period = c.basePeriod;

	// Flag bit 2 walks the period and drags the noise period with it.
	if (c.freqSweep) {
		c.basePeriod += _offsets.sweepStep;
		if (_offsets.sweepMask != 0)
			c.basePeriod &= _offsets.sweepMask;
		if (c.basePeriod == 0)
			c.basePeriod = 1;
		c.noisePeriod = (byte)((c.noisePeriod + _offsets.sweepNoiseDelta) & 0x3F);
		period = c.basePeriod;
	}

	if (c.effectMode != 0 && c.arpeggioTableLen > 0) {
		// Channel-local interval cycling (used by $7D/$7C paths).
		int note = c.note;
		if (!c.skipTranspose)
			note += c.transpose;
		int offset = c.arpeggioTable[c.arpeggioPos % c.arpeggioTableLen];
		note += offset;
		if (note < 1) note = 1;
		if (note >= kTENumPeriods) note = kTENumPeriods - 1;
		period = getPeriod(note);
		c.arpeggioPos++;
		if (c.arpeggioPos >= c.arpeggioTableLen)
			c.arpeggioPos = 0;
	} else if (c.effect7BActive) {
		// Asm ref: $061A — hold for `start` steps, slide to the base note over
		// `window` steps, then settle on it.
		int start = c.effect7BParam >> 4;
		int window = c.effect7BParam & 0x0F;
		int elapsed = (int)c.durationCounter + start;

		if (elapsed <= (int)c.duration) {
			int16 target = getPeriod(c.effect7BBaseNote);
			if (elapsed + window <= (int)c.duration) {
				c.effect7BPeriod = target;
			} else if (window > 0) {
				int divisor = window * ((int)_tickSpeed + 1);
				int16 step = (int16)(ABS(target - c.basePeriod) / divisor);
				if (step == 0)
					step = 1;
				if (c.effect7BPeriod < target)
					c.effect7BPeriod = MIN<int16>(c.effect7BPeriod + step, target);
				else
					c.effect7BPeriod = MAX<int16>(c.effect7BPeriod - step, target);
			}
			period = c.effect7BPeriod;
		}
	}

	// Byte-5 modulation only runs while the mode is clear.
	if (c.effectMode == 0 && c.modParam != 0 && c.modStep > 0 && c.modSpan > 0) {
		// $7A delay applies to this modulation path, not to all effects.
		if (c.delayCounter > 0) {
			c.delayCounter--;
		} else {
			if (c.modDir < 0) {
				if (c.modPos > 0) {
					c.modPos--;
				} else {
					c.modDir = 1;
					if (c.modPos < c.modSpan)
						c.modPos++;
				}
			} else {
				c.modPos++;
				if (c.modPos > c.modSpan) {
					c.modPos = c.modSpan;
					c.modDir = -1;
					if (c.modPos > 0)
						c.modPos--;
				}
			}

			int center = c.modSpan >> 1;
			int offset = (center - c.modPos) * c.modStep;
			period += offset;
			if (period < 1)
				period = 1;
		}
	}

	c.outputPeriod = period;
}

// ---------------------------------------------------------------------------
// Volume envelope — runs every tick
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::processEnvelope(int ch) {
	ChannelState &c = _channels[ch];
	// Noise-only instruments may validly run with zero tone period.
	if (c.outputPeriod == 0 && !c.noiseEnabled)
		return;

	byte env = c.envelopeFlags;

	// Instrument env byte bit7: oscillating level between attackLevel and target.
	// The YM hardware envelope is never used: the write loop at $0AD2 only
	// ever pushes registers 0-10.
	if (env & 0x80) {
		byte step = env & 0x0F;
		if (c.envelopeToggle == 0) {
			if (c.volume == c.decayTarget) {
				c.envelopeToggle = 1;
				if (c.volume == c.attackLevel) {
					c.envelopeToggle = 0;
				} else {
					c.volume = (byte)(c.volume + step);
				}
			} else {
				c.volume = (byte)(c.volume - step);
				c.envelopeToggle = 0;
			}
		} else {
			if (c.volume == c.attackLevel) {
				c.envelopeToggle = 0;
			} else {
				c.volume = (byte)(c.volume + step);
			}
		}
	} else {
		c.envelopeToggle = 0;
		// The original only skips this when the counter is exactly zero.
		if (c.durationCounter == 0)
			return;

		if (!c.envelopeDone) {
			if (env == 0x00) {
				// Special case: immediate jump to target, mark envelope done.
				c.envelopeDone = true;
				c.volume = c.decayTarget;
				if (c.volume > 63)
					c.volume = 63;
				return;
			}

			if (env != 0xFF) {
				byte div = env & 0x7F;
				byte triggerPoint = (div != 0) ? (c.duration / div) : 0;

				if (c.durationCounter == triggerPoint) {
					c.envelopeDone = true;
				} else if (c.volume != c.decayTarget) {
					c.volume = (byte)(c.volume + c.attackRate);
				}
			}
		}

		if (c.envelopeDone) {
			byte next = (byte)(c.volume - c.releaseRate);
			if ((int8)next < 0)
				c.volume = 0;
			else
				c.volume = next;
		}
	}

	if (c.volume > 63)
		c.volume = 63;
}

// ---------------------------------------------------------------------------
// Arpeggio table builder
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::buildArpeggioTable(ChannelState &c, byte mask) {
	// Asm ref: $0846 — intervals are written from slot 1 and playback wraps
	// back to slot 0, so the base note closes the cycle.
	byte len = WBCommon::buildArpeggioTable(_arpeggioIntervals, mask, c.arpeggioTable, 15, false);
	if (len > 0)
		c.arpeggioTable[len++] = 0;
	c.arpeggioTableLen = len;
	c.arpeggioPos = 0;
	c.effect7BActive = false; // Asm ref: TEXT+$08A2 clears the $7B state here
}

// ---------------------------------------------------------------------------
// Write channel state to YM2149 registers
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::writeYMRegisters() {
	byte mixer = 0x3F; // Start with all disabled (bits 0-2=tone, bits 3-5=noise)

	// The channel loop runs 2 -> 0, so the last writer owns the noise register.
	for (int ch = kTENumChannels - 1; ch >= 0; ch--) {
		ChannelState &c = _channels[ch];

		bool hasTone = c.toneEnabled && (c.outputPeriod > 0);
		bool hasNoise = c.noiseEnabled;
		if (!c.active || c.volume == 0 || (!hasTone && !hasNoise)) {
			// Channel silent
			setReg(8 + ch, 0); // Volume = 0
			continue;
		}

		// Enable tone for this channel (bits 0-2), unless in noise-only mode.
		if (hasTone)
			mixer &= ~(1 << ch);

		// Enable noise for this channel if instrument has noise flag (bits 3-5)
		if (hasNoise) {
			mixer &= ~(1 << (ch + 3));
			byte noisePeriod = c.noisePeriod;
			if (noisePeriod == 0 && c.outputPeriod > 0)
				noisePeriod = (c.outputPeriod >> 4) & 0x1F;
			if (noisePeriod == 0)
				noisePeriod = 1;
			setReg(6, noisePeriod);
		}

		// Set tone period (2 registers per channel) when tone path is active.
		if (hasTone) {
			uint16 period = (uint16)c.outputPeriod;
			setReg(ch * 2, period & 0xFF);       // Fine tune
			setReg(ch * 2 + 1, (period >> 8) & 0x0F); // Coarse tune
		}

		// Set volume (internal 0-63 → YM 0-15). Asm ref: TEXT+$0748.
		byte ymVol = c.volume >> 2;
		if (ymVol > 15) ymVol = 15;
		setReg(8 + ch, ymVol);
	}

	setReg(7, mixer);
}

// ---------------------------------------------------------------------------
// Main tick update — called at 50 Hz
// ---------------------------------------------------------------------------

void WallyBebenAtariPlayer::tickUpdate() {
	if (!_musicActive)
		return;

	// Sequencer step occurs when the tick counter is zero, then it advances.
	// Asm ref: $015C tests the speed counter first.
	bool sequencerTick = (_tickCounter == 0);

	if (sequencerTick) {
		for (int ch = kTENumChannels - 1; ch >= 0; ch--) {
			if (!_channels[ch].active)
				continue;

			ChannelState &c = _channels[ch];
			c.durationCounter--;

			if (c.durationCounter < 0) {
				// Original step boundary reset: clear transient note/effect flags
				// before parsing the next command stream, except mode 2 persistence.
				if (c.effectMode != 2) {
					c.effectMode = 0;
					c.arpeggioMask = 0;
					c.arpeggioPos = 0;
					c.arpeggioTableLen = 0;
				}
				c.effect7BActive = false;
				c.portaUp = false;
				c.portaDown = false;
				c.noiseEnabled = false;
				c.freqSweep = false;
				c.envelopeDone = false;
				readPatternCommands(ch);
			} else if (c.portaUp || c.portaDown) {
				// Asm ref: $0172 — a chromatic walk of one semitone per step
				// for the whole note, skipping the other effects.
				if (c.portaUp)
					c.note--;
				else
					c.note++;

				int n = c.note;
				if (!c.skipTranspose)
					n += c.transpose;
				if (n < 1)
					n = 1;
				if (n >= kTENumPeriods)
					n = kTENumPeriods - 1;
				c.basePeriod = getPeriod(n);
				c.outputPeriod = c.basePeriod;
				c.skipEffects = true;
			}
		}
	}

	// Every tick: process effects and envelope
	for (int ch = kTENumChannels - 1; ch >= 0; ch--) {
		if (!_channels[ch].active)
			continue;

		if (_channels[ch].skipEffects)
			_channels[ch].skipEffects = false;
		else
			processEffects(ch);
		processEnvelope(ch);
	}

	// Asm ref: $0808 — the counter only reloads once it goes negative, so a
	// speed of N leaves N + 1 ticks between sequencer steps.
	_tickCounter++;
	if (_tickCounter > _tickSpeed)
		_tickCounter = 0;

	writeYMRegisters();
}

// ---------------------------------------------------------------------------
// Factory function
// ---------------------------------------------------------------------------

MusicPlayer *makeWallyBebenAtariPlayer(const byte *data, uint32 dataSize,
                                       const WBAtariTableOffsets &offsets,
                                       int songNum) {
	if (!data || dataSize < 0x1000) {
		warning("WB-Atari music: invalid data (size %u)", dataSize);
		return nullptr;
	}

	return new WallyBebenAtariPlayer(data, dataSize, offsets, songNum);
}

} // End of namespace Freescape
