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

#include "director/score.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/unzip.h"

#include "common/system.h"
#include "director/dib.h"
#include "director/resource.h"
#include "director/lingo/lingo.h"
#include "director/sound.h"

#include "graphics/palette.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/managed_surface.h"
#include "graphics/macgui/macwindowmanager.h"
#include "image/bmp.h"
#include "graphics/fontman.h"
#include "graphics/fonts/bdf.h"

namespace Director {

static byte defaultPalette[768] = {
	  0,   0,   0,  17,  17,  17,  34,  34,  34,  68,  68,  68,  85,  85,  85, 119,
	119, 119, 136, 136, 136, 170, 170, 170, 187, 187, 187, 221, 221, 221, 238, 238,
	238,   0,   0,  17,   0,   0,  34,   0,   0,  68,   0,   0,  85,   0,   0, 119,
	  0,   0, 136,   0,   0, 170,   0,   0, 187,   0,   0, 221,   0,   0, 238,   0,
	 17,   0,   0,  34,   0,   0,  68,   0,   0,  85,   0,   0, 119,   0,   0, 136,
	  0,   0, 170,   0,   0, 187,   0,   0, 221,   0,   0, 238,   0,  17,   0,   0,
	 34,   0,   0,  68,   0,   0,  85,   0,   0, 119,   0,   0, 136,   0,   0, 170,
	  0,   0, 187,   0,   0, 221,   0,   0, 238,   0,   0,   0,   0,  51,   0,   0,
	102,   0,   0, 153,   0,   0, 204,   0,   0, 255,   0,  51,   0,   0,  51,  51,
	  0,  51, 102,   0,  51, 153,   0,  51, 204,   0,  51, 255,   0, 102,   0,   0,
	102,  51,   0, 102, 102,   0, 102, 153,   0, 102, 204,   0, 102, 255,   0, 153,
	  0,   0, 153,  51,   0, 153, 102,   0, 153, 153,   0, 153, 204,   0, 153, 255,
	  0, 204,   0,   0, 204,  51,   0, 204, 102,   0, 204, 153,   0, 204, 204,   0,
	204, 255,   0, 255,   0,   0, 255,  51,   0, 255, 102,   0, 255, 153,   0, 255,
	204,   0, 255, 255,  51,   0,   0,  51,   0,  51,  51,   0, 102,  51,   0, 153,
	 51,   0, 204,  51,   0, 255,  51,  51,   0,  51,  51,  51,  51,  51, 102,  51,
	 51, 153,  51,  51, 204,  51,  51, 255,  51, 102,   0,  51, 102,  51,  51, 102,
	102,  51, 102, 153,  51, 102, 204,  51, 102, 255,  51, 153,   0,  51, 153,  51,
	 51, 153, 102,  51, 153, 153,  51, 153, 204,  51, 153, 255,  51, 204,   0,  51,
	204,  51,  51, 204, 102,  51, 204, 153,  51, 204, 204,  51, 204, 255,  51, 255,
	  0,  51, 255,  51,  51, 255, 102,  51, 255, 153,  51, 255, 204,  51, 255, 255,
	102,   0,   0, 102,   0,  51, 102,   0, 102, 102,   0, 153, 102,   0, 204, 102,
	  0, 255, 102,  51,   0, 102,  51,  51, 102,  51, 102, 102,  51, 153, 102,  51,
	204, 102,  51, 255, 102, 102,   0, 102, 102,  51, 102, 102, 102, 102, 102, 153,
	102, 102, 204, 102, 102, 255, 102, 153,   0, 102, 153,  51, 102, 153, 102, 102,
	153, 153, 102, 153, 204, 102, 153, 255, 102, 204,   0, 102, 204,  51, 102, 204,
	102, 102, 204, 153, 102, 204, 204, 102, 204, 255, 102, 255,   0, 102, 255,  51,
	102, 255, 102, 102, 255, 153, 102, 255, 204, 102, 255, 255, 153,   0,   0, 153,
	  0,  51, 153,   0, 102, 153,   0, 153, 153,   0, 204, 153,   0, 255, 153,  51,
	  0, 153,  51,  51, 153,  51, 102, 153,  51, 153, 153,  51, 204, 153,  51, 255,
	153, 102,   0, 153, 102,  51, 153, 102, 102, 153, 102, 153, 153, 102, 204, 153,
	102, 255, 153, 153,   0, 153, 153,  51, 153, 153, 102, 153, 153, 153, 153, 153,
	204, 153, 153, 255, 153, 204,   0, 153, 204,  51, 153, 204, 102, 153, 204, 153,
	153, 204, 204, 153, 204, 255, 153, 255,   0, 153, 255,  51, 153, 255, 102, 153,
	255, 153, 153, 255, 204, 153, 255, 255, 204,   0,   0, 204,   0,  51, 204,   0,
	102, 204,   0, 153, 204,   0, 204, 204,   0, 255, 204,  51,   0, 204,  51,  51,
	204,  51, 102, 204,  51, 153, 204,  51, 204, 204,  51, 255, 204, 102,   0, 204,
	102,  51, 204, 102, 102, 204, 102, 153, 204, 102, 204, 204, 102, 255, 204, 153,
	  0, 204, 153,  51, 204, 153, 102, 204, 153, 153, 204, 153, 204, 204, 153, 255,
	204, 204,   0, 204, 204,  51, 204, 204, 102, 204, 204, 153, 204, 204, 204, 204,
	204, 255, 204, 255,   0, 204, 255,  51, 204, 255, 102, 204, 255, 153, 204, 255,
	204, 204, 255, 255, 255,   0,   0, 255,   0,  51, 255,   0, 102, 255,   0, 153,
	255,   0, 204, 255,   0, 255, 255,  51,   0, 255,  51,  51, 255,  51, 102, 255,
	 51, 153, 255,  51, 204, 255,  51, 255, 255, 102,   0, 255, 102,  51, 255, 102,
	102, 255, 102, 153, 255, 102, 204, 255, 102, 255, 255, 153,   0, 255, 153,  51,
	255, 153, 102, 255, 153, 153, 255, 153, 204, 255, 153, 255, 255, 204,   0, 255,
	204,  51, 255, 204, 102, 255, 204, 153, 255, 204, 204, 255, 204, 255, 255, 255,
	  0, 255, 255,  51, 255, 255, 102, 255, 255, 153, 255, 255, 204, 255, 255, 255 };

Score::Score(DirectorEngine *vm) {
	_vm = vm;
	_surface = new Graphics::ManagedSurface;
	_trailSurface = new Graphics::ManagedSurface;
	_movieArchive = _vm->getMainArchive();
	_lingo = _vm->getLingo();
	_soundManager = _vm->getSoundManager();
	_lingo->processEvent(kEventPrepareMovie, 0);
	_movieScriptCount = 0;
	_labels = NULL;

	if (_movieArchive->hasResource(MKTAG('M','C','N','M'), 0)) {
		_macName = _movieArchive->getName(MKTAG('M','C','N','M'), 0).c_str();
	}

	if (_movieArchive->hasResource(MKTAG('V','W','L','B'), 1024)) {
		loadLabels(*_movieArchive->getResource(MKTAG('V','W','L','B'), 1024));
	}
}

void Score::loadArchive() {
	Common::Array<uint16> clutList = _movieArchive->getResourceIDList(MKTAG('C','L','U','T'));

	if (clutList.size() > 1)
		warning("More than one palette was found (%d)", clutList.size());

	if (clutList.size() == 0) {
		warning("CLUT resource not found, using default Mac palette");
		g_system->getPaletteManager()->setPalette(defaultPalette, 0, 256);
		_vm->setPalette(defaultPalette, 256);
	} else {
		Common::SeekableSubReadStreamEndian *pal = _movieArchive->getResource(MKTAG('C', 'L', 'U', 'T'), clutList[0]);

		loadPalette(*pal);
		g_system->getPaletteManager()->setPalette(_vm->getPalette(), 0, _vm->getPaletteColorCount());
	}

	assert(_movieArchive->hasResource(MKTAG('V','W','S','C'), 1024));
	assert(_movieArchive->hasResource(MKTAG('V','W','C','F'), 1024));

	loadFrames(*_movieArchive->getResource(MKTAG('V','W','S','C'), 1024));
	loadConfig(*_movieArchive->getResource(MKTAG('V','W','C','F'), 1024));

	if (_vm->getVersion() < 4) {
		assert(_movieArchive->hasResource(MKTAG('V','W','C','R'), 1024));
		loadCastData(*_movieArchive->getResource(MKTAG('V','W','C','R'), 1024));
	}

	if (_movieArchive->hasResource(MKTAG('V','W','A','C'), 1024)) {
		loadActions(*_movieArchive->getResource(MKTAG('V','W','A','C'), 1024));
	}

	if (_movieArchive->hasResource(MKTAG('V','W','F','I'), 1024)) {
		loadFileInfo(*_movieArchive->getResource(MKTAG('V','W','F','I'), 1024));
	}

	if (_movieArchive->hasResource(MKTAG('V','W','F','M'), 1024)) {
		loadFontMap(*_movieArchive->getResource(MKTAG('V','W','F','M'), 1024));
	}

	Common::Array<uint16> vwci = _movieArchive->getResourceIDList(MKTAG('V','W','C','I'));

	if (vwci.size() > 0) {
		Common::Array<uint16>::iterator iterator;

		for (iterator = vwci.begin(); iterator != vwci.end(); ++iterator)
			loadCastInfo(*_movieArchive->getResource(MKTAG('V','W','C','I'), *iterator), *iterator);
	}

	Common::Array<uint16> stxt = _movieArchive->getResourceIDList(MKTAG('S','T','X','T'));

	if (stxt.size() > 0) {
		Common::Array<uint16>::iterator iterator;

		for (iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
			loadScriptText(*_movieArchive->getResource(MKTAG('S','T','X','T'), *iterator));
		}
	}
}

Score::~Score() {
	if (_surface)
		_surface->free();

	if (_trailSurface)
		_trailSurface->free();

	delete _surface;
	delete _trailSurface;

	if (_movieArchive)
		_movieArchive->close();

	delete _surface;
	delete _trailSurface;

	delete _font;
	delete _movieArchive;

	delete _labels;
}

void Score::loadPalette(Common::SeekableSubReadStreamEndian &stream) {
	uint16 steps = stream.size() / 6;
	uint16 index = (steps * 3) - 1;
	uint16 _paletteColorCount = steps;
	byte *_palette = new byte[index];

	for (uint8 i = 0; i < steps; i++) {
		_palette[index - 2] = stream.readByte();
		stream.readByte();

		_palette[index - 1] = stream.readByte();
		stream.readByte();

		_palette[index] = stream.readByte();
		stream.readByte();
		index -= 3;
	}
	_vm->setPalette(_palette, _paletteColorCount);
}

void Score::loadFrames(Common::SeekableSubReadStreamEndian &stream) {
	uint32 size = stream.readUint32();
	size -= 4;

	if (_vm->getVersion() > 3) {
		stream.skip(16);
		//Unknown, some bytes - constant (refer to contuinity).
	}

	uint16 channelSize;
	uint16 channelOffset;

	Frame *initial = new Frame(_vm);
	_frames.push_back(initial);

	while (size != 0) {
		uint16 frameSize = stream.readUint16();
		size -= frameSize;
		frameSize -= 2;
		Frame *frame = new Frame(*_frames.back());

		while (frameSize != 0) {
			if (_vm->getVersion() < 4) {
				channelSize = stream.readByte() * 2;
				channelOffset = stream.readByte() * 2;
				frameSize -= channelSize + 2;
			} else {
				channelSize = stream.readByte();
				channelOffset = stream.readByte();
				frameSize -= channelSize + 4;
			}
			frame->readChannel(stream, channelOffset, channelSize);

		}

		_frames.push_back(frame);
	}

	//remove initial frame
	_frames.remove_at(0);
}

void Score::loadConfig(Common::SeekableSubReadStreamEndian &stream) {
	/*uint16 unk1 = */ stream.readUint16();
	/*ver1 = */ stream.readUint16();
	_movieRect = Score::readRect(stream);

	_castArrayStart = stream.readUint16();
	_castArrayEnd = stream.readUint16();
	_currentFrameRate = stream.readByte();
	stream.skip(9);
	_stageColor = stream.readUint16();
}

void Score::readVersion(uint32 rid) {
	_versionMinor = rid & 0xffff;
	_versionMajor = rid >> 16;

	debug("Version: %d.%d", _versionMajor, _versionMinor);
}

void Score::loadCastData(Common::SeekableSubReadStreamEndian &stream) {
	for (uint16 id = _castArrayStart; id <= _castArrayEnd; id++) {
		byte size = stream.readByte();
		if (size == 0)
			continue;

		uint8 castType = stream.readByte();

		switch (castType) {
		case kCastBitmap:
			_casts[id] = new BitmapCast(stream);
			_casts[id]->type = kCastBitmap;
			break;
		case kCastText:
			_casts[id] = new TextCast(stream);
			_casts[id]->type = kCastText;
			break;
		case kCastShape:
			_casts[id] = new ShapeCast(stream);
			_casts[id]->type = kCastShape;
			break;
		case kCastButton:
			_casts[id] = new ButtonCast(stream);
			_casts[id]->type = kCastButton;
			break;
		default:
			warning("Unhandled cast type: %d", castType);
			stream.skip(size - 1);
			break;
		}
	}

	//Set cast pointers to sprites
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			byte castId = _frames[i]->_sprites[j]->_castId;

			if (_casts.contains(castId))
				_frames[i]->_sprites[j]->_cast = _casts.find(castId)->_value;
		}
	}
}

