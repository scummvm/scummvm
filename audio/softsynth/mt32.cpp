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

#include "common/scummsys.h"
#include "common/system.h"

#ifdef USE_MT32EMU

#include "audio/softsynth/emumidi.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/archive.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/osd_message_queue.h"

#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"
#include "graphics/palette.h"
#include "graphics/font.h"

#include "gui/message.h"

// prevents load of unused FileStream API because it includes a standard library
// include, per _sev
#define MT32EMU_FILE_STREAM_H

#include "audio/softsynth/mt32/c_interface/cpp_interface.h"

namespace MT32Emu {

class ScummVMReportHandler : public MT32Emu::IReportHandler {
public:
	// Callback for debug messages, in vprintf() format
	void printDebug(const char *fmt, va_list list) {
		Common::String out = Common::String::vformat(fmt, list);
		debug(4, "%s", out.c_str());
	}

	// Callbacks for reporting various errors and information
	void onErrorControlROM() {
		GUI::MessageDialog dialog("MT32Emu: Init Error - Missing or invalid Control ROM image", "OK");
		dialog.runModal();
		error("MT32emu: Init Error - Missing or invalid Control ROM image");
	}
	void onErrorPCMROM() {
		GUI::MessageDialog dialog("MT32Emu: Init Error - Missing PCM ROM image", "OK");
		dialog.runModal();
		error("MT32emu: Init Error - Missing PCM ROM image");
	}
	void showLCDMessage(const char *message) {
		Common::OSDMessageQueue::instance().addMessage(Common::U32String(message));
	}

	// Unused callbacks
	virtual void onMIDIMessagePlayed() {}
	virtual bool onMIDIQueueOverflow() { return false; }
	virtual void onMIDISystemRealtime(Bit8u /* system_realtime */) {}
	virtual void onDeviceReset() {}
	virtual void onDeviceReconfig() {}
	virtual void onNewReverbMode(Bit8u /* mode */) {}
	virtual void onNewReverbTime(Bit8u /* time */) {}
	virtual void onNewReverbLevel(Bit8u /* level */) {}
	virtual void onPolyStateChanged(Bit8u /* part_num */) {}
	virtual void onProgramChanged(Bit8u /* part_num */, const char * /* sound_group_name */, const char * /* patch_name */) {}

	virtual ~ScummVMReportHandler() {}
};

}	// end of namespace MT32Emu

class MidiChannel_MT32 : public MidiChannel_MPU401 {
	void effectLevel(byte value) { }
	void chorusLevel(byte value) { }
};

class MidiDriver_MT32 : public MidiDriver_Emulated {
private:
	MidiChannel_MT32 _midiChannels[16];
	uint16 _channelMask;
	MT32Emu::Service _service;
	MT32Emu::ScummVMReportHandler _reportHandler;
	byte *_controlData, *_pcmData;
	Common::Mutex _mutex;

	int _outputRate;

protected:
	void generateSamples(int16 *buf, int len) override;

public:
	MidiDriver_MT32(Audio::Mixer *mixer);
	virtual ~MidiDriver_MT32();

	int open() override;
	void close() override;
	void send(uint32 b) override;
	void setPitchBendRange(byte channel, uint range) override;
	void sysEx(const byte *msg, uint16 length) override;

	uint32 property(int prop, uint32 param) override;
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

	// AudioStream API
	bool isStereo() const override { return true; }
	int getRate() const override { return _outputRate; }
};

////////////////////////////////////////
//
// MidiDriver_MT32
//
////////////////////////////////////////

MidiDriver_MT32::MidiDriver_MT32(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer) {
	_channelMask = 0xFFFF; // Permit all 16 channels by default
	uint i;
	for (i = 0; i < ARRAYSIZE(_midiChannels); ++i) {
		_midiChannels[i].init(this, i);
	}
	_outputRate = 0;
	_controlData = nullptr;
	_pcmData = nullptr;
}

MidiDriver_MT32::~MidiDriver_MT32() {
	close();
}

