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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/memstream.h"

#include "engines/util.h"
#include "graphics/font.h"
#include "graphics/palette.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/cast.h"
#include "director/score.h"
#include "director/frame.h"
#include "director/archive.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/lingo/lingo.h"

namespace Director {

const char *scriptTypes[] = {
	"MovieScript",
	"SpriteScript",
	"FrameScript",
	"CastScript"
};

const char *scriptType2str(ScriptType scr) {
	if (scr < 0)
		return "NoneScript";

	if (scr > kMaxScriptType)
		return "<unknown>";

	return scriptTypes[scr];
}


Score::Score(DirectorEngine *vm, Archive *archive) {
	_vm = vm;
	_surface = new Graphics::ManagedSurface;
	_trailSurface = new Graphics::ManagedSurface;
	_movieArchive = archive;
	_lingo = _vm->getLingo();
	_soundManager = _vm->getSoundManager();

	// FIXME: TODO: Check whether the original truely does it
	if (_vm->getVersion() <= 3) {
		_lingo->executeScript(kMovieScript, 0);
	}

	_lingo->processEvent(kEventPrepareMovie, kMovieScript, 0);
	_movieScriptCount = 0;
	_labels = NULL;
	_font = NULL;

	_versionMinor = _versionMajor = 0;
	_currentFrameRate = 20;
	_castArrayStart = _castArrayEnd = 0;
	_currentFrame = 0;
	_nextFrameTime = 0;
	_flags = 0;
	_stopPlay = false;
	_stageColor = 0;

	if (archive->hasResource(MKTAG('M', 'C', 'N', 'M'), 0)) {
		_macName = archive->getName(MKTAG('M', 'C', 'N', 'M'), 0).c_str();
	} else {
		_macName = archive->getFileName();
	}

	if (archive->hasResource(MKTAG('V', 'W', 'L', 'B'), 1024)) {
		loadLabels(*archive->getResource(MKTAG('V', 'W', 'L', 'B'), 1024));
	}
}

void Score::loadArchive() {
	Common::Array<uint16> clutList = _movieArchive->getResourceIDList(MKTAG('C', 'L', 'U', 'T'));

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

	if (_movieArchive->hasResource(MKTAG('F', 'O', 'N', 'D'), -1)) {
		debug("Movie has fonts. Loading....");
	}

	assert(_movieArchive->hasResource(MKTAG('V', 'W', 'S', 'C'), 1024));
	assert(_movieArchive->hasResource(MKTAG('V', 'W', 'C', 'F'), 1024));

	loadFrames(*_movieArchive->getResource(MKTAG('V', 'W', 'S', 'C'), 1024));
	loadConfig(*_movieArchive->getResource(MKTAG('V', 'W', 'C', 'F'), 1024));

	if (_vm->getVersion() < 4) {
		assert(_movieArchive->hasResource(MKTAG('V', 'W', 'C', 'R'), 1024));
		loadCastDataVWCR(*_movieArchive->getResource(MKTAG('V', 'W', 'C', 'R'), 1024));
	}

	if (_movieArchive->hasResource(MKTAG('V', 'W', 'A', 'C'), 1024)) {
		loadActions(*_movieArchive->getResource(MKTAG('V', 'W', 'A', 'C'), 1024));
	}

	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'I'), 1024)) {
		loadFileInfo(*_movieArchive->getResource(MKTAG('V', 'W', 'F', 'I'), 1024));
	}

	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'M'), 1024)) {
		_vm->_wm->_fontMan->clearFontMapping();

		loadFontMap(*_movieArchive->getResource(MKTAG('V', 'W', 'F', 'M'), 1024));
	}

	Common::Array<uint16> vwci = _movieArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'I'));
	if (vwci.size() > 0) {
		for (Common::Array<uint16>::iterator iterator = vwci.begin(); iterator != vwci.end(); ++iterator)
			loadCastInfo(*_movieArchive->getResource(MKTAG('V', 'W', 'C', 'I'), *iterator), *iterator);
	}

	Common::Array<uint16> cast = _movieArchive->getResourceIDList(MKTAG('C', 'A', 'S', 't'));
	if (cast.size() > 0) {
		for (Common::Array<uint16>::iterator iterator = cast.begin(); iterator != cast.end(); ++iterator) {
			Common::SeekableSubReadStreamEndian *stream = _movieArchive->getResource(MKTAG('C', 'A', 'S', 't'), *iterator);
			Resource res = _movieArchive->getResourceDetail(MKTAG('C', 'A', 'S', 't'), *iterator);
			loadCastData(*stream, *iterator, &res);
		}
	}

	setSpriteCasts();

	// Try to load movie script, it sits in resource A11
	if (_vm->getVersion() <= 3) {
		Common::Array<uint16> stxt = _movieArchive->getResourceIDList(MKTAG('S','T','X','T'));
		if (stxt.size() > 0) {
			loadScriptText(*_movieArchive->getResource(MKTAG('S','T','X','T'), *stxt.begin()));
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

	delete _font;
	delete _labels;
}

void Score::loadPalette(Common::SeekableSubReadStreamEndian &stream) {
	uint16 steps = stream.size() / 6;
	uint16 index = (steps * 3) - 1;
	uint16 _paletteColorCount = steps;
	byte *_palette = new byte[index + 1];

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
		uint32 unk1 = stream.readUint32();
		uint32 unk2 = stream.readUint32();
		uint16 unk3 = stream.readUint16();
		uint16 unk4 = stream.readUint16();
		uint16 unk5 = stream.readUint16();
		uint16 unk6 = stream.readUint16();
		size -= 16;

		warning("STUB: Score::loadFrames. unk1: %x unk2: %x unk3: %x unk4: %x unk5: %x unk6: %x", unk1, unk2, unk3, unk4, unk5, unk6);
		// Unknown, some bytes - constant (refer to contuinity).
	}

	uint16 channelSize;
	uint16 channelOffset;

	Frame *initial = new Frame(_vm);
	_frames.push_back(initial);

	// This is a representation of the channelData. It gets overridden
	// partically by channels, hence we keep it and read the score from left to right
	//
	// TODO Merge it with shared cast
	byte channelData[kChannelDataSize];
	memset(channelData, 0, kChannelDataSize);

	while (size != 0) {
		uint16 frameSize = stream.readUint16();
		debugC(kDebugLoading, 8, "++++ score frame %d (frameSize %d) size %d", _frames.size(), frameSize, size);
		size -= frameSize;
		frameSize -= 2;

		Frame *frame = new Frame(_vm);

		while (frameSize != 0) {
			if (_vm->getVersion() < 4) {
				channelSize = stream.readByte() * 2;
				channelOffset = stream.readByte() * 2;
				frameSize -= channelSize + 2;
			} else {
				channelSize = stream.readUint16();
				channelOffset = stream.readUint16();
				frameSize -= channelSize + 4;
			}

			assert(channelOffset + channelSize < kChannelDataSize);

			stream.read(&channelData[channelOffset], channelSize);
		}

		Common::MemoryReadStreamEndian *str = new Common::MemoryReadStreamEndian(channelData, ARRAYSIZE(channelData), stream.isBE());
		//Common::hexdump(channelData, ARRAYSIZE(channelData));
		frame->readChannels(str);

		debugC(3, kDebugLoading, "Frame %d actionId: %d", _frames.size(), frame->_actionId);

		delete str;

		_frames.push_back(frame);
	}

	// Remove initial frame
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

void Score::loadCastDataVWCR(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "Score::loadCastDataVWCR(). start: %d, end: %d", _castArrayStart, _castArrayEnd);

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
			warning("Score::loadCastDataVWCR(): Unhandled cast type: %d", castType);
			stream.skip(size - 1);
			break;
		}
	}
}