void Score::loadLabels(Common::SeekableSubReadStreamEndian &stream) {
	_labels = new Common::SortedArray<Label *>(compareLabels);
	uint16 count = stream.readUint16() + 1;
	uint16 offset = count * 4 + 2;

	uint16 frame = stream.readUint16();
	uint16 stringPos = stream.readUint16() + offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 nextFrame = stream.readUint16();
		uint16 nextStringPos = stream.readUint16() + offset;
		uint16 streamPos = stream.pos();

		stream.seek(stringPos);
		Common::String label;
		for (uint16 j = stringPos; j < nextStringPos; j++) {
			label += stream.readByte();
		}
		_labels->insert(new Label(label, frame));
		stream.seek(streamPos);

		frame = nextFrame;
		stringPos = nextStringPos;
	}

	Common::SortedArray<Label *>::iterator j;

	for (j = _labels->begin(); j != _labels->end(); ++j) {
		debug("Frame %d, Label %s", (*j)->number, (*j)->name.c_str());
	}
}

int Score::compareLabels(const void *a, const void *b) {
	return ((const Label *)a)->number - ((const Label *)b)->number;
}

void Score::loadActions(Common::SeekableSubReadStreamEndian &stream) {
	uint16 count = stream.readUint16() + 1;
	uint16 offset = count * 4 + 2;

	byte id = stream.readByte();
	/*byte subId = */ stream.readByte(); //I couldn't find how it used in continuity (except print). Frame actionId = 1 byte.
	uint16 stringPos = stream.readUint16() + offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 nextId = stream.readByte();
		/*byte subId = */ stream.readByte();
		uint16 nextStringPos = stream.readUint16() + offset;
		uint16 streamPos = stream.pos();

		stream.seek(stringPos);

		for (uint16 j = stringPos; j < nextStringPos; j++) {
			byte ch = stream.readByte();
			if (ch == 0x0d) {
				ch = '\n';
			}
			_actions[id] += ch;
		}

		stream.seek(streamPos);

		id = nextId;
		stringPos = nextStringPos;

		if (stringPos == stream.size())
			break;
	}

	Common::HashMap<uint16, Common::String>::iterator j;

	if (ConfMan.getBool("dump_scripts"))
		for (j = _actions.begin(); j != _actions.end(); ++j) {
			if (!j->_value.empty())
				dumpScript(j->_value.c_str(), kFrameScript, j->_key);
		}

	for (j = _actions.begin(); j != _actions.end(); ++j)
		if (!j->_value.empty())
			_lingo->addCode(j->_value.c_str(), kFrameScript, j->_key);
}

