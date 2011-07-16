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
 * $URL$
 * $Id$
 *
 */
#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/fs.h"
#include "common/keyboard.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"
#include "graphics/palette.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "composer/composer.h"
#include "composer/resource.h"

namespace Composer {

// bitmap compression types
enum {
	kBitmapUncompressed = 0,
	kBitmapSpp32 = 1,
	kBitmapSLW8 = 3,
	kBitmapRLESLWM = 4,
	kBitmapSLWM = 5
};

// new script ops
enum {
	kOpPlusPlus = 0x1,
	kOpMinusMinus = 0x2,
	kOpAssign = 0x3,
	kOpAdd = 0x4,
	kOpSubtract = 0x5,
	kOpMultiply = 0x6,
	kOpDivide = 0x7,
	kOpModulo = 0x8,
	kOpMaybeAlsoAssign = 0x9,
	kOpBooleanAssign = 0xA,
	kOpNegate = 0xB,
	kOpAnd = 0xC,
	kOpOr = 0xD,
	kOpXor = 0xE,
	kOpNotPositive = 0xF,
	kOpSqrt = 0x10,
	kOpRandom = 0x11,
	kOpExecuteScript = 0x12,
	kOpCallFunc = 0x13,
	kOpBoolLessThanEq = 0x14,
	kOpBoolLessThan = 0x15,
	kOpBoolGreaterThanEq = 0x16,
	kOpBoolGreaterThan = 0x17,
	kOpBoolEqual = 0x18,
	kOpBoolNotEqual = 0x19,
	kOpSaveArgs = 0x1A,
	kOpRestoreArgs = 0x1B,
	kOpSetReturnValue = 0x20,
	kOpLessThanEq = 0x21,
	kOpLessThan = 0x22,
	kOpGreaterThanEq = 0x23,
	kOpGreaterThan = 0x24,
	kOpEqual = 0x25,
	kOpNotEqual = 0x26,
	kOpJump = 0x80,
	kOpJumpIfNot = 0x81,
	kOpJumpIf = 0x82,
	kOpJumpIfNotValue = 0x83,
	kOpJumpIfValue = 0x84
};

enum {
	kFuncPlayAnim = 35001,
	kFuncStopAnim = 35002,
	// (no 35003)
	kFuncQueueScript = 35004,
	kFuncDequeueScript = 35005,
	kFuncSetCursor = 35006,
	kFuncGetCursor = 35007,
	kFuncShowCursor = 35008,
	kFuncHideCursor = 35009,
	// (no 35010)
	kFuncActivateButton = 35011,
	kFuncDeactivateButton = 35012,
	kFuncNewPage = 35013,
	kFuncLoadPage = 35014,
	kFuncUnloadPage = 35015,
	kFuncSetPalette = 35016,
	kFuncSaveVars = 35017,
	kFuncLoadVars = 35018,
	kFuncQueueScriptOnce = 35019,
	kFuncGetMousePos = 35020,
	kFuncChangeBackground = 35021,
	kFuncSetBackgroundColor = 35022,
	kFuncClearSprites = 35023,
	kFuncAddSprite = 35024,
	kFuncRemoveSprite = 35025,
	kFuncQuit = 35026,
	kFuncSaveData = 35027,
	kFuncLoadData = 35028,
	kFuncGetSpriteSize = 35029
};

// TODO: params: x, y, event param for done
Animation::Animation(Common::SeekableReadStream *stream, uint16 id, Common::Point basePos, uint32 eventParam)
	: _stream(stream), _id(id), _basePos(basePos), _eventParam(eventParam) {
	uint32 size = _stream->readUint32LE();
	_state = _stream->readUint32LE() + 1;

	// probably total size?
	uint32 unknown = _stream->readUint32LE();

	debug(8, "anim: size %d, state %08x, unknown %08x", size, _state, unknown);

	for (uint i = 0; i < size; i++) {
		AnimationEntry entry;
		entry.op = _stream->readUint16LE();
		entry.word6 = _stream->readUint16LE();
		entry.dword0 = _stream->readUint16LE();
		entry.counter = 0;
		entry.word10 = 0;
		debug(8, "anim entry: %04x, %04x, %04x", entry.op, entry.word6, entry.dword0);
		_entries.push_back(entry);
	}

	_offset = _stream->pos();
}

Animation::~Animation() {
	delete _stream;
}

void Animation::seekToCurrPos() {
	_stream->seek(_offset, SEEK_SET);
}

Pipe::Pipe(Common::SeekableReadStream *stream) {
	_offset = 0;
	_stream = stream;

	nextFrame();
}

void Pipe::nextFrame() {
	if (_offset == (uint)_stream->size())
		return;

	_stream->seek(_offset, SEEK_SET);

	uint32 tagCount = _stream->readUint32LE();
	_offset += 4;
	for (uint i = 0; i < tagCount; i++) {
		uint32 tag = _stream->readUint32BE();
		uint32 count = _stream->readUint32LE();
		_offset += 8;

		ResourceMap &resMap = _types[tag];

		_offset += (12 * count);
		//uint32 baseOffset = _offset;
		for (uint j = 0; j < count; j++) {
			uint32 offset = _stream->readUint32LE();
			uint32 size = _stream->readUint32LE();
			uint16 id = _stream->readUint16LE();
			uint32 unknown = _stream->readUint16LE(); // frame id?
			debug(9, "pipe: %s/%d: offset %d, size %d, unknown %d", tag2str(tag), id, offset, size, unknown);

			PipeResourceEntry entry;
			entry.size = size;
			entry.offset = _offset;
			resMap[id].entries.push_back(entry);

			_offset += size;
		}
		_stream->seek(_offset, SEEK_SET);
	}
}

bool Pipe::hasResource(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		return false;

	return _types[tag].contains(id);
}

Common::SeekableReadStream *Pipe::getResource(uint32 tag, uint16 id, bool buffering) {
	if (!_types.contains(tag))
		error("Pipe does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const PipeResource &res = resMap[id];

	if (res.entries.size() == 1) {
		Common::SeekableReadStream *stream = new Common::SeekableSubReadStream(_stream,
			res.entries[0].offset, res.entries[0].offset + res.entries[0].size);
		if (buffering)
			_types[tag].erase(id);
		return stream;
	}

	// If there are multiple entries in the pipe, we have to concaternate them together.

	uint32 size = 0;
	for (uint i = 0; i < res.entries.size(); i++)
		size += res.entries[i].size;

	byte *buffer = (byte *)malloc(size);
	uint32 offset = 0;
	for (uint i = 0; i < res.entries.size(); i++) {
		_stream->seek(res.entries[i].offset, SEEK_SET);
		_stream->read(buffer + offset, res.entries[i].size);
		offset += res.entries[i].size;
	}
	if (buffering)
		_types[tag].erase(id);
	return new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
}

void ComposerEngine::playAnimation(uint16 animId, int16 x, int16 y, int16 eventParam) {
	// First, we check if this animation is already playing,
	// and if it is, we sabotage that running one first.
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;
		if (anim->_id != animId)
			continue;

		stopAnimation(*i);
		break;
	}

	Common::SeekableReadStream *stream = NULL;
	Pipe *newPipe = NULL;

	// First, check the existing pipes.
	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		Pipe *pipe = *j;
		if (!pipe->hasResource(ID_ANIM, animId))
			continue;
		stream = pipe->getResource(ID_ANIM, animId, false);
		break;
	}

