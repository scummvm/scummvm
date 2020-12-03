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

#include "sci/sci.h"

#include "sci/resource/resource.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/util.h"

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/mutex.h"

// The original driver uses the master volume setting of the hardware device by sending sysex messages
// (applies to SCI0 and SCI1). We have a software mastervolume implementation instead. I don't know the
// reason for this. Nor do I know whether our software effective volume calculation matches the device's
// internal volume calculation algorithm.
// I add the original style master volume handling, but make it optional via a #define
#define		HARDWARE_MASTERVOLUME

namespace Sci {

static byte volumeTable[64] = {
	0x00, 0x10, 0x14, 0x18, 0x1f, 0x26, 0x2a, 0x2e,
	0x2f, 0x32, 0x33, 0x33, 0x34, 0x35, 0x35, 0x36,
	0x36, 0x37, 0x37, 0x38, 0x38, 0x38, 0x39, 0x39,
	0x39, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b,
	0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
	0x3d, 0x3d, 0x3d, 0x3d, 0x3d, 0x3e, 0x3e, 0x3e,
	0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
};

class MidiPlayer_Fb01 : public MidiPlayer {
public:
	enum {
		kVoices = 8,
		kMaxSysExSize = 264
	};

	MidiPlayer_Fb01(SciVersion version);
	~MidiPlayer_Fb01() override;

	int open(ResourceManager *resMan) override;
	void close() override;
	void initTrack(SciSpan<const byte>& header) override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;
	bool hasRhythmChannel() const override { return false; }
	byte getPlayId() const override;
	int getPolyphony() const override { return _version <= SCI_VERSION_0_LATE ? 8 : 9; }
	void setVolume(byte volume) override;
	int getVolume() override;
	void playSwitch(bool play) override;

	bool isOpen() const { return _isOpen; }

	const char *reportMissingFiles() override { return _missingFiles; }

private:
	void noteOn(int channel, int note, int velocity);
	void noteOff(int channel, int note);
	void setPatch(int channel, int patch);
	void controlChange(int channel, int control, int value);

	void setVoiceParam(byte voice, byte param, byte value);
	void setSystemParam(byte sysChan, byte param, byte value);
	void sendVoiceData(byte instrument, const SciSpan<const byte> &data);
	void sendBanks(const SciSpan<const byte> &data);
	void storeVoiceData(byte instrument, byte bank, byte index);
	void initVoices();

	void voiceOn(int voice, int note, int velocity);
	void voiceOff(int voice);
	int findVoice(int channel);
	void voiceMapping(int channel, int voices);
	void assignVoices(int channel, int voices);
	void releaseVoices(int channel, int voices);
	void donateVoices();
	void sendToChannel(byte channel, byte command, byte op1, byte op2);

	struct Channel {
		uint8 patch;			// Patch setting
		uint8 volume;			// Channel volume (0-63)
		uint8 pan;				// Pan setting (0-127, 64 is center)
		uint8 holdPedal;		// Hold pedal setting (0 to 63 is off, 127 to 64 is on)
		uint8 extraVoices;		// The number of additional voices this channel optimally needs
		uint16 pitchWheel;		// Pitch wheel setting (0-16383, 8192 is center)
		uint8 lastVoice;		// Last voice used for this MIDI channel
		bool enableVelocity;	// Enable velocity control (SCI0)

		Channel() : patch(0), volume(127), pan(64), holdPedal(0), extraVoices(0),
					pitchWheel(8192), lastVoice(0), enableVelocity(false) { }
	};

	struct Voice {
		int8 channel;			// MIDI channel that this voice is assigned to or -1
		uint8 poly;				// Number of hardware voices (SCI0); for SCI1 we just set this to 1
		int8 note;				// Currently playing MIDI note or -1
		int bank;				// Current bank setting or -1
		int patch;				// Currently playing patch or -1
		//uint8 velocity;		// Note velocity
		//bool isSustained;		// Flag indicating a note that is being sustained by the hold pedal
		uint16 age;				// Age of the current note

		Voice() : channel(-1), note(-1), bank(-1), patch(-1), /*velocity(0), isSustained(false),*/ age(0), poly(1) { }
	};

	bool _playSwitch;
	int _masterVolume;
	int _numParts;

	bool _isOpen;

