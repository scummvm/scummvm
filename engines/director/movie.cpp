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

#include "common/config-manager.h"
#include "common/substream.h"

#include "engines/util.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"

namespace Director {

Movie::Movie(Window *window) {
	_window = window;
	_vm = _window->getVM();
	_lingo = _vm->getLingo();

	_flags = 0;
	_stageColor = _window->_wm->_colorWhite;

	_currentClickOnSpriteId = 0;
	_currentEditableTextChannel = 0;
	_lastEventTime = _vm->getMacTicks();
	_lastKeyTime = _lastEventTime;
	_lastClickTime = _lastEventTime;
	_lastClickTime2 = 0;
	_lastRollTime = _lastEventTime;
	_lastTimerReset = _lastEventTime;
	_nextEventId = 0;

	_videoPlayback = false;

	_key = 0;
	_keyCode = 0;
	_keyFlags = 0;

	_currentDraggedChannel = nullptr;
	_currentHiliteChannelId = 0;
	_mouseDownWasInButton = false;

	_version = 0;
	_platform = Common::kPlatformMacintosh;
	_allowOutdatedLingo = false;

	_movieArchive = nullptr;

	_cast = new Cast(this, 0);
	_sharedCast = nullptr;
	_score = new Score(this);

	_selEnd = -1;
	_selStart = -1;

	_checkBoxType = 0;
	_checkBoxAccess = 0;

	_lastTimeOut = _lastEventTime;
	_timeOutLength = 10800;	// D4 dictionary p297, default value is 3minutes
	// default value of keydown and mouse is true, for timeOutPlay is false. check D4 dictionary p297
	_timeOutKeyDown = true;
	_timeOutMouse = true;
	_timeOutPlay = false;

	_isBeepOn = false; // Beep is off by default in the original
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
		Common::SeekableReadStreamEndian *r;
		_score->loadLabels(*(r = archive->getFirstResource(MKTAG('V', 'W', 'L', 'B'))));
		delete r;
	}
}

bool Movie::loadArchive() {
	Common::SeekableReadStreamEndian *r = nullptr;

	// Config
	if (!_cast->loadConfig())
		return false;

	_version = _cast->_version;
	_platform = _cast->_platform;
	_movieRect = _cast->_movieRect;
	// Wait to handle _stageColor until palette is loaded in loadCast...

	// File Info
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'I'), -1)) {
		loadFileInfo(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'I'))));
		delete r;
	}

	// Cast
	_cast->loadCast();
	_stageColor = _vm->transformColor(_cast->_stageColor);

	bool recenter = false;
	// If the stage dimensions are different, delete it and start again.
	// Otherwise, do not clear it so there can be a nice transition.
	if (_window->getSurface()->w != _movieRect.width() || _window->getSurface()->h != _movieRect.height()) {
		_window->resize(_movieRect.width(), _movieRect.height(), true);
		recenter = true;
	}

	// TODO: Add more options for desktop dimensions
	if (_window == _vm->getStage()) {
		uint16 windowWidth = g_director->desktopEnabled() ? 1024 : _movieRect.width();
		uint16 windowHeight = g_director->desktopEnabled() ? 768 : _movieRect.height();
		if (_vm->_wm->_screenDims.width() != windowWidth || _vm->_wm->_screenDims.height() != windowHeight) {
			_vm->_wm->resizeScreen(windowWidth, windowHeight);
			recenter = true;

			initGraphics(windowWidth, windowHeight, &_vm->_pixelformat);
		}
	}

	if (recenter && g_director->desktopEnabled())
		_window->center(g_director->_centerStage);

	_window->setStageColor(_stageColor, true);

	// Score
	if (!_movieArchive->hasResource(MKTAG('V', 'W', 'S', 'C'), -1)) {
		warning("Movie::loadArchive(): Wrong movie format. VWSC resource missing");
		return false;
	}
	_score->loadFrames(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'S', 'C'))), _version);
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
	rect.top = stream.readSint16();
	rect.left = stream.readSint16();
	rect.bottom = stream.readSint16();
	rect.right = stream.readSint16();

	return rect;
}

