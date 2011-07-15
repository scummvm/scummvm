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
	kFuncQueueScript = 35004,
	kFuncDequeueScript = 35005,
	kFuncHideMouse = 35009,
	kFuncLoadPage = 35014,
	kFuncUnloadPage = 35015,
	kFuncSetPalette = 35016,
	kFuncQueueScriptOnce = 35019
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
	if (_offset == _stream->size())
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

	if (!buffering) {
		assert(res.entries.size() == 1);
		return new Common::SeekableSubReadStream(_stream, res.entries[0].offset, res.entries[0].offset + res.entries[0].size);
	}

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

		// disable the animation
		anim->_state = 0;

		// stop any animations it may have spawned
		for (uint j = 0; j < anim->_entries.size(); j++) {
			if (anim->_entries[j].op == 3)
				; // TODO: stop anim
		}

		// kill any pipes owned by the animation
		for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
			Pipe *pipe = *j;
			if (pipe->_anim != anim)
				continue;
			j = _pipes.reverse_erase(j);
			delete pipe;
			break;
		}

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
	_audioStream->queueBuffer(buffer, stream->size(), DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	delete stream;
	if (!_mixer->isSoundHandleActive(_soundHandle))
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
}

void ComposerEngine::processAnimFrame() {
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;

		if (anim->_state <= 1) {
			if (anim->_state == 1) {
				runEvent(2, anim->_id, anim->_eventParam, 0);
			} else {
				// TODO: stop anything which was running
			}
			// TODO: kill pipes
			delete anim;
			i = _anims.reverse_erase(i);

			continue;
		}

		anim->seekToCurrPos();

		for (uint j = 0; j < anim->_entries.size(); j++) {
			AnimationEntry &entry = anim->_entries[j];
			if (entry.op != 1)
				break;
			if (entry.counter) {
				entry.counter--;
			} else {
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
							removeSprite(data, anim->_id);
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
	Sprite sprite;
	sprite.id = id;
	sprite.animId = animId;
	sprite.zorder = zorder;
	sprite.pos = pos;

	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (sprite.zorder < i->zorder)
			continue;
		_sprites.insert(i, sprite);
		return;
	}
	_sprites.push_back(sprite);
}

void ComposerEngine::removeSprite(uint16 id, uint16 animId) {
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (i->id != id)
			continue;
		if (animId && i->animId != animId)
			continue;
		i = _sprites.reverse_erase(i);
		return;
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

	delete _audioStream;
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

	_bookIni.loadFromFile("programs/book.ini");

	uint width = 640;
	if (_bookIni.hasKey("Width", "Common"))
		width = atoi(getStringFromConfig("Common", "Width").c_str());
	uint height = 480;
	if (_bookIni.hasKey("Height", "Common"))
		height = atoi(getStringFromConfig("Common", "Height").c_str());
	initGraphics(width, height, true);
	_surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	_audioStream = Audio::makeQueuingAudioStream(22050, false);

	loadLibrary(0);

	uint fps = atoi(getStringFromConfig("Common", "FPS").c_str());
	uint frameTime = 1000 / fps;
	uint32 lastDrawTime = 0;
	while (!shouldQuit()) {
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

			if (hasResource(ID_BMAP, 1000))
				drawBMAP(1000, 0, 0);
			for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
				drawBMAP(i->id, i->pos.x, i->pos.y);
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

	// TODO: set background properly
	//
	// TODO: better CTBL logic
	loadCTBL(1000, 100);

	// Run the startup script.
	runScript(1000, 0, 0, 0);

	runEvent(3, id, 0, 0);
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
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "[%d/%d]++ (now %d)", script[pos + 1], arg1, val1 + 1);
			setArg(script[pos + 1], arg1, val1 + 1);
			break;
		case kOpAdd:
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d + [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 + val3);
			setArg(script[pos + 1], arg1, val2 + val3);
			break;
		case kOpAssign:
			val2 = getArg(script[pos + 2], arg2);
			if (numParams != 2)
				error("kOpAssign had wrong number of params (%d)", numParams);
			debug(9, "[%d/%d] = [%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2);
			break;
		case kOpCallFunc:
			val1 = getArg(script[pos + 1], arg1);
			if (numParams != 1)
				error("kOpCallFunc had wrong number of params (%d)", numParams);
			debug(8, "%d(%d, %d, %d)", (uint16)val1, _vars[1], _vars[2], _vars[3]);
			_vars[0] = scriptFuncCall(val1, _vars[1], _vars[2], _vars[3]);
			break;
		/*case kOpBoolGreaterThan:
			lastResult = (getArg(script[pos + 1], arg1) > getArg(script[pos + 2], arg2));
			break;*/
		case kOpBoolEqual:
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] == [%d/%d]? (%d > %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 == val2);
			break;
		case kOpSaveArgs:
			debug(9, "save args");
			for (uint i = 1; i < 19; i++)
				_stack[stackBase + i] = _vars[i];
			break;
		case kOpRestoreArgs:
			debug(9, "restore args");
			for (uint i = 1; i < 19; i++)
				_vars[i] = _stack[stackBase + i];
			break;
		case kOpGreaterThan:
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] > [%d/%d]? (%d > %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 > val2) ? 1 : 0);
			break;
		case kOpJump:
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIfNot:
			if (lastResult)
				break;
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump if not, by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIfNotValue:
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "jump if not [%d/%d]=%d", script[pos + 1], arg1, val1);
			if (val1)
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
		break;
	case kFuncStopAnim:
		// TODO
		warning("ignoring kFuncStopAnim(%d)", param1);
		break;
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
	case kFuncHideMouse:
		// TODO
		warning("ignoring kFuncHideMouse(%d)", param1);
		return 0;
		break;
	case kFuncLoadPage:
		debug(3, "kFuncLoadPage(%d)", param1);
		loadLibrary(param1);
		return 1;
	case kFuncUnloadPage:
		// TODO
		warning("ignoring kFuncUnloadPage(%d)", param1);
		return 1;
	case kFuncSetPalette:
		// TODO: incomplete?
		debug(4, "kFuncSetPalette(%d, %d)", param1, param2);
		loadCTBL(param1, param2);
		return 1;
		break;
	case kFuncQueueScriptOnce:
		debug(3, "kFuncQueueScriptOnce(%d, %d, %d)", param1, param2, param3);
		_queuedScripts[param1]._baseTime = _system->getMillis();
		_queuedScripts[param1]._duration = 10 * param2;
		_queuedScripts[param1]._count = 1;
		_queuedScripts[param1]._scriptId = param3;
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