	Channel _channels[16];
	Voice _voices[kVoices];

	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	static void midiTimerCallback(void *p);
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;

	const char *_missingFiles;
	static const char _requiredFiles[2][12];

	byte _sysExBuf[kMaxSysExSize];
};

MidiPlayer_Fb01::MidiPlayer_Fb01(SciVersion version) : MidiPlayer(version), _playSwitch(true), _masterVolume(15), _timerParam(NULL), _timerProc(NULL),
	_numParts(version > SCI_VERSION_0_LATE ? kVoices : 0), _isOpen(false), _missingFiles(0) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI);
	_driver = MidiDriver::createMidi(dev);

	_sysExBuf[0] = 0x43;
	_sysExBuf[1] = 0x75;
}

MidiPlayer_Fb01::~MidiPlayer_Fb01() {
	if (_driver)
		_driver->setTimerCallback(NULL, NULL);
	close();
	delete _driver;
}

void MidiPlayer_Fb01::voiceMapping(int channel, int voices) {
	if (_version <= SCI_VERSION_0_LATE) {
		// The original SCI0 drivers don't do any software voice mapping. Only inside the device...
		for (int i = 0; i < _numParts; ++i) {
			if (_voices[i].channel == channel && _voices[i].poly != voices) {
				_voices[i].poly = voices;
				setVoiceParam(i, 0, voices);
			}
		}
		return;
	}

	int curVoices = 0;

	for (int i = 0; i < kVoices; i++)
		if (_voices[i].channel == channel)
			curVoices++;

	curVoices += _channels[channel].extraVoices;

	if (curVoices < voices) {
		debug(3, "FB-01: assigning %i additional voices to channel %i", voices - curVoices, channel);
		assignVoices(channel, voices - curVoices);
	} else if (curVoices > voices) {
		debug(3, "FB-01: releasing %i voices from channel %i", curVoices - voices, channel);
		releaseVoices(channel, curVoices - voices);
		donateVoices();
	}
}

void MidiPlayer_Fb01::assignVoices(int channel, int voices) {
	assert(voices > 0);

	for (int i = 0; i < kVoices; i++) {
		if (_voices[i].channel == -1) {
			_voices[i].channel = channel;
			if (_voices[i].note != -1)
				voiceOff(i);
			if (--voices == 0)
				break;
		}
	}

	_channels[channel].extraVoices += voices;
	setPatch(channel, _channels[channel].patch);
	sendToChannel(channel, 0xe0, _channels[channel].pitchWheel & 0x7f, _channels[channel].pitchWheel >> 7);
	controlChange(channel, 0x07, _channels[channel].volume);
	controlChange(channel, 0x0a, _channels[channel].pan);
	controlChange(channel, 0x40, _channels[channel].holdPedal);
}

void MidiPlayer_Fb01::releaseVoices(int channel, int voices) {
	if (_channels[channel].extraVoices >= voices) {
		_channels[channel].extraVoices -= voices;
		return;
	}

	voices -= _channels[channel].extraVoices;
	_channels[channel].extraVoices = 0;

	for (int i = 0; i < kVoices; i++) {
		if ((_voices[i].channel == channel) && (_voices[i].note == -1)) {
			_voices[i].channel = -1;
			if (--voices == 0)
				return;
		}
	}

	for (int i = 0; i < kVoices; i++) {
		if (_voices[i].channel == channel) {
			voiceOff(i);
			_voices[i].channel = -1;
			if (--voices == 0)
				return;
		}
	}
}

void MidiPlayer_Fb01::donateVoices() {
	int freeVoices = 0;

	for (int i = 0; i < kVoices; i++)
		if (_voices[i].channel == -1)
			freeVoices++;

	if (freeVoices == 0)
		return;

	for (int i = 0; i < MIDI_CHANNELS; i++) {
		if (_channels[i].extraVoices >= freeVoices) {
			assignVoices(i, freeVoices);
			_channels[i].extraVoices -= freeVoices;
			return;
		} else if (_channels[i].extraVoices > 0) {
			assignVoices(i, _channels[i].extraVoices);
			freeVoices -= _channels[i].extraVoices;
			_channels[i].extraVoices = 0;
		}
	}
}

