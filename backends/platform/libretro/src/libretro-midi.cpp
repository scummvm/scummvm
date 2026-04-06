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
 * Libretro MIDI output driver
 * Based on the ScummVM CoreMIDI and STMIDI drivers
 * Implements RETRO_ENVIRONMENT_GET_MIDI_INTERFACE for MIDI Out support
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <libretro.h>
#include "common/scummsys.h"
#include "audio/mpu401.h"
#include "common/error.h"
#include "common/util.h"
#include "common/system.h"
#include "audio/musicplugin.h"
#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"

// External libretro MIDI interface
extern struct retro_midi_interface *retro_midi_interface;

class MidiDriver_Libretro : public MidiDriver_MPU401 {
public:
	MidiDriver_Libretro() : _isOpen(false), _lastWriteUs(0) { }
	int open();
	bool isOpen() const {
		return _isOpen;
	}
	void close();
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;

private:
	bool _isOpen;
	uint64 _lastWriteUs;
	uint32 calcDeltaUs();
	inline bool outputAvailable() const {
		return retro_midi_interface && retro_midi_interface->output_enabled();
	}

};

uint32 MidiDriver_Libretro::calcDeltaUs() {
	uint64 nowUs = (uint64)g_system->getMillis() * 1000ULL;
	uint32 delta = 0;
	if (_lastWriteUs && nowUs >= _lastWriteUs) {
		uint64 d = nowUs - _lastWriteUs;
		delta = (d > 0xFFFFFFFFULL) ? 0xFFFFFFFFU : (uint32)d;
	}
	_lastWriteUs = nowUs;
	return delta;
}


int MidiDriver_Libretro::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (!retro_midi_interface) {
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	if (!retro_midi_interface->output_enabled()) {
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	_isOpen = true;
	_lastWriteUs = 0;
	return 0;
}

void MidiDriver_Libretro::close() {
	if (_isOpen && outputAvailable())
		stopAllNotes(true);
	MidiDriver_MPU401::close();
	_isOpen = false;
}

void MidiDriver_Libretro::send(uint32 b) {
	midiDriverCommonSend(b);

	if (!_isOpen)
		return;
	if (!outputAvailable())
		return;

	byte status = (byte)(b & 0xFF);
	byte d1	 = (byte)((b >> 8) & 0xFF);
	byte d2	 = (byte)((b >> 16) & 0xFF);

	// to be handled by sysEx()
	if (status == 0xF0)
		return;

	// Realtime messages: 1 byte (F8..FF)
	if (status >= 0xF8) {
		retro_midi_queue_push(status, calcDeltaUs());
		return;
	}

	// System Common
	switch (status) {
	case 0xF1: // MTC quarter frame (2 bytes total)
		retro_midi_queue_push(status, calcDeltaUs());
		retro_midi_queue_push(d1, calcDeltaUs());
		return;
	case 0xF2: // Song Position Pointer (3 bytes total)
		retro_midi_queue_push(status, calcDeltaUs());
		retro_midi_queue_push(d1, calcDeltaUs());
		retro_midi_queue_push(d2, calcDeltaUs());
		return;
	case 0xF3: // Song Select (2 bytes total)
		retro_midi_queue_push(status, calcDeltaUs());
		retro_midi_queue_push(d1, calcDeltaUs());
		return;
	case 0xF6: // Tune request (1 byte)
	case 0xF7: // EOX
		retro_midi_queue_push(status, calcDeltaUs());
		return;
	default:
		break;
	}

	// Channel voice
	retro_midi_queue_push(status, calcDeltaUs());
	switch (status & 0xF0) {
	case 0xC0: // Program change: 1 data
	case 0xD0: // Channel pressure: 1 data
		retro_midi_queue_push(d1, calcDeltaUs());
		break;
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xE0:
		retro_midi_queue_push(d1, calcDeltaUs());
		retro_midi_queue_push(d2, calcDeltaUs());
		break;
	default:
		break;
	}
}

void MidiDriver_Libretro::sysEx(const byte *msg, uint16 length) {
	midiDriverCommonSysEx(msg, length);

	if (!_isOpen)
		return;
	if (!outputAvailable())
		return;
	if (!msg || length == 0)
		return;

	// Per MidiDriver_BASE info: SysEx max 268 bytes
	if (length > 268)
		length = 268;

	// Send SysEx start
	retro_midi_queue_push(0xF0, calcDeltaUs());

	// Send SysEx data (excluding F0 start and F7 end)
	for (uint16 i = 0; i < length; i++) {
		retro_midi_queue_push(msg[i], calcDeltaUs());
	}

	// Send SysEx end
	retro_midi_queue_push(0xF7, calcDeltaUs());
}

// Plugin interface

class LibretroMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "Libretro MIDI Out";
	}

	const char *getId() const {
		return "libretro_midi";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
	bool checkDevice(MidiDriver::DeviceHandle hdl, int checkFlags, bool quiet) const override;
};

MusicDevices LibretroMusicPlugin::getDevices() const {
	MusicDevices devices;
	// Return GM device as default
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error LibretroMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_Libretro();

	return Common::kNoError;
}

bool LibretroMusicPlugin::checkDevice(MidiDriver::DeviceHandle hdl, int checkFlags, bool quiet) const {
	if (!retro_midi_interface || !retro_midi_interface->output_enabled()) {
		if (!quiet)
			warning("Libretro MIDI: interface not available or output disabled");
		return false;
	}

	// Allow auto selection if available
	if (checkFlags & MDCK_AUTO)
		return true;

	// For explicit selection, handle should normally be non-zero
	return hdl != 0;
}

REGISTER_PLUGIN_STATIC(LIBRETRO_MIDI, PLUGIN_TYPE_MUSIC, LibretroMusicPlugin);