void Score::setSpriteCasts() {
	// Set cast pointers to sprites
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			byte castId = _frames[i]->_sprites[j]->_castId;

			if (_casts.contains(castId))
				_frames[i]->_sprites[j]->_cast = _casts.find(castId)->_value;
		}
	}
}

void Score::loadCastData(Common::SeekableSubReadStreamEndian &stream, uint16 id, Resource *res) {
	// d4+ variant
	if (stream.size() == 0)
		return;

	//TODO: Determine if there really is a minimum size.
	//This value was too small for Shape Casts.
	if (stream.size() < 10) {
		warning("CAST data id %d is too small", id);
		return;
	}

	debugC(3, kDebugLoading, "CASt: id: %d", id);

	if (debugChannelSet(5, kDebugLoading))
		stream.hexdump(stream.size());

	uint32 size1, size2, size3, castType;
	byte unk1 = 0, unk2 = 0, unk3 = 0;

	if (_vm->getVersion() < 4) {
		size1 = stream.readUint16();
		size2 = stream.readUint32();
		size3 = 0;
		castType = stream.readByte();
		unk1 = stream.readByte();
		unk2 = stream.readByte();
		unk3 = stream.readByte();
	} else if (_vm->getVersion() < 5) {
		size1 = stream.readUint16() + 2;
		size2 = stream.readUint32();
		size3 = 0;
		castType = stream.readByte();
		unk1 = stream.readByte();
	} else {
		// FIXME: only the cast type and the strings are good
		castType = stream.readUint32();
		size2 = stream.readUint32();
		size3 = stream.readUint32();
		size1 = stream.readUint32();
		assert(size1 == 0x14);
		size1 = 0;
	}

	debugC(3, kDebugLoading, "CASt: id: %d type: %x size1: %d size2: %d (%x) size3: %d unk1: %d unk2: %d unk3: %d",
		id, castType, size1, size2, size2, size3, unk1, unk2, unk3);

	byte *data = (byte *)calloc(size1 + 16, 1); // 16 is for bounding rects
	stream.read(data, size1 + 16);

	Common::MemoryReadStreamEndian castStream(data, size1 + 16, stream.isBE());

	switch (castType) {
	case kCastBitmap:
		_casts[id] = new BitmapCast(castStream, _vm->getVersion());
		_casts[id]->type = kCastBitmap;
		break;
	case kCastText:
		_casts[id] = new TextCast(castStream, _vm->getVersion());
		_casts[id]->type = kCastText;
		break;
	case kCastShape:
		_casts[id] = new ShapeCast(castStream, _vm->getVersion());
		_casts[id]->type = kCastShape;
		break;
	case kCastButton:
		_casts[id] = new ButtonCast(castStream, _vm->getVersion());
		_casts[id]->type = kCastButton;
		break;
	case kCastLingoScript:
		_casts[id] = new ScriptCast(castStream, _vm->getVersion());
		_casts[id]->type = kCastLingoScript;
		break;
	default:
		warning("Score::loadCastData(): Unhandled cast type: %d", castType);
		break;
	}

	if (res != NULL) {
		for (uint child = 0; child < res->children.size(); child++)
			_casts[id]->children.push_back(res->children[child]);
	}

	free(data);

	if (size2) {
		uint32 entryType = 0;
		Common::Array<Common::String> castStrings = loadStrings(stream, entryType, false);

		debugCN(4, kDebugLoading, "str(%d): '", castStrings.size());

		for (uint i = 0; i < castStrings.size(); i++) {
			debugCN(4, kDebugLoading, "%s'", castStrings[i].c_str());
			if (i != castStrings.size() - 1)
				debugCN(4, kDebugLoading, ", '");
		}
		debugC(4, kDebugLoading, "'");

		CastInfo *ci = new CastInfo();

		if (castStrings.size() >= 5) {
			ci->script = castStrings[0];
			ci->name = castStrings[1];
			ci->directory = castStrings[2];
			ci->fileName = castStrings[3];
			ci->type = castStrings[4];

			if (!ci->script.empty()) {
				// the script type here could be wrong!
				if (ConfMan.getBool("dump_scripts"))
					dumpScript(ci->script.c_str(), kCastScript, id);

				_lingo->addCode(ci->script.c_str(), kCastScript, id);
			}
		}

		_castsInfo[id] = ci;
	}

	if (size3)
		warning("size3: %x", size3);
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
		debugC(2, kDebugLoading, "Frame %d, Label %s", (*j)->number, (*j)->name.c_str());
	}
}

