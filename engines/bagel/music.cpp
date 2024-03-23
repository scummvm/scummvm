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

#include "audio/mididrv.h"
#include "audio/midiparser.h"

#include "bagel/music.h"
#include "bagel/boflib/sound.h"

namespace Bagel {

MusicPlayer::MusicPlayer() {

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MusicPlayer::play(CBofSound *sound) {
	Common::StackLock lock(_mutex);

	if (_isPlaying && sound == _sound) {
		// Already playing
		return;
	}

	MidiParser *parser = nullptr;
	if (sound->m_chType == SOUND_TYPE_XM) {
		parser = MidiParser::createParser_XMIDI();
	} else if (sound->m_chType == SOUND_TYPE_QT) {
		parser = MidiParser::createParser_QT();
	} else {
		warning("Invalid sound %s passed to MusicPlayer", sound->m_szFileName);
		return;
	}

	if (parser->loadMusic(sound->m_pFileBuf, sound->m_iFileSize)) {
		stop();
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		// TODO: Set channel volume
		syncVolume();

		_isLooping = (sound->m_wLoops == 0);
		_isPlaying = true;
		_sound = sound;
	} else {
		warning("Failed to play %s", sound->m_szFileName);
		delete parser;
	}
}

void MusicPlayer::stop() {
	Audio::MidiPlayer::stop();
	_sound = nullptr;
}

} // End of namespace Bagel