	// If we didn't find it, try the libraries.
	if (!stream) {
		if (!hasResource(ID_ANIM, animId)) {
			warning("ignoring attempt to play invalid anim %d", animId);
			return;
		}
		stream = getResource(ID_ANIM, animId);

		uint32 type = 0;
		for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
			if (i->_archive->hasResource(ID_ANIM, animId)) {
				type = i->_archive->getResourceFlags(ID_ANIM, animId);
				break;
			}

		// If the resource is a pipe itself, then load the pipe
		// and then fish the requested animation out of it.
		if (type != 1) {
			newPipe = new Pipe(stream);
			_pipes.push_front(newPipe);
			stream = newPipe->getResource(ID_ANIM, animId, false);
		}
	}

	Animation *anim = new Animation(stream, animId, Common::Point(x, y), eventParam);
	_anims.push_back(anim);
	if (newPipe)
		newPipe->_anim = anim;
	runEvent(1, animId, eventParam, 0);
}

void ComposerEngine::stopAnimation(Animation *anim, bool localOnly, bool pipesOnly) {
	// disable the animation
	anim->_state = 0;

	// stop any animations it may have spawned
	for (uint j = 0; j < anim->_entries.size(); j++) {
		AnimationEntry &entry = anim->_entries[j];
		if (!entry.word10)
			continue;
		if (localOnly) {
			if (pipesOnly)
				continue;
			// TODO: stop audio if needed
			if (entry.op != 4)
				continue;
			removeSprite(entry.word10, anim->_id);
		} else {
			if (entry.op != 3)
				continue;
			for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
				if ((*i)->_id == entry.word10)
					stopAnimation(*i);
			}
		}
	}

	// kill any pipe owned by the animation
	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		Pipe *pipe = *j;
		if (pipe->_anim != anim)
			continue;
		j = _pipes.reverse_erase(j);
		delete pipe;
		break;
	}
}

