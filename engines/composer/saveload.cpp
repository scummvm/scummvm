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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef SAVING_ANYWHERE
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/zlib.h"
#include "graphics/palette.h"

#include "composer/composer.h"
#include "composer/graphics.h"

namespace Composer {
Common::String ComposerEngine::makeSaveGameName(int slot) {
	return (_targetName + Common::String::format(".%02d", slot));
}

Common::Error ComposerEngine::loadGameState(int slot) {
	Common::String filename = makeSaveGameName(slot);
	Common::InSaveFile *in;
	if (!(in = _saveFileMan->openForLoading(filename)))
		return Common::kPathNotFile;

	Common::Serializer ser(in, NULL);
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
	uint32 timeDelta = _system->getMillis() - _currentTime;
	_currentTime += timeDelta;
	ser.syncAsUint32LE(_lastTime);
	_lastTime += timeDelta;
	ser.syncString(_bookGroup);
	Common::Array<uint16> libIds;
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++) 
		libIds.push_back((*i)._id);
	for (uint32 i = 0; i < libIds.size(); i++) 
		unloadLibrary(libIds[i]);
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		ser.syncAsUint16LE(id);
		loadLibrary(id);
	}

	_pendingPageChanges.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		bool remove;
		ser.syncAsUint16LE(id);
		ser.syncAsByte(remove);
		_pendingPageChanges.push_back(PendingPageChange(id, remove));
	}
	_stack.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 svar;
		ser.syncAsUint16LE(svar);
		_stack.push_back(svar);
	}
	_vars.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 var;
		ser.syncAsUint16LE(var);
		_vars.push_back(var);
	}

	for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		delete *i;
	}
	_oldScripts.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		uint32 delay;
		ser.syncAsUint16LE(id);
		ser.syncAsUint32LE(delay);
		OldScript *oTmp = new OldScript(id, getResource(ID_SCRP, id));
		oTmp->_currDelay = delay;
		_oldScripts.push_back(oTmp);
	}
	_queuedScripts.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		QueuedScript qTmp;
		ser.syncAsUint32LE(qTmp._baseTime);
		qTmp._baseTime += timeDelta;
		ser.syncAsUint32LE(qTmp._duration);
		ser.syncAsUint32LE(qTmp._count);
		if(qTmp._count != 0) {
			assert(qTmp._count != 0);
		}
		ser.syncAsUint16LE(qTmp._scriptId);
		_queuedScripts.push_back(qTmp);
	}
	ser.syncAsSint16LE(_lastMousePos.x);
	ser.syncAsSint16LE(_lastMousePos.y);
	ser.syncAsByte(_mouseEnabled);
	ser.syncAsByte(_mouseVisible);
	ser.syncAsUint16LE(_mouseSpriteId);

	for (Common::List<Pipe *>::iterator i = _pipes.begin(); i != _pipes.end(); i++) {
		delete *i;
	}
	_pipes.clear();
	for (Common::Array<Common::SeekableReadStream *>::iterator i = _pipeStreams.begin(); i != _pipeStreams.end(); i++) {
		delete *i;
	}
	_pipeStreams.clear();

	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		uint32 offset;
		ser.syncAsUint16LE(id);
		ser.syncAsUint32LE(offset);
		Common::SeekableReadStream *stream = getResource(ID_ANIM, id);
		Pipe *pipe = new Pipe(stream, id);
		_pipes.push_front(pipe);
		_pipeStreams.push_back(stream);
		ser.syncAsUint32LE(tmp);
		for (uint32 j = tmp; j > 0; j--) {
			ser.syncAsUint32LE(tmp);
			for (uint32 k = tmp; k > 0; k--) {
				uint32 tag;
				ser.syncAsUint32LE(tag);
				ser.syncAsUint32LE(tmp);
				for (uint32 l = tmp; l > 0; l--) {
					ser.syncAsUint16LE(id);
					pipe->getResource(tag, id, true);
				}
			}
			pipe->nextFrame();
		}
		pipe->setOffset(offset);
	}

	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		delete *i;
	}
	_anims.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 animId, x, y;
		uint32 offset, state, param;
		int32 size;
		ser.syncAsUint16LE(animId);
		ser.syncAsUint32LE(offset);
		ser.syncAsUint16LE(x);
		ser.syncAsUint16LE(y);
		ser.syncAsUint32LE(state);
		ser.syncAsUint32LE(param);
		ser.syncAsUint32LE(size);
		Common::SeekableReadStream *stream = NULL;

		//TODO: extract following out into "loadAnim"
		// First, check the existing pipes.
		for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
			Pipe *pipe = *j;
			if (!pipe->hasResource(ID_ANIM, animId))
				continue;
			stream = pipe->getResource(ID_ANIM, animId, false);
			if (stream->size() >= size) break;
			stream = NULL;
		}
		// If we didn't find it, try the libraries.
		if (!stream) {
			Common::List<Library>::iterator i;
			for (i = _libraries.begin(); i != _libraries.end(); i++) {
				if (!hasResource(ID_ANIM, animId)) continue;
				stream = getResource(ID_ANIM, animId);
				if (stream->size() >= size) break;
				stream = NULL;
			}
			if (!stream) {
				warning("ignoring attempt to play invalid anim %d", animId);
				continue;
			}

			uint32 type = i->_archive->getResourceFlags(ID_ANIM, animId);

			// If the resource is a pipe itself, then load the pipe
			// and then fish the requested animation out of it.
			if (type != 1) {
				_pipeStreams.push_back(stream);
				Pipe *newPipe = new Pipe(stream, animId);
				_pipes.push_front(newPipe);
				newPipe->nextFrame();
				stream = newPipe->getResource(ID_ANIM, animId, false);
			}
		}

		Animation *anim = new Animation(stream, animId, Common::Point(x, y), param);
		anim->_offset = offset;
		anim->_state = state;
		uint32 tmp2;
		ser.syncAsUint32LE(tmp2);
		for (uint32 j = 0; j < tmp2; j++) {
			ser.syncAsUint32LE(anim->_entries[j].state);
			ser.syncAsUint16LE(anim->_entries[j].counter);
			ser.syncAsUint16LE(anim->_entries[j].prevValue);
		}
		_anims.push_back(anim);
	}
	_sprites.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		Sprite sprite;
		ser.syncAsUint16LE(sprite._id);
		ser.syncAsUint16LE(sprite._animId);
		ser.syncAsSint16LE(sprite._pos.x);
		ser.syncAsSint16LE(sprite._pos.y);
		ser.syncAsUint16LE(sprite._surface.w);
		ser.syncAsUint16LE(sprite._surface.h);
		ser.syncAsUint16LE(sprite._surface.pitch);
		ser.syncAsUint16LE(sprite._zorder);
		sprite._surface.pixels = malloc(sprite._surface.h * sprite._surface.pitch);
		byte *dest = static_cast<byte *>(sprite._surface.pixels);
		for (uint16 y = 0; y < sprite._surface.h; y++) {
			for (uint16 x = 0; x < sprite._surface.w; x++) {
				ser.syncAsByte(dest[x]);
			}
			dest += sprite._surface.pitch;
		}
		_sprites.push_back(sprite);
	}
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	byte palbuf[256 * 3];
	ser.syncBytes(palbuf, 256 * 3);
	_system->getPaletteManager()->setPalette(palbuf, 0, 256);
	_needsUpdate = true;

	_mixer->stopAll();
	_audioStream = NULL;

	ser.syncAsSint16LE(_currSoundPriority);
	int32 numSamples;
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