int MidiDriver_MT32::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	debug(4, _s("Initializing MT-32 Emulator"));

	Common::File controlFile;
	if (!controlFile.open("CM32L_CONTROL.ROM") && !controlFile.open("MT32_CONTROL.ROM"))
		error("Error opening MT32_CONTROL.ROM / CM32L_CONTROL.ROM. Check that your Extra Path in Paths settings is set to the correct directory");

	Common::File pcmFile;
	if (!pcmFile.open("CM32L_PCM.ROM") && !pcmFile.open("MT32_PCM.ROM"))
		error("Error opening MT32_PCM.ROM / CM32L_PCM.ROM. Check that your Extra Path in Paths settings is set to the correct directory");

	_controlData = new byte[controlFile.size()];
	controlFile.read(_controlData, controlFile.size());
	_pcmData = new byte[pcmFile.size()];
	pcmFile.read(_pcmData, pcmFile.size());

	_service.createContext(_reportHandler);

	if (_service.addROMData(_controlData, controlFile.size()) != MT32EMU_RC_ADDED_CONTROL_ROM) {
		error("Adding control ROM failed. Check that your control ROM is valid");
	}

	controlFile.close();

	if (_service.addROMData(_pcmData, pcmFile.size()) != MT32EMU_RC_ADDED_PCM_ROM) {
		error("Adding PCM ROM failed. Check that your PCM ROM is valid");
	}

	pcmFile.close();

	if (_service.openSynth() != MT32EMU_RC_OK)
		return MERR_DEVICE_NOT_AVAILABLE;

	double gain = (double)ConfMan.getInt("midi_gain") / 100.0;
	_service.setOutputGain(1.0f * gain);
	_service.setReverbOutputGain(1.0f * gain);
	// We let the synthesizer play MIDI messages immediately. Our MIDI
	// handling is synchronous to sample generation. This makes delaying MIDI
	// events result in odd sound output in some cases. For example, the
	// shattering window in the Indiana Jones and the Fate of Atlantis intro
	// will sound like a bell if we use any delay here.
	// Bug #6242 "AUDIO: Built-In MT-32 MUNT Produces Wrong Sounds".
	_service.setMIDIDelayMode(MT32Emu::MIDIDelayMode_IMMEDIATE);

	// We need to report the sample rate MUNT renders at as sample rate of our
	// AudioStream.
	_outputRate = _service.getActualStereoOutputSamplerate();

	MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return 0;
}

void MidiDriver_MT32::send(uint32 b) {
	midiDriverCommonSend(b);

	Common::StackLock lock(_mutex);
	_service.playMsg(b);
}

// Indiana Jones and the Fate of Atlantis (including the demo) uses
// setPitchBendRange, if you need a game for testing purposes
void MidiDriver_MT32::setPitchBendRange(byte channel, uint range) {
	if (range > 24) {
		warning("setPitchBendRange() called with range > 24: %d", range);
	}
	byte benderRangeSysex[4] = { 0, 0, 4, (uint8)range };
	Common::StackLock lock(_mutex);
	_service.writeSysex(channel, benderRangeSysex, 4);
}

void MidiDriver_MT32::sysEx(const byte *msg, uint16 length) {
	midiDriverCommonSysEx(msg, length);
	if (msg[0] == 0xf0) {
		Common::StackLock lock(_mutex);
		_service.playSysex(msg, length);
	} else {
		enum {
			SYSEX_CMD_DT1 = 0x12,
			SYSEX_CMD_DAT = 0x42
		};

		if (msg[3] == SYSEX_CMD_DT1 || msg[3] == SYSEX_CMD_DAT) {
			Common::StackLock lock(_mutex);
			_service.writeSysex(msg[1], msg + 4, length - 5);
		} else {
			warning("Unused sysEx command %d", msg[3]);
		}
	}
}

void MidiDriver_MT32::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	// Detach the player callback handler
	setTimerCallback(NULL, NULL);
	// Detach the mixer callback handler
	_mixer->stopHandle(_mixerSoundHandle);

	Common::StackLock lock(_mutex);
	_service.closeSynth();
	_service.freeContext();
	delete[] _controlData;
	_controlData = nullptr;
	delete[] _pcmData;
	_pcmData = nullptr;
}

void MidiDriver_MT32::generateSamples(int16 *data, int len) {
	Common::StackLock lock(_mutex);
	_service.renderBit16s(data, len);
}

uint32 MidiDriver_MT32::property(int prop, uint32 param) {
	switch (prop) {
	case PROP_CHANNEL_MASK:
		_channelMask = param & 0xFFFF;
		return 1;
	default:
		break;
	}

	return 0;
}

MidiChannel *MidiDriver_MT32::allocateChannel() {
	MidiChannel_MT32 *chan;
	uint i;

	for (i = 0; i < ARRAYSIZE(_midiChannels); ++i) {
		if (i == 9 || !(_channelMask & (1 << i)))
			continue;
		chan = &_midiChannels[i];
		if (chan->allocate()) {
			return chan;
		}
	}
	return NULL;
}

MidiChannel *MidiDriver_MT32::getPercussionChannel() {
	return &_midiChannels[9];
}

// This code should be used when calling the timer callback from the mixer thread is undesirable.
// Note that it results in less accurate timing.
#if 0
class MidiEvent_MT32 {
public:
	MidiEvent_MT32 *_next;
	uint32 _msg; // 0xFFFFFFFF indicates a sysex message
	byte *_data;
	uint32 _len;