void Score::loadScriptText(Common::SeekableSubReadStreamEndian &stream) {
	/*uint32 unk1 = */ stream.readUint32();
	uint32 strLen = stream.readUint32();
	/*uin32 dataLen = */ stream.readUint32();
	Common::String script;

	for (uint32 i = 0; i < strLen; i++) {
		byte ch = stream.readByte();

		if (ch == 0x0d) {
			//in old Mac systems \r was the code for end-of-line instead.
			ch = '\n';
		}
		script += ch;
	}

	if (!script.empty() && ConfMan.getBool("dump_scripts"))
		dumpScript(script.c_str(), kMovieScript, _movieScriptCount);

	if (!script.empty())
		_lingo->addCode(script.c_str(), kMovieScript, _movieScriptCount);

	_movieScriptCount++;
}

void Score::setStartToLabel(Common::String label) {
	if (!_labels) {
		warning("setStartToLabel: No labels set");
		return;
	}

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name == label) {
			_currentFrame = (*i)->number;
			return;
		}
	}
	warning("Label %s not found", label.c_str());
}

void Score::dumpScript(const char *script, ScriptType type, uint16 id) {
	Common::DumpFile out;
	Common::String typeName;
	char buf[256];

	switch (type) {
	case kFrameScript:
		typeName = "frame";
		break;
	case kMovieScript:
		typeName = "movie";
		break;
	case kSpriteScript:
		typeName = "sprite";
		break;
	}

	sprintf(buf, "./dumps/%s-%s-%d.txt", _macName.c_str(), typeName.c_str(), id);

	if (!out.open(buf)) {
		warning("Can not open dump file %s", buf);
		return;
	}

	out.write(script, strlen(script));

	out.flush();
	out.close();
}

