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

#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/ultima8.h"
#include "audio/midiparser.h"

namespace Ultima {
namespace Ultima8 {

byte MidiPlayer::_callbackData[2];

MidiPlayer::MidiPlayer() {
	MidiPlayer::createDriver();
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_isFMSynth = MidiDriver::getMusicType(dev) == MT_ADLIB;
	_callbackData[0] = 0;
	_callbackData[1] = 0;

	if (_driver) {
		int retValue = _driver->open();
		if (retValue == 0) {
			if (_nativeMT32)
				_driver->sendMT32Reset();
			else
				_driver->sendGMReset();

			_driver->setTimerCallback(this, &timerCallback);
		}
	}
}

MidiPlayer::~MidiPlayer() {
	_driver->close();
}

void MidiPlayer::load(byte *data, size_t size, int seqNo, bool speedHack) {
	if (!_driver)
		return;

	assert(seqNo == 0 || seqNo == 1);

	stop();

	if (size < 4)
		error("load() wrong music resource size");

	if (READ_BE_UINT32(data) != MKTAG('F', 'O', 'R', 'M')) {
		warning("load() Unexpected signature");
		_isPlaying = false;
	} else {
		_parser = MidiParser::createParser_XMIDI(xmidiCallback, _callbackData + seqNo);

		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		if (speedHack)
			_parser->setTempo(_driver->getBaseTempo() * 2);
		_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);
		_parser->property(MidiParser::mpDisableAutoStartPlayback, 1);

		int volume = g_engine->_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		setVolume(volume);

		if (!_parser->loadMusic(data, size))
			error("load() wrong music resource");
	}
}
void MidiPlayer::play(int trackNo, int branchIndex) {
	if (!_parser)
		return;

	if (!_parser->setTrack(trackNo)) {
		warning("play() invalid track number %i", trackNo);
		return;
	}
		
	if (branchIndex >= 0) {
		if (!_parser->jumpToIndex(branchIndex, false)) {
			warning("play() invalid branch index %i", branchIndex);
			// Track will play from the beginning instead
		}
	}

	if (!_parser->startPlaying()) {
		warning("play() failed to start playing");
	} else {
		_isPlaying = true;
	}
}

bool MidiPlayer::hasBranchIndex(uint8 index) {
	return _parser && _parser->hasJumpIndex(index);
}

void MidiPlayer::setLooping(bool loop) {
	_parser->property(MidiParser::mpAutoLoop, loop);
}

void MidiPlayer::xmidiCallback(byte eventData, void *data) {
	if (data == nullptr)
		return;

	*static_cast<byte*>(data) = eventData;
}


} // End of namespace Ultima8
} // End of namespace Ultima
