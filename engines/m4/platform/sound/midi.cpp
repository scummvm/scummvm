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

#include "audio/midiparser.h"
#include "m4/platform/sound/midi.h"
#include "m4/adv_r/adv_file.h"
#include "m4/vars.h"

namespace M4 {
namespace Sound {

int Midi::_midiEndTrigger;

Midi::Midi(Audio::Mixer *mixer) : _mixer(mixer) {
	Midi::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void Midi::midi_play(const char *name, int volume, int loop, int trigger, int roomNum) {
	MemHandle workHandle;
	int32 assetSize;

	_midiEndTrigger = trigger;

	// Load in the resource
	Common::String fileName = expand_name_2_HMP(name, roomNum);
	if ((workHandle = rget(fileName, &assetSize)) == nullptr)
		error("Could not find music - %s", fileName.c_str());

	HLock(workHandle);
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

	HUnLock(workHandle);
	rtoss(fileName);
}

void Midi::task() {
	// No implementation
}

void Midi::loop() {
	// No implementation
}

void Midi::set_overall_volume(int vol) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol * 255 / 100);
}

int Midi::get_overall_volume() const {
	return _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) * 100 / 255;
}

} // namespace Sound

void midi_play(const char *name, int volume, int loop, int trigger, int roomNum) {
	_G(midi).midi_play(name, volume, loop, trigger, roomNum);
}

void midi_loop() {
	_G(midi).loop();
}

void midi_stop() {
	_G(midi).stop();
}

void midi_set_overall_volume(int vol) {
	_G(midi).set_overall_volume(vol);
}

int midi_get_overall_volume() {
	return _G(midi).get_overall_volume();
}

void midi_fade_volume(int val1, int val2) {
	warning("TODO: midi_fade_volume");
}

} // namespace M4