void Score::loadCastInfo(Common::SeekableSubReadStreamEndian &stream, uint16 id) {
	uint32 entryType = 0;
	Common::Array<Common::String> castStrings = loadStrings(stream, entryType);
	CastInfo *ci = new CastInfo();

	ci->script = castStrings[0];

	if (!ci->script.empty() && ConfMan.getBool("dump_scripts"))
		dumpScript(ci->script.c_str(), kSpriteScript, id);

	if (!ci->script.empty())
		_lingo->addCode(ci->script.c_str(), kSpriteScript, id);

	ci->name = getString(castStrings[1]);
	ci->directory = getString(castStrings[2]);
	ci->fileName = getString(castStrings[3]);
	ci->type = castStrings[4];

	_castsInfo[id] = ci;
}

void Score::gotoloop() {
	//This command has the playback head contonuously return to the first marker to to the left and then loop back.
	//If no marker are to the left of the playback head, the playback head continues to the right.
	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name == _currentLabel) {
			_currentFrame = (*i)->number;
			return;
		}
	}
}

void Score::gotonext() {
	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name == _currentLabel) {
			if (i != _labels->end()) {
				//return to the first marker to to the right
				++i;
				_currentFrame = (*i)->number;
				return;
			} else {
				//if no markers are to the right of the playback head,
				//the playback head goes to the first marker to the left
				_currentFrame = (*i)->number;
				return;
			}
		}
	}
	//If there are not markers to the left,
	//the playback head goes to frame 1, (Director frame array start from 1, engine from 0)
	_currentFrame = 0;
}

void Score::gotoprevious() {
	//One label
	if (_labels->begin() == _labels->end()) {
		_currentFrame = (*_labels->begin())->number;
		return;
	}

	Common::SortedArray<Label *>::iterator previous = _labels->begin();
	Common::SortedArray<Label *>::iterator i = previous++;

	for (i = _labels->begin(); i != _labels->end(); ++i, ++previous) {
		if ((*i)->name == _currentLabel) {
			_currentFrame = (*previous)->number;
			return;
		} else {
			_currentFrame = (*i)->number;
			return;
		}
	}
	_currentFrame = 0;
}

Common::String Score::getString(Common::String str) {
	if (str.size() == 0) {
		return str;
	}

	uint8 f = static_cast<uint8>(str.firstChar());

	if (f == 0) {
		return "";
	}

	str.deleteChar(0);

	if (str.lastChar() == '\x00') {
		str.deleteLastChar();
	}

	return str;
}

void Score::loadFileInfo(Common::SeekableSubReadStreamEndian &stream) {
	Common::Array<Common::String> fileInfoStrings = loadStrings(stream, _flags);
	_script = fileInfoStrings[0];

	if (!_script.empty() && ConfMan.getBool("dump_scripts"))
		dumpScript(_script.c_str(), kMovieScript, _movieScriptCount);

	if (!_script.empty())
		_lingo->addCode(_script.c_str(), kMovieScript, _movieScriptCount);

	_movieScriptCount++;
	_changedBy = fileInfoStrings[1];
	_createdBy = fileInfoStrings[2];
	_directory = fileInfoStrings[3];
}

Common::Array<Common::String> Score::loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader) {
	Common::Array<Common::String> strings;
	uint32 offset = 0;

	if (hasHeader) {
		offset = stream.readUint32();
		/*uint32 unk1 = */ stream.readUint32();
		/*uint32 unk2 = */ stream.readUint32();
		entryType = stream.readUint32();
		stream.seek(offset);
	}

	uint16 count = stream.readUint16();
	offset += (count + 1) * 4 + 2; //positions info + uint16 count
	uint32 startPos = stream.readUint32() + offset;

	for (uint16 i = 0; i < count; i++) {
		Common::String entryString;
		uint32 nextPos = stream.readUint32() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(startPos);

		while (startPos != nextPos) {
			entryString += stream.readByte();
			++startPos;
		}

		strings.push_back(entryString);

		stream.seek(streamPos);
		startPos = nextPos;
	}

	return strings;
}

void Score::loadFontMap(Common::SeekableSubReadStreamEndian &stream) {
	uint16 count = stream.readUint16();
	uint32 offset = (count * 2) + 2;
	uint16 currentRawPosition = offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 id = stream.readUint16();
		uint32 positionInfo = stream.pos();

		stream.seek(currentRawPosition);

		uint16 size = stream.readByte();
		Common::String font;

		for (uint16 k = 0; k < size; k++) {
			font += stream.readByte();
		}

		_fontMap[id] = font;
		debug(3, "Fontmap. ID %d Font %s", id, font.c_str());
		currentRawPosition = stream.pos();
		stream.seek(positionInfo);
	}
}

BitmapCast::BitmapCast(Common::SeekableSubReadStreamEndian &stream) {
	/*byte flags = */ stream.readByte();
	uint16 someFlaggyThing = stream.readUint16();
	initialRect = Score::readRect(stream);
	boundingRect = Score::readRect(stream);
	regY = stream.readUint16();
	regX = stream.readUint16();

	if (someFlaggyThing & 0x8000) {
		/*uint16 unk1 =*/ stream.readUint16();
		/*uint16 unk2 =*/ stream.readUint16();
	}
	modified = 0;
}

TextCast::TextCast(Common::SeekableSubReadStreamEndian &stream) {
	/*byte flags =*/ stream.readByte();
	borderSize = static_cast<SizeType>(stream.readByte());
	gutterSize = static_cast<SizeType>(stream.readByte());
	boxShadow = static_cast<SizeType>(stream.readByte());
	textType = static_cast<TextType>(stream.readByte());
	textAlign = static_cast<TextAlignType>(stream.readUint16());
	stream.skip(6); //palinfo
	//for now, just supposition
	fontId = stream.readUint32();

	initialRect = Score::readRect(stream);
	textShadow = static_cast<SizeType>(stream.readByte());
	byte flags = stream.readByte();
	if (flags & 0x1)
		textFlags.push_back(kTextFlagEditable);
	if (flags & 0x2)
		textFlags.push_back(kTextFlagAutoTab);
	if (flags & 0x4)
		textFlags.push_back(kTextFlagDoNotWrap);
	//again supposition
	fontSize = stream.readUint16();
	modified = 0;
}