void ComposerEngine::playWaveForAnim(uint16 id, bool bufferingOnly) {
	Common::SeekableReadStream *stream = NULL;
	if (!bufferingOnly && hasResource(ID_WAVE, id)) {
		stream = getResource(ID_WAVE, id);
	} else {
		for (Common::List<Pipe *>::iterator k = _pipes.begin(); k != _pipes.end(); k++) {
			Pipe *pipe = *k;
			if (!pipe->hasResource(ID_WAVE, id))
				continue;
			stream = pipe->getResource(ID_WAVE, id, true);
			break;
		}
	}
	if (!stream)
		return;
	// FIXME: non-pipe buffers have fixed wav header (data at +44, size at +40)
	// FIXME: deal with word6 (priority)
	byte *buffer = (byte *)malloc(stream->size());
	stream->read(buffer, stream->size());
	if (!_audioStream)
		_audioStream = Audio::makeQueuingAudioStream(22050, false);
	_audioStream->queueBuffer(buffer, stream->size(), DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	delete stream;
	if (!_mixer->isSoundHandleActive(_soundHandle))
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
}

void ComposerEngine::processAnimFrame() {
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;

		anim->seekToCurrPos();

		if (anim->_state <= 1) {
			bool normalEnd = (anim->_state == 1);
			if (normalEnd) {
				runEvent(2, anim->_id, anim->_eventParam, 0);
			}
			stopAnimation(anim, true, normalEnd);
			delete anim;
			i = _anims.reverse_erase(i);

			continue;
		}

		for (uint j = 0; j < anim->_entries.size(); j++) {
			AnimationEntry &entry = anim->_entries[j];
			if (entry.op != 1)
				break;
			if (entry.counter) {
				entry.counter--;
			} else {
				if ((anim->_state > 1) && (anim->_stream->pos() == anim->_stream->size())) {
					warning("anim with id %d ended too soon", anim->_id);
					anim->_state = 0;
					break;
				}

				uint16 event = anim->_stream->readUint16LE();
				anim->_offset += 2;
				if (event == 0xffff) {
					entry.counter = anim->_stream->readUint16LE() - 1;
					anim->_offset += 2;
				} else {
					runEvent(event, anim->_id, 0, 0);
				}
			}
		}
	}

	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;

		anim->_state--;

		for (uint j = 0; j < anim->_entries.size(); j++) {
			AnimationEntry &entry = anim->_entries[j];

			// TODO: only skip these at the start
			if (entry.op == 1)
				continue;

			if (entry.counter) {
				entry.counter--;
				if (entry.op == 2 && entry.word10) {
					debug(4, "anim: continue play wave %d", entry.word10);
					playWaveForAnim(entry.word10, true);
				}
			} else {
				anim->seekToCurrPos();
				if ((anim->_state > 1) && (anim->_stream->pos() == anim->_stream->size())) {
					warning("anim with id %d ended too soon", anim->_id);
					anim->_state = 0;
					break;
				}

				uint16 data = anim->_stream->readUint16LE();
				anim->_offset += 2;
				if (data == 0xffff) {
					entry.counter = anim->_stream->readUint16LE() - 1;
					anim->_offset += 2;
				} else {
					switch (entry.op) {
					case 1:
						// TODO
						warning("ignoring tingie (%d)", data);
						break;
					case 2:
						debug(4, "anim: play wave %d", data);
						playWaveForAnim(data, false);
						break;
					case 3:
						debug(4, "anim: play anim %d", data);
						playAnimation(data, anim->_basePos.x, anim->_basePos.y, 1);
						break;
					case 4:
						if (entry.word10 && (!data || data != entry.word10)) {
							debug(4, "anim: erase sprite %d", entry.word10);
							removeSprite(entry.word10, anim->_id);
						}
						if (data) {
							uint16 x = anim->_stream->readUint16LE();
							uint16 y = anim->_stream->readUint16LE();
							Common::Point pos(x, y);
							anim->_offset += 4;
							uint16 animId = anim->_id;
							if (anim->_state == entry.dword0)
								animId = 0;
							debug(4, "anim: draw sprite %d at (relative) %d,%d", data, x, y);
							addSprite(data, animId, entry.word6, anim->_basePos + pos);
						}
						break;
					default:
						warning("unknown anim op %d", entry.op);
					}

					entry.word10 = data;
				}
			}
		}
	}

	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		Pipe *pipe = *j;
		pipe->nextFrame();
	}
}

void ComposerEngine::addSprite(uint16 id, uint16 animId, uint16 zorder, const Common::Point &pos) {
	// TODO: re-use old sprite
	removeSprite(id, animId);

	Sprite sprite;
	sprite.id = id;
	sprite.animId = animId;
	sprite.zorder = zorder;
	sprite.pos = pos;
	if (!initSprite(sprite)) {
		warning("ignoring addSprite on invalid sprite %d", id);
		return;
	}

	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (sprite.zorder <= i->zorder)
			continue;
		// insert *before* this sprite
		_sprites.insert(i, sprite);
		return;
	}
	_sprites.push_back(sprite);
}

void ComposerEngine::removeSprite(uint16 id, uint16 animId) {
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (i->id != id)
			continue;
		if (i->animId && animId && (i->animId != animId))
			continue;
		i->surface.free();
		i = _sprites.reverse_erase(i);
	}
}

ComposerEngine::ComposerEngine(OSystem *syst, const ComposerGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_rnd = new Common::RandomSource("composer");
	_audioStream = NULL;
}

ComposerEngine::~ComposerEngine() {
	DebugMan.clearAllDebugChannels();

	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		delete i->_archive;

	delete _rnd;
}