int MidiPlayer_Fb01::findVoice(int channel) {
	int voice = -1;
	int oldestVoice = -1;
	uint32 oldestAge = 0;

	// Try to find a voice assigned to this channel that is free (round-robin)
	for (int i = 0; i < kVoices; i++) {
		int v = (_channels[channel].lastVoice + i + 1) % kVoices;

		if (_voices[v].channel == channel) {
			if (_voices[v].note == -1) {
				voice = v;
				break;
			}

			// We also keep track of the oldest note in case the search fails
			// Notes started in the current time slice will not be selected
			if (_voices[v].age > oldestAge) {
				oldestAge = _voices[v].age;
				oldestVoice = v;
			}
		}
	}

	if (voice == -1) {
		if (oldestVoice >= 0) {
			voiceOff(oldestVoice);
			voice = oldestVoice;
		} else {
			return -1;
		}
	}

	_channels[channel].lastVoice = voice;
	return voice;
}

void MidiPlayer_Fb01::sendToChannel(byte channel, byte command, byte op1, byte op2) {
	for (int i = 0; i < _numParts; i++) {
		// Send command to all voices assigned to this channel
		// I case of SCI0 the voice mapping is done inside the device.
		if (_voices[i].channel == channel)
			_driver->send(command | (_version <= SCI_VERSION_0_LATE ? channel : i), op1, op2);
	}
}

void MidiPlayer_Fb01::setPatch(int channel, int patch) {
	int bank = 0;

	if (_version <= SCI_VERSION_0_LATE && channel == 15) {
		// The original driver has some parsing related handling for program 127.
		// We can't handle that here.
		return;
	}

	_channels[channel].patch = patch;

	if (patch >= 48) {
		patch -= 48;
		bank = 1;
	}

	for (int voice = 0; voice < _numParts; voice++) {
		if (_voices[voice].channel == channel) {
			if (_voices[voice].bank != bank) {
				_voices[voice].bank = bank;
				setVoiceParam(voice, 4, bank);
			}
			_driver->send(0xc0 | (_version <= SCI_VERSION_0_LATE ? channel : voice), patch, 0);
		}
	}
}

void MidiPlayer_Fb01::voiceOn(int voice, int note, int velocity) {
	if (_playSwitch) {
		_voices[voice].note = note;
		_voices[voice].age = 0;
		_driver->send(0x90 | voice, note, velocity);
	}
}

void MidiPlayer_Fb01::voiceOff(int voice) {
	_voices[voice].note = -1;
	_driver->send(0xb0 | voice, 0x7b, 0x00);
}

void MidiPlayer_Fb01::noteOff(int channel, int note) {
	int voice;
	for (voice = 0; voice < kVoices; voice++) {
		if ((_voices[voice].channel == channel) && (_voices[voice].note == note)) {
			voiceOff(voice);
			return;
		}
	}
}

void MidiPlayer_Fb01::noteOn(int channel, int note, int velocity) {
	if (velocity == 0)
		return noteOff(channel, note);

	if (_version > SCI_VERSION_0_LATE)
		velocity >>= 1;

	int voice;
	for (voice = 0; voice < kVoices; voice++) {
		if ((_voices[voice].channel == channel) && (_voices[voice].note == note)) {
			voiceOff(voice);

			// Original bug #1:	The table is only applied here, but not to the voiceOn() call below.
			// Original bug #2:	The velocity value also gets another right shift although the table
			//					size of the volume table is 64 and not 32.
			// --> disable this ? It certainly can't do any good.
			if (_version > SCI_VERSION_0_LATE)
				velocity = volumeTable[velocity >> 1] << 1;

			voiceOn(voice, note, velocity);
			return;
		}
	}

	voice = findVoice(channel);

	if (voice == -1) {
		debug(3, "FB-01: failed to find free voice assigned to channel %i", channel);
		return;
	}

	voiceOn(voice, note, velocity);
}