ShapeCast::ShapeCast(Common::SeekableSubReadStreamEndian &stream) {
	/*byte flags = */ stream.readByte();
	/*unk1 = */ stream.readByte();
	shapeType = static_cast<ShapeType>(stream.readByte());
	initialRect = Score::readRect(stream);
	pattern = stream.readUint16BE();
	fgCol = stream.readByte();
	bgCol = stream.readByte();
	fillType = stream.readByte();
	lineThickness = stream.readByte();
	lineDirection = stream.readByte();
	modified = 0;
}

Common::Rect Score::readRect(Common::SeekableSubReadStreamEndian &stream) {
	Common::Rect *rect = new Common::Rect();
	rect->top = stream.readUint16();
	rect->left = stream.readUint16();
	rect->bottom = stream.readUint16();
	rect->right = stream.readUint16();

	return *rect;
}

void Score::startLoop() {
	initGraphics(_movieRect.width(), _movieRect.height(), true);

	_surface->create(_movieRect.width(), _movieRect.height());
	_trailSurface->create(_movieRect.width(), _movieRect.height());

	if (_stageColor == 0)
		_trailSurface->clear(_vm->getPaletteColorCount() - 1);
	else
		_trailSurface->clear(_stageColor);

	_currentFrame = 0;
	_stopPlay = false;
	_nextFrameTime = 0;

	_lingo->processEvent(kEventStartMovie, 0);
	_frames[_currentFrame]->prepareFrame(this);

	while (!_stopPlay && _currentFrame < _frames.size() - 2) {
		update();
		processEvents();

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void Score::update() {
	if (g_system->getMillis() < _nextFrameTime)
		return;

	_surface->clear();
	_surface->copyFrom(*_trailSurface);

	//Enter and exit from previous frame (Director 4)
	_lingo->processEvent(kEventEnterFrame, _currentFrame);
	_lingo->processEvent(kEventExitFrame, _currentFrame);
	//TODO Director 6 - another order


	//TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
	//for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
	//	if (_frames[_currentFrame]->_sprites[i]->_enabled)
	//		_lingo->processEvent(kEventBeginSprite, i);
	//}

	//TODO Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	//_lingo->processEvent(kEventPrepareFrame, _currentFrame);
	_currentFrame++;

	Common::SortedArray<Label *>::iterator i;
	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number == _currentFrame) {
			_currentLabel = (*i)->name;
		}
	}

	_frames[_currentFrame]->prepareFrame(this);
	//Stage is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell)

	byte tempo = _frames[_currentFrame]->_tempo;

	if (tempo) {
		if (tempo > 161) {
			//Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;

			return;
		} else if (tempo <= 60) {
			//FPS
			_nextFrameTime = g_system->getMillis() + (float)tempo / 60 * 1000;
			_currentFrameRate = tempo;
		} else if (tempo >= 136) {
			//TODO Wait for channel tempo - 135
		} else if (tempo == 128) {
			//TODO Wait for Click/Key
		} else if (tempo == 135) {
			//Wait for sound channel 1
			while (_soundManager->isChannelActive(1)) {
				processEvents();
				g_system->delayMillis(10);
			}
		} else if (tempo == 134) {
			//Wait for sound channel 2
			while (_soundManager->isChannelActive(2)) {
				processEvents();
				g_system->delayMillis(10);
			}
		}
	}
	_nextFrameTime = g_system->getMillis() + (float)_currentFrameRate / 60 * 1000;
}

void Score::processEvents() {
	if (_currentFrame > 0)
		_lingo->processEvent(kEventIdle, _currentFrame - 1);

	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT)
			_stopPlay = true;

		if (event.type == Common::EVENT_LBUTTONDOWN) {
			Common::Point pos = g_system->getEventManager()->getMousePos();

			//TODO there is dont send frame id
			_lingo->processEvent(kEventMouseDown, _frames[_currentFrame]->getSpriteIDFromPos(pos));
		}

		if (event.type == Common::EVENT_LBUTTONUP) {
			Common::Point pos = g_system->getEventManager()->getMousePos();

			_lingo->processEvent(kEventMouseUp, _frames[_currentFrame]->getSpriteIDFromPos(pos));
		}
	}
}

Sprite *Score::getSpriteById(uint16 id) {
	if (_frames[_currentFrame]->_sprites[id]) {
		return _frames[_currentFrame]->_sprites[id];
	} else {
		warning("Sprite on frame %d width id %d not found", _currentFrame, id);
		return nullptr;
	}
}

Frame::Frame(DirectorEngine *vm) {
	_vm = vm;
	_transDuration = 0;
	_transType = kTransNone;
	_transArea = 0;
	_transChunkSize = 0;
	_tempo = 0;

	_sound1 = 0;
	_sound2 = 0;
	_soundType1 = 0;
	_soundType2 = 0;

	_actionId = 0;
	_skipFrameFlag = 0;
	_blend = 0;

	_sprites.resize(CHANNEL_COUNT);

	for (uint16 i = 0; i < _sprites.size(); i++) {
		Sprite *sp = new Sprite();
		_sprites[i] = sp;
	}
}

Frame::Frame(const Frame &frame) {
	_vm = frame._vm;
	_actionId = frame._actionId;
	_transArea = frame._transArea;
	_transDuration = frame._transDuration;
	_transType = frame._transType;
	_transChunkSize = frame._transChunkSize;
	_tempo = frame._tempo;
	_sound1 = frame._sound1;
	_sound2 = frame._sound2;
	_soundType1 = frame._soundType1;
	_soundType2 = frame._soundType2;
	_skipFrameFlag = frame._skipFrameFlag;
	_blend = frame._blend;
	_palette = new PaletteInfo();

	_sprites.resize(CHANNEL_COUNT);

	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		_sprites[i] = new Sprite(*frame._sprites[i]);
	}
}

