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

#include "common/config-manager.h"
#include "common/substream.h"

#include "engines/util.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/stage.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"

namespace Director {

Movie::Movie(Stage *stage) {
	_stage = stage;
	_vm = _stage->getVM();
	_lingo = _vm->getLingo();

	_flags = 0;
	_stageColor = 0xFF;

	_currentClickOnSpriteId = 0;
	_currentEditableTextChannel = 0;
	_lastEventTime = _vm->getMacTicks();
	_lastKeyTime = _lastEventTime;
	_lastClickTime = _lastEventTime;
	_lastRollTime = _lastEventTime;
	_lastTimerReset = _lastEventTime;
	_nextEventId = 0;

	_key = 0;
	_keyCode = 0;
	_keyFlags = 0;

	_currentDraggedChannel = nullptr;

	_allowOutdatedLingo = false;

	_movieArchive = nullptr;

	_cast = new Cast(this);
	_sharedCast = nullptr;
	_score = new Score(this);
}

Movie::~Movie() {
	// _movieArchive is shared with the cast, so the cast will free it
	delete _cast;
	delete _sharedCast;
	delete _score;
}

void Movie::setArchive(Archive *archive) {
	_movieArchive = archive;

	if (archive->hasResource(MKTAG('M', 'C', 'N', 'M'), 0)) {
		_macName = archive->getName(MKTAG('M', 'C', 'N', 'M'), 0).c_str();
	} else {
		_macName = archive->getFileName();
	}

	_cast->setArchive(archive);

	// Frame Labels
	if (archive->hasResource(MKTAG('V', 'W', 'L', 'B'), -1)) {
		Common::SeekableSubReadStreamEndian *r;
		_score->loadLabels(*(r = archive->getFirstResource(MKTAG('V', 'W', 'L', 'B'))));
		delete r;
	}
}

bool Movie::loadArchive() {
	Common::SeekableSubReadStreamEndian *r = nullptr;

	// File Info
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'I'), -1)) {
		loadFileInfo(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'I'))));
		delete r;
	}

	// Cast
	_cast->loadArchive();

	// _movieRect and _stageColor are in VWCF, which the cast handles

	// If the stage dimensions are different, delete it and start again.
	// Otherwise, do not clear it so there can be a nice transition.
	if (_stage->getSurface()->w != _movieRect.width() || _stage->getSurface()->h != _movieRect.height()) {
		_stage->resize(_movieRect.width(), _movieRect.height(), true);
	}
	// TODO: Add more options for desktop dimensions
	if (_stage == _vm->getMainStage()) {
		uint16 windowWidth = debugChannelSet(-1, kDebugDesktop) ? 1024 : _movieRect.width();
		uint16 windowHeight = debugChannelSet(-1, kDebugDesktop) ? 768 : _movieRect.height();
		if (_vm->_surface->w != windowWidth || _vm->_surface->h != windowHeight) {
			_vm->_surface->free();
			_vm->_surface->create(windowWidth, windowHeight, Graphics::PixelFormat::createFormatCLUT8());
		}
		initGraphics(windowWidth, windowHeight);
	}

	_stage->setStageColor(_stageColor);

	// Score
	if (!_movieArchive->hasResource(MKTAG('V', 'W', 'S', 'C'), -1)) {
		warning("Movie::loadArchive(): Wrong movie format. VWSC resource missing");
		return false;
	}
	_score->loadFrames(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'S', 'C'))));
	delete r;

	// Action list
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'A', 'C'), -1)) {
		_score->loadActions(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'A', 'C'))));
		delete r;
	}

	_score->setSpriteCasts();

	return true;
}

Common::Rect Movie::readRect(Common::ReadStreamEndian &stream) {
	Common::Rect rect;
	rect.top = stream.readUint16();
	rect.left = stream.readUint16();
	rect.bottom = stream.readUint16();
	rect.right = stream.readUint16();

	return rect;
}

InfoEntries Movie::loadInfoEntries(Common::SeekableSubReadStreamEndian &stream) {
	uint32 offset = stream.pos();
	offset += stream.readUint32();

	InfoEntries res;
	res.unk1 = stream.readUint32();
	res.unk2 = stream.readUint32();
	res.flags = stream.readUint32();

	if (g_director->getVersion() >= 4)
		res.scriptId = stream.readUint32();

	stream.seek(offset);
	uint16 count = stream.readUint16() + 1;

	debugC(3, kDebugLoading, "Movie::loadInfoEntries(): InfoEntry: %d entries", count - 1);

	if (count == 1)
		return res;

	uint32 *entries = (uint32 *)calloc(count, sizeof(uint32));

	for (uint i = 0; i < count; i++)
		entries[i] = stream.readUint32();

	res.strings.resize(count - 1);

	for (uint16 i = 0; i < count - 1; i++) {
		res.strings[i].len = entries[i + 1] - entries[i];
		res.strings[i].data = (byte *)malloc(res.strings[i].len);
		stream.read(res.strings[i].data, res.strings[i].len);

		debugC(6, kDebugLoading, "InfoEntry %d: %d bytes", i, res.strings[i].len);
	}

	free(entries);

	return res;
}

