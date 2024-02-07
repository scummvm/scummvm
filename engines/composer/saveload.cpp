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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/compression/deflate.h"
#include "graphics/paletteman.h"

#include "composer/composer.h"
#include "composer/graphics.h"

namespace Composer {

template<class T>
void ComposerEngine::syncArray(Common::Serializer &ser, Common::Array<T> &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	if (ser.isSaving()) {
		uint32 size = data.size();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (typename Common::Array<T>::iterator i = data.begin(); i != data.end(); i++) {
			sync<T>(ser, *i, minVersion, maxVersion);
		}
	} else {
		uint32 size = 0;
		data.clear();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (uint32 i = 0; i < size; i++) {
			T item;
			sync<T>(ser, item, minVersion, maxVersion);
			data.push_back(item);
		}
	}
}
template<class T>
void ComposerEngine::syncList(Common::Serializer &ser, Common::List<T> &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	if (ser.isSaving()) {
		uint32 size = data.size();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (typename Common::List<T>::iterator i = data.begin(); i != data.end(); i++) {
			sync<T>(ser, *i, minVersion, maxVersion);
		}
	} else {
		uint32 size = 0;
		data.clear();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (uint32 i = 0; i < size; i++) {
			T item;
			sync<T>(ser, item, minVersion, maxVersion);
			data.push_back(item);
		}
	}
}
template<class T>
void ComposerEngine::syncListReverse(Common::Serializer &ser, Common::List<T> &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	if (ser.isSaving()) {
		uint32 size = data.size();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (typename Common::List<T>::iterator i = data.reverse_begin(); i != data.end(); i--) {
			sync<T>(ser, *i, minVersion, maxVersion);
		}
	} else {
		uint32 size = 0;
		data.clear();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (uint32 i = 0; i < size; i++) {
			T item;
			sync<T>(ser, item, minVersion, maxVersion);
			data.push_front(item);
		}
	}
}
template<>
void ComposerEngine::sync<uint16>(Common::Serializer &ser, uint16 &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	ser.syncAsUint16LE(data, minVersion, maxVersion);
}
template<>
void ComposerEngine::sync<uint32>(Common::Serializer &ser, uint32 &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	ser.syncAsUint32LE(data, minVersion, maxVersion);
}
template<>
void ComposerEngine::sync<Library>(Common::Serializer &ser, Library &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	if (ser.isSaving()) {
		ser.syncAsUint16LE(data._id, minVersion, maxVersion);
		ser.syncString(data._group, minVersion, maxVersion);
	} else {
		uint16 id = 0;
		ser.syncAsUint16LE(id, minVersion, maxVersion);
		ser.syncString(_bookGroup, minVersion, maxVersion);
		loadLibrary(id);
	}
}
template<>
void ComposerEngine::syncListReverse<Library>(Common::Serializer &ser, Common::List<Library> &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	if (ser.isSaving()) {
		uint32 size = data.size();
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (Common::List<Library>::iterator i = data.reverse_begin(); i != data.end(); i--) {
			sync<Library>(ser, *i, minVersion, maxVersion);
		}
	} else {
		uint32 size = 0;
		ser.syncAsUint32LE(size, minVersion, maxVersion);
		for (uint32 i = 0; i < size; i++) {
			Library item;
			sync<Library>(ser, item, minVersion, maxVersion);
		}
	}
}
template<>
void ComposerEngine::sync<PendingPageChange>(Common::Serializer &ser, PendingPageChange &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	ser.syncAsUint16LE(data._pageId, minVersion, maxVersion);
	ser.syncAsByte(data._remove, minVersion, maxVersion);
}
template<>
void ComposerEngine::sync<OldScript *>(Common::Serializer &ser, OldScript *&data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	uint16 id = 0;
	uint32 pos = 0, delay = 0;
	if (ser.isSaving()) {
		pos = data->_stream->pos();
		id = data->_id;
		delay = data->_currDelay;
	}
	ser.syncAsUint32LE(pos);
	ser.syncAsUint16LE(id);
	ser.syncAsUint32LE(delay);
	if (ser.isLoading()) {
		data = new OldScript(id, getResource(ID_SCRP, id));
		data->_currDelay = delay;
		data->_stream->seek(pos, SEEK_SET);
	}
}
template<>
void ComposerEngine::sync<QueuedScript>(Common::Serializer &ser, QueuedScript &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	ser.syncAsUint32LE(data._baseTime);
	ser.syncAsUint32LE(data._duration);
	ser.syncAsUint32LE(data._count);
	ser.syncAsUint16LE(data._scriptId);
	if (ser.isLoading()) data._baseTime += _timeDelta;
}
template<>
void ComposerEngine::sync<Pipe *>(Common::Serializer &ser, Pipe *&data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	uint16 id = 0;
	uint32 offset = 0, tmp = 0;
	if (ser.isSaving()) {
		id = data->getPipeId();
		offset = data->getOffset();
		tmp = data->_bufferedResources.size();
	}
	ser.syncAsUint16LE(id);
	ser.syncAsUint32LE(offset);

	if (ser.isLoading()) {
		// On load, get and initialize streams
		Common::SeekableReadStream *stream;
		if (getGameType() == GType_ComposerV1) {
			stream = getResource(ID_PIPE, id);
			data = new OldPipe(stream, id);
		} else {
			stream = getResource(ID_ANIM, id);
			data = new Pipe(stream, id);
		}
		_pipeStreams.push_back(stream);
		data->setOffset(offset);
		ser.syncAsUint32LE(tmp);
		for (uint32 j = tmp; j > 0; j--) {
			uint32 tag = 0;
			ser.syncAsUint32LE(tag);
			ser.syncAsUint32LE(tmp);
			for (uint32 k = tmp; k > 0; k--) {
				ser.syncAsUint16LE(id);
				if (data->hasResource(tag, id)) {
					Common::SeekableReadStream *s = data->getResource(tag, id, true);
					delete s;
				}
			}
		}
	} else {
		ser.syncAsUint32LE(tmp);
		for (Pipe::DelMap::iterator i = data->_bufferedResources.begin(); i != data->_bufferedResources.end(); i++) {
			uint32 key = (*i)._key;
			ser.syncAsUint32LE(key);
			syncList<uint16>(ser, (*i)._value, minVersion, maxVersion);
		}
	}
}
template<>
void ComposerEngine::sync<AnimationEntry>(Common::Serializer &ser, AnimationEntry &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	ser.syncAsUint32LE(data.state);
	ser.syncAsUint16LE(data.counter);
	ser.syncAsUint16LE(data.prevValue);
}
template<>
void ComposerEngine::sync<Animation *>(Common::Serializer &ser, Animation *&data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	uint16 animId = 0, x = 0, y = 0;
	uint32 offset = 0, state = 0, param = 0;
	int32 size = 0;
	if (ser.isSaving()) {
		animId = data->_id;
		offset = data->_offset;
		x = data->_basePos.x;
		y = data->_basePos.x;
		state = data->_state;
		param = data->_eventParam;
		size = data->_size;
	}
	ser.syncAsUint16LE(animId);
	ser.syncAsUint32LE(offset);
	ser.syncAsUint16LE(x);
	ser.syncAsUint16LE(y);
	ser.syncAsUint32LE(state);
	ser.syncAsUint32LE(param);
	ser.syncAsUint32LE(size);
	if (ser.isLoading()) {
		// On load, get and initialize streams
		loadAnimation(data, animId, x, y, param, size);
		data->_offset = offset;
		data->_state = state;
		uint32 tmp = 0;
		ser.syncAsUint32LE(tmp);
		for (uint32 i = 0; i < tmp; i++) {
			sync<AnimationEntry>(ser, data->_entries[i], minVersion, maxVersion);
		}
	} else {
		syncArray<AnimationEntry>(ser, data->_entries, minVersion, maxVersion);
	}
}
template<>
void ComposerEngine::sync<Sprite>(Common::Serializer &ser, Sprite &data, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
		ser.syncAsUint16LE(data._id);
		ser.syncAsUint16LE(data._animId);
		ser.syncAsSint16LE(data._pos.x);
		ser.syncAsSint16LE(data._pos.y);
		ser.syncAsUint16LE(data._surface.w);
		ser.syncAsUint16LE(data._surface.h);
		ser.syncAsUint16LE(data._surface.pitch);
		ser.syncAsUint16LE(data._zorder);
		if (ser.isLoading())
			data._surface.setPixels(malloc(data._surface.h * data._surface.pitch));
		byte *pix = static_cast<byte *>(data._surface.getPixels());
		for (uint16 y = 0; y < data._surface.h; y++) {
			for (uint16 x = 0; x < data._surface.w; x++) {
				ser.syncAsByte(pix[x]);
			}
			pix += data._surface.pitch;
		}

}

Common::String ComposerEngine::getSaveStateName(int slot) const {
	return Common::String::format("%s.%02d", _targetName.c_str(), slot);
}

Common::Error ComposerEngine::loadGameState(int slot) {
	Common::String filename = getSaveStateName(slot);
	Common::InSaveFile *in;
	if (!(in = _saveFileMan->openForLoading(filename)))
		return Common::kPathNotFile;

	Common::Serializer ser(in, nullptr);
	byte magic[4];
	ser.syncBytes(magic, 4);
	if (magic[0] != 'C' || magic[1] != 'M' || magic[2] != 'P' || magic[3] != 'S')
		return Common::kUnknownError;

	ser.syncVersion(0);
	Common::String desc;
	ser.syncString(desc);
	uint32 tmp;
	ser.syncAsUint32LE(tmp);
	_rnd->setSeed(tmp);
	ser.syncAsUint32LE(_currentTime);
	_timeDelta = _system->getMillis() - _currentTime;
	_currentTime += _timeDelta;
	ser.syncAsUint32LE(_lastTime);
	_lastTime += _timeDelta;

	// Unload all Libraries
	Common::Array<uint16> libIds;
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		libIds.push_back((*i)._id);
	for (uint32 i = 0; i < libIds.size(); i++)
		unloadLibrary(libIds[i]);

	syncListReverse<Library>(ser, _libraries);
	ser.syncString(_bookGroup);

	syncArray<PendingPageChange>(ser, _pendingPageChanges);
	syncArray<uint16>(ser, _stack);
	syncArray<uint16>(ser, _vars);

	// Free outdated pointers
	for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		delete *i;
	}