int Score::compareLabels(const void *a, const void *b) {
	return ((const Label *)a)->number - ((const Label *)b)->number;
}

void Score::loadActions(Common::SeekableSubReadStreamEndian &stream) {
	uint16 count = stream.readUint16() + 1;
	uint16 offset = count * 4 + 2;

	byte id = stream.readByte();

	byte subId = stream.readByte(); // I couldn't find how it used in continuity (except print). Frame actionId = 1 byte.
	uint16 stringPos = stream.readUint16() + offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 nextId = stream.readByte();
		byte nextSubId = stream.readByte();
		uint16 nextStringPos = stream.readUint16() + offset;
		uint16 streamPos = stream.pos();

		stream.seek(stringPos);

		for (uint16 j = stringPos; j < nextStringPos; j++) {
			byte ch = stream.readByte();
			if (ch == 0x0d) {
				ch = '\n';
			}
			_actions[i + 1] += ch;
		}

		debugC(3, kDebugLoading, "id: %d nextId: %d subId: %d, code: %s", id, nextId, subId, _actions[id].c_str());

		stream.seek(streamPos);

		id = nextId;
		subId = nextSubId;
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

		// Convert Mac line endings
		if (ch == 0x0d)
			ch = '\n';

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
	case kNoneScript:
		error("Incorrect dumpScript() call");
	case kFrameScript:
		typeName = "frame";
		break;
	case kMovieScript:
		typeName = "movie";
		break;
	case kSpriteScript:
		typeName = "sprite";
		break;
	case kCastScript:
		typeName = "cast";
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

void Score::gotoLoop() {
	// This command has the playback head contonuously return to the first marker to to the left and then loop back.
	// If no marker are to the left of the playback head, the playback head continues to the right.
	Common::SortedArray<Label *>::iterator i;

	if (_labels == NULL) {
		_currentFrame = 0;
		return;
	} else {
		for (i = _labels->begin(); i != _labels->end(); ++i) {
			if ((*i)->name == _currentLabel) {
				_currentFrame = (*i)->number;
				return;
			}
		}
	}

	g_director->_skipFrameAdvance = true;
}

int Score::getCurrentLabelNumber() {
	Common::SortedArray<Label *>::iterator i;

	int frame = 0;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number <= _currentFrame)
			frame = (*i)->number;
	}

	return frame;
}