Frame::~Frame() {
	delete[] &_sprites;
	delete[] &_drawRects;
	delete _palette;
}

void Frame::readChannel(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (offset >= 32) {
		if (size <= 16)
			readSprite(stream, offset, size);
		else {
			//read > 1 sprites channel
			while (size > 16) {
				byte spritePosition = (offset - 32) / 16;
				uint16 nextStart = (spritePosition + 1) * 16 + 32;
				uint16 needSize = nextStart - offset;
				readSprite(stream, offset, needSize);
				offset += needSize;
				size -= needSize;
			}
			readSprite(stream, offset, size);
		}
	} else {
		readMainChannels(stream, offset, size);
	}
}

void Frame::readMainChannels(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 finishPosition = offset + size;

	while (offset < finishPosition) {
		switch(offset) {
		case kScriptIdPosition:
			_actionId = stream.readByte();
			offset++;
			break;
		case kSoundType1Position:
			_soundType1 = stream.readByte();
			offset++;
			break;
		case kTransFlagsPosition: {
			uint8 transFlags = stream.readByte();
			if (transFlags & 0x80)
				_transArea = 1;
			else
				_transArea = 0;
			_transDuration = transFlags & 0x7f;
			offset++;
			}
			break;
		case kTransChunkSizePosition:
			_transChunkSize = stream.readByte();
			offset++;
			break;
		case kTempoPosition:
			_tempo = stream.readByte();
			offset++;
			break;
		case kTransTypePosition:
			_transType = static_cast<TransitionType>(stream.readByte());
			offset++;
			break;
		case kSound1Position:
			_sound1 = stream.readUint16();
			offset+=2;
			break;
		case kSkipFrameFlagsPosition:
			_skipFrameFlag = stream.readByte();
			offset++;
			break;
		case kBlendPosition:
			_blend = stream.readByte();
			offset++;
			break;
		case kSound2Position:
			_sound2 = stream.readUint16();
			offset += 2;
			break;
		case kSound2TypePosition:
			_soundType2 = stream.readByte();
			offset += 1;
			break;
		case kPaletePosition:
			if (stream.readUint16())
				readPaletteInfo(stream);
			offset += 16;
		default:
			offset++;
			stream.readByte();
			debug("Field Position %d, Finish Position %d", offset, finishPosition);
			break;
		}
	}
}

void Frame::readPaletteInfo(Common::SeekableSubReadStreamEndian &stream) {
	_palette->firstColor = stream.readByte();
	_palette->lastColor = stream.readByte();
	_palette->flags = stream.readByte();
	_palette->speed = stream.readByte();
	_palette->frameCount = stream.readUint16();
	stream.skip(8); //unknown
}

void Frame::readSprite(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - 32) / 16;
	uint16 spriteStart = spritePosition * 16 + 32;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

	Sprite &sprite = *_sprites[spritePosition];

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case kSpritePositionUnk1:
			/*byte x1 = */ stream.readByte();
			fieldPosition++;
			break;
		case kSpritePositionEnabled:
			sprite._enabled = (stream.readByte() != 0);
			fieldPosition++;
			break;
		case kSpritePositionUnk2:
			/*byte x2 = */ stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionFlags:
			sprite._flags = stream.readUint16();
			sprite._ink = static_cast<InkType>(sprite._flags & 0x3f);

			if (sprite._flags & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			fieldPosition += 2;
			break;
		case kSpritePositionCastId:
			sprite._castId = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionY:
			sprite._startPoint.y = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionX:
			sprite._startPoint.x = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionWidth:
			sprite._width = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionHeight:
			sprite._height = stream.readUint16();
			fieldPosition += 2;
			break;
		default:
			//end cycle, go to next sprite channel
			readSprite(stream, spriteStart + 16, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		}
	}
}

void Frame::prepareFrame(Score *score) {
	renderSprites(*score->_surface, false);
	renderSprites(*score->_trailSurface, true);

	if (_transType != 0)
		//TODO Handle changing area case
		playTransition(score);

	if (_sound1 != 0 || _sound2 != 0) {
		playSoundChannel();
	}

	g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, score->_surface->getBounds().width(), score->_surface->getBounds().height());
}

void Frame::playSoundChannel() {
	debug(0, "Sound2 %d", _sound2);
	debug(0, "Sound1 %d", _sound1);
}

