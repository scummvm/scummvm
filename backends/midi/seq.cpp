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

/*
 * Raw output support by Michael Pearce
 * Alsa support by Nicolas Noble <nicolas@nobis-crew.org> copied from
 * both the QuickTime support and (vkeybd https://web.archive.org/web/20070629122111/http://www.alsa-project.org/~iwai/alsa.html)
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(USE_SEQ_MIDI)

#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

////////////////////////////////////////
//
// Unix dev/sequencer driver
//
////////////////////////////////////////

#define SEQ_MIDIPUTC 5

class MidiDriver_SEQ : public MidiDriver_MPU401 {
public:
	MidiDriver_SEQ(int port, bool isSynth);
	int open() override;
	bool isOpen() const override { return _isOpen; }
	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;

	static const char *getDeviceName();

private:
	bool _isSynth;
	bool _isOpen;
	int _device;
	int _port;
};

MidiDriver_SEQ::MidiDriver_SEQ(int port, bool isSynth) {
	_isOpen = false;
	_isSynth = isSynth;
	_device = -1;
	_port = port;
}

int MidiDriver_SEQ::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	const char *deviceName = getDeviceName();

	_isOpen = true;
	_device = ::open(deviceName, O_RDWR, 0);

	if (_device < 0) {
		warning("Cannot open rawmidi device %s - using /dev/null (no music will be heard)", deviceName);
		_device = (::open(("/dev/null"), O_RDWR, 0));
		if (_device < 0)
			error("Cannot open /dev/null to dump midi output");
	}

	return 0;
}

void MidiDriver_SEQ::close() {
	MidiDriver_MPU401::close();

	if (_isOpen)
		::close(_device);
	_isOpen = false;
}

void MidiDriver_SEQ::send(uint32 b) {
	midiDriverCommonSend(b);

	unsigned char buf[256];
	int position = 0;

	if (_isSynth) {
		switch (b & 0xf0) {
		case 0x80:
		case 0x90:
		case 0xa0:
			buf[position++] = EV_CHN_VOICE;
			buf[position++] = _port;
			buf[position++] = (unsigned char)(b & 0xf0);
			buf[position++] = (unsigned char)(b & 0x0f);
			buf[position++] = (unsigned char)((b >> 8) & 0xff);
			buf[position++] = (unsigned char)((b >> 16) & 0xff);
			buf[position++] = 0;
			buf[position++] = 0;
			break;
		case 0xc0:	// Program change
		case 0xd0:	// Channel pressure
			buf[position++] = EV_CHN_COMMON;
			buf[position++] = _port;
			buf[position++] = (unsigned char)(b & 0xf0);
			buf[position++] = (unsigned char)(b & 0x0f);
			buf[position++] = (unsigned char)((b >> 8) & 0xff);
			buf[position++] = 0;
			buf[position++] = 0;
			buf[position++] = 0;
			break;
		case 0xb0:	// Control change
			buf[position++] = EV_CHN_COMMON;
			buf[position++] = _port;
			buf[position++] = (unsigned char)(b & 0xf0);
			buf[position++] = (unsigned char)(b & 0x0f);
			buf[position++] = (unsigned char)((b >> 8) & 0xff);
			buf[position++] = 0;

			// TODO/FIXME?: Main volume control in the soundcard.h macros is value*16383/100, expression is value*128, and pan is (value+128)/2
			// Not sure how those translate to the scales we're using here.
			*reinterpret_cast<uint16 *>(buf + position) = static_cast<uint16>((b >> 16) & 0xffff);
			position += 2;
			break;
		case 0xe0: // Pitch bend
			buf[position++] = EV_CHN_COMMON;
			buf[position++] = _port;
			buf[position++] = (unsigned char)(b & 0xf0);
			buf[position++] = (unsigned char)(b & 0x0f);
			buf[position++] = 0;
			buf[position++] = 0;

			*reinterpret_cast<uint16 *>(buf + position) = static_cast<uint16>((b >> 8) & 0xffff);
			position += 2;
			break;
		default:
			warning("MidiDriver_SEQ::send: unknown: %08x", (int)b);
			break;

		}
	} else {
		switch (b & 0xF0) {
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			buf[position++] = SEQ_MIDIPUTC;
			buf[position++] = (unsigned char)b;
			buf[position++] = _port;
			buf[position++] = 0;
			buf[position++] = SEQ_MIDIPUTC;
			buf[position++] = (unsigned char)((b >> 8) & 0x7F);
			buf[position++] = _port;
			buf[position++] = 0;
			buf[position++] = SEQ_MIDIPUTC;
			buf[position++] = (unsigned char)((b >> 16) & 0x7F);
			buf[position++] = _port;
			buf[position++] = 0;
			break;
		case 0xC0:
		case 0xD0:
			buf[position++] = SEQ_MIDIPUTC;
			buf[position++] = (unsigned char)b;
			buf[position++] = _port;
			buf[position++] = 0;
			buf[position++] = SEQ_MIDIPUTC;
			buf[position++] = (unsigned char)((b >> 8) & 0x7F);
			buf[position++] = _port;
			buf[position++] = 0;
			break;
		default:
			warning("MidiDriver_SEQ::send: unknown: %08x", (int)b);
			break;
		}
	}

	if (write(_device, buf, position) == -1)
		warning("MidiDriver_SEQ::send: write failed (%s)", strerror(errno));
}

void MidiDriver_SEQ::sysEx(const byte *msg, uint16 length) {
	unsigned char buf [266*4];
	int position = 0;
	const byte *chr = msg;

	assert(length + 2 <= 266);

	midiDriverCommonSysEx(msg, length);

	if (_isSynth) {
		int chunksRequired = (length + 7) / 6;

		assert(chunksRequired * 8 <= static_cast<int>(sizeof(buf)));

		for (int i = 0; i < chunksRequired; i++) {
			int chunkStart = i * 6;

			buf[position++] = EV_SYSEX;
			buf[position++] = _port;

			for (int j = 0; j < 6; j++) {
				int pos = chunkStart + j - 1;
				if (pos < 0)
					buf[position++] = 0xf0;
				else if (pos < length)
					buf[position++] = msg[pos];
				else if (pos == length)
					buf[position++] = 0x7f;
				else
					buf[position++] = 0xff;
			}
		}
	} else {
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = 0xF0;
		buf[position++] = _port;
		buf[position++] = 0;
		for (; length; --length, ++chr) {
			buf[position++] = SEQ_MIDIPUTC;
			buf[position++] = (unsigned char)*chr & 0x7F;
			buf[position++] = _port;
			buf[position++] = 0;
		}
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = 0xF7;
		buf[position++] = _port;
		buf[position++] = 0;
	}

	if (write(_device, buf, position) == -1)
		warning("MidiDriver_SEQ::send: write failed (%s)", strerror(errno));
}


const char *MidiDriver_SEQ::getDeviceName() {
	const char *devName = getenv("SCUMMVM_MIDI");

	if (devName)
		return devName;
	else
		return "/dev/sequencer";
}

// Plugin interface

class SeqMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "SEQ";
	}

	const char *getId() const {
		return "seq";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
	bool checkDevice(MidiDriver::DeviceHandle hdl, int checkFlags, bool quiet) const;

private:
	void addMidiDevices(int deviceFD, MusicDevices &devices, Common::Array<int> *portIDs) const;
	void addSynthDevices(int deviceFD, MusicDevices &devices, Common::Array<int> *portIDs) const;
};

MusicDevices SeqMusicPlugin::getDevices() const {
	MusicDevices devices;

	int deviceFD = ::open(MidiDriver_SEQ::getDeviceName(), O_RDWR, 0);
	if (deviceFD >= 0) {
		addMidiDevices(deviceFD, devices, nullptr);
		addSynthDevices(deviceFD, devices, nullptr);
		::close(deviceFD);
	}

	return devices;
}

void SeqMusicPlugin::addMidiDevices(int deviceFD, MusicDevices &devices, Common::Array<int> *portIDs) const {
	int midiDeviceCount = 0;
	if (ioctl(deviceFD, SNDCTL_SEQ_NRMIDIS, &midiDeviceCount) == 0) {
		for (int i = 0; i < midiDeviceCount; i++) {
			midi_info midiInfo;
			midiInfo.device = i;
			if (ioctl(deviceFD, SNDCTL_MIDI_INFO, &midiInfo) == 0) {
				devices.push_back(MusicDevice(this, midiInfo.name, MT_GM));	// dev_type is unimplemented so we just assume GM
				if (portIDs)
					portIDs->push_back(i);
			}
		}
	}
}

void SeqMusicPlugin::addSynthDevices(int deviceFD, MusicDevices &devices, Common::Array<int> *portIDs) const {
	int synthDeviceCount = 0;
	if (ioctl(deviceFD, SNDCTL_SEQ_NRSYNTHS, &synthDeviceCount) == 0) {
		for (int i = 0; i < synthDeviceCount; i++) {
			synth_info synthInfo;
			synthInfo.device = i;
			if (ioctl(deviceFD, SNDCTL_SYNTH_ID, &synthInfo) == 0) {
				MusicType musicType = MT_GM;

				if (synthInfo.synth_type == SYNTH_TYPE_FM)
					musicType = MT_ADLIB;

				devices.push_back(MusicDevice(this, synthInfo.name, musicType)); // dev_type is unimplemented so we just assume GM
				if (portIDs)
					portIDs->push_back(i);
			}
		}
	}
}

Common::Error SeqMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle dev) const {
	int port = 0;
	bool isSynth = false;
	bool found = false;

	if (dev) {
		Common::String deviceIDString = MidiDriver::getDeviceString(dev, MidiDriver::kDeviceId);

		MusicDevices devices;
		Common::Array<int> ports;
		int firstSynthIndex = 0;

		int deviceFD = ::open(MidiDriver_SEQ::getDeviceName(), O_RDONLY, 0);
		if (deviceFD >= 0) {
			addMidiDevices(deviceFD, devices, &ports);

			firstSynthIndex = static_cast<int>(ports.size());

			addSynthDevices(deviceFD, devices, &ports);

			::close(deviceFD);
		} else {
			warning("Device enumeration failed when creating device");
		}

		int devIndex = 0;
		for (MusicDevices::iterator d = devices.begin(); d != devices.end(); d++) {
			if (d->getCompleteId().equals(deviceIDString)) {
				found = true;
				isSynth = (devIndex >= firstSynthIndex);
				port = ports[devIndex];
				break;
			}
			devIndex++;
		}
	}

	if (found) {
		*mididriver = new MidiDriver_SEQ(port, isSynth);
		return Common::kNoError;
	}

	return Common::kAudioDeviceInitFailed;
}

bool SeqMusicPlugin::checkDevice(MidiDriver::DeviceHandle hdl, int checkFlags, bool quiet) const {
	return true;
}

//#if PLUGIN_ENABLED_DYNAMIC(SEQ)
	//REGISTER_PLUGIN_DYNAMIC(SEQ, PLUGIN_TYPE_MUSIC, SeqMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(SEQ, PLUGIN_TYPE_MUSIC, SeqMusicPlugin);
//#endif

#endif
