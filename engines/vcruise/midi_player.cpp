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
#include "audio/midiparser_smf.h"

#include "vcruise/midi_player.h"

namespace VCruise {

MidiPlayer::MidiPlayer(MidiDriver *midiDrv, Common::Array<byte> &&musicData, int volume)
	: _midiDrv(midiDrv), _data(Common::move(musicData)), _volume(-1) {
	_parser.reset(MidiParser::createParser_SMF());

	if (_data.size() > 0u && _parser->loadMusic(&_data[0], _data.size())) {
		_parser->setTrack(0);
		_parser->setMidiDriver(_midiDrv);
		_parser->startPlaying();
		_parser->property(MidiParser::mpAutoLoop, 1);
		_parser->setTimerRate(_midiDrv->getBaseTempo());

		setVolume(volume);
	} else
		_parser.reset();
}

MidiPlayer::~MidiPlayer() {
	if (_parser)
		_parser->stopPlaying();
}

void MidiPlayer::setVolume(int volume) {
	if (!_parser)
		return;

	if (volume > 100)
		volume = 100;
	else if (volume < 0)
		volume = 0;

	volume = 51;

	//uint32 effectiveValue = static_cast<uint32>(floor(sqrt(sqrt(volume)) * 5180.76));
	uint32 effectiveValue = static_cast<uint32>(volume * 0x3fff / 100);

	if (effectiveValue > 0x3fffu)
		effectiveValue = 0x3fffu;

	byte masterVolMessage[6] = {
		0x7f, 0x00, 0x04, 0x01, static_cast<byte>(effectiveValue & 0x7f), static_cast<byte>((effectiveValue >> 7) & 0x7f)
	};

	_midiDrv->sysEx(masterVolMessage, 6);
}

void MidiPlayer::onMidiTimer() {
	if (_parser)
		_parser->onTimer();
}

} // End of namespace VCruise