void Frame::playTransition(Score *score) {
	uint16 duration = _transDuration * 250; // _transDuration in 1/4 of sec
	duration = (duration == 0 ? 250 : duration); // director support transition duration = 0, but animation play like value = 1, idk.

	if (_transChunkSize == 0)
		_transChunkSize = 1; //equal 1 step

	uint16 stepDuration = duration / _transChunkSize;
	uint16 steps = duration / stepDuration;

	switch (_transType) {
	case kTransCoverDown:
		{
			uint16 stepSize = score->_movieRect.height() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverUp:
		{
			uint16 stepSize = score->_movieRect.height() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverRight: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverLeft: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverUpLeft: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, score->_movieRect.height() - stepSize * i, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverUpRight: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverDownLeft: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverDownRight: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	default:
		warning("Unhandled transition type %d %d %d", _transType, duration, _transChunkSize);
		break;

	}
}

void Frame::renderSprites(Graphics::ManagedSurface &surface, bool renderTrail) {
	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		if (_sprites[i]->_enabled) {
			if ((_sprites[i]->_trails == 0 && renderTrail) || (_sprites[i]->_trails == 1 && !renderTrail))
				continue;

			Cast *cast;
			if (!_vm->_currentScore->_casts.contains(_sprites[i]->_castId)) {
				if (!_vm->getSharedCasts()->contains(_sprites[i]->_castId)) {
					warning("Cast id %d not found", _sprites[i]->_castId);
					continue;
				} else {
					cast = _vm->getSharedCasts()->getVal(_sprites[i]->_castId);
				}
			} else {
				cast = _vm->_currentScore->_casts[_sprites[i]->_castId];
			}

			if (cast->type == kCastText) {
				renderText(surface, i);
				continue;
			}

			Image::ImageDecoder *img = getImageFrom(_sprites[i]->_castId);

			if (!img) {
				warning("Image with id %d not found", _sprites[i]->_castId);
				continue;
			}

			if (!img->getSurface()) {
				//TODO
				//BMPDecoder doesnt cover all BITD resources (not all have first two bytes 'BM')
				//Some BITD's first two bytes 0x6 0x0
				warning("Can not load image %d", _sprites[i]->_castId);
				continue;
			}

			uint32 regX = static_cast<BitmapCast *>(_sprites[i]->_cast)->regX;
			uint32 regY = static_cast<BitmapCast *>(_sprites[i]->_cast)->regY;
			uint32 rectLeft = static_cast<BitmapCast *>(_sprites[i]->_cast)->initialRect.left;
			uint32 rectTop = static_cast<BitmapCast *>(_sprites[i]->_cast)->initialRect.top;

			int x = _sprites[i]->_startPoint.x - regX + rectLeft;
			int y = _sprites[i]->_startPoint.y - regY + rectTop;
			int height = _sprites[i]->_height;
			int width = _sprites[i]->_width;

			Common::Rect drawRect = Common::Rect(x, y, x + width, y + height);
			_drawRects.push_back(drawRect);

			switch (_sprites[i]->_ink) {
			case kInkTypeCopy:
				surface.blitFrom(*img->getSurface(), Common::Point(x, y));
				break;
			case kInkTypeBackgndTrans:
				drawBackgndTransSprite(surface, *img->getSurface(), drawRect);
				break;
			case kInkTypeMatte:
				drawMatteSprite(surface, *img->getSurface(), drawRect);
				break;
			case kInkTypeGhost:
				drawGhostSprite(surface, *img->getSurface(), drawRect);
				break;
			case kInkTypeReverse:
				drawReverseSprite(surface, *img->getSurface(), drawRect);
				break;
			default:
				warning("Unhandled ink type %d", _sprites[i]->_ink);
				surface.blitFrom(*img->getSurface(), Common::Point(x, y));
				break;
			}
		}
	}
}

void Frame::renderButton(Graphics::ManagedSurface &surface, uint16 spriteId) {
	renderText(surface, spriteId);

	uint16 castID = _sprites[spriteId]->_castId;
	ButtonCast *button = static_cast<ButtonCast *>(_vm->_currentScore->_casts[castID]);

	uint32 rectLeft = button->initialRect.left;
	uint32 rectTop = button->initialRect.top;

	int x = _sprites[spriteId]->_startPoint.x + rectLeft;
	int y = _sprites[spriteId]->_startPoint.y + rectTop;
	int height = _sprites[spriteId]->_height;
	int width = _sprites[spriteId]->_width;

	switch (button->buttonType) {
	case kTypeCheckBox:
		//Magic numbers: checkbox square need to move left about 5px from text and 12px side size (d4)
		surface.frameRect(Common::Rect(x - 17, y, x + 12, y + 12), 0);
		break;
	case kTypeButton:
		surface.frameRect(Common::Rect(x, y, x + width, y + height), 0);
		break;
	case kTypeRadio:
		warning("STUB: renderButton: kTypeRadio");
		break;
	}
}

Image::ImageDecoder *Frame::getImageFrom(uint16 spriteId) {
	uint16 imgId = spriteId + 1024;
	Image::ImageDecoder *img = NULL;

	if (_vm->_currentScore->getArchive()->hasResource(MKTAG('D', 'I', 'B', ' '), imgId)) {
		img = new DIBDecoder();
		img->loadStream(*_vm->_currentScore->getArchive()->getResource(MKTAG('D', 'I', 'B', ' '), imgId));
		return img;
	}

	if (_vm->getSharedDIB() != NULL && _vm->getSharedDIB()->contains(imgId)) {
		img = new DIBDecoder();
		img->loadStream(*_vm->getSharedDIB()->getVal(imgId));
		return img;
	}

	if (_vm->_currentScore->getArchive()->hasResource(MKTAG('B', 'I', 'T', 'D'), imgId)) {
		img = new Image::BitmapDecoder();
		img->loadStream(*_vm->_currentScore->getArchive()->getResource(MKTAG('B', 'I', 'T', 'D'), imgId));
		return img;
	}

	if (_vm->getSharedBMP() != NULL && _vm->getSharedBMP()->contains(imgId)) {
		img = new Image::BitmapDecoder();
		img->loadStream(*_vm->getSharedBMP()->getVal(imgId));
		return img;
	}

	warning("Image %d not found", spriteId);
	return img;
}


void Frame::renderText(Graphics::ManagedSurface &surface, uint16 spriteID) {
	uint16 castID = _sprites[spriteID]->_castId;

	TextCast *textCast = static_cast<TextCast *>(_vm->_currentScore->_casts[castID]);
	Common::SeekableSubReadStreamEndian *textStream;

	if (_vm->_currentScore->_movieArchive->hasResource(MKTAG('S','T','X','T'), castID + 1024)) {
		textStream = _vm->_currentScore->_movieArchive->getResource(MKTAG('S','T','X','T'), castID + 1024);
	} else {
		textStream = _vm->getSharedSTXT()->getVal(spriteID + 1024);
	}
	/*uint32 unk1 = */ textStream->readUint32();
	uint32 strLen = textStream->readUint32();
	/*uin32 dataLen = */ textStream->readUint32();
	Common::String text;

	for (uint32 i = 0; i < strLen; i++) {
		byte ch = textStream->readByte();
		if (ch == 0x0d) {
			ch = '\n';
		}
		text += ch;
	}

	uint32 rectLeft = static_cast<TextCast *>(_sprites[spriteID]->_cast)->initialRect.left;
	uint32 rectTop = static_cast<TextCast *>(_sprites[spriteID]->_cast)->initialRect.top;

	int x = _sprites[spriteID]->_startPoint.x + rectLeft;
	int y = _sprites[spriteID]->_startPoint.y + rectTop;
	int height = _sprites[spriteID]->_height;
	int width = _sprites[spriteID]->_width;

	const char *fontName;

	if (_vm->_currentScore->_fontMap.contains(textCast->fontId)) {
		fontName = _vm->_currentScore->_fontMap[textCast->fontId].c_str();
	} else if ((fontName = _vm->_wm->getFontName(textCast->fontId, textCast->fontSize)) == NULL) {
		warning("Unknown font id %d, falling back to default", textCast->fontId);
		fontName = _vm->_wm->getFontName(0, 12);
	}

	const Graphics::Font *font = _vm->_wm->getFont(fontName, Graphics::FontManager::kBigGUIFont);

	font->drawString(&surface, text, x, y, width, 0);

	if (textCast->borderSize != kSizeNone) {
		uint16 size = textCast->borderSize;

		//Indent from borders, measured in d4
		x -= 1;
		y -= 4;

		height += 4;
		width += 1;

		while (size) {
			surface.frameRect(Common::Rect(x, y, x + height, y + width), 0);
			x--;
			y--;
			height += 2;
			width += 2;
			size--;
		}
	}

	if (textCast->gutterSize != kSizeNone) {
		x -= 1;
		y -= 4;

		height += 4;
		width += 1;
		uint16 size = textCast->gutterSize;

		surface.frameRect(Common::Rect(x, y, x + height, y + width), 0);

		while (size) {
			surface.drawLine(x + width, y, x + width, y + height, 0);
			surface.drawLine(x, y + height, x + width, y + height, 0);
			x++;
			y++;
			size--;
		}
	}
}

void Frame::drawBackgndTransSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1; //FIXME is it always white (last entry in pallette) ?

	for (int ii = 0; ii < sprite.h; ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(0, ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if (*src != skipColor)
				*dst = *src;

			src++;
			dst++;
		}
	}
}