Common::Error ComposerEngine::run() {
	Common::Event event;

	_vars.resize(1000);
	for (uint i = 0; i < _vars.size(); i++)
		_vars[i] = 0;

	_queuedScripts.resize(10);
	for (uint i = 0; i < _queuedScripts.size(); i++) {
		_queuedScripts[i]._count = 0;
		_queuedScripts[i]._scriptId = 0;
	}

	_directoriesToStrip = 1;
	if (!_bookIni.loadFromFile("book.ini")) {
		_directoriesToStrip = 0;
		if (!_bookIni.loadFromFile("programs/book.ini"))
			error("failed to find book.ini");
	}

	uint width = 640;
	if (_bookIni.hasKey("Width", "Common"))
		width = atoi(getStringFromConfig("Common", "Width").c_str());
	uint height = 480;
	if (_bookIni.hasKey("Height", "Common"))
		height = atoi(getStringFromConfig("Common", "Height").c_str());
	initGraphics(width, height, true);
	_surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	loadLibrary(0);

	uint fps = atoi(getStringFromConfig("Common", "FPS").c_str());
	uint frameTime = 1000 / fps;
	uint32 lastDrawTime = 0;
	while (!shouldQuit()) {
		for (uint i = 0; i < _pendingPageChanges.size(); i++) {
			if (_pendingPageChanges[i]._remove)
				unloadLibrary(_pendingPageChanges[i]._pageId);
			else
				loadLibrary(_pendingPageChanges[i]._pageId);
		}
		_pendingPageChanges.clear();

		uint32 thisTime = _system->getMillis();
		for (uint i = 0; i < _queuedScripts.size(); i++) {
			QueuedScript &script = _queuedScripts[i];
			if (!script._count)
				continue;
			if (script._baseTime + script._duration > thisTime)
				continue;
			if (script._count != 0xffff)
				script._count--;
			script._baseTime = thisTime;
			runScript(script._scriptId, i, 0, 0);
		}

		// _system->delayMillis(frameTime + lastDrawTime - thisTime);
		if (lastDrawTime + frameTime <= thisTime) {
			lastDrawTime += frameTime;

			for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
				drawSprite(*i);
			}

			_system->copyRectToScreen((byte *)_surface.pixels, _surface.pitch, 0, 0, _surface.w, _surface.h);
			_system->updateScreen();

			processAnimFrame();
		}

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				break;

			case Common::EVENT_LBUTTONUP:
				break;

			case Common::EVENT_RBUTTONDOWN:
				break;

			case Common::EVENT_MOUSEMOVE:
				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					/*if (event.kbd.hasFlags(Common::KBD_CTRL)) {
						// Start the debugger
						getDebugger()->attach();
						getDebugger()->onFrame();
					}*/
					break;

				case Common::KEYCODE_ESCAPE:
					quitGame();
					break;

				default:
					break;
				}
				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				quitGame();
				break;

			default:
				break;
			}
		}

		_system->delayMillis(20);
	}

	return Common::kNoError;
}

Common::String ComposerEngine::getStringFromConfig(const Common::String &section, const Common::String &key) {
	Common::String value;
	if (!_bookIni.getKey(key, section, value))
		error("failed to find key '%s' in section '%s' of book config", key.c_str(), section.c_str());
	return value;
}

Common::String ComposerEngine::getFilename(const Common::String &section, uint id) {
	Common::String key = Common::String::format("%d", id);
	Common::String filename = getStringFromConfig(section, key);
	while (filename.size() && (filename[0] == '~' || filename[0] == ':' || filename[0] == '\\'))
		filename = filename.c_str() + 1;

	uint slashesToStrip = _directoriesToStrip;
	while (slashesToStrip--) {
		for (uint i = 0; i < filename.size(); i++) {
			if (filename[i] != '\\')
				continue;
			filename = filename.c_str() + i + 1;
			break;
		}
	}

	Common::String outFilename;
	for (uint i = 0; i < filename.size(); i++) {
		if (filename[i] == '\\')
			outFilename += '/';
		else
			outFilename += filename[i];
	}
	return outFilename;
}

void ComposerEngine::loadLibrary(uint id) {
	if (!id)
		id = atoi(getStringFromConfig("Common", "StartUp").c_str());
	Common::String filename = getFilename("Libs", id);

	Library library;

	library._id = id;
	library._archive = new ComposerArchive();
	if (!library._archive->openFile(filename))
		error("failed to open '%s'", filename.c_str());
	// FIXME: push in front?
	_libraries.push_front(library);

	/*Common::SeekableReadStream *stream = _archive->getResource(ID_ANIM, 1004);
	byte buf[stream->size()];
	stream->read(buf, stream->size());
	Common::hexdump(buf, stream->size());
	delete stream;*/

	// add background sprite, if it exists
	if (hasResource(ID_BMAP, 1000))
		addSprite(1000, 0, 0xffff, Common::Point());

	// TODO: better CTBL logic
	loadCTBL(1000, 100);

	// Run the startup script.
	runScript(1000, 0, 0, 0);

	runEvent(3, id, 0, 0);
}

void ComposerEngine::unloadLibrary(uint id) {
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++) {
		if (i->_id != id)
			continue;

		for (Common::List<Animation *>::iterator j = _anims.begin(); j != _anims.end(); j++) {
			delete *j;
		}
		_anims.clear();
		for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
			delete *j;
		}
		_pipes.clear();

		for (Common::List<Sprite>::iterator j = _sprites.begin(); j != _sprites.end(); j++) {
			j->surface.free();
		}
		_sprites.clear();

		_mixer->stopAll();
		_audioStream = NULL;

		for (uint j = 0; j < _queuedScripts.size(); j++) {
			_queuedScripts[j]._count = 0;
			_queuedScripts[j]._scriptId = 0;
		}

		_libraries.erase(i);
		return;
	}

	error("tried to unload library %d, which isn't loaded", id);
}

bool ComposerEngine::hasResource(uint32 tag, uint16 id) {
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		if (i->_archive->hasResource(tag, id))
			return true;

	return false;
}

Common::SeekableReadStream *ComposerEngine::getResource(uint32 tag, uint16 id) {
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		if (i->_archive->hasResource(tag, id))
			return i->_archive->getResource(tag, id);

	error("No loaded library contains '%s' %04x", tag2str(tag), id);
}

void ComposerEngine::runEvent(uint16 id, int16 param1, int16 param2, int16 param3) {
	if (!hasResource(ID_EVNT, id))
		return;

	Common::SeekableReadStream *stream = getResource(ID_EVNT, id);
	if (stream->size() != 2)
		error("bad EVNT size %d", stream->size());
	uint16 scriptId = stream->readUint16LE();
	delete stream;

	debug(2, "running event %d via script %d(%d, %d, %d)", id, scriptId, param1, param2, param3);

	runScript(scriptId, param1, param2, param3);
}