	syncList<OldScript *>(ser, _oldScripts);
	syncArray<QueuedScript>(ser, _queuedScripts);

	ser.syncAsSint16LE(_lastMousePos.x);
	ser.syncAsSint16LE(_lastMousePos.y);
	g_system->warpMouse(_lastMousePos.x, _lastMousePos.y);
	ser.syncAsByte(_mouseEnabled);
	ser.syncAsByte(_mouseVisible);
	ser.syncAsUint16LE(_mouseSpriteId);

	// Free outdated pointers
	for (Common::List<Pipe *>::iterator i = _pipes.begin(); i != _pipes.end(); i++) {
		delete *i;
	}
	for (Common::Array<Common::SeekableReadStream *>::iterator i = _pipeStreams.begin(); i != _pipeStreams.end(); i++) {
		delete *i;
	}

	_pipeStreams.clear();
	syncListReverse<Pipe *>(ser, _pipes);

	// Free outdated pointers
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		delete *i;
	}

	syncList<Animation *>(ser, _anims);
	syncList<Sprite>(ser, _sprites);

	_dirtyRects.clear();

	// Redraw the whole screen
	_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	byte palbuf[256 * 3];
	ser.syncBytes(palbuf, 256 * 3);
	_system->getPaletteManager()->setPalette(palbuf, 0, 256);
	_needsUpdate = true;

	_mixer->stopAll();
	_audioStream = nullptr;

	// Restore the buffered audio
	ser.syncAsSint16LE(_currSoundPriority);
	int32 numSamples = 0;
	ser.syncAsSint32LE(numSamples);
	int16 *audioBuffer = (int16 *)malloc(numSamples * 2);
	for (int32 i = 0; i < numSamples; i++)
		ser.syncAsSint16LE(audioBuffer[i]);
	_audioStream = Audio::makeQueuingAudioStream(22050, false);
	_audioStream->queueBuffer((byte *)audioBuffer, numSamples * 2, DisposeAfterUse::YES, Audio::FLAG_16BITS);
	if (!_mixer->isSoundHandleActive(_soundHandle))
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);

	return Common::kNoError;
}