void ComposerEngine::drawBMAP(uint id, uint x, uint y) {
	Common::SeekableReadStream *stream = NULL;
	if (hasResource(ID_BMAP, id))
		stream = getResource(ID_BMAP, id);
	else
		for (Common::List<Pipe *>::iterator k = _pipes.begin(); k != _pipes.end(); k++) {
			Pipe *pipe = *k;
			if (!pipe->hasResource(ID_BMAP, id))
				continue;
			stream = pipe->getResource(ID_BMAP, id, true);
			break;
		}

	if (!stream) {
		// FIXME
		warning("couldn't find BMAP %d", id);
		return;
	}

	uint16 type = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	uint16 width = stream->readUint16LE();
	uint32 size = stream->readUint32LE();
	debug(1, "BMAP: type %d, width %d, height %d, size %d", type, width, height, size);

	byte *buffer = new byte[width * height];
	decompressBitmap(type, stream, buffer, size, width, height);

	debug(1, "draw at %d,%d", x, y);

	// incoming data is BMP-style (bottom-up), so flip it
	byte *pixels = (byte *)_surface.pixels;
	for (uint j = 0; j < height; j++) {
		byte *in = buffer + (height - j - 1) * width;
		byte *out = pixels + ((j + y) * _surface.w) + x;
		for (uint i = 0; i < width; i++)
			if (in[i])
				out[i] = in[i];
	}

	delete[] buffer;
	delete stream;
}

} // End of namespace Composer