void MidiPlayer_Fb01::controlChange(int channel, int control, int value) {
	// Events for the control channel shouldn't arrive here.
	// The original driver handles some specific parsing related midi events
	// sent on channel 15, but we (hopefully) do that in the SCI music engine.
	if (_version <= SCI_VERSION_0_LATE && channel == 15)
		return;

	switch (control) {
	case 0x07: {
		_channels[channel].volume = value;

		if (_version > SCI_VERSION_0_LATE)
			value = volumeTable[value >> 1] << 1;

#ifdef HARDWARE_MASTERVOLUME
		sendToChannel(channel, 0xb0, control, value);
#else
		byte vol = _masterVolume;

		if (vol > 0)
			vol = CLIP<byte>(vol + 3, 0, 15);

		sendToChannel(channel, 0xb0, control, (value * vol / 15) & 0x7f);
#endif
		break;
	}
	case 0x0a:
		_channels[channel].pan = value;
		sendToChannel(channel, 0xb0, control, value);
		break;
	case 0x40:
		_channels[channel].holdPedal = value;
		sendToChannel(channel, 0xb0, control, value);
		break;
	case 0x4b:
		voiceMapping(channel, value);
		break;
	case 0x7b:
		for (int i = 0; i < _numParts; i++) {
			if ((_voices[i].channel == channel) && (_voices[i].note != -1)) {
				_voices[i].note = -1;
				sendToChannel(channel, 0xb0, control, value);
			}
		}
		break;
	default:
		sendToChannel(channel, 0xb0, control, value);
		break;
	}
}

void MidiPlayer_Fb01::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0x7f;
	byte op2 = (b >> 16) & 0x7f;

	if (_version <= SCI_VERSION_0_LATE && command != 0xB0 && command != 0xC0) {
		// Since the voice mapping takes place inside the hardware, most messages
		// are simply passed through. Channel 15 is never assigned to a part but is
		// used for certain parsing related events which we cannot handle here.
		// Just making sure that no nonsense is sent to the device...
		if (channel != 15)
			sendToChannel(channel, command, op1, op2);
		return;
	}

	switch (command) {
	case 0x80:
		noteOff(channel, op1);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		controlChange(channel, op1, op2);
		break;
	case 0xc0:
		setPatch(channel, op1);
		break;
	case 0xe0:
		_channels[channel].pitchWheel = (op1 & 0x7f) | ((op2 & 0x7f) << 7);
		sendToChannel(channel, command, op1, op2);
		break;
	default:
		warning("FB-01: Ignoring MIDI event %02x %02x %02x", command | channel, op1, op2);
	}
}

void MidiPlayer_Fb01::setVolume(byte volume) {
	_masterVolume = volume;
#ifdef HARDWARE_MASTERVOLUME
	setSystemParam(0, 0x24, (CLIP<byte>(_masterVolume + 3, 0, 15) << 3) + 7);
#else
	for (uint i = 0; i < MIDI_CHANNELS; i++)
		controlChange(i, 0x07, _channels[i].volume & 0x7f);
#endif
}

int MidiPlayer_Fb01::getVolume() {
	return _masterVolume;
}

void MidiPlayer_Fb01::playSwitch(bool play) {
	_playSwitch = play;
}

void MidiPlayer_Fb01::midiTimerCallback(void *p) {
	if (!p)
		return;

	MidiPlayer_Fb01 *m = (MidiPlayer_Fb01 *)p;

	if (!m->isOpen())
		return;

	// Increase the age of the notes
	for (int i = 0; i < kVoices; i++) {
		if (m->_voices[i].note != -1)
			m->_voices[i].age++;
	}

	if (m->_timerProc)
		m->_timerProc(m->_timerParam);
}

void MidiPlayer_Fb01::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_driver->setTimerCallback(NULL, NULL);

	_timerParam = timer_param;
	_timerProc = timer_proc;

	_driver->setTimerCallback(this, midiTimerCallback);
}

void MidiPlayer_Fb01::sendBanks(const SciSpan<const byte> &data) {
	if (data.size() < 3072)
		error("Failed to read FB-01 patch");

	// SSCI sends bank dumps containing 48 instruments at once. We cannot do that
	// due to the limited maximum SysEx length. Instead we send the instruments
	// one by one and store them in the banks.
	for (int i = 0; i < 48; i++) {
		sendVoiceData(0, data.subspan(i * 64));
		storeVoiceData(0, 0, i);
	}

	// Send second bank if available
	if (data.size() < 6146)
		return;

	if (data.getUint16BEAt(3072) != 0xabcd)
		return;

	for (int i = 0; i < 48; i++) {
		sendVoiceData(0, data.subspan(3074 + i * 64));
		storeVoiceData(0, 1, i);
	}
}

