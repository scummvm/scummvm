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

#include "m4/platform/sound/midi.h"
#include "m4/adv_r/adv_file.h"
#include "m4/vars.h"

#include "common/config-manager.h"
#include "audio/adlib_hmisos.h"
#include "audio/adlib_ms.h"
#include "audio/fmopl.h"
#include "audio/midiparser.h"
#include "audio/midiparser_hmp.h"
#include "audio/mt32gm.h"

namespace M4 {
namespace Sound {

int Midi::_midiEndTrigger;

Midi::Midi() {
	_driver = nullptr;
	_paused = false;
	_deviceType = MT_NULL;
	_midiParser = nullptr;
	_midiData = nullptr;
}

Midi::~Midi() {
	stop();

	if (_driver != nullptr) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}

	Common::StackLock lock(_mutex);

	if (_midiParser != nullptr)
		delete _midiParser;
	if (_midiData != nullptr)
		delete[] _midiData;
	if (_driver != nullptr) {
		delete _driver;
		_driver = nullptr;
	}
}

int Midi::open() {
	assert(_driver == nullptr);

	// Check the type of device that the user has configured.
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_deviceType = MidiDriver::getMusicType(dev);
	if (_deviceType == MT_GM && ConfMan.getBool("native_mt32"))
		_deviceType = MT_MT32;

	OPL::Config::OplType oplType;
	switch (_deviceType) {
	case MT_ADLIB:
		oplType = MidiDriver_ADLIB_HMISOS::detectOplType(OPL::Config::kOpl3) ? OPL::Config::kOpl3 : OPL::Config::kOpl2;
		MidiDriver_ADLIB_HMISOS *adLibDriver;
		adLibDriver = new MidiDriver_ADLIB_HMISOS(oplType);
		_driver = adLibDriver;

		Common::SeekableReadStream *instrumentBankStream;
		instrumentBankStream = SearchMan.createReadStreamForMember(Common::Path("MELODIC.BNK"));
		Common::SeekableReadStream *rhythmBankStream;
		rhythmBankStream = SearchMan.createReadStreamForMember(Common::Path("DRUM.BNK"));

		adLibDriver->loadInstrumentBanks(instrumentBankStream, rhythmBankStream);

		break;
	case MT_GM:
	case MT_MT32:
		_driver = new MidiDriver_MT32GM(MusicType::MT_GM);
		break;
	default:
		_driver = new MidiDriver_NULL_Multisource();
		break;
	}

	_midiParser = new MidiParser_HMP(0);

	_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
	// Riddle's MIDI data does not consistently set values for every controller
	// at the start of every track
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PITCH_BEND);
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_MODULATION);
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PANNING);
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_REVERB);
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_CHORUS);

	_midiParser->property(MidiParser::mpDisableAutoStartPlayback, true);
	// Riddle's MIDI data uses sustain
	_midiParser->property(MidiParser::mpSendSustainOffOnNotesOff, true);

	// Open the MIDI driver.
	int returnCode = _driver->open();
	if (returnCode != 0)
		error("Midi::open - Failed to open MIDI music driver - error code %d.", returnCode);

	syncSoundSettings();

	// Connect the driver and the parser.
	_midiParser->setMidiDriver(_driver);
	_midiParser->setTimerRate(_driver->getBaseTempo());
	_driver->setTimerCallback(this, &onTimer);

	return 0;
}

void Midi::load(byte* in, int32 size) {
	Common::StackLock lock(_mutex);

	if (_midiParser == nullptr)
		return;

	_midiParser->unloadMusic();

	if (_midiData != nullptr)
		delete[] _midiData;
	_midiData = new byte[size];

	Common::copy(in, in + size, _midiData);

	_midiParser->loadMusic(_midiData, size);
}

void Midi::play() {
	Common::StackLock lock(_mutex);

	if (_midiParser == nullptr || _driver == nullptr)
		return;

	_midiParser->startPlaying();
}

void Midi::pause(bool pause) {
	if (_paused == pause || _driver == nullptr)
		return;

	_paused = pause;

	if (_midiParser != nullptr) {
		Common::StackLock lock(_mutex);
		if (_paused) {
			_midiParser->pausePlaying();
		} else {
			_midiParser->resumePlaying();
		}
	}
}

void Midi::stop() {
	Common::StackLock lock(_mutex);

	if (_midiParser != nullptr) {
		_midiParser->stopPlaying();
		if (_driver != nullptr)
			_driver->deinitSource(0);
	}
}

bool Midi::isPlaying() {
	Common::StackLock lock(_mutex);

	return _midiParser->isPlaying();
}

void Midi::startFade(uint16 duration, uint16 targetVolume) {
	if (_driver == nullptr || _midiParser == nullptr || !_midiParser->isPlaying())
		return;

	_driver->startFade(0, duration, targetVolume);
}

bool Midi::isFading() {
	return _driver->isFading(0);
}

void Midi::syncSoundSettings() {
	if (_driver != nullptr)
		_driver->syncSoundSettings();
}

void Midi::midi_play(const char *name, int volume, bool loop, int trigger, int roomNum) {
	if (_driver == nullptr || _midiParser == nullptr)
		return;

	_midiEndTrigger = trigger;

	// Load in the resource
	Common::String fileName = expand_name_2_HMP(name, roomNum);
	int32 assetSize;
	MemHandle workHandle = rget(fileName, &assetSize);
	if (workHandle == nullptr)
		error("Could not find music - %s", fileName.c_str());

	HLock(workHandle);
	/*
	Common::DumpFile dump;
	dump.open(fileName.c_str());
	dump.write(*workHandle, assetSize);
	dump.close();
	*/

	load((byte *)*workHandle, assetSize);
	_midiParser->setTrack(0);
	_midiParser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
	// TODO Some calls use volume 0? What is that supposed to do?
	_driver->setSourceVolume(0, volume);

	play();

	/*
#ifdef TODO
	byte *pSrc = (byte *)*workHandle;

	MidiParser *parser = MidiParser::createParser_SMF();
	bool loaded = parser->loadMusic(pSrc, assetSize);

	if (loaded) {
		stop();
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;
		_isLooping = false;
		_isPlaying = true;
	}
#else
	// TODO: When music is properly implemented, trigger when music done
	if (trigger != -1)
		kernel_timing_trigger(10, trigger);
#endif
	*/

	HUnLock(workHandle);
	rtoss(fileName);
}

void Midi::task() {
	// No implementation
}

void Midi::loop() {
	// No implementation
}

void Midi::midi_fade_volume(int targetVolume, int duration) {
	uint16 durationMsec = duration * 1000 / 30;
	startFade(durationMsec, targetVolume);
	// TODO Should this stop playback when fade is completed?
	// Should this call return after the fade has completed?
}

void Midi::onTimer(void* data) {
	Midi *m = (Midi *)data;
	Common::StackLock lock(m->_mutex);

	if (m->_midiParser != nullptr) {
		m->_midiParser->onTimer();
		if (!m->_midiParser->isPlaying() && _midiEndTrigger >= 0) {
			// FIXME Can this trigger a deadlock on the mutex?
			kernel_timing_trigger(10, _midiEndTrigger);
			_midiEndTrigger = -1;
		}
	}
}

} // namespace Sound

void midi_play(const char *name, int volume, bool loop, int trigger, int roomNum) {
	_G(midi).midi_play(name, volume, loop, trigger, roomNum);
}

void midi_loop() {
	_G(midi).loop();
}

void midi_stop() {
	_G(midi).stop();
}

void midi_fade_volume(int targetVolume, int duration) {
	_G(midi).midi_fade_volume(targetVolume, duration);
}

} // namespace M4
