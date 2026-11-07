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

#include "scooby/sound/gems_driver.h"

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "scooby/assets/rom.h"

namespace Scooby {

namespace {

// Z80 RAM layout (Z80.ASM "DATA AREA").
const uint32 kHighRamBase = 0x1B20;
const uint32 kMailboxes = 0x1B20;      // MBOXES: [0] 68k DMA block, [1] Z80 unsafe, [2..] seq mailboxes
const uint32 kCommandFifo = 0x1B40;    // CMDFIFO, 64 bytes
const uint32 kChannelBlocks = 0x1B80;  // CCB, 16 x 32 bytes
const uint32 kEnvelopePage = 0x1E00;   // ENV0BUF = 0x1E80; ECBBUFP holds the low byte
const uint32 kDacFifo = 0x1F00;        // DACFIFO, 256 bytes
const int kCommandFifoMask = 0x3F;

// Channel control block (CCB) fields.
const int kCcbAddress = 3;    // CCBADDRL/M/H
const int kCcbTag = 0;        // CCBTAGL/M/H
const int kCcbFlags = 6;      // CCBFLAGS
const int kCcbTimer = 7;      // CCBTIMERL/H
const int kCcbDelay = 9;      // CCBDELL/H
const int kCcbDuration = 11;  // CCBDURL/H
const int kCcbPatch = 13;     // CCBPNUM
const int kCcbSequence = 14;  // CCBSNUM
const int kCcbVirtualChannel = 15; // CCBVCHAN
const int kCcbLoop0 = 16;     // CCBLOOP0
const int kCcbLoop3 = 25;     // CCBLOOP3
const int kCcbPriority = 28;  // CCBPRIO
const int kCcbEnvelope = 29;  // CCBENV
const int kCcbAttenuation = 30; // CCBATN
const int kCcbSize = 32;

// CCBFLAGS bits.
const uint8 kChannelSustain = 0x80;
const uint8 kChannelRetrigger = 0x40;
const uint8 kChannelLocked = 0x20;
const uint8 kChannelRunning = 0x10;
const uint8 kChannelSfxTimebase = 0x08;
const uint8 kChannelMuted = 0x02;
const uint8 kChannelInUse = 0x01;

// Voice table entries (FMVTBL, PSGVTBL, PSGVTBLNG).
const int kVoiceFlags = 0;     // FRLxxVVV: free, release, locked, voice number
const int kVoicePriority = 1;
const int kVoiceNote = 2;
const int kVoiceChannel = 3;
const int kVoiceDuration = 4;  // VTBLDL/H
const int kVoiceRelease = 6;   // VTBLRT
const int kVoiceEntrySize = 7;
const uint8 kVoiceFree = 0x80;
const uint8 kVoiceReleasing = 0x40;
const uint8 kVoiceLocked = 0x20;
const uint8 kVoiceSelfTimed = 0x10;
const uint8 kVoiceSfxTimebase = 0x08;
const uint8 kVoiceTableEnd = 0xFF;
const int kFmVoiceChannel6 = 4 * kVoiceEntrySize; // FMVTBLCH6
const int kFmVoiceChannel3 = 5 * kVoiceEntrySize; // FMVTBLCH3
const int kPsgVoiceTone3 = 2 * kVoiceEntrySize;   // PSGVTBLTG3

// PSG pseudo register groups (psgcom ... whdflg).
const int kPsgCommand = 0;
const int kPsgLevel = 4;
const int kPsgAttackRate = 8;
const int kPsgDecayRate = 12;
const int kPsgSustainLevel = 16;
const int kPsgReleaseRate = 20;
const int kPsgMode = 24;
const int kPsgToneLow = 28;
const int kPsgToneHigh = 32;
const int kPsgAttackLevel = 36;
const int kPsgUpdate = 40;

// PBTBL groups.
const int kPitchBendLow = 0;
const int kPitchBendHigh = 16;
const int kEnvelopeBendLow = 32;
const int kEnvelopeBendHigh = 48;
const int kBendRetrigger = 64;

// ECB groups.
const int kEnvelopeChannel = 0;  // [7]=eot, [6]=free, [5]=sfx timebase
const int kEnvelopePointerLow = 5;
const int kEnvelopePointerHigh = 9;
const int kEnvelopeCounter = 13;
const int kEnvelopeDeltaLow = 17;
const int kEnvelopeDeltaHigh = 21;
const int kEnvelopeBuffer = 25;

// Sample header cache (SAMPFLAGS, SAMPPTR, SAMPSKIP, SAMPFIRST, SAMPLOOP, SAMPEND).
const int kSampleFlags = 0;
const int kSamplePointer = 1;
const int kSampleSkip = 4;
const int kSampleFirst = 6;
const int kSampleLoop = 8;
const uint8 kSampleCompressed = 0x80;
const uint8 kSampleClipAtNoteOff = 0x20;
const uint8 kSampleLooped = 0x10;

const int kPatchSize = 39;
const uint8 kDefaultSubBeatsPerTick = 204; // SBPT: 120 bpm

// fmftbl: F-numbers for C..C of one octave.
const uint16 kFmFrequencies[13] = {644, 682, 723, 766, 811, 859, 910, 965, 1022, 1083, 1147, 1215, 1288};

// psgftbl: tone wavelengths for A2..B7, plus one entry for interpolating B7.
const uint16 kPsgWavelengths[64] = {
	0x03F9, 0x03C0, 0x038A,
	0x0357, 0x0327, 0x02FA, 0x02CF, 0x02A7, 0x0281, 0x025D, 0x023B, 0x021B, 0x01FC, 0x01E0, 0x01C5,
	0x01AC, 0x0194, 0x017D, 0x0168, 0x0153, 0x0140, 0x012E, 0x011D, 0x010D, 0x00FE, 0x00F0, 0x00E2,
	0x00D6, 0x00CA, 0x00BE, 0x00B4, 0x00AA, 0x00A0, 0x0097, 0x008F, 0x0087, 0x007F, 0x0078, 0x0071,
	0x006B, 0x0065, 0x005F, 0x005A, 0x0055, 0x0050, 0x004C, 0x0047, 0x0043, 0x0040, 0x003C, 0x0039,
	0x0035, 0x0032, 0x002F, 0x002D, 0x002A, 0x0028, 0x0026, 0x0023, 0x0021, 0x0020, 0x001E, 0x001C,
	0x001C};

// CARRIERTBL: carrier operator mask by algorithm, in FMADDRTBL total-level order.
const uint8 kCarrierMasks[8] = {0x08, 0x08, 0x08, 0x08, 0x0A, 0x0E, 0x0E, 0x0F};

// FMADDRTBL: register, patch offset (relative to the byte after the patch type). Offsets with bit 7
// set are total levels, which are scaled by the note attenuation when the operator is a carrier.
// Offset 0 writes zero (the SSG-EG register).
const uint8 kFmPatchRegisters[] = {
	0xB0, 2, 0xB4, 3,
	0x30, 4, 0x40, 133, 0x50, 6, 0x60, 7, 0x70, 8, 0x80, 9, 0x90, 0,
	0x38, 16, 0x48, 145, 0x58, 18, 0x68, 19, 0x78, 20, 0x88, 21, 0x98, 0,
	0x34, 10, 0x44, 139, 0x54, 12, 0x64, 13, 0x74, 14, 0x84, 15, 0x94, 0,
	0x3C, 22, 0x4C, 151, 0x5C, 24, 0x6C, 25, 0x7C, 26, 0x8C, 27, 0x9C, 0,
	0};

// Initial voice tables.
const uint8 kInitialFmVoices[6 * 7 + 1] = {
	0x80, 0, 0x50, 0, 0, 0, 0,
	0x81, 0, 0x50, 0, 0, 0, 0,
	0x84, 0, 0x50, 0, 0, 0, 0,
	0x85, 0, 0x50, 0, 0, 0, 0,
	0x86, 0, 0x50, 0, 0, 0, 0, // FMVTBLCH6, also the DAC voice
	0x82, 0, 0x50, 0, 0, 0, 0, // FMVTBLCH3, also the CH3 mode voice
	0xFF};
const uint8 kInitialPsgVoices[3 * 7 + 1] = {
	0x80, 0, 0x50, 0, 0, 0, 0,
	0x81, 0, 0x50, 0, 0, 0, 0,
	0x82, 0, 0x50, 0, 0, 0, 0, // PSGVTBLTG3
	0xFF};
const uint8 kInitialNoiseVoices[1 * 7 + 1] = {0x83, 0, 0x50, 0, 0, 0, 0, 0xFF};
const uint8 kInitialEnvelopeBlocks[29] = {
	0x40, 0x40, 0x40, 0x40, 0xFF,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0x80, 0xA0, 0xC0, 0xE0};

// Converts a YM2612 key-on voice code (0, 1, 2, 4, 5, 6) to its register part and channel.
void splitVoice(uint8 voice, uint8 &part, uint8 &channel) {
	if (voice < 3) {
		part = 0;
		channel = voice;
	} else {
		part = 1;
		channel = static_cast<uint8>(voice - 4);
	}
}

} // namespace

GemsDriver::GemsDriver(const ScoobyDooRom &rom, Audio::Mixer *mixer)
	: _rom(rom), _chip(nullptr), _ready(false), _cmdWritePtr(0), _cmdReadPtr(0), _ramDummy(0),
	  _currentPatch(0), _needBend(0), _subBeatsPerTick(kDefaultSubBeatsPerTick), _subBeatAccumulator(0),
	  _tickCount(0), _timebaseFlags(0), _masterAttenuation(0), _noteOnAttenuation(0), _noteOnVoice(0),
	  _carriers(0), _noteOnFrequency(0), _startSequenceNumber(0), _lowestPriorityVoice(0),
	  _lowestPriorityTable(nullptr), _dacmeEnabled(false), _fillEnabled(false), _dacReadPtr(0),
	  _dacWritePtr(0), _dacNibblePattern(0), _dacNibbleMode(false), _fdfState(0), _samplePtr(0),
	  _sampleCounter(0), _dacPeriod(2), _dacPhase(0), _channel3Mode(0) {
	_chip = new GenesisSoundChip(mixer, this);
}

GemsDriver::~GemsDriver() {
	if (_chip) {
		_chip->detachClient();
		delete _chip;
	}
}

bool GemsDriver::init(uint32 patchBank, uint32 envelopeBank, uint32 sequenceBank, uint32 sampleBank) {
	if (!_chip->init()) {
		warning("GemsDriver: failed to initialize the sound chip");
		return false;
	}

	Common::StackLock lock(_chip->mutex());
	// gemsloadz80 / gemsstartz80: reload the Z80 program and run it from main.
	resetDriver();
	_ready = true;

	// gemsinit: command 11 followed by the four 24-bit bank pointers.
	putCommandByte(0xFF);
	putCommandByte(11);
	const uint32 pointers[4] = {patchBank, envelopeBank, sequenceBank, sampleBank};
	for (int i = 0; i < 4; i++) {
		putCommandByte(static_cast<uint8>(pointers[i]));
		putCommandByte(static_cast<uint8>(pointers[i] >> 8));
		putCommandByte(static_cast<uint8>(pointers[i] >> 16));
	}
	processCommands();
	return true;
}

void GemsDriver::startSong(uint8 song) {
	Common::StackLock lock(_chip->mutex());
	putCommand(16, song);
}

void GemsDriver::stopSong(uint8 song) {
	Common::StackLock lock(_chip->mutex());
	putCommand(18, song);
}

void GemsDriver::pauseAll() {
	Common::StackLock lock(_chip->mutex());
	putCommand(12);
}

void GemsDriver::resumeAll() {
	Common::StackLock lock(_chip->mutex());
	putCommand(13);
}

void GemsDriver::stopAll() {
	Common::StackLock lock(_chip->mutex());
	putCommand(22);
}

void GemsDriver::setVolume(int volume) {
	_chip->setOutputVolume(volume);
}

// stdwrite: append one byte to the 64-byte command FIFO.
void GemsDriver::putCommandByte(uint8 value) {
	ram(kCommandFifo + _cmdWritePtr) = value;
	_cmdWritePtr = static_cast<uint8>((_cmdWritePtr + 1) & kCommandFifoMask);
}

// stdcmdwrite: a 0xFF start marker followed by the command number.
void GemsDriver::putCommand(uint8 command) {
	if (!_ready) {
		return;
	}
	putCommandByte(0xFF);
	putCommandByte(command);
	processCommands();
}

void GemsDriver::putCommand(uint8 command, uint8 argument) {
	if (!_ready) {
		return;
	}
	putCommandByte(0xFF);
	putCommandByte(command);
	putCommandByte(argument);
	processCommands();
}

uint8 &GemsDriver::ram(uint32 address) {
	if (address >= kHighRamBase && address < 0x2000) {
		return _highRam[address - kHighRamBase];
	}
	// Addresses outside the mirrored data area (only reachable through malformed data) read as zero.
	_ramDummy = 0;
	return _ramDummy;
}

uint8 *GemsDriver::ccb(int channel) {
	return &_highRam[kChannelBlocks - kHighRamBase + (channel & 0x0F) * kCcbSize];
}

uint8 *GemsDriver::patch(int channel) {
	return &_patchData[CLIP(channel, 0, 16) * kPatchSize];
}

uint8 GemsDriver::readRom(uint32 address) const {
	address &= 0xFFFFFF;
	if (address < static_cast<uint32>(_rom.size())) {
		return _rom.readByte(static_cast<int>(address));
	}
	return 0;
}

// Words in the GEMS banks are stored little-endian for the Z80.
uint16 GemsDriver::readRomWord(uint32 address) const {
	return static_cast<uint16>(readRom(address) | (readRom(address + 1) << 8));
}

uint32 GemsDriver::bankPointer(int index) const {
	return readPointer(&_bankPointers[index * 3]);
}

void GemsDriver::fmWrite(uint8 part, uint8 reg, uint8 value) {
	if (part == 0) {
		switch (reg) {
		case 0x24:
		case 0x25:
			// Timer A reload: the driver uses it to pace DAC output, which is emulated in onRenderSample().
			return;
		case 0x27:
			// Keep the vertical-blank timer running; only the CH3 mode bits come from the driver.
			_channel3Mode = value & 0xC0;
			_chip->writeReg(0, 0x27, static_cast<uint8>(_channel3Mode | 0x05));
			return;
		case 0x2A:
			_chip->setDacData(value);
			return;
		case 0x2B:
			_chip->setDacEnabled((value & 0x80) != 0);
			return;
		default:
			break;
		}
	}
	_chip->writeReg(part, reg, value);
}

void GemsDriver::fmKeyOff(uint8 voice) {
	fmWrite(0, 0x28, voice);
}

void GemsDriver::psgWrite(uint8 value) {
	_chip->writePsg(value);
}

// main: the state the driver is in after gemsloadz80 zero-fills Z80 RAM and main initializes it.
void GemsDriver::resetDriver() {
	memset(_highRam, 0, sizeof(_highRam));
	_cmdWritePtr = 0;
	_cmdReadPtr = 0;

	memset(_patchData, 0, sizeof(_patchData));
	for (int i = 0; i < 16; i++) {
		patch(i)[0] = 0xFF; // pinitloop: every patch buffer starts undefined
	}
	_currentPatch = 0;

	memset(_bankPointers, 0, sizeof(_bankPointers));
	memset(_psgRegisters, 0, sizeof(_psgRegisters));
	for (int i = 0; i < 4; i++) {
		_psgRegisters[kPsgLevel + i] = 0xFF;
	}
	memcpy(_fmVoices, kInitialFmVoices, sizeof(_fmVoices));
	memcpy(_psgVoices, kInitialPsgVoices, sizeof(_psgVoices));
	memcpy(_noiseVoices, kInitialNoiseVoices, sizeof(_noiseVoices));
	memset(_pitchBends, 0, sizeof(_pitchBends));
	memcpy(_envelopeBlocks, kInitialEnvelopeBlocks, sizeof(_envelopeBlocks));
	_needBend = 0;

	_subBeatsPerTick = kDefaultSubBeatsPerTick;
	_subBeatAccumulator = 0;
	_tickCount = 0;
	_timebaseFlags = 0;
	_masterAttenuation = 0;
	_noteOnAttenuation = 0;
	_noteOnVoice = 0;
	_carriers = 0;
	_noteOnFrequency = 0;
	_startSequenceNumber = 0;
	_lowestPriorityTable = nullptr;
	_lowestPriorityVoice = 0;

	memset(_sampleHeader, 0, sizeof(_sampleHeader));
	_dacmeEnabled = false; // DACME starts disabled (RET)
	_fillEnabled = false;  // FILLDACFIFO starts disabled (RET)
	_dacReadPtr = 0;
	_dacWritePtr = 0;
	_dacNibblePattern = 0;
	_dacNibbleMode = false;
	_fdfState = 0;
	_samplePtr = 0;
	_sampleCounter = 0;
	_dacPeriod = 2;
	_dacPhase = 0;
	_channel3Mode = 0;
	_chip->setDacEnabled(false);

	// main: silence the four PSG voices.
	psgWrite(0x9F);
	psgWrite(0xBF);
	psgWrite(0xDF);
	psgWrite(0xFF);
}

// GETCBYTE. The 68000 library writes complete commands, so the FIFO never runs dry mid-command.
uint8 GemsDriver::getCByte() {
	if (_cmdReadPtr == _cmdWritePtr) {
		return 0;
	}
	uint8 value = ram(kCommandFifo + _cmdReadPtr);
	_cmdReadPtr = static_cast<uint8>((_cmdReadPtr + 1) & kCommandFifoMask);
	return value;
}

// GETCCBPTR: the channel byte selects one of the 16 CCBs (only the low four bits survive the shift).
int GemsDriver::getCcbChannel() {
	return getCByte() & 0x0F;
}

// The command dispatcher of the main loop.
void GemsDriver::processCommands() {
	while (_cmdReadPtr != _cmdWritePtr) {
		if (getCByte() != 0xFF) {
			continue; // not the start of a command
		}

		uint8 command = getCByte();
		switch (command) {
		case 0: // cmdnoteon
			commandNoteOn();
			break;
		case 1: // cmdnoteoff
			commandNoteOff();
			break;
		case 2: { // cmdpchange (PCHANGE)
			int channel = getCcbChannel();
			_currentPatch = channel;
			ccb(channel)[kCcbPatch] = getCByte();
			fetchPatch(channel);
			break;
		}
		case 3: // cmdpupdate
			patchLoad(getCByte());
			break;
		case 4: { // cmdpbend
			int channel = getCByte() & 0x0F;
			uint8 low = getCByte();
			uint8 high = getCByte();
			setPitchBend(channel, low, high);
			break;
		}
		case 5: // cmdtempo
			setTempo(getCByte());
			break;
		case 6: { // cmdenv
			int channel = getCcbChannel();
			uint8 *block = ccb(channel);
			uint8 envelope = getCByte();
			block[kCcbEnvelope] = envelope;
			if (!(block[kCcbFlags] & kChannelRetrigger)) {
				triggerEnvelope(envelope, static_cast<uint8>(channel), block[kCcbFlags]);
			}
			break;
		}
		case 7: { // cmdretrig
			uint8 *block = ccb(getCcbChannel());
			if (getCByte()) {
				block[kCcbFlags] |= kChannelRetrigger;
			} else {
				block[kCcbFlags] &= ~kChannelRetrigger;
			}
			break;
		}
		case 11: // cmdgetptrs
			for (int i = 0; i < 12; i++) {
				_bankPointers[i] = getCByte();
			}
			break;
		case 12: // cmdpause
			commandPause();
			break;
		case 13: // cmdresume
			resumeAllSequences();
			break;
		case 14: { // cmdsussw
			uint8 *block = ccb(getCcbChannel());
			if (getCByte()) {
				block[kCcbFlags] |= kChannelSustain;
			} else {
				block[kCcbFlags] &= ~kChannelSustain;
			}
			break;
		}
		case 16: // cmdstartseq
			startSequence(getCByte());
			break;
		case 18: // cmdstopseq
			stopSequence(getCByte());
			break;
		case 20: { // cmdsetprio
			uint8 *block = ccb(getCcbChannel());
			block[kCcbPriority] = getCByte();
			break;
		}
		case 22: // cmdstopall
			for (int channel = 0; channel < 16; channel++) {
				uint8 *block = ccb(channel);
				block[kCcbFlags] = 0;
				writeWord(&block[kCcbDuration], 0);
			}
			clipAll();
			break;
		case 23: // cmdmute
			commandMute();
			break;
		case 26: // cmdsamprate
			commandSampleRate();
			break;
		case 27: { // cmdstore
			uint8 mailbox = getCByte();
			ram(kMailboxes + 2 + mailbox) = getCByte();
			break;
		}
		case 28: // cmdlockch
			ccb(getCcbChannel())[kCcbFlags] |= kChannelLocked;
			break;
		case 29: // cmdunlockch
			ccb(getCcbChannel())[kCcbFlags] &= ~kChannelLocked;
			break;
		case 30: // cmdpbendvch
			commandPitchBendVirtualChannel();
			break;
		case 31: { // cmdvolume
			uint8 *block = ccb(getCcbChannel());
			block[kCcbAttenuation] = getCByte();
			break;
		}
		case 32: // cmdmasteratn
			_masterAttenuation = getCByte();
			break;
		default:
			break;
		}
	}
}

void GemsDriver::commandNoteOn() {
	int channel = getCcbChannel();
	_currentPatch = channel;
	uint8 note = getCByte();
	noteOn(channel, note);
}

void GemsDriver::commandNoteOff() {
	uint8 channel = getCByte();
	uint8 note = getCByte();
	noteOff(channel, note);
}

void GemsDriver::commandPitchBendVirtualChannel() {
	uint8 sequence = getCByte();
	uint8 virtualChannel = getCByte();
	for (int channel = 0; channel < 16; channel++) {
		uint8 *block = ccb(channel);
		if ((block[kCcbFlags] & kChannelInUse) && block[kCcbSequence] == sequence &&
			block[kCcbVirtualChannel] == virtualChannel) {
			uint8 low = getCByte();
			uint8 high = getCByte();
			setPitchBend(channel, low, high);
			return;
		}
	}
	getCByte();
	getCByte();
}

void GemsDriver::commandPause() {
	for (int channel = 0; channel < 16; channel++) {
		ccb(channel)[kCcbFlags] &= ~kChannelRunning;
	}
	clipAll();
}

void GemsDriver::commandMute() {
	uint8 sequence = getCByte();
	uint8 virtualChannel = getCByte();
	uint8 mute = getCByte();
	for (int channel = 0; channel < 16; channel++) {
		uint8 *block = ccb(channel);
		if (!(block[kCcbFlags] & kChannelInUse) || block[kCcbSequence] != sequence ||
			block[kCcbVirtualChannel] != virtualChannel) {
			continue;
		}
		if (mute & 1) {
			block[kCcbFlags] |= kChannelMuted;
		} else {
			block[kCcbFlags] &= ~kChannelMuted;
		}
	}
}

void GemsDriver::commandSampleRate() {
	uint8 *patchData = patch(getCcbChannel());
	uint8 rate = getCByte();
	if (patchData[0] == 1) {
		patchData[1] = rate;
	}
}

// VBLINT + CHECKTICK, followed by the tick and beat handling of the main loop. The Z80 loop handles
// at most one 60 Hz tick and one sub-beat per pass, so passes repeat until both counters drain.
void GemsDriver::onVerticalBlank() {
	if (!_ready) {
		return;
	}

	_tickCount++;
	_subBeatAccumulator = static_cast<uint16>(_subBeatAccumulator + _subBeatsPerTick);
	doTicks();
}

void GemsDriver::doTicks() {
	for (;;) {
		uint8 flags = 0;
		if (_tickCount) {
			_tickCount--;
			doPsgEnvelopes();
			flags |= 1;
		}
		if (_subBeatAccumulator >= 0x100) {
			_subBeatAccumulator = static_cast<uint16>(_subBeatAccumulator - 0x100);
			flags |= 2;
		}
		if (flags) {
			_timebaseFlags = flags;
			doEnvelopes();
			voiceTimers();
			updateSequencers();
		}
		applyBends();
		if (!flags) {
			break;
		}
	}
}

// DACME is polled continuously by the Z80 and outputs one sample each time Timer A overflows.
void GemsDriver::onRenderSample() {
	if (!_dacmeEnabled) {
		return;
	}
	if (++_dacPhase < _dacPeriod) {
		return;
	}
	_dacPhase = 0;
	dacSample();
	fillDacFifo();
}

// ************************************ SEQUENCER CODE ************************************

// GETSBYTE
uint8 GemsDriver::getSByte(uint8 *channelBlock) {
	uint32 address = readPointer(&channelBlock[kCcbAddress]);
	uint8 value = readRom(address);
	address = (address + 1) & 0xFFFFFF;
	channelBlock[kCcbAddress] = static_cast<uint8>(address);
	channelBlock[kCcbAddress + 1] = static_cast<uint8>(address >> 8);
	channelBlock[kCcbAddress + 2] = static_cast<uint8>(address >> 16);
	return value;
}

// UPDSEQ
void GemsDriver::updateSequencers() {
	uint8 timebase = _timebaseFlags;
	for (int channel = 0; channel < 16; channel++) {
		_currentPatch = channel;
		uint8 flags = ccb(channel)[kCcbFlags];
		if (!(flags & kChannelRunning)) {
			continue;
		}
		bool due = (flags & kChannelSfxTimebase) ? (timebase & 1) != 0 : (timebase & 2) != 0;
		if (due) {
			runSequencer(channel);
		}
	}
}

// SEQUENCER
void GemsDriver::runSequencer(int channel) {
	uint8 *block = ccb(channel);
	uint32 blockAddress = kChannelBlocks + channel * kCcbSize;

	uint16 timer = static_cast<uint16>(readWord(&block[kCcbTimer]) + 1);
	writeWord(&block[kCcbTimer], timer);
	if (timer != 0) {
		return;
	}

	uint8 value = getSByte(block); // seqcmdloop0
	for (;;) {
		// seqcmdloop: dispatch on command type.
		if (value & 0x80) {
			// seqdel / seqdur: runs of delay (11xxxxxx) or duration (10xxxxxx) bytes form one value.
			bool isDelay = (value & 0x40) != 0;
			uint16 data = value & 0x3F;
			for (;;) {
				value = getSByte(block);
				if (!(value & 0x80) || ((value & 0x40) != 0) != isDelay) {
					break;
				}
				data = static_cast<uint16>((data << 6) | (value & 0x3F));
			}
			writeWord(&block[isDelay ? kCcbDelay : kCcbDuration], static_cast<uint16>(-data));
			continue;
		}

		bool toDelay = true;
		if (value < 96) {
			// seqnote
			if (!(block[kCcbFlags] & kChannelMuted)) {
				noteOn(channel, value);
			}
		} else {
			switch (value - 96) {
			case 0: // seqeos
				block[kCcbFlags] = 0;
				writeWord(&block[kCcbDuration], 0);
				return;
			case 1: // seqpchange
				block[kCcbPatch] = getSByte(block);
				fetchPatch(channel);
				break;
			case 2: // seqenv
				block[kCcbEnvelope] = getSByte(block);
				if (!(block[kCcbFlags] & kChannelRetrigger)) {
					triggerEnvelope(block[kCcbEnvelope], static_cast<uint8>(channel), block[kCcbFlags]);
				}
				break;
			case 3: // nop
				break;
			case 4: { // seqsloop: claim the first free loop stack entry
				uint32 entry = blockAddress + kCcbLoop0;
				while (entry < 0x2000 && ram(entry) != 0) {
					entry += 3;
				}
				ram(entry) = getSByte(block);
				ram(entry + 1) = block[kCcbAddress];
				ram(entry + 2) = block[kCcbAddress + 1];
				toDelay = false;
				break;
			}
			case 5: { // seqeloop: use the last active loop stack entry
				uint32 entry = blockAddress + kCcbLoop3;
				while (entry >= kHighRamBase && ram(entry) == 0) {
					entry -= 3;
				}
				toDelay = false;
				if (entry < kHighRamBase) {
					break;
				}
				uint8 count = ram(entry);
				if (count != 127) {
					count--;
					ram(entry) = count;
					if (count == 0) {
						break; // end of a finite loop
					}
				}
				uint16 loopAddress = static_cast<uint16>(ram(entry + 1) | (ram(entry + 2) << 8));
				uint16 currentAddress = readWord(&block[kCcbAddress]);
				writeWord(&block[kCcbAddress], loopAddress);
				if (loopAddress >= currentAddress) {
					block[kCcbAddress + 2]--; // the loop start lies in the previous 64k page
				}
				break;
			}
			case 6: // seqretrig
				if (getSByte(block)) {
					block[kCcbFlags] |= kChannelRetrigger;
				} else {
					block[kCcbFlags] &= ~kChannelRetrigger;
				}
				break;
			case 7: // seqsus
				if (getSByte(block)) {
					block[kCcbFlags] |= kChannelSustain;
				} else {
					block[kCcbFlags] &= ~kChannelSustain;
				}
				break;
			case 8: // seqtempo: offset by -40
				setTempo(static_cast<uint8>(getSByte(block) + 40));
				break;
			case 9: { // seqmute: [4] set unmutes, [3:0] is the MIDI channel
				uint8 data = getSByte(block);
				for (int other = 0; other < 16; other++) {
					uint8 *otherBlock = ccb(other);
					if (!(otherBlock[kCcbFlags] & kChannelInUse) ||
						otherBlock[kCcbSequence] != block[kCcbSequence] ||
						otherBlock[kCcbVirtualChannel] != (data & 0x0F)) {
						continue;
					}
					if (data & 0x10) {
						otherBlock[kCcbFlags] &= ~kChannelMuted;
					} else {
						otherBlock[kCcbFlags] |= kChannelMuted;
					}
					break;
				}
				break;
			}
			case 10: // seqprio
				block[kCcbPriority] = getSByte(block);
				break;
			case 11: // seqssong
				startSequence(getSByte(block));
				break;
			case 12: { // seqpbend: 16-bit signed, 8 fraction bits, in semitones
				uint8 low = getSByte(block);
				uint8 high = getSByte(block);
				setPitchBend(channel, low, high);
				break;
			}
			case 13: // seqsfx
				block[kCcbFlags] |= kChannelSfxTimebase;
				break;
			case 14: { // seqsamprate
				uint8 rate = getSByte(block);
				uint8 *patchData = patch(_currentPatch);
				if (patchData[0] == 1) {
					patchData[1] = rate;
				}
				break;
			}
			case 15: { // seqgoto: 16-bit signed offset from the byte after the operand
				uint8 low = getSByte(block);
				uint8 high = getSByte(block);
				int32 offset = static_cast<int16>(low | (high << 8));
				uint32 address = (readPointer(&block[kCcbAddress]) + offset) & 0xFFFFFF;
				block[kCcbAddress] = static_cast<uint8>(address);
				block[kCcbAddress + 1] = static_cast<uint8>(address >> 8);
				block[kCcbAddress + 2] = static_cast<uint8>(address >> 16);
				toDelay = false;
				break;
			}
			case 16: { // seqstore
				uint8 mailbox = getSByte(block);
				ram(kMailboxes + 2 + mailbox) = getSByte(block);
				break;
			}
			case 17: { // seqif: mailbox, relation, value, skip offset
				uint8 mailbox = getSByte(block);
				uint8 relation = getSByte(block);
				uint8 compare = getSByte(block);
				uint8 stored = ram(kMailboxes + 2 + mailbox);
				bool doIt;
				switch (relation) {
				case 1:
					doIt = stored != compare;
					break;
				case 2:
					doIt = stored > compare;
					break;
				case 3:
					doIt = stored >= compare;
					break;
				case 4:
					doIt = stored < compare;
					break;
				case 5:
					doIt = stored <= compare;
					break;
				default:
					doIt = stored == compare;
					break;
				}
				uint8 skip = getSByte(block);
				if (!doIt) {
					uint32 address = (readPointer(&block[kCcbAddress]) + skip) & 0xFFFFFF;
					block[kCcbAddress] = static_cast<uint8>(address);
					block[kCcbAddress + 1] = static_cast<uint8>(address >> 8);
					block[kCcbAddress + 2] = static_cast<uint8>(address >> 16);
				}
				toDelay = false;
				break;
			}
			case 18: { // seqseekrit: extra functions
				uint8 code = getSByte(block);
				uint8 data = getSByte(block);
				switch (code) {
				case 0:
					stopSequence(data);
					break;
				case 1:
					pauseSequence(data);
					break;
				case 2:
					resumeAllSequences();
					break;
				case 3:
					pauseSequence(ram(kMailboxes + 2));
					break;
				case 4:
					_masterAttenuation = data;
					break;
				case 5:
					block[kCcbAttenuation] = data;
					break;
				default:
					break;
				}
				break;
			}
			default:
				// Unknown command: fetch the next command without a delay.
				toDelay = false;
				break;
			}
		}

		if (toDelay) {
			// seqdelay: a non-zero delay ends this pass.
			uint16 delay = readWord(&block[kCcbDelay]);
			if (delay != 0) {
				writeWord(&block[kCcbTimer], delay);
				return;
			}
		}
		value = getSByte(block); // seqcmdloop0
	}
}

// STARTSEQ: a free CCB is allocated for each track of the sequence.
void GemsDriver::startSequence(uint8 sequence) {
	_startSequenceNumber = sequence;
	uint32 sequenceTable = bankPointer(2);
	uint32 header = (sequenceTable + readRomWord(sequenceTable + sequence * 2)) & 0xFFFFFF;
	uint8 trackCount = readRom(header);
	if (trackCount == 0) {
		return;
	}

	uint8 started = 0;
	int track = 1;
	for (int channel = 0; channel < 16; channel++) {
		uint8 *block = ccb(channel);
		if (block[kCcbFlags] & (kChannelInUse | kChannelLocked)) {
			continue;
		}
		block[kCcbFlags] = kChannelInUse | kChannelRunning;

		uint16 trackOffset = static_cast<uint16>(readRom(header + track) | (readRom(header + track + 1) << 8));
		track += 2;
		uint32 address = (sequenceTable + trackOffset) & 0xFFFFFF;
		block[kCcbAddress] = static_cast<uint8>(address);
		block[kCcbAddress + 1] = static_cast<uint8>(address >> 8);
		block[kCcbAddress + 2] = static_cast<uint8>(address >> 16);
		block[kCcbTag] = block[kCcbTag + 1] = block[kCcbTag + 2] = 0xFF;
		writeWord(&block[kCcbTimer], 0xFFFF); // trigger the sequencer on the next tick
		block[kCcbSequence] = _startSequenceNumber;
		block[kCcbVirtualChannel] = started;
		block[kCcbLoop0] = 0;
		block[kCcbLoop0 + 3] = 0;
		block[kCcbLoop0 + 6] = 0;
		block[kCcbLoop3] = 0;
		block[kCcbEnvelope] = 0;
		block[kCcbPriority] = 0;
		block[kCcbAttenuation] = 0;
		_pitchBends[kEnvelopeBendLow + channel] = 0;
		_pitchBends[kEnvelopeBendHigh + channel] = 0;
		_pitchBends[kPitchBendLow + channel] = 0;
		_pitchBends[kPitchBendHigh + channel] = 0;

		started++;
		if (started == trackCount) {
			return;
		}
	}
}

// STOPSEQ: stops every occurrence of a sequence; 255 stops all unlocked channels.
void GemsDriver::stopSequence(uint8 sequence) {
	for (int channel = 0; channel < 16; channel++) {
		uint8 *block = ccb(channel);
		if (!(block[kCcbFlags] & kChannelInUse) || (block[kCcbFlags] & kChannelLocked)) {
			continue;
		}
		if (sequence == 255 || sequence == block[kCcbSequence]) {
			block[kCcbFlags] = 0;
			writeWord(&block[kCcbDuration], 0);
		}
	}
	clipAll();
}

// PAUSESEQ
void GemsDriver::pauseSequence(uint8 sequence) {
	for (int channel = 0; channel < 16; channel++) {
		uint8 *block = ccb(channel);
		if (!(block[kCcbFlags] & kChannelInUse) || (block[kCcbFlags] & kChannelLocked)) {
			continue;
		}
		if (sequence == block[kCcbSequence]) {
			block[kCcbFlags] &= ~kChannelRunning;
		}
	}
	clipAll();
}

// RESUMEALL
void GemsDriver::resumeAllSequences() {
	for (int channel = 0; channel < 16; channel++) {
		uint8 *block = ccb(channel);
		if (block[kCcbFlags] & kChannelLocked) {
			continue;
		}
		if (block[kCcbFlags] & kChannelInUse) {
			block[kCcbFlags] |= kChannelRunning;
		}
	}
}

// CLIPALL: cuts off voices and envelopes owned by channels that are no longer running.
void GemsDriver::clipAll() {
	clipVoiceTable(_fmVoices, false);
	clipVoiceTable(_psgVoices, true);
	clipVoiceTable(_noiseVoices, true);

	for (int i = 0;; i++) {
		uint8 owner = _envelopeBlocks[kEnvelopeChannel + i];
		if (owner & 0x80) {
			return;
		}
		if (owner & 0x40) {
			continue;
		}
		if (ccb(owner & 0x0F)[kCcbFlags] & kChannelRunning) {
			continue;
		}
		_envelopeBlocks[kEnvelopeChannel + i] |= 0x40;
	}
}

// CLIPLOOP
void GemsDriver::clipVoiceTable(uint8 *table, bool psg) {
	for (uint8 *entry = table; entry[kVoiceFlags] != kVoiceTableEnd; entry += kVoiceEntrySize) {
		uint8 flags = entry[kVoiceFlags];
		if (ccb(entry[kVoiceChannel] & 0x0F)[kCcbFlags] & kChannelRunning) {
			continue;
		}

		uint8 voice = flags & 7;
		entry[kVoiceFlags] = voice | kVoiceFree;
		entry[kVoiceDuration] = 0;
		entry[kVoiceDuration + 1] = 0;
		entry[kVoiceRelease] = 0;

		if (psg) {
			_psgRegisters[kPsgCommand + voice] = 4; // stop
			continue;
		}

		if (flags & kVoiceLocked) {
			// clipdig: the DAC voice.
			_dacmeEnabled = false;
			_fillEnabled = false;
			_chip->setDacEnabled(false);
			continue;
		}

		uint8 part, channel;
		splitVoice(voice, part, channel);
		fmWrite(part, static_cast<uint8>(0x40 + channel), 0x7F); // clamp all EGs
		fmWrite(part, static_cast<uint8>(0x44 + channel), 0x7F);
		fmWrite(part, static_cast<uint8>(0x48 + channel), 0x7F);
		fmWrite(part, static_cast<uint8>(0x4C + channel), 0x7F);
		fmKeyOff(voice);
	}
}

// SETTEMPO: SBPT holds 1/24 beats per 1/60 s tick with 8 fraction bits.
void GemsDriver::setTempo(uint8 beatsPerMinute) {
	uint32 product = static_cast<uint32>(beatsPerMinute) * 218u;
	_subBeatsPerTick = static_cast<uint16>(((product & 0xFFFF) << 1) >> 8);
}

// FETCHPATCH: copies the channel's program into the current patch buffer (CHPATPTR).
void GemsDriver::fetchPatch(int channel) {
	uint32 patchTable = bankPointer(0);
	uint32 source = (patchTable + readRomWord(patchTable + ccb(channel)[kCcbPatch] * 2)) & 0xFFFFFF;
	uint8 *destination = patch(_currentPatch);
	for (int i = 0; i < kPatchSize; i++) {
		destination[i] = readRom(source + i);
	}
}

// PATCHLOAD: reloads a program into every channel that uses it.
void GemsDriver::patchLoad(uint8 patchNumber) {
	_currentPatch = 0;
	for (int channel = 0; channel < 16; channel++) {
		if (ccb(channel)[kCcbPatch] == patchNumber) {
			fetchPatch(channel);
		}
		_currentPatch++;
	}
}

// VTIMER: release timers and note durations.
void GemsDriver::voiceTimers() {
	voiceTimerTable(_fmVoices, false);
	voiceTimerTable(_psgVoices, true);
	voiceTimerTable(_noiseVoices, true);
}

void GemsDriver::voiceTimerTable(uint8 *table, bool psg) {
	uint8 timebase = _timebaseFlags;
	for (uint8 *entry = table; entry[kVoiceFlags] != kVoiceTableEnd; entry += kVoiceEntrySize) {
		uint8 flags = entry[kVoiceFlags];
		bool due = (flags & kVoiceSfxTimebase) ? (timebase & 1) != 0 : (timebase & 2) != 0;
		if (!due) {
			continue;
		}

		if (flags & kVoiceReleasing) {
			if (--entry[kVoiceRelease] == 0) {
				entry[kVoiceFlags] = flags & ~kVoiceReleasing;
			}
			continue;
		}
		if (!(flags & kVoiceSelfTimed)) {
			continue;
		}

		uint8 voice = flags & 7;
		uint16 timer = static_cast<uint16>(readWord(&entry[kVoiceDuration]) + 1);
		writeWord(&entry[kVoiceDuration], timer);
		if (timer != 0) {
			continue;
		}

		// Timed out: clear the self-timer and sfx bits, then note off.
		entry[kVoiceFlags] &= ~(kVoiceSelfTimed | kVoiceSfxTimebase);
		if ((entry[kVoiceFlags] & 0x2F) == 0x26) {
			noteOffDigital(); // voice 6, locked: a digital note
			continue;
		}
		entry[kVoiceFlags] |= kVoiceReleasing | kVoiceFree;
		if (psg) {
			_psgRegisters[kPsgCommand + voice] |= 2; // key off
		} else {
			fmKeyOff(voice);
		}
	}
}

// ************************************ ENVELOPES AND PITCH ************************************

// TRIGENV
void GemsDriver::triggerEnvelope(uint8 envelope, uint8 channel, uint8 channelFlags) {
	int block = -1;
	// First see if an envelope already exists for this channel.
	for (int i = 0;; i++) {
		uint8 owner = _envelopeBlocks[kEnvelopeChannel + i];
		if (owner & 0x80) {
			break;
		}
		if (owner == channel) {
			block = i;
			break;
		}
	}
	// Then try to find a free one.
	if (block < 0) {
		for (int i = 0;; i++) {
			uint8 owner = _envelopeBlocks[kEnvelopeChannel + i];
			if (owner & 0x80) {
				return;
			}
			if (owner & 0x40) {
				block = i;
				break;
			}
		}
	}

	uint8 owner = channel;
	if (channelFlags & kChannelSfxTimebase) {
		owner |= 0x20;
	}
	_envelopeBlocks[kEnvelopeChannel + block] = owner;
	_envelopeBlocks[kEnvelopeCounter + block] = 0;

	uint32 envelopeTable = bankPointer(1);
	uint32 source = (envelopeTable + readRomWord(envelopeTable + envelope * 2)) & 0xFFFFFF;
	uint32 buffer = kEnvelopePage + _envelopeBlocks[kEnvelopeBuffer + block];
	for (int i = 0; i < 32; i++) {
		ram(buffer + i) = readRom(source + i);
	}

	int bendChannel = channel & 0x0F;
	_pitchBends[kEnvelopeBendLow + bendChannel] = ram(buffer);
	_pitchBends[kEnvelopeBendHigh + bendChannel] = ram(buffer + 1);
	buffer += 2;
	_envelopeBlocks[kEnvelopePointerLow + block] = static_cast<uint8>(buffer);
	_envelopeBlocks[kEnvelopePointerHigh + block] = static_cast<uint8>(buffer >> 8);
}

// DOENVELOPE
void GemsDriver::doEnvelopes() {
	uint8 timebase = _timebaseFlags;
	for (int i = 0;; i++) {
		uint8 owner = _envelopeBlocks[kEnvelopeChannel + i];
		if (owner & 0x80) {
			return;
		}
		if (owner & 0x40) {
			continue;
		}
		bool due = (owner & 0x20) ? (timebase & 1) != 0 : (timebase & 2) != 0;
		if (!due) {
			continue;
		}

		int channel = owner & 0x0F;
		uint8 counter = _envelopeBlocks[kEnvelopeCounter + i];
		if (counter != 0) {
			counter--;
		} else {
			// envnextseg
			uint32 pointer = static_cast<uint32>(_envelopeBlocks[kEnvelopePointerLow + i] |
												 (_envelopeBlocks[kEnvelopePointerHigh + i] << 8));
			uint8 segmentCount = ram(pointer);
			if (segmentCount == 0) {
				// envdone
				_pitchBends[kEnvelopeBendLow + channel] = 0;
				_pitchBends[kEnvelopeBendHigh + channel] = 0;
				_envelopeBlocks[kEnvelopeChannel + i] = 0x40;
				_pitchBends[kBendRetrigger + channel] |= 1;
				_needBend = 1;
				continue;
			}
			counter = static_cast<uint8>(segmentCount - 1);
			_envelopeBlocks[kEnvelopeDeltaLow + i] = ram(pointer + 1);
			_envelopeBlocks[kEnvelopeDeltaHigh + i] = ram(pointer + 2);
			pointer += 3;
			_envelopeBlocks[kEnvelopePointerLow + i] = static_cast<uint8>(pointer);
			_envelopeBlocks[kEnvelopePointerHigh + i] = static_cast<uint8>(pointer >> 8);
		}

		// envseg: this channel's envelope bend += this segment's delta.
		_envelopeBlocks[kEnvelopeCounter + i] = counter;
		uint16 bend = static_cast<uint16>(_pitchBends[kEnvelopeBendLow + channel] |
										  (_pitchBends[kEnvelopeBendHigh + channel] << 8));
		uint16 delta = static_cast<uint16>(_envelopeBlocks[kEnvelopeDeltaLow + i] |
										   (_envelopeBlocks[kEnvelopeDeltaHigh + i] << 8));
		bend = static_cast<uint16>(bend + delta);
		_pitchBends[kEnvelopeBendLow + channel] = static_cast<uint8>(bend);
		_pitchBends[kEnvelopeBendHigh + channel] = static_cast<uint8>(bend >> 8);
		_pitchBends[kBendRetrigger + channel] |= 1;
		_needBend = 1;
	}
}

// DOPITCHBEND
void GemsDriver::setPitchBend(int channel, uint8 low, uint8 high) {
	channel &= 0x0F;
	_pitchBends[kPitchBendLow + channel] = low;
	_pitchBends[kPitchBendHigh + channel] = high;
	_pitchBends[kBendRetrigger + channel] |= 1;
	_needBend = 1;
}

// APPLYBEND
void GemsDriver::applyBends() {
	if (!_needBend) {
		return;
	}
	_needBend = 0;

	for (uint8 *entry = _fmVoices; entry[kVoiceFlags] != kVoiceTableEnd; entry += kVoiceEntrySize) {
		uint8 voice = entry[kVoiceFlags] & 7;
		int channel = entry[kVoiceChannel] & 0x0F;
		if (!(_pitchBends[kBendRetrigger + channel] & 1)) {
			continue;
		}
		uint16 frequency = getFrequency(false, entry[kVoiceNote], channel);
		uint8 part, fmChannel;
		splitVoice(voice, part, fmChannel);
		fmWrite(part, static_cast<uint8>(0xA4 + fmChannel), static_cast<uint8>(frequency >> 8));
		fmWrite(part, static_cast<uint8>(0xA0 + fmChannel), static_cast<uint8>(frequency));
	}

	for (uint8 *entry = _psgVoices; entry[kVoiceFlags] != kVoiceTableEnd; entry += kVoiceEntrySize) {
		uint8 voice = entry[kVoiceFlags] & 7;
		int channel = entry[kVoiceChannel] & 0x0F;
		if (!(_pitchBends[kBendRetrigger + channel] & 1)) {
			continue;
		}
		uint16 wavelength = getFrequency(true, entry[kVoiceNote], channel);
		psgWrite(static_cast<uint8>((wavelength & 0x0F) | 0x80 | (voice << 5)));
		psgWrite(static_cast<uint8>(wavelength >> 4));
	}

	for (int channel = 0; channel < 16; channel++) {
		_pitchBends[kBendRetrigger + channel] = 0;
	}
}

// GETFREQ: an FM block/F-number or PSG wavelength for a note plus the channel's pitch and
// envelope bends (semitones with 8 fraction bits), interpolated linearly between table entries.
uint16 GemsDriver::getFrequency(bool psg, uint8 note, int channel) {
	channel &= 0x0F;
	uint16 bend = static_cast<uint16>(
		(_pitchBends[kPitchBendLow + channel] | (_pitchBends[kPitchBendHigh + channel] << 8)) +
		(_pitchBends[kEnvelopeBendLow + channel] | (_pitchBends[kEnvelopeBendHigh + channel] << 8)));
	uint8 fraction = static_cast<uint8>(bend);
	uint8 semitone = static_cast<uint8>(note + (bend >> 8));
	if (semitone >= 96) {
		if (bend & 0x8000) {
			semitone = 0; // bent down: peg at C0
			fraction = 0;
		} else {
			semitone = 95; // bent up: peg at B7 with the maximum fraction
			fraction = 0xFF;
		}
	}

	const uint16 *table;
	uint8 octave = 0;
	if (!psg) {
		if (semitone >= 48) {
			semitone -= 48;
			octave |= 4;
		}
		if (semitone >= 24) {
			semitone -= 24;
			octave |= 2;
		}
		if (semitone >= 12) {
			semitone -= 12;
			octave |= 1;
		}
		table = kFmFrequencies;
	} else {
		if (semitone < 33) {
			semitone = 0; // the lowest PSG note is A2
			fraction = 0;
		} else {
			semitone -= 33;
		}
		table = kPsgWavelengths;
	}

	// MULTIPLY is an unsigned 8x16 multiply; the high byte is then used as a signed step.
	uint16 delta = static_cast<uint16>(table[semitone + 1] - table[semitone]);
	uint16 product = static_cast<uint16>(fraction * delta);
	int8 step = static_cast<int8>(product >> 8);
	uint16 value = static_cast<uint16>(table[semitone] + step);
	if (!psg) {
		value = static_cast<uint16>(value | (octave << 11));
	}
	return value;
}

// ************************************ NOTES ************************************

// NOTEON
void GemsDriver::noteOn(int channel, uint8 note) {
	uint8 *block = ccb(channel);
	// Sum the master and channel attenuations, limited to 127.
	uint8 attenuation = static_cast<uint8>(_masterAttenuation + block[kCcbAttenuation]);
	if (attenuation & 0x80) {
		attenuation = 127;
	}
	_noteOnAttenuation = attenuation;

	switch (patch(_currentPatch)[0]) {
	case 0:
		noteOnFm(channel, note);
		break;
	case 1:
		noteOnDigital(channel, note);
		break;
	case 2:
		noteOnPsg(channel, note, false);
		break;
	case 3:
		noteOnPsg(channel, note, true);
		break;
	default:
		break;
	}
}

// ALLOC: dynamic voice allocation. Returns the allocated entry, or nullptr.
int GemsDriver::allocateVoice(uint8 *table, int channel) {
	uint8 *block = ccb(channel);
	uint8 lowestPriority = 0xFF;
	uint8 freest = 0xFF;
	int freestOffset = -1;

	for (int offset = 0; table[offset + kVoiceFlags] != kVoiceTableEnd; offset += kVoiceEntrySize) {
		uint8 *entry = &table[offset];
		uint8 flags = entry[kVoiceFlags];
		if (flags & kVoiceLocked) {
			continue;
		}
		if (!(flags & kVoiceFree)) {
			// In use: track the lowest priority.
			if (entry[kVoicePriority] < lowestPriority) {
				lowestPriority = entry[kVoicePriority];
				_lowestPriorityTable = table;
				_lowestPriorityVoice = static_cast<uint8>(offset);
			}
			continue;
		}
		// Free: a voice last used by the same channel wins unless sustain is on.
		if (entry[kVoiceChannel] == channel && !(block[kCcbFlags] & kChannelSustain)) {
			return offset;
		}
		if (entry[kVoiceRelease] < freest) {
			freest = entry[kVoiceRelease];
			freestOffset = offset;
		}
	}

	if (freest != 0xFF) {
		return freestOffset;
	}
	// No free voice: steal the lowest priority one if this channel's priority is at least as high.
	if (lowestPriority <= block[kCcbPriority] && _lowestPriorityTable == table) {
		return _lowestPriorityVoice;
	}
	return -1;
}

// ALLOCSPEC: only allocates (or not) the single voice at entry. Returns its offset or -1.
int GemsDriver::allocateSpecificVoice(uint8 *entry, int channel) {
	if (entry[kVoiceFlags] & kVoiceFree) {
		return 0;
	}
	if (entry[kVoicePriority] <= ccb(channel)[kCcbPriority]) {
		return 0;
	}
	return -1;
}

// VTANDET: fills the voice table entry and retriggers the channel's envelope when enabled.
void GemsDriver::storeVoice(uint8 *entry, uint8 flags, int channel, uint8 note) {
	uint8 *block = ccb(channel);
	_noteOnVoice = flags & 7;
	entry[kVoiceFlags] = flags & 7;

	uint16 duration = readWord(&block[kCcbDuration]);
	if (duration) {
		entry[kVoiceFlags] |= kVoiceSelfTimed;
	}
	if (block[kCcbFlags] & kChannelSfxTimebase) {
		entry[kVoiceFlags] |= kVoiceSfxTimebase;
	}
	entry[kVoicePriority] = block[kCcbPriority];
	entry[kVoiceNote] = note;
	entry[kVoiceChannel] = static_cast<uint8>(channel);
	writeWord(&entry[kVoiceDuration], duration);
	entry[kVoiceRelease] = 254;

	if (block[kCcbFlags] & kChannelRetrigger) {
		triggerEnvelope(block[kCcbEnvelope], static_cast<uint8>(channel), block[kCcbFlags]);
	}
}

// noteonfm
void GemsDriver::noteOnFm(int channel, uint8 note) {
	const uint8 *patchData = patch(_currentPatch);
	const uint8 *parameters = patchData + 1;
	bool channel3Mode = (parameters[1] & 0x40) != 0;

	uint8 *entry;
	if (channel3Mode) {
		// Only CH3 will do for a CH3 mode patch.
		if (allocateSpecificVoice(&_fmVoices[kFmVoiceChannel3], channel) < 0) {
			return;
		}
		entry = &_fmVoices[kFmVoiceChannel3];
	} else {
		int offset = allocateVoice(_fmVoices, channel);
		if (offset < 0) {
			return;
		}
		entry = &_fmVoices[offset];
	}

	uint8 flags = entry[kVoiceFlags];
	if (!(flags & kVoiceFree)) {
		fmKeyOff(flags & 7); // stolen voice
		flags = entry[kVoiceFlags];
	}
	storeVoice(entry, flags, channel, note);

	if (!channel3Mode) {
		_noteOnFrequency = getFrequency(false, note, channel);
	}

	uint8 voice = _noteOnVoice;
	if (voice == 2) {
		// Channel 3: apply the patch's CH3 mode bits while keeping the timer running.
		fmWrite(0, 0x27, static_cast<uint8>(parameters[1] | 5));
	}

	uint8 part, fmChannel;
	splitVoice(voice, part, fmChannel);
	if (parameters[0] & 8) {
		fmWrite(0, 0x22, parameters[0]); // LFO, only loaded when this patch enables it
	}

	_carriers = kCarrierMasks[parameters[2] & 7];
	writeFmPatch(part, fmChannel, parameters);

	if (!channel3Mode) {
		fmWrite(part, static_cast<uint8>(0xA4 + fmChannel), static_cast<uint8>(_noteOnFrequency >> 8));
		fmWrite(part, static_cast<uint8>(0xA0 + fmChannel), static_cast<uint8>(_noteOnFrequency));
	} else {
		// fmc3on: per-operator frequencies stored in the patch.
		fmWrite(0, 0xA6, parameters[28]);
		fmWrite(0, 0xA2, parameters[29]);
		fmWrite(0, 0xAC, parameters[30]);
		fmWrite(0, 0xA8, parameters[31]);
		fmWrite(0, 0xAD, parameters[32]);
		fmWrite(0, 0xA9, parameters[33]);
		fmWrite(0, 0xAE, parameters[34]);
		fmWrite(0, 0xAA, parameters[35]);
	}

	// fmkon: operator mask from the patch plus the voice code.
	fmWrite(0, 0x28, static_cast<uint8>((parameters[36] << 4) | voice));
}

// WRITEFM: total levels of carrier operators are reduced by the note attenuation.
void GemsDriver::writeFmPatch(uint8 part, uint8 channelInPart, const uint8 *patchData) {
	for (const uint8 *pair = kFmPatchRegisters; pair[0] != 0; pair += 2) {
		uint8 reg = static_cast<uint8>(pair[0] + channelInPart);
		uint8 offset = pair[1];
		uint8 value;
		if (offset == 0) {
			value = 0;
		} else if (offset & 0x80) {
			offset &= 0x7F;
			bool carrier = (_carriers & 1) != 0;
			_carriers >>= 1;
			if (carrier) {
				uint8 level = static_cast<uint8>(127 - patchData[offset]); // 0 = soft, 127 = loud
				uint16 scaledLevel = static_cast<uint8>(level << 1);
				uint16 product = static_cast<uint16>(_noteOnAttenuation * scaledLevel);
				uint8 reduced = static_cast<uint8>(level - (product >> 8));
				value = static_cast<uint8>(127 - reduced);
			} else {
				value = patchData[offset];
			}
		} else {
			value = patchData[offset];
		}
		fmWrite(part, reg, value);
	}
}

// noteontone / noteonnoise
void GemsDriver::noteOnPsg(int channel, uint8 note, bool noise) {
	uint8 *entry;
	if (noise) {
		// Try to get TG4, the noise channel.
		if (allocateSpecificVoice(_noiseVoices, channel) < 0) {
			return;
		}
		entry = _noiseVoices;
	} else {
		int offset = allocateVoice(_psgVoices, channel);
		if (offset < 0) {
			return;
		}
		entry = &_psgVoices[offset];
	}

	storeVoice(entry, entry[kVoiceFlags], channel, note);
	uint16 wavelength = getFrequency(true, note, channel);

	const uint8 *patchData = patch(_currentPatch);
	const uint8 *parameters = patchData + 1;
	uint8 voice = _noteOnVoice;
	_psgRegisters[kPsgToneLow + voice] = wavelength & 0x0F;
	_psgRegisters[kPsgToneHigh + voice] = static_cast<uint8>(wavelength >> 4);

	if (voice == 3) {
		// Assume TG3 is not locked by this noise patch.
		_psgVoices[kPsgVoiceTone3 + kVoiceFlags] &= ~kVoiceLocked;
		if ((parameters[0] & 3) == 3) {
			psgWrite(static_cast<uint8>(_psgRegisters[kPsgToneLow + 3] | 0xC0));
			psgWrite(_psgRegisters[kPsgToneHigh + 3]);
			_psgVoices[kPsgVoiceTone3 + kVoiceFlags] = 0xA2; // TG3 free and locked
			_psgVoices[kPsgVoiceTone3 + kVoiceNote] = note;
			_psgVoices[kPsgVoiceTone3 + kVoiceChannel] = static_cast<uint8>(channel);
			_psgRegisters[kPsgCommand + 2] = 4; // stop TG3's envelope
		}
		_psgRegisters[kPsgToneLow + voice] = parameters[0]; // noise data
	}

	// pskon
	_psgRegisters[kPsgAttackRate + voice] = parameters[1];
	_psgRegisters[kPsgSustainLevel + voice] = static_cast<uint8>(parameters[2] << 4);
	_psgRegisters[kPsgAttackLevel + voice] = static_cast<uint8>(parameters[3] << 4);
	_psgRegisters[kPsgDecayRate + voice] = parameters[4];
	_psgRegisters[kPsgReleaseRate + voice] = parameters[5];
	_psgRegisters[kPsgCommand + voice] |= 1; // key on
}

// noteondig
void GemsDriver::noteOnDigital(int channel, uint8 note) {
	uint8 *entry = &_fmVoices[kFmVoiceChannel6];
	if (allocateSpecificVoice(entry, channel) < 0) {
		return;
	}

	uint8 flags = entry[kVoiceFlags];
	if (!(flags & kVoiceFree)) {
		_dacmeEnabled = false;
		if (!(flags & kVoiceLocked)) {
			fmKeyOff(flags & 7); // it was playing FM
		}
		flags = entry[kVoiceFlags];
	}
	storeVoice(entry, flags, channel, note);
	entry[kVoiceFlags] |= kVoiceLocked; // lock the voice from FM allocation

	// C4..B7 select samples 0-47 and C0..B3 samples 48-95.
	uint8 sample = static_cast<uint8>(note - 48);
	if (note < 48) {
		sample = static_cast<uint8>(sample + 96);
	}
	uint32 sampleTable = bankPointer(3);
	uint32 header = (sampleTable + sample * 12) & 0xFFFFFF;
	for (int i = 0; i < 12; i++) {
		_sampleHeader[i] = readRom(header + i);
	}

	uint16 firstLength = readWord(&_sampleHeader[kSampleFirst]);
	if (firstLength == 0) {
		entry[kVoiceFlags] = 0xC6; // empty sample: voice 6 free and releasing
		return;
	}

	// A second patch byte other than 4 overrides the sample's playback rate.
	const uint8 *patchData = patch(_currentPatch);
	if (patchData[1] != 4) {
		_sampleHeader[kSampleFlags] = static_cast<uint8>((_sampleHeader[kSampleFlags] & 0xF0) | patchData[1]);
	}

	_dacReadPtr = 0;
	// Timer A is reloaded with -rate, so a DAC sample is due every 'rate' Timer A counts, i.e. every
	// 2 * rate rendered samples.
	uint32 rate = _sampleHeader[kSampleFlags] & 0x0F;
	_dacPeriod = 2 * (rate ? rate : 1024);
	_dacPhase = 0;
	fmWrite(0, 0x2B, 0x80);                      // enable the DAC
	fmWrite(0, 0x27, static_cast<uint8>(0x15 | _channel3Mode)); // start Timer A
	fmWrite(1, 0xB6, 0xC0);                      // channel 6 to both outputs

	uint32 samplePointer = static_cast<uint32>(_sampleHeader[kSamplePointer] |
											   (_sampleHeader[kSamplePointer + 1] << 8) |
											   (_sampleHeader[kSamplePointer + 2] << 16));
	_samplePtr = (sampleTable + samplePointer + readWord(&_sampleHeader[kSampleSkip])) & 0xFFFFFF;
	_sampleCounter = firstLength;

	_fillEnabled = true;
	_dacWritePtr = 0;
	_fdfState = (_sampleHeader[kSampleFlags] & kSampleLooped) ? 5 : 4;
	fillDacFifoNeeded(); // FORCEFILLDF

	_dacmeEnabled = true;
	_dacNibbleMode = (_sampleHeader[kSampleFlags] & kSampleCompressed) != 0;
	if (_dacNibbleMode) {
		_dacNibblePattern = 0xAA;
	}
}

// NOTEOFF (68000 command 1)
void GemsDriver::noteOff(uint8 channel, uint8 note) {
	int flags = deallocateVoice(_fmVoices, channel, note);
	if (flags >= 0) {
		if ((flags & 0x27) == 0x26) {
			noteOffDigital();
		} else {
			fmKeyOff(static_cast<uint8>(flags & 7));
		}
		return;
	}

	flags = deallocateVoice(_psgVoices, channel, note);
	if (flags < 0) {
		flags = deallocateVoice(_noiseVoices, channel, note);
		if (flags < 0) {
			return;
		}
	}
	_psgRegisters[kPsgCommand + (flags & 3)] |= 2; // key off
}

// DEALLOC: returns the flags of the matching in-use voice, or -1.
int GemsDriver::deallocateVoice(uint8 *table, uint8 channel, uint8 note) {
	for (uint8 *entry = table; entry[kVoiceFlags] != kVoiceTableEnd; entry += kVoiceEntrySize) {
		uint8 flags = entry[kVoiceFlags];
		if (flags & kVoiceFree) {
			continue;
		}
		if (entry[kVoiceNote] != note || entry[kVoiceChannel] != channel) {
			continue;
		}
		if ((flags & 0x27) != 0x26) {
			// Keep the lock and voice number, set free and release.
			entry[kVoiceFlags] = static_cast<uint8>((flags & 0x27) | kVoiceFree | kVoiceReleasing);
		}
		return flags;
	}
	return -1;
}

// ************************************ DIGITAL ************************************

// DACME: outputs one sample from the DAC FIFO. Compressed (4-bit) samples play the low nibble of a
// byte first, then its high nibble, and only then advance the read pointer.
void GemsDriver::dacSample() {
	uint8 value = ram(kDacFifo + _dacReadPtr);
	if (_dacNibbleMode) {
		bool highNibble = (_dacNibblePattern & 1) != 0;
		_dacNibblePattern = static_cast<uint8>((_dacNibblePattern >> 1) | ((_dacNibblePattern & 1) << 7));
		if (highNibble) {
			_dacReadPtr++;
			value &= 0xF0;
		} else {
			value = static_cast<uint8>((value << 4) & 0xF0);
		}
	} else {
		_dacReadPtr++;
	}
	_chip->setDacData(value);
}

// FILLDACFIFO: refills the half of the 256-byte FIFO the DAC is not reading.
void GemsDriver::fillDacFifo() {
	if (!_fillEnabled) {
		return;
	}
	if (((_dacWritePtr ^ _dacReadPtr) & 0x80) == 0) {
		return;
	}
	fillDacFifoNeeded();
}

void GemsDriver::fillDacFifoNeeded() {
	if (_fdfState >= 7) {
		stopDac();
		return;
	}

	// States 4 (one shot), 5 (looped), and 6 (looped, note off pending).
	if (_sampleCounter > 128) {
		_sampleCounter = static_cast<uint16>(_sampleCounter - 128);
		transferSample(_dacWritePtr, _samplePtr, 128);
		_dacWritePtr = static_cast<uint8>(_dacWritePtr + 128);
		_samplePtr = (_samplePtr + 128) & 0xFFFFFF;
		return;
	}

	// FDF4DONE: transfer what is left.
	uint8 count = static_cast<uint8>(_sampleCounter);
	uint8 destination = _dacWritePtr;
	transferSample(destination, _samplePtr, count);
	_dacWritePtr = static_cast<uint8>(_dacWritePtr + 128);

	if (_fdfState != 5) {
		// The Z80 shuts the DAC off as soon as the last block is loaded, so the final buffered
		// samples of a one-shot sample are not played.
		stopDac();
		return;
	}

	// Looped: continue from the loop start to complete this 128-byte block.
	uint16 loopLength = readWord(&_sampleHeader[kSampleLoop]);
	_samplePtr = (_samplePtr + count - loopLength) & 0xFFFFFF;
	_sampleCounter = loopLength;
	uint8 remaining = static_cast<uint8>(128 - count);
	if (remaining == 0) {
		return;
	}
	_sampleCounter = static_cast<uint16>(_sampleCounter - remaining);
	transferSample(static_cast<uint8>(destination + count), _samplePtr, remaining);
	_samplePtr = (_samplePtr + remaining) & 0xFFFFFF;
}

void GemsDriver::transferSample(uint8 fifoOffset, uint32 source, int count) {
	for (int i = 0; i < count; i++) {
		ram(kDacFifo + static_cast<uint8>(fifoOffset + i)) = readRom(source + i);
	}
}

// FDF7: DAC off, voice 6 free, unlocked, and releasing.
void GemsDriver::stopDac() {
	_dacmeEnabled = false;
	_fillEnabled = false;
	_chip->setDacEnabled(false);
	_fmVoices[kFmVoiceChannel6 + kVoiceFlags] = 0xC6;
	_fmVoices[kFmVoiceChannel6 + kVoiceDuration] = 0;
	_fmVoices[kFmVoiceChannel6 + kVoiceDuration + 1] = 0;
}

// NOTEOFFDIG
void GemsDriver::noteOffDigital() {
	uint8 flags = _sampleHeader[kSampleFlags];
	if (flags & kSampleClipAtNoteOff) {
		_fdfState = 7;
	} else if (flags & kSampleLooped) {
		_fdfState = 6; // end of loop
	}
}

// ************************************ PSG ENVELOPES ************************************

// DOPSGENV: the PSG command processor and envelope emulator, run every 60 Hz tick.
void GemsDriver::doPsgEnvelopes() {
	for (int voice = 0; voice < 4; voice++) {
		uint8 *r = &_psgRegisters[voice];
		bool noise = voice == 3;
		uint8 command = r[kPsgCommand];
		r[kPsgCommand] = 0;

		if (command & 4) {
			// stop
			r[kPsgLevel] = 0xFF;
			r[kPsgUpdate] = 1;
			r[kPsgMode] = 0;
			if (noise) {
				_psgVoices[kPsgVoiceTone3 + kVoiceFlags] &= ~kVoiceLocked;
			}
		}
		if ((command & 2) && r[kPsgMode] != 0) {
			// key off: switch to the release phase
			r[kPsgUpdate] = 1;
			r[kPsgMode] = 4;
		}
		if (command & 1) {
			// key on: write the tone (or noise control) and start the attack phase
			r[kPsgLevel] = 0xFF;
			psgWrite(static_cast<uint8>(r[kPsgToneLow] | (0x80 + 0x20 * voice)));
			if (!noise) {
				psgWrite(r[kPsgToneHigh]);
			}
			r[kPsgUpdate] = 1;
			r[kPsgMode] = 1;
		}

		switch (r[kPsgMode]) {
		case 1: { // attack
			r[kPsgUpdate] = 1;
			uint8 level = r[kPsgLevel];
			uint8 target = r[kPsgAttackLevel];
			if (level <= r[kPsgAttackRate] || static_cast<uint8>(level - r[kPsgAttackRate]) <= target) {
				r[kPsgLevel] = target;
				r[kPsgMode] = 2;
			} else {
				r[kPsgLevel] = static_cast<uint8>(level - r[kPsgAttackRate]);
			}
			break;
		}
		case 2: { // decay
			r[kPsgUpdate] = 1;
			uint8 level = r[kPsgLevel];
			uint8 target = r[kPsgSustainLevel];
			bool done;
			if (level < target) {
				uint16 sum = static_cast<uint16>(level + r[kPsgDecayRate]);
				done = sum > 0xFF || sum >= target;
				level = static_cast<uint8>(sum);
			} else if (level == target) {
				done = true;
			} else {
				done = level < r[kPsgDecayRate] || static_cast<uint8>(level - r[kPsgDecayRate]) < target;
				level = static_cast<uint8>(level - r[kPsgDecayRate]);
			}
			if (done) {
				r[kPsgLevel] = target;
				r[kPsgMode] = 3;
			} else {
				r[kPsgLevel] = level;
			}
			break;
		}
		case 4: { // release
			r[kPsgUpdate] = 1;
			uint16 sum = static_cast<uint16>(r[kPsgLevel] + r[kPsgReleaseRate]);
			if (sum > 0xFF) {
				r[kPsgLevel] = 0xFF;
				r[kPsgMode] = 0;
				if (noise) {
					_psgVoices[kPsgVoiceTone3 + kVoiceFlags] &= ~kVoiceLocked;
				}
			} else {
				r[kPsgLevel] = static_cast<uint8>(sum);
			}
			break;
		}
		default:
			break;
		}
	}

	// Write the attenuation of every voice whose level changed.
	for (int voice = 0; voice < 4; voice++) {
		uint8 *r = &_psgRegisters[voice];
		if (!(r[kPsgUpdate] & 1)) {
			continue;
		}
		r[kPsgUpdate] = 0;
		psgWrite(static_cast<uint8>((r[kPsgLevel] >> 4) | (0x90 + 0x20 * voice)));
	}
}

} // namespace Scooby
