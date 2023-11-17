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

#include "ultima/ultima8/audio/midi_player.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/music_flex.h"
#include "ultima/ultima8/games/game_data.h"

#include "audio/midiparser.h"
#include "audio/miles.h"

namespace Ultima {
namespace Ultima8 {

byte MidiPlayer::_callbackData[2];

MidiPlayer::MidiPlayer() : _parser(nullptr), _transitionParser(nullptr), _playingTransition(false) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	MusicType musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_ADLIB:
		MusicFlex *musicFlex;
		musicFlex = GameData::get_instance()->getMusic();
		_driver = Audio::MidiDriver_Miles_AdLib_create("", "", musicFlex->getAdlibTimbres(), nullptr);
		break;
	case MT_MT32:
	case MT_GM:
		_driver = Audio::MidiDriver_Miles_MIDI_create(MT_GM, "");
		break;
	default:
		_driver = new MidiDriver_NULL_Multisource();
		break;
	}

	_isFMSynth = (musicType == MT_ADLIB);
	_callbackData[0] = 0;
	_callbackData[1] = 0;

	if (_driver) {
		int retValue = _driver->open();
		if (retValue == 0) {
			_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
			_driver->setTimerCallback(this, &timerCallback);
			syncSoundSettings();
		} else {
			delete _driver;
			_driver = nullptr;
		}
	}
}

MidiPlayer::~MidiPlayer() {
	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
	}

	if (_transitionParser) {
		_transitionParser->unloadMusic();
		delete _transitionParser;
	}

	if (_driver) {
		_driver->close();
		delete _driver;
	}
}

void MidiPlayer::load(byte *data, size_t size, int seqNo) {
	if (!_driver)
		return;

	assert(seqNo == 0 || seqNo == 1);

	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = nullptr;
	}

	if (size < 4)
		error("load() wrong music resource size");

	if (READ_BE_UINT32(data) != MKTAG('F', 'O', 'R', 'M')) {
		warning("load() Unexpected signature");
	} else {
		_parser = MidiParser::createParser_XMIDI(xmidiCallback, _callbackData + seqNo, 0);

		_parser->setMidiDriver(_driver);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);
		_parser->property(MidiParser::mpDisableAutoStartPlayback, 1);

		if (!_parser->loadMusic(data, size))
			error("load() wrong music resource");
	}
}

void MidiPlayer::loadTransitionData(byte* data, size_t size) {
	if (!_driver)
		return;

	if (size < 4)
		error("loadTransitionData() wrong music resource size");

	if (READ_BE_UINT32(data) != MKTAG('F', 'O', 'R', 'M'))
		error("loadTransitionData() Unexpected signature");

	_transitionParser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);
	_transitionParser->setMidiDriver(_driver);
	_transitionParser->setTimerRate(_driver->getBaseTempo());
	_transitionParser->property(MidiParser::mpDisableAutoStartPlayback, 1);

	if (!_transitionParser->loadMusic(data, size))
		error("loadTransitionData() wrong music resource");
}

void MidiPlayer::play(int trackNo, int branchIndex) {
	if (!_parser || !_driver)
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

	// Abort any active fades and reset the source volume to neutral.
	if (_driver->isFading(0))
		_driver->abortFade(0);
	_driver->resetSourceVolume(0);
	if (_transitionParser) {
		_transitionParser->stopPlaying();
		_playingTransition = false;
	}

	if (!_parser->startPlaying()) {
		warning("play() failed to start playing");
	}
}

void MidiPlayer::playTransition(int trackNo, bool overlay) {
	if (!overlay && _parser)
		_parser->stopPlaying();

	if (!_transitionParser) {
		warning("playTransition() transition data not loaded");
		if (_parser)
			_parser->stopPlaying();
		return;
	}

	_transitionParser->setTrack(trackNo);
	if (overlay)
		_transitionParser->setTempo(_driver->getBaseTempo() * 2);
	_transitionParser->property(MidiParser::mpDisableAllNotesOffMidiEvents, overlay);

	_transitionParser->startPlaying();
	_playingTransition = true;
}

void MidiPlayer::stop() {
	if (_parser)
		_parser->stopPlaying();
	if (_transitionParser) {
		_transitionParser->stopPlaying();
		_playingTransition = false;
	}
}

void MidiPlayer::pause(bool pause) {
	if (pause) {
		if (_parser)
			_parser->pausePlaying();
		if (_transitionParser)
			_transitionParser->pausePlaying();
	} else {
		if (_parser)
			_parser->resumePlaying();
		if (_transitionParser)
			_transitionParser->resumePlaying();
	}
}

bool MidiPlayer::isPlaying() {
	return (_parser && _parser->isPlaying()) || _playingTransition;
}

void MidiPlayer::startFadeOut(uint16 length) {
	if (_driver)
		_driver->startFade(0, 1500, 0);
}

bool MidiPlayer::isFading() {
	return _driver && _driver->isFading(0);
}

void MidiPlayer::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

bool MidiPlayer::hasBranchIndex(uint8 index) {
	return _parser && _parser->hasJumpIndex(index);
}

void MidiPlayer::setLooping(bool loop) {
	if (_parser)
		_parser->property(MidiParser::mpAutoLoop, loop);
}

void MidiPlayer::xmidiCallback(byte eventData, void *data) {
	if (data == nullptr)
		return;

	*static_cast<byte*>(data) = eventData;
}

void MidiPlayer::onTimer() {
	if (_parser)
		_parser->onTimer();
	if (_transitionParser) {
		_transitionParser->onTimer();
		if (_playingTransition && !_transitionParser->isPlaying()) {
			// Transition has finished.
			if (_parser)
				// Stop the main track (which is still playing if the
				// transition was overlaid).
				_parser->stopPlaying();
			_playingTransition = false;
		}
	}
}

void MidiPlayer::timerCallback(void *data) {
	((MidiPlayer *)data)->onTimer();
}

} // End of namespace Ultima8
} // End of namespace Ultima