int MidiPlayer_Fb01::open(ResourceManager *resMan) {
	assert(resMan != NULL);

	int retval = _driver->open();
	if (retval != 0) {
		warning("Failed to open MIDI driver");
		return retval;
	}

	// Set system channel to 0.
	setSystemParam(0, 0x20, 0);

	// Turn off memory protection
	setSystemParam(0, 0x21, 0);

	Resource *res = resMan->findResource(ResourceId(kResourceTypePatch, 2), false);

	if (res) {
		sendBanks(*res);
	} else {
		// Early SCI0 games have the sound bank embedded in the IMF driver.
		// Note that these games didn't actually support the FB-01 as a device,
		// but the IMF, which is the same device on an ISA card. Check:
		// http://wiki.vintage-computer.com/index.php/IBM_Music_feature_card

		warning("FB-01 patch file not found, attempting to load sound bank from IMF.DRV");
		// Try to load sound bank from IMF.DRV
		Common::File f;

		if (f.open("IMF.DRV")) {
			Common::SpanOwner<SciSpan<const byte> > buf;
			buf->allocateFromStream(f);

			// Search for start of sound bank
			uint offset;
			for (offset = 0; offset < buf->size() - 7; ++offset) {
				if (!strncmp((const char *)buf->getUnsafeDataAt(offset, 7), "SIERRA ", 7))
					break;
			}

			// Skip to voice data
			offset += 0x20;

			if (offset >= buf->size())
				error("Failed to locate start of FB-01 sound bank");

			// The newer IMF.DRV versions without the sound bank will still have the 32 byte
			// "SIERRA 1" header, but after that they want to send the patch.002 resource.
			// These driver version are easy to identify by their small size.
			if (buf->subspan(offset).size() < 3072) {
				_missingFiles = _requiredFiles[1];
				return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
			}

			sendBanks(buf->subspan(offset));
		} else {
			_missingFiles = _version == SCI_VERSION_0_EARLY ? _requiredFiles[0] : _requiredFiles[1];
			return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
		}
	}

	// Set up voices to use MIDI channels 0 - 7
	for (int i = 0; i < kVoices; i++)
		setVoiceParam(i, 1, i);

	initVoices();

	// Set master volume
	setSystemParam(0, 0x24, 0x7f);

	_isOpen = true;

	return 0;
}

void MidiPlayer_Fb01::close() {
	if (_driver)
		_driver->setTimerCallback(NULL, NULL);
	_isOpen = false;
	if (_driver)
		_driver->close();
}

void MidiPlayer_Fb01::initTrack(SciSpan<const byte>& header) {
	if (!_isOpen || _version > SCI_VERSION_0_LATE)
		return;

	uint8 readPos = 0;
	uint8 caps = header.getInt8At(readPos++);
	if (caps != 0 && (_version == SCI_VERSION_0_EARLY || caps != 2))
		return;

	for (int i = 0; i < 8; ++i)
		_voices[i] = Voice();

	_numParts = 0;
	for (int i = 0; i < 16; ++i) {
		if (_version == SCI_VERSION_0_LATE) {
			uint8 num = header.getInt8At(readPos++) & 0x7F;
			uint8 flags = header.getInt8At(readPos++);

			if (flags & 0x02) {
				_voices[_numParts].channel = i;
				_voices[_numParts].poly = num;
				_numParts++;
			}
		} else {
			uint8 val = header.getInt8At(readPos++);
			if (val & 0x01) {
				if (val & 0x08) {
					if (val >> 4)
						debugC(9, kDebugLevelSound, "MidiPlayer_Fb01::initTrack(): Unused rhythm channel found: 0x%.02x", i);
				} else if ((val >> 4) != 0x0F) {
					_voices[_numParts].channel = i;
					_voices[_numParts].poly = val >> 4;
					_numParts++;
				}
			} else if (val & 0x08) {
				debugC(9, kDebugLevelSound, "MidiPlayer_Fb01::initTrack(): Control channel found: 0x%.02x", i);
			}
		}
	}

	for (int i = 0; i < _numParts; ++i)
		setVoiceParam(i, 1, _voices[i].channel);

	initVoices();
	
	setVolume(_masterVolume);
}