void Movie::loadFileInfo(Common::SeekableSubReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading FileInfo VWFI");

	InfoEntries fileInfo = Movie::loadInfoEntries(stream);

	_allowOutdatedLingo = (fileInfo.flags & kMovieFlagAllowOutdatedLingo) != 0;

	_script = fileInfo.strings[0].readString(false);

	if (!_script.empty() && ConfMan.getBool("dump_scripts"))
		_cast->dumpScript(_script.c_str(), kMovieScript, _cast->_movieScriptCount);

	if (!_script.empty())
		_cast->_lingoArchive->addCode(_script.c_str(), kMovieScript, _cast->_movieScriptCount);

	_cast->_movieScriptCount++;
	_changedBy = fileInfo.strings[1].readString();
	_createdBy = fileInfo.strings[2].readString();
	_createdBy = fileInfo.strings[3].readString();

	uint16 preload = 0;
	if (fileInfo.strings[4].len) {
		if (stream.isBE())
			preload = READ_BE_INT16(fileInfo.strings[4].data);
		else
			preload = READ_LE_INT16(fileInfo.strings[4].data);
	}

	if (debugChannelSet(3, kDebugLoading)) {
		debug("VWFI: flags: %d", fileInfo.flags);
		debug("VWFI: allow outdated lingo: %d", _allowOutdatedLingo);
		debug("VWFI: script: '%s'", _script.c_str());
		debug("VWFI: changed by: '%s'", _changedBy.c_str());
		debug("VWFI: created by: '%s'", _createdBy.c_str());
		debug("VWFI: directory: '%s'", _createdBy.c_str());
		debug("VWFI: preload: %d (0x%x)", preload, preload);

		for (uint i = 5; i < fileInfo.strings.size(); i++) {
			debug("VWFI: entry %d (%d bytes)", i, fileInfo.strings[i].len);
			Common::hexdump(fileInfo.strings[i].data, fileInfo.strings[i].len);
		}
	}
}

void Movie::clearSharedCast() {
	if (!_sharedCast)
		return;

	delete _sharedCast;

	_sharedCast = nullptr;
}

void Movie::loadSharedCastsFrom(Common::String filename) {
	clearSharedCast();

	Archive *sharedCast = _vm->createArchive();

	if (!sharedCast->openFile(filename)) {
		warning("loadSharedCastsFrom(): No shared cast %s", filename.c_str());

		delete sharedCast;

		return;
	}
	sharedCast->setPathName(filename);

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Loading shared cast '%s'", filename.c_str());
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	_sharedCast = new Cast(this, true);
	_sharedCast->setArchive(sharedCast);
	_sharedCast->loadArchive();
}

CastMember *Movie::getCastMember(int castId) {
	CastMember *result = _cast->getCastMember(castId);
	if (result == nullptr && _sharedCast) {
		result = _sharedCast->getCastMember(castId);
	}
	return result;
}

CastMember *Movie::getCastMemberByName(const Common::String &name) {
	CastMember *result = _cast->getCastMemberByName(name);
	if (result == nullptr && _sharedCast) {
		result = _sharedCast->getCastMemberByName(name);
	}
	return result;
}

CastMember *Movie::getCastMemberByScriptId(int scriptId) {
	CastMember *result = _cast->getCastMemberByScriptId(scriptId);
	if (result == nullptr && _sharedCast) {
		result = _sharedCast->getCastMemberByScriptId(scriptId);
	}
	return result;
}

CastMemberInfo *Movie::getCastMemberInfo(int castId) {
	CastMemberInfo *result = _cast->getCastMemberInfo(castId);
	if (result == nullptr && _sharedCast) {
		result = _sharedCast->getCastMemberInfo(castId);
	}
	return result;
}

const Stxt *Movie::getStxt(int castId) {
	const Stxt *result = _cast->getStxt(castId);
	if (result == nullptr && _sharedCast) {
		result = _sharedCast->getStxt(castId);
	}
	return result;
}

LingoArchive *Movie::getMainLingoArch() {
	return _cast->_lingoArchive;
}

LingoArchive *Movie::getSharedLingoArch() {
	return _sharedCast ? _sharedCast->_lingoArchive : nullptr;
}

ScriptContext *Movie::getScriptContext(ScriptType type, uint16 id) {
	ScriptContext *result = _cast->_lingoArchive->getScriptContext(type, id);
	if (result == nullptr && _sharedCast) {
		result = _sharedCast->_lingoArchive->getScriptContext(type, id);
	}
	return result;
}

Symbol Movie::getHandler(const Common::String &name) {
	if (!g_lingo->_eventHandlerTypeIds.contains(name)) {
		if (_cast->_lingoArchive->functionHandlers.contains(name))
			return _cast->_lingoArchive->functionHandlers[name];

		if (_sharedCast && _sharedCast->_lingoArchive->functionHandlers.contains(name))
			return _sharedCast->_lingoArchive->functionHandlers[name];
	}
	return Symbol();
}

} // End of namespace Director