Common::Error ComposerEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String filename = makeSaveGameName(slot);
	Common::OutSaveFile *out;
	if (!(out = _saveFileMan->openForSaving(filename)))
		return Common::kWritingFailed;

	Common::Serializer ser(NULL, out);
	byte magic[4] = {'C', 'M', 'P', 'S'};
	ser.syncBytes(magic, 4);
	ser.syncVersion(0);
	Common::String desctmp = desc;
	ser.syncString(desctmp);
	uint32 tmp = _rnd->getSeed();
	ser.syncAsUint32LE(tmp);
	ser.syncAsUint32LE(_currentTime);
	ser.syncAsUint32LE(_lastTime);
	ser.syncString(_bookGroup);
	tmp = _libraries.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<Library>::const_iterator i = _libraries.reverse_begin(); i != _libraries.end(); i--) {
		uint16 tmp16 = (*i)._id;
		ser.syncAsUint16LE(tmp16);
	}
	tmp = _pendingPageChanges.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<PendingPageChange>::const_iterator i = _pendingPageChanges.begin(); i != _pendingPageChanges.end(); i++) {
		uint16 tmp = (*i)._pageId;
		bool tmpb = (*i)._remove;
		ser.syncAsUint16LE(tmp);
		ser.syncAsByte(tmpb);
	}
	tmp = _stack.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<uint16>::const_iterator i = _stack.begin(); i != _stack.end(); i++) {
		uint16 tmp16 = (*i);
		ser.syncAsUint16LE(tmp16);
	}
	tmp = _vars.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<uint16>::const_iterator i = _vars.begin(); i != _vars.end(); i++) {
		uint16 tmp16 = (*i);
		ser.syncAsUint16LE(tmp16);
	}
	tmp = _oldScripts.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<OldScript *>::const_iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		uint16 tmp16 = (*i)->_id;
		tmp = (*i)->_currDelay;
		ser.syncAsUint16LE(tmp16);
		ser.syncAsUint32LE(tmp);
	}
	tmp = _queuedScripts.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<QueuedScript>::const_iterator i = _queuedScripts.begin(); i != _queuedScripts.end(); i++) {
		tmp = (*i)._baseTime;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)._duration;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)._count;
		if(tmp != 0) {
			assert(tmp != 0);
		}
		uint16 tmp16 = (*i)._scriptId;
		ser.syncAsUint32LE(tmp);
		ser.syncAsUint16LE(tmp16);
	}
	ser.syncAsSint16LE(_lastMousePos.x);
	ser.syncAsSint16LE(_lastMousePos.y);
	ser.syncAsByte(_mouseEnabled);
	ser.syncAsByte(_mouseVisible);
	ser.syncAsUint16LE(_mouseSpriteId);

	tmp = _pipes.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<Pipe *>::const_iterator i = _pipes.reverse_begin(); i != _pipes.end(); i--) {
		uint16 tmp16 = (*i)->pipeId();
		tmp = (*i)->offset();
		ser.syncAsUint16LE(tmp16);
		ser.syncAsUint32LE(tmp);
		tmp = (*i)->_bufferedResources.size();
		ser.syncAsUint32LE(tmp);
		for (Common::Array<Pipe::DelMap>::const_iterator j = (*i)->_bufferedResources.begin(); j != (*i)->_bufferedResources.end(); j++) {
			tmp = (*j).size();
			ser.syncAsUint32LE(tmp);
			for (Pipe::DelMap::const_iterator k = (*j).begin(); k != (*j).end(); k++) {
				tmp = (*k)._key;
				ser.syncAsUint32LE(tmp);
				tmp = (*k)._value.size();
				ser.syncAsUint32LE(tmp);
				for (Common::List<uint16>::const_iterator l = (*k)._value.begin(); l != (*k)._value.end(); l++) {
					tmp16 = (*l);
					ser.syncAsUint16LE(tmp16);
				}
			}
		}
	}

	tmp = _anims.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<Animation *>::const_iterator i = _anims.begin(); i != _anims.end(); i++) {
		uint16 tmp16 = (*i)->_id;
		tmp = (*i)->_offset;
		ser.syncAsUint16LE(tmp16);
		ser.syncAsUint32LE(tmp);
		tmp16 = (*i)->_basePos.x;
		ser.syncAsUint16LE(tmp16);
		tmp16 = (*i)->_basePos.y;
		ser.syncAsUint16LE(tmp16);
		tmp = (*i)->_state;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)->_eventParam;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)->_size;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)->_entries.size();
		ser.syncAsUint32LE(tmp);
		for (Common::Array<AnimationEntry>::const_iterator j = (*i)->_entries.begin(); j != (*i)->_entries.end(); j++) {
			tmp = (*j).state;
			tmp16 = (*j).counter;
			ser.syncAsUint32LE(tmp);
			ser.syncAsUint16LE(tmp16);
			tmp16 = (*j).prevValue;
			ser.syncAsUint16LE(tmp16);
		}
	}
	tmp = _sprites.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<Sprite>::const_iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		Sprite sprite(*i);
		ser.syncAsUint16LE(sprite._id);
		ser.syncAsUint16LE(sprite._animId);
		ser.syncAsSint16LE(sprite._pos.x);
		ser.syncAsSint16LE(sprite._pos.y);
		ser.syncAsUint16LE(sprite._surface.w);
		ser.syncAsUint16LE(sprite._surface.h);
		ser.syncAsUint16LE(sprite._surface.pitch);
		ser.syncAsUint16LE(sprite._zorder);
		byte *src = static_cast<byte *>((*i)._surface.pixels);
		for (uint16 y = 0; y < sprite._surface.h; y++) {
			for (uint x = 0; x < sprite._surface.w; x++) {
				ser.syncAsByte(src[x]);
			}
			src += (*i)._surface.pitch;
		}
	}
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
}
#endif