InfoEntries Movie::loadInfoEntries(Common::SeekableReadStreamEndian &stream, uint16 version) {
	uint32 offset = stream.pos();
	offset += stream.readUint32();

	InfoEntries res;
	res.unk1 = stream.readUint32();
	res.unk2 = stream.readUint32();
	res.flags = stream.readUint32();

	if (version >= kFileVer400)
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

void Movie::loadFileInfo(Common::SeekableReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading FileInfo VWFI");

	InfoEntries fileInfo = Movie::loadInfoEntries(stream, _version);

	_allowOutdatedLingo = (fileInfo.flags & kMovieFlagAllowOutdatedLingo) != 0;

	_script = fileInfo.strings[0].readString(false);

	if (!_script.empty() && ConfMan.getBool("dump_scripts"))
		_cast->dumpScript(_script.c_str(), kMovieScript, 0);

	if (!_script.empty())
		_cast->_lingoArchive->addCode(_script, kMovieScript, 0);

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

	_sharedCast = new Cast(this, 0, true);
	_sharedCast->setArchive(sharedCast);
	_sharedCast->loadArchive();
}

CastMember *Movie::getCastMember(CastMemberID memberID) {
	CastMember *result = nullptr;
	if (memberID.castLib == 0) {
		result = _cast->getCastMember(memberID.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getCastMember(memberID.member);
		}
	} else {
		warning("Movie::getCastMember: Unknown castLib %d", memberID.castLib);
	}
	return result;
}

CastMember* Movie::createOrReplaceCastMember(CastMemberID memberID, CastMember* cast) {
	CastMember *result = nullptr;

	if (memberID.castLib == 0) {
		result = _cast->setCastMember(memberID, cast);
	} else if (memberID.castLib == 1) {
		result = _sharedCast->setCastMember(memberID, cast);
	}

	return result;
}

CastMember *Movie::getCastMemberByName(const Common::String &name, int castLib) {
	CastMember *result = nullptr;
	if (castLib == 0) {
		result = _cast->getCastMemberByName(name);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getCastMemberByName(name);
		}
	} else {
		warning("Movie::getCastMemberByName: Unknown castLib %d", castLib);
	}
	return result;
}

CastMemberInfo *Movie::getCastMemberInfo(CastMemberID memberID) {
	CastMemberInfo *result = nullptr;
	if (memberID.castLib == 0) {
		result = _cast->getCastMemberInfo(memberID.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getCastMemberInfo(memberID.member);
		}
	} else {
		warning("Movie::getCastMemberInfo: Unknown castLib %d", memberID.castLib);
	}
	return result;
}

const Stxt *Movie::getStxt(CastMemberID memberID) {
	const Stxt *result = nullptr;
	if (memberID.castLib == 0) {
		result = _cast->getStxt(memberID.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getStxt(memberID.member);
		}
	} else {
		warning("Movie::getStxt: Unknown castLib %d", memberID.castLib);
	}
	return result;
}

LingoArchive *Movie::getMainLingoArch() {
	return _cast->_lingoArchive;
}

LingoArchive *Movie::getSharedLingoArch() {
	return _sharedCast ? _sharedCast->_lingoArchive : nullptr;
}

ScriptContext *Movie::getScriptContext(ScriptType type, CastMemberID id) {
	ScriptContext *result = nullptr;
	if (id.castLib == 0) {
		result = _cast->_lingoArchive->getScriptContext(type, id.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->_lingoArchive->getScriptContext(type, id.member);
		}
	} else {
		warning("Movie::getScriptContext: Unknown castLib %d", id.castLib);
	}
	return result;
}

Symbol Movie::getHandler(const Common::String &name) {
	if (_cast->_lingoArchive->functionHandlers.contains(name))
		return _cast->_lingoArchive->functionHandlers[name];

	if (_sharedCast && _sharedCast->_lingoArchive->functionHandlers.contains(name))
		return _sharedCast->_lingoArchive->functionHandlers[name];

	return Symbol();
}

Common::String InfoEntry::readString(bool pascal) {
	Common::String res;

	if (len == 0)
		return res;

	uint start = pascal ? 1 : 0; // skip length for Pascal string

	Common::String encodedStr;
	for (uint i = start; i < len; i++) {
		if (!Common::isCntrl(data[i]) || Common::isSpace(data[i]))
			encodedStr += data[i];
	}

	// FIXME: Use the case which contains this string, not the main cast.
	return g_director->getCurrentMovie()->getCast()->decodeString(encodedStr).encode(Common::kUtf8);
}

} // End of namespace Director