int16 ComposerEngine::runScript(uint16 id, int16 param1, int16 param2, int16 param3) {
	_vars[1] = param1;
	_vars[2] = param2;
	_vars[3] = param3;

	runScript(id);

	return _vars[0];
}

int16 ComposerEngine::getArg(uint16 arg, uint16 type) {
	switch (type) {
	case 0:
		return (int16)arg;
	case 1:
		return (int16)_vars[arg];
	case 2:
		return (int16)_vars[_vars[arg]];
	default:
		error("invalid argument type %d (getting arg %d)", type, arg);
	}
}

void ComposerEngine::setArg(uint16 arg, uint16 type, uint16 val) {
	switch (type) {
	case 1:
		_vars[arg] = val;
		break;
	case 2:
		_vars[_vars[arg]] = val;
		break;
	default:
		error("invalid argument type %d (setting arg %d)", type, arg);
	}

}

void ComposerEngine::runScript(uint16 id) {
	if (!hasResource(ID_SCRP, id)) {
		warning("ignoring attempt to run script %d, because it doesn't exist", id);
		return;
	}

	uint stackBase = _stack.size();
	_stack.resize(_stack.size() + 19);

	Common::SeekableReadStream *stream = getResource(ID_SCRP, id);
	if (stream->size() < 2)
		error("SCRP was too small (%d)", stream->size());
	uint16 size = stream->readUint16LE();
	if (stream->size() < 2 + 2*size)
		error("SCRP was too small (%d, but claimed %d entries)", stream->size(), size);
	uint16 *script = new uint16[size];
	for (uint i = 0; i < size; i++)
		script[i] = stream->readUint16LE();
	delete stream;

	uint16 pos = 0;
	bool lastResult = false;
	while (pos < size) {
		int16 val1, val2, val3;

		byte op = (byte)script[pos];
		uint numParams = (script[pos] & 0x300) >> 8; // 2 bits
		if (pos + numParams >= size)
			error("script ran out of content");
		uint arg1 = (script[pos] & 0xc00) >> 10; // 2 bits
		uint arg2 = (script[pos] & 0x3000) >> 12; // 2 bits
		uint arg3 = (script[pos] & 0xC000) >> 14; // 2 bits
		switch (op) {
		case kOpPlusPlus:
			if (numParams != 1)
				error("kOpPlusPlus had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "[%d/%d]++ (now %d)", script[pos + 1], arg1, val1 + 1);
			setArg(script[pos + 1], arg1, val1 + 1);
			break;
		case kOpMinusMinus:
			if (numParams != 1)
				error("kOpMinusMinus had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "[%d/%d]-- (now %d)", script[pos + 1], arg1, val1 - 1);
			setArg(script[pos + 1], arg1, val1 - 1);
			break;
		case kOpAssign:
			if (numParams != 2)
				error("kOpAssign had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = [%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2);
			break;
		case kOpAdd:
			if (numParams != 3)
				error("kOpAdd had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d + [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 + val3);
			setArg(script[pos + 1], arg1, val2 + val3);
			break;
		case kOpSubtract:
			if (numParams != 3)
				error("kOpSubtract had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d - [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 - val3);
			setArg(script[pos + 1], arg1, val2 - val3);
			break;
		case kOpMultiply:
			if (numParams != 3)
				error("kOpMultiply had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d * [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 * val3);
			setArg(script[pos + 1], arg1, val2 * val3);
			break;
		case kOpDivide:
			if (numParams != 3)
				error("kOpDivide had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			if (val3 == 0)
				error("script tried to divide by zero");
			debug(9, "[%d/%d] = [%d/%d]=%d / [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 / val3);
			setArg(script[pos + 1], arg1, val2 / val3);
			break;
		case kOpModulo:
			if (numParams != 3)
				error("kOpModulo had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			if (val3 == 0)
				error("script tried to divide by zero (modulo)");
			debug(9, "[%d/%d] = [%d/%d]=%d %% [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 % val3);
			setArg(script[pos + 1], arg1, val2 % val3);
			break;
		case kOpMaybeAlsoAssign:
			if (numParams != 2)
				error("kOpMaybeAlsoAssign had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] =(?) [%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2);
			break;
		case kOpBooleanAssign:
			if (numParams != 2)
				error("kOpBooleanAssign had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = [%d/%d] (%d) ? 1 : 0", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2 ? 1 : 0);
			break;
		case kOpNegate:
			if (numParams != 2)
				error("kOpNegate had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = -[%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, -val2);
			break;
		case kOpAnd:
			if (numParams != 3)
				error("kOpAnd had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d & [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 & val3);
			setArg(script[pos + 1], arg1, val2 & val3);
			break;
		case kOpOr:
			if (numParams != 3)
				error("kOpOr had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d | [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 | val3);
			setArg(script[pos + 1], arg1, val2 | val3);
			break;
		case kOpXor:
			if (numParams != 3)
				error("kOpXor had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d ^ [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 ^ val3);
			setArg(script[pos + 1], arg1, val2 ^ val3);
			break;
		case kOpNotPositive:
			if (numParams != 2)
				error("kOpNotPositive had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = [%d/%d] (%d) < 1", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, (val2 < 1) ? 1 : 0);
			break;
		case kOpSqrt:
			if (numParams != 2)
				error("kOpSqrt had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = sqrt([%d/%d] (%d))", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, (int16)sqrt((double)val2));
			break;
		case kOpRandom:
			if (numParams != 3)
				error("kOpRandom had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			val1 = _rnd->getRandomNumberRng(val2, val3);
			debug(9, "[%d/%d] = rnd([%d/%d]=%d, [%d/%d]=%d) (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val1);
			setArg(script[pos + 1], arg1, val1);
			break;
		case kOpExecuteScript:
			if (numParams != 1)
				error("kOpExecuteScript had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(8, "run script [%d/%d]=%d", script[pos + 1], arg1, val1);
			runScript(val1);
			debug(8, "done run script");
			break;
		case kOpCallFunc:
			if (numParams != 1)
				error("kOpCallFunc had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(8, "%d(%d, %d, %d)", (uint16)val1, _vars[1], _vars[2], _vars[3]);
			_vars[0] = scriptFuncCall(val1, _vars[1], _vars[2], _vars[3]);
			break;
		case kOpBoolLessThanEq:
			if (numParams != 2)
				error("kOpBoolLessThanEq had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] <= [%d/%d]? (%d <= %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 <= val2);
			break;
		case kOpBoolLessThan:
			if (numParams != 2)
				error("kOpBoolLessThan had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] < [%d/%d]? (%d < %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 < val2);
			break;
		case kOpBoolGreaterThanEq:
			if (numParams != 2)
				error("kOpBoolGreaterThanEq had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] >= [%d/%d]? (%d >= %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 >= val2);
			break;
		case kOpBoolGreaterThan:
			if (numParams != 2)
				error("kOpBoolGreaterThan had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] > [%d/%d]? (%d > %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 > val2);
			break;
		case kOpBoolEqual:
			if (numParams != 2)
				error("kOpBoolEqual had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] == [%d/%d]? (%d == %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 == val2);
			break;
		case kOpBoolNotEqual:
			if (numParams != 2)
				error("kOpBoolNotEqual had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] != [%d/%d]? (%d != %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 != val2);
			break;
		case kOpSaveArgs:
			if (numParams != 0)
				error("kOpSaveArgs had wrong number of params (%d)", numParams);
			debug(9, "save args");
			for (uint i = 1; i < 19; i++)
				_stack[stackBase + i] = _vars[i];
			break;
		case kOpRestoreArgs:
			if (numParams != 0)
				error("kOpRestoreArgs had wrong number of params (%d)", numParams);
			debug(9, "restore args");
			for (uint i = 1; i < 19; i++)
				_vars[i] = _stack[stackBase + i];
			break;
		case kOpSetReturnValue:
			if (numParams != 1)
				error("kOpSetReturnValue had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "return [%d/%d]=%d", script[pos + 1], arg1, val1);
			_vars[0] = val1;
			break;
		case kOpLessThanEq:
			if (numParams != 3)
				error("kOpLessThanEq had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] <= [%d/%d]? (%d <= %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 <= val2) ? 1 : 0);
			break;
		case kOpLessThan:
			if (numParams != 3)
				error("kOpLessThan had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] < [%d/%d]? (%d < %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 < val2) ? 1 : 0);
			break;
		case kOpGreaterThanEq:
			if (numParams != 3)
				error("kOpGreaterThanEq had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] >= [%d/%d]? (%d >= %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 >= val2) ? 1 : 0);
			break;
		case kOpGreaterThan:
			if (numParams != 3)
				error("kOpGreaterThan had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] > [%d/%d]? (%d > %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 > val2) ? 1 : 0);
			break;
		case kOpEqual:
			if (numParams != 3)
				error("kOpEqual had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] == [%d/%d]? (%d == %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 == val2) ? 1 : 0);
			break;
		case kOpNotEqual:
			if (numParams != 3)
				error("kOpNotEqual had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] != [%d/%d]? (%d != %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 != val2) ? 1 : 0);
			break;
		case kOpJump:
			if (numParams != 1)
				error("kOpJump had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIfNot:
			if (numParams != 1)
				error("kOpJumpIfNot had wrong number of params (%d)", numParams);
			if (lastResult)
				break;
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump if not, by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIf:
			if (numParams != 1)
				error("kOpJumpIf had wrong number of params (%d)", numParams);
			if (!lastResult)
				break;
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump if, by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIfNotValue:
			if (numParams != 2)
				error("kOpJumpIfNotValue had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "jump if not [%d/%d]=%d", script[pos + 1], arg1, val1);
			if (val1)
				break;
			debug(9, "--> jump by [%d/%d]=%d", script[pos + 2], arg2, val2);
			pos += val2;
			break;
		case kOpJumpIfValue:
			if (numParams != 2)
				error("kOpJumpIfValue had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "jump if [%d/%d]=%d", script[pos + 1], arg1, val1);
			if (!val1)
				break;
			debug(9, "--> jump by [%d/%d]=%d", script[pos + 2], arg2, val2);
			pos += val2;
			break;
		default:
			error("unknown script op 0x%02x", op);
		}
		pos += (1 + numParams);
	}

	delete[] script;
	_stack.resize(_stack.size() - 19);
}

int16 ComposerEngine::scriptFuncCall(uint16 id, int16 param1, int16 param2, int16 param3) {
	switch (id) {
	case kFuncPlayAnim:
		debug(3, "kFuncPlayAnim(%d, %d, %d)", param1, param2, param3);
		playAnimation(param1, param2, param3, 0);
		return 1; // TODO: return 0 on failure
	case kFuncStopAnim:
		debug(3, "ignoring kFuncStopAnim(%d)", param1);
		for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
			if ((*i)->_id == param1)
				stopAnimation(*i);
		}
		return 0;
	case kFuncQueueScript:
		debug(3, "kFuncQueueScript(%d, %d, %d)", param1, param2, param3);
		_queuedScripts[param1]._baseTime = _system->getMillis();
		_queuedScripts[param1]._duration = 10 * param2;
		_queuedScripts[param1]._count = 0xffff;
		_queuedScripts[param1]._scriptId = param3;
		return 0;
	case kFuncDequeueScript:
		debug(3, "kFuncDequeueScript(%d)", param1);
		_queuedScripts[param1]._count = 0;
		_queuedScripts[param1]._scriptId = 0;
		return 0;
	case kFuncSetCursor:
		warning("ignoring kSetCursor(%d, %d, %d)", param1, param2, param3);
		// TODO: return old cursor
		return 0;
	case kFuncGetCursor:
		warning("ignoring kFuncGetCursor()");
		// TODO: return cursor
		return 0;
	case kFuncShowCursor:
		// TODO
		warning("ignoring kFuncShowCursor(%d)", param1);
		return 0;
	case kFuncHideCursor:
		// TODO
		warning("ignoring kFuncHideCursor(%d)", param1);
		return 0;
	case kFuncActivateButton:
		// TODO
		warning("ignoring kFuncActivateButton(%d)", param1);
		return 1;
	case kFuncDeactivateButton:
		// TODO
		warning("ignoring kFuncDeactivateButton(%d)", param1);
		return 1;
	case kFuncNewPage:
		debug(3, "kFuncNewPage(%d, %d)", param1, param2);
		_pendingPageChanges.push_back(PendingPageChange(param1, true));
		_pendingPageChanges.push_back(PendingPageChange(param2, false));
		return 1;
	case kFuncLoadPage:
		debug(3, "kFuncLoadPage(%d)", param1);
		_pendingPageChanges.push_back(PendingPageChange(param1, false));
		return 1;
	case kFuncUnloadPage:
		debug(3, "ignoring kFuncUnloadPage(%d)", param1);
		_pendingPageChanges.push_back(PendingPageChange(param1, true));
		return 1;
	case kFuncSetPalette:
		// TODO: return 0 if not disabling (0) and doesn't exist
		debug(4, "kFuncSetPalette(%d, %d)", param1, param2);
		loadCTBL(param1, param2);
		// TODO: incomplete?
		return 1;
	case kFuncSaveVars:
		// TODO
		warning("ignoring kFuncSaveVars(%d)", param1);
		return 1;
	case kFuncLoadVars:
		// TODO
		warning("ignoring kFuncLoadVars(%d, %d, %d)", param1, param2, param3);
		return 1;
	case kFuncQueueScriptOnce:
		debug(3, "kFuncQueueScriptOnce(%d, %d, %d)", param1, param2, param3);
		_queuedScripts[param1]._baseTime = _system->getMillis();
		_queuedScripts[param1]._duration = 10 * param2;
		_queuedScripts[param1]._count = 1;
		_queuedScripts[param1]._scriptId = param3;
		return 0;
	case kFuncGetMousePos:
		// TODO
		warning("ignoring kFuncGetMousePos(%d, %d)", param1, param2);
		return 0;
	case kFuncChangeBackground:
		// TODO
		warning("ignoring kFuncChangeBackground(%d)", param1);
		// TODO: return 1 if background existed, else 0
		return 0;
	case kFuncSetBackgroundColor:
		// TODO
		warning("ignoring kFuncSetBackgroundColor(%d)", param1);
		return 0;
	case kFuncClearSprites:
		// TODO
		warning("ignoring kFuncClearSprites()");
		return 0;
	case kFuncAddSprite:
		// TODO
		warning("ignoring kFuncAddSprite(%d, %d, %d)", param1, param2, param3);
		return 0;
	case kFuncRemoveSprite:
		// TODO
		warning("ignoring kFuncRemoveSprite(%d, %d)", param1, param2);
		return 0;
	case kFuncQuit:
		// TODO
		warning("ignoring kFuncQuit()");
		return 0;
	case kFuncSaveData:
		// TODO
		warning("ignoring kFuncSaveData(%d, %d, %d)", param1, param2, param3);
		return 1;
	case kFuncLoadData:
		// TODO
		warning("ignoring kFuncLoadData(%d, %d, %d)", param1, param2, param3);
		return 1;
	case kFuncGetSpriteSize:
		// TODO
		warning("ignoring kFuncGetSpriteSize(%d, %d, %d)", param1, param2, param3);
		return 0;
	default:
		error("unknown scriptFuncCall %d(%d, %d, %d)", (uint32)id, param1, param2, param3);
	}
}

void ComposerEngine::loadCTBL(uint id, uint fadePercent) {
	Common::SeekableReadStream *stream = getResource(ID_CTBL, id);

	uint16 numEntries = stream->readUint16LE();
	debug(1, "CTBL: %d entries", numEntries);

	assert(numEntries <= 256);
	assert(stream->size() == 2 + (numEntries * 3));

	byte buffer[256 * 3];
	stream->read(buffer, numEntries * 3);
	delete stream;

	for (uint i = 0; i < numEntries * 3; i++)
		buffer[i] = ((unsigned int)buffer[i] * fadePercent) / 100;

	_system->getPaletteManager()->setPalette(buffer, 0, numEntries);
}

static void decompressSLWM(byte *buffer, Common::SeekableReadStream *stream) {
	uint bitsLeft = 0;
	uint16 lastBits;
	byte currBit;
	while (true) {
		if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
		currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

		if (currBit) {
			// single byte
			*buffer++ = stream->readByte();
			continue;
		}

		if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
		currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

		uint start;
		uint count;
		if (currBit) {
			uint orMask = stream->readByte();
			uint in = stream->readByte();
			count = in & 7;
			start = ((in & ~7) << 5) | orMask;
			if (!count) {
				count = stream->readByte();
				if (!count)
					break;
				count -= 2;
			}
		} else {
			// count encoded in the next two bits
			count = 0;

			if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
			currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

			count = (count << 1) | currBit;

			if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
			currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

			count = (count << 1) | currBit;

			start = stream->readByte();
		}

		count += 2;
		start++;
		memcpy(buffer, buffer - start, count);
		buffer += count;
	}
}

void ComposerEngine::decompressBitmap(uint16 type, Common::SeekableReadStream *stream, byte *buffer, uint32 size, uint width, uint height) {
	switch (type) {
	case kBitmapUncompressed:
		assert(stream->size() - (uint)stream->pos() == size);
		assert(size == width * height);
		stream->read(buffer, size);
		break;
	case kBitmapSpp32:
		byte lookup[16];
		stream->read(lookup, 16);
		while (size--) {
			uint in = stream->readByte();
			byte lowBits = in & 0xF;
			byte highBits = (in & 0xF0) >> 4;
			if (highBits == 0xf) {
				// run of a single color
				uint count = (uint)stream->readByte() + 3;
				size--;
				memset(buffer, lookup[lowBits], count);
				buffer += count;
			} else {
				// two pixels
				*buffer++ = lookup[highBits];
				*buffer++ = lookup[lowBits];
			}
		}
		break;
	case kBitmapSLW8:
		while (size--) {
			byte val = stream->readByte();
			if (val != 0xff) {
				*buffer++ = val;
				continue;
			}
			uint count = stream->readByte();
			size--;

			uint16 step;
			if (!(count & 0x80)) {
				step = stream->readByte();
				size--;
			} else {
				count = (count ^ 0x80);
				step = stream->readUint16LE();
				size -= 2;
			}
			count += 4;
			memcpy(buffer, buffer - step - 1, count);
			buffer += count;
		}
		break;
	case kBitmapRLESLWM:
		{
		uint32 bufSize = stream->readUint32LE();
		byte *tempBuf = new byte[bufSize];
		decompressSLWM(tempBuf, stream);

		uint instrPos = tempBuf[0] + 1;
		instrPos += READ_LE_UINT16(tempBuf + instrPos) + 2;
		byte *instr = tempBuf + instrPos;

		uint line = 0;
		while (line++ < height) {
			uint pixels = 0;

			while (pixels < width) {
				byte data = *instr++;
				byte color = tempBuf[(data & 0x7F) + 1];
				if (!(data & 0x80)) {
					*buffer++ = color;
					pixels++;
				} else {
					byte count = *instr++;
					if (!count) {
						while (pixels++ < width)
							*buffer++ = color;
						break;
					}
					for (uint i = 0; i < count; i++) {
						*buffer++ = color;
						pixels++;
					}
				}
			}
		}
		delete[] tempBuf;
		}
		break;
	case kBitmapSLWM:
		decompressSLWM(buffer, stream);
		break;
	default:
		error("decompressBitmap can't handle type %d", type);
	}
}

bool ComposerEngine::initSprite(Sprite &sprite) {
	Common::SeekableReadStream *stream = NULL;
	if (hasResource(ID_BMAP, sprite.id))
		stream = getResource(ID_BMAP, sprite.id);
	else
		for (Common::List<Pipe *>::iterator k = _pipes.begin(); k != _pipes.end(); k++) {
			Pipe *pipe = *k;
			if (!pipe->hasResource(ID_BMAP, sprite.id))
				continue;
			stream = pipe->getResource(ID_BMAP, sprite.id, true);
			break;
		}

	if (!stream)
		return false;

	uint16 type = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	uint16 width = stream->readUint16LE();
	uint32 size = stream->readUint32LE();
	debug(1, "loading BMAP: type %d, width %d, height %d, size %d", type, width, height, size);

	if (width && height) {
		sprite.surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		decompressBitmap(type, stream, (byte *)sprite.surface.pixels, size, width, height);
	}
	delete stream;

	return true;
}

void ComposerEngine::drawSprite(const Sprite &sprite) {
	int x = sprite.pos.x;
	int y = sprite.pos.y;

	// incoming data is BMP-style (bottom-up), so flip it
	byte *pixels = (byte *)_surface.pixels;
	for (uint j = 0; j < sprite.surface.h; j++) {
		byte *in = (byte *)sprite.surface.pixels + (sprite.surface.h - j - 1) * sprite.surface.w;
		byte *out = pixels + ((j + y) * _surface.w) + x;
		for (uint i = 0; i < sprite.surface.w; i++)
			if (in[i])
				out[i] = in[i];
	}
}

} // End of namespace Composer