void MidiPlayer_Fb01::setVoiceParam(byte voice, byte param, byte value) {
	_sysExBuf[2] = 0x00;
	_sysExBuf[3] = 0x18 | voice;
	_sysExBuf[4] = param;
	_sysExBuf[5] = value;

	_driver->sysEx(_sysExBuf, 6);
}

void MidiPlayer_Fb01::setSystemParam(byte sysChan, byte param, byte value) {
	_sysExBuf[2] = sysChan;
	_sysExBuf[3] = 0x10;
	_sysExBuf[4] = param;
	_sysExBuf[5] = value;

	sysEx(_sysExBuf, 6);
}

void MidiPlayer_Fb01::sendVoiceData(byte instrument, const SciSpan<const byte> &data) {
	_sysExBuf[2] = 0x00;
	_sysExBuf[3] = 0x08 | instrument;
	_sysExBuf[4] = 0x00;
	_sysExBuf[5] = 0x00;
	_sysExBuf[6] = 0x01;
	_sysExBuf[7] = 0x00;

	for (int i = 0; i < 64; i++) {
		_sysExBuf[8 + i * 2] = data[i] & 0xf;
		_sysExBuf[8 + i * 2 + 1] = data[i] >> 4;
	}

	byte checksum = 0;
	for (int i = 8; i < 136; i++)
		checksum += _sysExBuf[i];

	_sysExBuf[136] = (-checksum) & 0x7f;

	sysEx(_sysExBuf, 137);
}

void MidiPlayer_Fb01::storeVoiceData(byte instrument, byte bank, byte index) {
	_sysExBuf[2] = 0x00;
	_sysExBuf[3] = 0x28 | instrument;
	_sysExBuf[4] = 0x40;
	_sysExBuf[5] = (bank > 0 ? 48 : 0) + index;

	sysEx(_sysExBuf, 6);
}

void MidiPlayer_Fb01::initVoices() {
	int i = 2;
	_sysExBuf[i++] = 0x70;

	// Set all MIDI channels to 0 voices
	for (int j = 0; j < MIDI_CHANNELS; j++) {
		_sysExBuf[i++] = 0x70 | j;
		_sysExBuf[i++] = 0x00;
		_sysExBuf[i++] = 0x00;
	}

	// Set up the MIDI channels we will be using
	for (int j = 0; j < _numParts; j++) {
		int8 chan = _version > SCI_VERSION_0_LATE ? j : _voices[j].channel;
		// One voice
		_sysExBuf[i++] = 0x70 | chan;
		_sysExBuf[i++] = 0x00;
		_sysExBuf[i++] = _voices[j].poly;

		// Full range of keys
		_sysExBuf[i++] = 0x70 | chan;
		_sysExBuf[i++] = 0x02;
		_sysExBuf[i++] = 0x7f;
		_sysExBuf[i++] = 0x70 | chan;
		_sysExBuf[i++] = 0x03;
		_sysExBuf[i++] = 0x00;

		// Voice bank 0
		_sysExBuf[i++] = 0x70 | chan;
		_sysExBuf[i++] = 0x04;
		_sysExBuf[i++] = 0x00;

		// Voice 10
		_sysExBuf[i++] = 0x70 | chan;
		_sysExBuf[i++] = 0x05;
		_sysExBuf[i++] = 0x0a;
	}

	sysEx(_sysExBuf, i);
}

void MidiPlayer_Fb01::sysEx(const byte *msg, uint16 length) {
	_driver->sysEx(msg, length);

	// Wait the time it takes to send the SysEx data
	uint32 delay = (length + 2) * 1000 / 3125;

	delay += 10;

	g_system->delayMillis(delay);
	g_system->updateScreen();
}

byte MidiPlayer_Fb01::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
		return 0x09;
	case SCI_VERSION_0_LATE:
		return 0x02;
	default:
		return 0x00;
	}
}

const char MidiPlayer_Fb01::_requiredFiles[2][12] = {
	"'IMF.DRV'",
	"'PATCH.002'"
};

MidiPlayer *MidiPlayer_Fb01_create(SciVersion version) {
	return new MidiPlayer_Fb01(version);
}

} // End of namespace Sci

#undef HARDWARE_MASTERVOLUME