void Score::gotoNext() {
	// we can just try to use the current frame and get the next label
	_currentFrame = getNextLabelNumber(_currentFrame);

	g_director->_skipFrameAdvance = true;
}

void Score::gotoPrevious() {
	// we actually need the frame of the label prior to the most recent label.
	_currentFrame = getPreviousLabelNumber(getCurrentLabelNumber());

	g_director->_skipFrameAdvance = true;
}

int Score::getNextLabelNumber(int referenceFrame) {
	if (_labels == NULL || _labels->size() == 0)
		return 0;

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number >= referenceFrame) {
			if (i != _labels->end()) {
				// return to the first marker to to the right
				++i;
				return (*i)->number;
			} else {
				// if no markers are to the right of the playback head,
				// the playback head goes to the first marker to the left
				return (*i)->number;
			}
		}
	}

	// If there are not markers to the left,
	// the playback head goes to frame 1, (Director frame array start from 1, engine from 0)
	return 0;
}

int Score::getPreviousLabelNumber(int referenceFrame) {
	if (_labels == NULL || _labels->size() == 0)
		return 0;

	// One label
	if (_labels->begin() == _labels->end())
		return (*_labels->begin())->number;

	Common::SortedArray<Label *>::iterator previous = _labels->begin();
	Common::SortedArray<Label *>::iterator i;

	for (i = (previous + 1); i != _labels->end(); ++i, ++previous) {
		if ((*i)->number >= referenceFrame)
			return (*previous)->number;
	}

	return 0;
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

	uint16 count = stream.readUint16() + 1;

	uint32 *entries = (uint32 *)calloc(count, sizeof(uint32));

	for (uint i = 0; i < count; i++)
		entries[i] = stream.readUint32();

	byte *data = (byte *)malloc(entries[count - 1]);
	stream.read(data, entries[count - 1]);

	for (uint16 i = 0; i < count - 1; i++) {
		Common::String entryString;

		for (uint j = entries[i]; j < entries[i + 1]; j++)
			if (data[j] == '\r')
				entryString += '\n';
			else
				entryString += data[j];

		strings.push_back(entryString);
	}

	free(data);
	free(entries);

	return strings;
}

void Score::loadFontMap(Common::SeekableSubReadStreamEndian &stream) {
	if (stream.size() == 0)
		return;

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
		_vm->_wm->_fontMan->registerFontMapping(id, font);

		debug(3, "Fontmap. ID %d Font %s", id, font.c_str());
		currentRawPosition = stream.pos();
		stream.seek(positionInfo);
	}
}

Common::Rect Score::readRect(Common::ReadStreamEndian &stream) {
	Common::Rect rect;
	rect.top = stream.readUint16();
	rect.left = stream.readUint16();
	rect.bottom = stream.readUint16();
	rect.right = stream.readUint16();

	return rect;
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

	_frames[_currentFrame]->prepareFrame(this);

	while (!_stopPlay && _currentFrame < _frames.size()) {
		debugC(1, kDebugImages, "Current frame: %d", _currentFrame);
		update();
		processEvents();
	}
}

