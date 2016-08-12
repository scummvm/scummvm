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

#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "titanic/sound/wave_file.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

CWaveFile::CWaveFile() : _owner(nullptr), _stream(nullptr),
		_soundType(Audio::Mixer::kPlainSoundType) {
}

CWaveFile::CWaveFile(QSoundManager *owner) : _owner(owner), _stream(nullptr),
		_soundType(Audio::Mixer::kPlainSoundType) {
}

CWaveFile::~CWaveFile() {
	if (_stream) {
		_owner->soundFreed(_soundHandle);
		delete _stream;
	}
}

int CWaveFile::fn1() {
	// TODO
	return 0;
}

bool CWaveFile::loadSound(const CString &name) {
	assert(!_stream);

	StdCWadFile file;
	if (!file.open(name))
		return false;

	Common::SeekableReadStream *stream = file.readStream();
	_size = stream->size();
	_stream = Audio::makeWAVStream(stream->readStream(_size), DisposeAfterUse::YES);
	_soundType = Audio::Mixer::kSFXSoundType;

	return true;
}

bool CWaveFile::loadSpeech(CDialogueFile *dialogueFile, int speechIndex) {
	DialogueResource *res = dialogueFile->openWaveEntry(speechIndex);
	if (!res)
		return false;

	byte *data = (byte *)malloc(res->_size);
	dialogueFile->read(res, data, res->_size);

	_size = res->_size;
	_stream = Audio::makeWAVStream(new Common::MemoryReadStream(data, _size, DisposeAfterUse::YES),
		DisposeAfterUse::YES);
	_soundType = Audio::Mixer::kSpeechSoundType;

	return true;
}

bool CWaveFile::loadMusic(const CString &name) {
	assert(!_stream);

	StdCWadFile file;
	if (!file.open(name))
		return false;

	Common::SeekableReadStream *stream = file.readStream();
	_size = stream->size();
	_stream = Audio::makeWAVStream(stream->readStream(_size), DisposeAfterUse::YES);
	_soundType = Audio::Mixer::kMusicSoundType;

	return true;
}

uint CWaveFile::getFrequency() const {
	return _stream->getRate();
}

} // End of namespace Titanic z