void Frame::drawGhostSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < sprite.h; ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(0, ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0) && (*src != skipColor))
				*dst = (_vm->getPaletteColorCount() - 1) - *src; //Oposite color

			src++;
			dst++;
		}
	}
}

void Frame::drawReverseSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < sprite.h; ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(0, ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0))
				*dst = (_vm->getPaletteColorCount() - 1) - *src;
			else if (*src != skipColor)
				*dst = *src;
			src++;
			dst++;
		}
	}
}

void Frame::drawMatteSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	//Like background trans, but all white pixels NOT ENCLOSED by coloured pixels are transparent
	Graphics::Surface tmp;
	tmp.copyFrom(sprite);

	// Searching white color in the corners
	int whiteColor = -1;

	for (int corner = 0; corner < 4; corner++) {
		int x = (corner & 0x1) ? tmp.w - 1 : 0;
		int y = (corner & 0x2) ? tmp.h - 1 : 0;

		byte color = *(byte *)tmp.getBasePtr(x, y);

		if (_vm->getPalette()[color * 3 + 0] == 0xff &&
			_vm->getPalette()[color * 3 + 1] == 0xff &&
			_vm->getPalette()[color * 3 + 2] == 0xff) {
			whiteColor = color;
			break;
		}
	}

	if (whiteColor == -1) {
		warning("No white color for Matte image");
		whiteColor = *(byte *)tmp.getBasePtr(0, 0);
	}

	Graphics::FloodFill ff(&tmp, whiteColor, 0, true);

	for (int yy = 0; yy < tmp.h; yy++) {
		ff.addSeed(0, yy);
		ff.addSeed(tmp.w - 1, yy);
	}

	for (int xx = 0; xx < tmp.w; xx++) {
		ff.addSeed(xx, 0);
		ff.addSeed(xx, tmp.h - 1);
	}
	ff.fillMask();

	for (int yy = 0; yy < tmp.h; yy++) {
		const byte *src = (const byte *)tmp.getBasePtr(0, yy);
		const byte *mask = (const byte *)ff.getMask()->getBasePtr(0, yy);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + yy);

		for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++, mask++)
			if (*mask == 0)
				*dst = *src;
	}

	tmp.free();
}

uint16 Frame::getSpriteIDFromPos(Common::Point pos) {
	//Find first from top to bottom
	for (uint16 i = _drawRects.size() - 1; i > 0; i--) {
		if (_drawRects[i].contains(pos))
			return i;
	}

	return 0;
}

Sprite::Sprite() {
	_enabled = false;
	_trails = 0;
	_width = 0;
	_ink = kInkTypeCopy;
	_flags = 0;
	_height = 0;
	_castId = 0;
	_constraint = 0;
	_moveable = 0;
	_castId = 0;
	_backColor = 0;
	_foreColor = 0;
	_left = 0;
	_right = 0;
	_top = 0;
	_bottom = 0;
	_visible = false;
	_movieRate = 0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;
	_volume = 0;
	_stretch = 0;
	_type = kInactiveSprite;
}

Sprite::Sprite(const Sprite &sprite) {
	_enabled = sprite._enabled;
	_castId = sprite._castId;
	_flags = sprite._flags;
	_trails = sprite._trails;
	_ink = sprite._ink;
	_width = sprite._width;
	_height = sprite._height;
	_startPoint.x = sprite._startPoint.x;
	_startPoint.y = sprite._startPoint.y;
	_backColor = sprite._backColor;
	_foreColor = sprite._foreColor;
	_left = sprite._left;
	_right = sprite._right;
	_top = sprite._top;
	_bottom = sprite._bottom;
	_visible = sprite._visible;
	_movieRate = sprite._movieRate;
	_movieTime = sprite._movieTime;
	_stopTime = sprite._stopTime;
	_volume = sprite._volume;
	_stretch = sprite._stretch;
	_type = sprite._type;
}

Sprite::~Sprite() {
	delete _cast;
	delete &_startPoint;
}

} //End of namespace Director