void Score::update() {
	if (g_system->getMillis() < _nextFrameTime)
		return;

	_surface->clear();
	_surface->copyFrom(*_trailSurface);

	// Enter and exit from previous frame (Director 4)
	_lingo->processEvent(kEventEnterFrame, kFrameScript, _frames[_currentFrame]->_actionId);
	_lingo->processEvent(kEventExitFrame, kFrameScript, _frames[_currentFrame]->_actionId);
	// TODO Director 6 - another order

	// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
	if (_vm->getVersion() >= 6) {
		for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
			if (_frames[_currentFrame]->_sprites[i]->_enabled) {
				//TODO: Check if this is also possibly a kSpriteScript?
				_lingo->processEvent(kEventBeginSprite, kCastScript, _frames[_currentFrame]->_sprites[i]->_scriptId);
			}
		}
	}

	// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	if (_vm->getVersion() >= 6)
		_lingo->processEvent(kEventPrepareFrame, kFrameScript, _currentFrame);

	Common::SortedArray<Label *>::iterator i;
	if (_labels != NULL) {
		for (i = _labels->begin(); i != _labels->end(); ++i) {
			if ((*i)->number == _currentFrame) {
				_currentLabel = (*i)->name;
			}
		}
	}

	if (!g_director->_playbackPaused && !g_director->_skipFrameAdvance)
		_currentFrame++;

	g_director->_skipFrameAdvance = false;

	if (_currentFrame >= _frames.size())
		return;

	_frames[_currentFrame]->prepareFrame(this);
	// Stage is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell)

	byte tempo = _frames[_currentFrame]->_tempo;

	if (tempo) {
		if (tempo > 161) {
			// Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;

			return;
		} else if (tempo <= 60) {
			// FPS
			_nextFrameTime = g_system->getMillis() + (float)tempo / 60 * 1000;
			_currentFrameRate = tempo;
		} else if (tempo >= 136) {
			// TODO Wait for channel tempo - 135
			warning("STUB: tempo >= 136");
		} else if (tempo == 128) {
			// TODO Wait for Click/Key
			warning("STUB: tempo == 128");
		} else if (tempo == 135) {
			// Wait for sound channel 1
			while (_soundManager->isChannelActive(1)) {
				processEvents();
			}
		} else if (tempo == 134) {
			// Wait for sound channel 2
			while (_soundManager->isChannelActive(2)) {
				processEvents();
			}
		}
	}

	_nextFrameTime = g_system->getMillis() + (float)_currentFrameRate / 60 * 1000;
}

void Score::processEvents() {
	// TODO: re-instate when we know which script to run.
	//if (_currentFrame > 0)
	//	_lingo->processEvent(kEventIdle, _currentFrame - 1);

	Common::Event event;

	uint endTime = g_system->getMillis() + 200;

	while (g_system->getMillis() < endTime) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT)
				_stopPlay = true;

			if (event.type == Common::EVENT_LBUTTONDOWN) {
				Common::Point pos = g_system->getEventManager()->getMousePos();

				// D3 doesn't have both mouse up and down.
				if (_vm->getVersion() > 3) {
					// TODO: check that this is the order of script execution!
					uint16 spriteId = _frames[_currentFrame]->getSpriteIDFromPos(pos);
					_lingo->processEvent(kEventMouseDown, kCastScript, _frames[_currentFrame]->_sprites[spriteId]->_castId);
					_lingo->processEvent(kEventMouseDown, kSpriteScript, _frames[_currentFrame]->_sprites[spriteId]->_scriptId);
				}
			}

			if (event.type == Common::EVENT_LBUTTONUP) {
				Common::Point pos = g_system->getEventManager()->getMousePos();

				uint16 spriteId = _frames[_currentFrame]->getSpriteIDFromPos(pos);
				if (_vm->getVersion() > 3) {
					// TODO: check that this is the order of script execution!
					_lingo->processEvent(kEventMouseUp, kCastScript, _frames[_currentFrame]->_sprites[spriteId]->_castId);
					_lingo->processEvent(kEventMouseUp, kSpriteScript, _frames[_currentFrame]->_sprites[spriteId]->_scriptId);
				} else {
					// D3 doesn't have cast member or sprite scripts. Just Frame Scripts.
					_lingo->processEvent(kEventMouseUp, kFrameScript, _frames[_currentFrame]->_sprites[spriteId]->_scriptId);
				}
			}

			if (event.type == Common::EVENT_KEYDOWN) {
				_vm->_keyCode = event.kbd.keycode;
				_vm->_key = (unsigned char)(event.kbd.ascii & 0xff);

				switch (_vm->_keyCode) {
				case Common::KEYCODE_LEFT:
					_vm->_keyCode = 123;
					break;
				case Common::KEYCODE_RIGHT:
					_vm->_keyCode = 124;
					break;
				case Common::KEYCODE_DOWN:
					_vm->_keyCode = 125;
					break;
				case Common::KEYCODE_UP:
					_vm->_keyCode = 126;
					break;
				default:
					warning("Keycode: %d", _vm->_keyCode);
				}

				// TODO: is movie script correct? Can this be elsewhere?
				_lingo->processEvent(kEventKeyDown, kMovieScript, 0);
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
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

} // End of namespace Director