	MidiEvent_MT32(uint32 msg, byte *data, uint32 len) {
		_msg = msg;
		if (len > 0) {
			_data = new byte[len];
			memcpy(_data, data, len);
		}
		_len = len;
		_next = NULL;
	}

	MidiEvent_MT32() {
		if (_len > 0)
			delete _data;
	}
};

class MidiDriver_ThreadedMT32 : public MidiDriver_MT32 {
private:
	OSystem::Mutex _eventMutex;
	MidiEvent_MT32 *_events;
	TimerManager::TimerProc _timer_proc;

	void pushMidiEvent(MidiEvent_MT32 *event);
	MidiEvent_MT32 *popMidiEvent();

protected:
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

public:
	MidiDriver_ThreadedMT32(Audio::Mixer *mixer);

	void onTimer();
	void close();
	void setTimerCallback(void *timer_param, TimerManager::TimerProc timer_proc);
};


MidiDriver_ThreadedMT32::MidiDriver_ThreadedMT32(Audio::Mixer *mixer) : MidiDriver_MT32(mixer) {
	_events = NULL;
	_timer_proc = NULL;
}

void MidiDriver_ThreadedMT32::close() {
	MidiDriver_MT32::close();
	while ((popMidiEvent() != NULL)) {
		// Just eat any leftover events
	}
}

void MidiDriver_ThreadedMT32::setTimerCallback(void *timer_param, TimerManager::TimerProc timer_proc) {
	if (!_timer_proc || !timer_proc) {
		if (_timer_proc)
			_vm->_timer->removeTimerProc(_timer_proc);
		_timer_proc = timer_proc;
		if (timer_proc)
			_vm->_timer->installTimerProc(timer_proc, getBaseTempo(), timer_param, "MT32tempo");
	}
}

void MidiDriver_ThreadedMT32::pushMidiEvent(MidiEvent_MT32 *event) {
	Common::StackLock lock(_eventMutex);
	if (_events == NULL) {
		_events = event;
	} else {
		MidiEvent_MT32 *last = _events;
		while (last->_next != NULL)
			last = last->_next;
		last->_next = event;
	}
}

MidiEvent_MT32 *MidiDriver_ThreadedMT32::popMidiEvent() {
	Common::StackLock lock(_eventMutex);
	MidiEvent_MT32 *event;
	event = _events;
	if (event != NULL)
		_events = event->_next;
	return event;
}

void MidiDriver_ThreadedMT32::send(uint32 b) {
	MidiEvent_MT32 *event = new MidiEvent_MT32(b, NULL, 0);
	pushMidiEvent(event);
}

void MidiDriver_ThreadedMT32::sysEx(const byte *msg, uint16 length) {
	MidiEvent_MT32 *event = new MidiEvent_MT32(0xFFFFFFFF, msg, length);
	pushMidiEvent(event);
}

void MidiDriver_ThreadedMT32::onTimer() {
	MidiEvent_MT32 *event;
	while ((event = popMidiEvent()) != NULL) {
		if (event->_msg == 0xFFFFFFFF) {
			MidiDriver_MT32::sysEx(event->_data, event->_len);
		} else {
			MidiDriver_MT32::send(event->_msg);
		}
		delete event;
	}
}
#endif


// Plugin interface

class MT32EmuMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return _s("MT-32 emulator");
	}

	const char *getId() const {
		return "mt32";
	}

	MusicDevices getDevices() const;
	bool checkDevice(MidiDriver::DeviceHandle) const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices MT32EmuMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_MT32));
	return devices;
}

bool MT32EmuMusicPlugin::checkDevice(MidiDriver::DeviceHandle) const {
	if (!((Common::File::exists("MT32_CONTROL.ROM") && Common::File::exists("MT32_PCM.ROM")) ||
		(Common::File::exists("CM32L_CONTROL.ROM") && Common::File::exists("CM32L_PCM.ROM")))) {
			warning("The MT-32 emulator requires one of the two following file sets (not bundled with ScummVM):\n Either 'MT32_CONTROL.ROM' and 'MT32_PCM.ROM' or 'CM32L_CONTROL.ROM' and 'CM32L_PCM.ROM'");
			return false;
	}

	return true;
}

Common::Error MT32EmuMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_MT32(g_system->getMixer());

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(MT32)
	//REGISTER_PLUGIN_DYNAMIC(MT32, PLUGIN_TYPE_MUSIC, MT32EmuMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(MT32, PLUGIN_TYPE_MUSIC, MT32EmuMusicPlugin);
//#endif

#endif