Common::Error ComposerEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String filename = getSaveStateName(slot);
	Common::OutSaveFile *out;
	if (!(out = _saveFileMan->openForSaving(filename)))
		return Common::kWritingFailed;

	Common::Serializer ser(nullptr, out);
	byte magic[4] = {'C', 'M', 'P', 'S'};
	ser.syncBytes(magic, 4);
	ser.syncVersion(0);
	Common::String desctmp = desc;
	ser.syncString(desctmp);
	uint32 tmp = _rnd->getSeed();
	ser.syncAsUint32LE(tmp);
	ser.syncAsUint32LE(_currentTime);
	ser.syncAsUint32LE(_lastTime);

	syncListReverse<Library>(ser, _libraries);
	ser.syncString(_bookGroup);

	syncArray<PendingPageChange>(ser, _pendingPageChanges);
	syncArray<uint16>(ser, _stack);
	syncArray<uint16>(ser, _vars);
	syncList<OldScript *>(ser, _oldScripts);
	syncArray<QueuedScript>(ser, _queuedScripts);

	ser.syncAsSint16LE(_lastMousePos.x);
	ser.syncAsSint16LE(_lastMousePos.y);
	ser.syncAsByte(_mouseEnabled);
	ser.syncAsByte(_mouseVisible);
	ser.syncAsUint16LE(_mouseSpriteId);

	syncListReverse<Pipe *>(ser, _pipes);
	syncList<Animation *>(ser, _anims);
	syncList<Sprite>(ser, _sprites);

	byte paletteBuffer[256 * 3];
	_system->getPaletteManager()->grabPalette(paletteBuffer, 0, 256);
	ser.syncBytes(paletteBuffer, 256 * 3);

	ser.syncAsSint16LE(_currSoundPriority);
	int16 audioBuffer[22050];
	int32 numSamples = _audioStream->readBuffer(audioBuffer, 22050);
	if (numSamples  == -1) numSamples = 0;
	ser.syncAsSint32LE(numSamples);
	for (int32 i = 0; i < numSamples; i++)
		ser.syncAsSint16LE(audioBuffer[i]);

	out->finalize();
	return Common::kNoError;
}
} // End of namespace Composer
