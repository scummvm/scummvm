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
#include "common/memstream.h"
#include "common/substream.h"

#include "director/types.h"
#include "engines/util.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#include "director/castmember/castmember.h"

namespace Director {

#include "director/blank-score.h"

Movie::Movie(Window *window) {
	_window = window;
	_vm = _window->getVM();
	_lingo = _vm->getLingo();

	_flags = 0;
	_stageColor = _window->_wm->_colorWhite;

	_currentActiveSpriteId = 0;
	_currentMouseSpriteId = 0;
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
	_remapPalettesWhenNeeded = false;

	_movieArchive = nullptr;

	_cast = new Cast(this, DEFAULT_CAST_LIB);
	_casts.setVal(_cast->_castLibID, _cast);
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
	if (_sharedCast && _sharedCast->getArchive()) {
		debug(0, "@@   Clearing shared cast '%s'", _sharedCast->getArchive()->getPathName().toString().c_str());

		g_director->_allOpenResFiles.remove(_sharedCast->getArchive()->getPathName());
	}

	if (_cast && _cast->getArchive()) {
		debug(0, "@@   Clearing movie cast '%s'", _cast->getArchive()->getPathName().toString().c_str());
		g_director->_allOpenResFiles.remove(_cast->getArchive()->getPathName());
	}

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

	Common::SeekableReadStreamEndian *r = nullptr;
	if ((r = archive->getMovieResourceIfPresent(MKTAG('M', 'C', 's', 'L'))) != nullptr) {
		// D5 archive, can contain multiple internal/external casts
		loadCastLibMapping(*r);
		delete r;
	} else {
		// D4 or lower, only 1 cast
		_cast->setArchive(archive);
	}
	// Frame Labels
	if ((r = archive->getMovieResourceIfPresent(MKTAG('V', 'W', 'L', 'B')))) {
		_score->loadLabels(*r);
		delete r;
	}
}

void Movie::loadCastLibMapping(Common::SeekableReadStreamEndian &stream) {
	debugC(5, kDebugLoading, "Movie::loadCastLibMapping: loading cast libraries");
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(stream.size());
	}
	stream.readUint32(); // header size
	uint32 count = stream.readUint32();
	stream.readUint16();
	uint32 unkCount = stream.readUint32() + 1;
	for (uint32 i = 0; i < unkCount; i++) {
		stream.readUint32();
	}
	for (uint32 i = 0; i < count; i++) {
		int nameSize = stream.readByte();
		Common::String name = stream.readString('\0', nameSize);
		stream.readByte(); // null
		int pathSize = stream.readByte();
		Common::String path = stream.readString('\0', pathSize);
		stream.readByte(); // null
		if (pathSize > 1)
			stream.readUint16();
		stream.readUint16();
		uint16 itemCount = stream.readUint16();
		stream.readUint16();
		uint16 libResourceId = stream.readUint16();
		uint16 libId = i + 1;
		debugC(5, kDebugLoading, "Movie::loadCastLibMapping: name: %s, path: %s, itemCount: %d, libResourceId: %d, libId: %d", utf8ToPrintable(name).c_str(), utf8ToPrintable(path).c_str(), itemCount, libResourceId, libId);
		Archive *castArchive = _movieArchive;
		bool isExternal = !path.empty();
		if (isExternal) {
			Common::Path archivePath = findMoviePath(path);
			castArchive = loadExternalCastFrom(archivePath);
			if (!castArchive) {
				continue;	// couldn't load external cast
			}
		}

		Cast *cast = nullptr;
		if (_casts.contains(libId)) {
			cast = _casts.getVal(libId);
		} else {
			cast = new Cast(this, libId, false, isExternal, libResourceId);
			_casts.setVal(libId, cast);
		}
		_castNames[name] = libId;
		cast->setArchive(castArchive);
	}
	return;
}

bool Movie::loadArchive() {
	Common::SeekableReadStreamEndian *r = nullptr;

	// Config
	if (!_cast->loadConfig())
		return false;

	_version = _cast->_version;
	_platform = _cast->_platform;
	_movieRect = _cast->_movieRect;
	_score->_currentFrameRate = _cast->_frameRate;
	_stageColor = _vm->transformColor(_cast->_stageColor);
	// Wait to handle _stageColor until palette is loaded in loadCast...

	// File Info
	if ((r = _movieArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'F', 'I'))) != nullptr) {
		loadFileInfo(*r);
		delete r;
	}

	// Cast
	for (auto &it : _casts) {
		if (it._value != _cast)
			it._value->loadConfig();
		it._value->loadCast();
	}
	_stageColor = _vm->transformColor(_cast->_stageColor);
	// Need to check if the default palette is valid; if not, assume it's the Mac one.
	if (g_director->hasPalette(_cast->_defaultPalette)) {
		_defaultPalette = _cast->_defaultPalette;
	} else {
		_defaultPalette = CastMemberID(kClutSystemMac, -1);
	}
	g_director->_lastPalette = CastMemberID();

	bool recenter = false;
	// If the stage dimensions are different, delete it and start again.
	// Otherwise, do not clear it so there can be a nice transition.
	if (_window->getSurface()->w != _movieRect.width() || _window->getSurface()->h != _movieRect.height()) {
		_window->resizeInner(_movieRect.width(), _movieRect.height());
		recenter = true;
	}

	// TODO: Add more options for desktop dimensions
	if (_window == _vm->getStage()) {
		uint16 windowWidth = g_director->desktopEnabled() ? g_director->_wmWidth : _movieRect.width();
		uint16 windowHeight = g_director->desktopEnabled() ? g_director->_wmHeight : _movieRect.height();
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
	if (!(r = _movieArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'S', 'C')))) {
		warning("Movie::loadArchive(): No VWSC resource, injecting a blank score with 1 frame");
		if (_version < kFileVer400) {
			r = new Common::MemoryReadStreamEndian(kBlankScoreD2, sizeof(kBlankScoreD2), true);
		} else if (_version < kFileVer600) {
			r = new Common::MemoryReadStreamEndian(kBlankScoreD4, sizeof(kBlankScoreD4), true);
		} else {
			error("Movie::loadArchive(): score format not yet supported for version %d", _version);
		}
	}

	_score->loadFrames(*r, _version);
	delete r;

	// Action list
	if ((r = _movieArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'A', 'C'))) != nullptr) {
		_score->loadActions(*r);
		delete r;
	}

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
	_remapPalettesWhenNeeded = (fileInfo.flags & kMovieFlagRemapPalettesWhenNeeded) != 0;

	_script = fileInfo.strings[0].readString(false);

	if (!_script.empty() && ConfMan.getBool("dump_scripts"))
		_cast->dumpScript(_script.c_str(), kMovieScript, 0);

	if (!_script.empty())
		_cast->_lingoArchive->addCode(_script, kMovieScript, 0, nullptr, kLPPTrimGarbage);

	_changedBy = fileInfo.strings[1].readString();
	_createdBy = fileInfo.strings[2].readString();
	_origDirectory = fileInfo.strings[3].readString();

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
		debug("VWFI: original directory: '%s'", _origDirectory.c_str());
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

	debug(0, "@@   Clearing shared cast '%s'", _sharedCast->getArchive()->getPathName().toString().c_str());

	g_director->_allOpenResFiles.remove(_sharedCast->getArchive()->getPathName());

	delete _sharedCast;
	_sharedCast = nullptr;
}

void Movie::loadSharedCastsFrom(Common::Path &filename) {
	clearSharedCast();

	Archive *sharedCast = _vm->openArchive(filename);

	if (!sharedCast) {
		warning("loadSharedCastsFrom(): No shared cast %s", filename.toString().c_str());

		return;
	}
	sharedCast->setPathName(filename);

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Loading shared cast '%s' in '%s'", sharedCast->getFileName().c_str(), filename.getParent().toString().c_str());
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	_sharedCast = new Cast(this, DEFAULT_CAST_LIB, true, false);
	_sharedCast->setArchive(sharedCast);
	_sharedCast->loadArchive();
}

Archive *Movie::loadExternalCastFrom(Common::Path &filename) {
	Archive *externalCast = nullptr;
	externalCast = _vm->openArchive(filename);

	if (!externalCast) {
		warning("Movie::loadExternalCastFrom(): Cast file %s not found", filename.toString().c_str());

		return nullptr;
	}

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Loading external cast '%s'", externalCast->getFileName().c_str());
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	return externalCast;
}

CastMember *Movie::getCastMember(CastMemberID memberID) {
	CastMember *result = nullptr;
	if (_casts.contains(memberID.castLib)) {
		result = _casts.getVal(memberID.castLib)->getCastMember(memberID.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getCastMember(memberID.member);
		}
	} else if (memberID.castLib != 0) {
		warning("Movie::getCastMember: Unknown castLib %d", memberID.castLib);
	}
	return result;
}

Cast *Movie::getCast(CastMemberID memberID) {
	if (memberID.castLib == SHARED_CAST_LIB)
		return _sharedCast;

	if (_casts.contains(memberID.castLib)) {
		return _casts.getVal(memberID.castLib);
	} else if (memberID.castLib != 0) {
		warning("Movie::getCast: Unknown castLib %d", memberID.castLib);
		return nullptr;
	}
	return nullptr;
}

CastMember* Movie::createOrReplaceCastMember(CastMemberID memberID, CastMember* cast) {
	warning("Movie::createOrReplaceCastMember: stubbed: functions only handles create");
	CastMember *result = nullptr;

	if (_casts.contains(memberID.castLib)) {
		// Delete existing cast member
		_casts.getVal(memberID.castLib)->eraseCastMember(memberID.member);

		_casts.getVal(memberID.castLib)->setCastMember(memberID.member, cast);
	}

	return result;
}

bool Movie::eraseCastMember(CastMemberID memberID) {
	if (_casts.contains(memberID.castLib)) {
		bool result = _casts.getVal(memberID.castLib)->eraseCastMember(memberID.member);
		_score->refreshPointersForCastMemberID(memberID);
		return result;
	}

	return false;
}

bool Movie::duplicateCastMember(CastMemberID source, CastMemberID target) {
	Cast *sourceCast = nullptr;
	Cast *targetCast = nullptr;
	if (_casts.contains(target.castLib)) {
		if (_casts[target.castLib]->getCastMember(source.member)) {
			sourceCast = _casts[target.castLib];
		} else if (_sharedCast && _sharedCast->getCastMember(source.member)) {
			sourceCast = _sharedCast;
		}
	}
	// for shared + movie casts, duplications from the shared cast should be
	// in the shared cast namespace
	if (source.castLib == target.castLib) {
		targetCast = sourceCast;
	} else if (_casts.contains(target.castLib)) {
		targetCast = _casts.getVal(target.castLib);
	}
	if (!sourceCast) {
		warning("Movie::duplicateCastMember(): couldn't find source cast member %s", source.asString().c_str());
	} else if (!targetCast) {
		warning("Movie::duplicateCastMember(): couldn't find destination castLib %d", target.castLib);
	} else {
		CastMember *sourceMember = sourceCast->getCastMember(source.member);
		CastMemberInfo *sourceInfo = sourceCast->getCastMemberInfo(source.member);
		debugC(3, kDebugLoading, "Movie::DuplicateCastMember(): copying cast data from %s to %s (%s)", source.asString().c_str(), target.asString().c_str(), castType2str(sourceMember->_type));
		bool result = targetCast->duplicateCastMember(sourceMember, sourceInfo, target.member);
		_score->refreshPointersForCastMemberID(target);
		return result;
	}
	return false;
}

CastMemberID Movie::getCastMemberIDByMember(int memberID) {
	CastMemberID result(-1, 0);
	// Search all cast libraries for a match
	for (auto &cast : _casts) {
		CastMember *member = cast._value->getCastMember(memberID);
		if (member) {
			result = CastMemberID(member->getID(), cast._key);
			break;
		}
	}
	if (result.member == -1 && _sharedCast) {
		CastMember *member = _sharedCast->getCastMember(memberID);
		if (member)
			result = CastMemberID(member->getID(), DEFAULT_CAST_LIB);
	}
	if (result.member == -1) {
		warning("Movie::getCastMemberIDByMemberID: No match found for member ID %d", memberID);
	}
	return result;
}

int Movie::getCastLibIDByName(const Common::String &name) {
	for (auto &it : _castNames) {
		if (it._key.equalsIgnoreCase(name)) {
			return it._value;
		}
	}
	return -1;
}

CastMemberID Movie::getCastMemberIDByName(const Common::String &name) {
	return getCastMemberIDByNameAndType(name, 0, kCastTypeAny);
}

CastMemberID Movie::getCastMemberIDByNameAndType(const Common::String &name, int castLib, CastType type) {
	CastMemberID result(-1, 0);
	if (_casts.contains(castLib)) {
		CastMember *member = _casts.getVal(castLib)->getCastMemberByNameAndType(name, type);
		if (member) {
			result = CastMemberID(member->getID(), castLib);
		}
		if (result.member == -1 && _sharedCast) {
			member = _sharedCast->getCastMemberByNameAndType(name, type);
			if (member) {
				result = CastMemberID(member->getID(), castLib);
			}
		}
	} else if (castLib == 0) {
		// Search all cast libraries for a match
		for (auto &cast : _casts) {
			CastMember *member = cast._value->getCastMemberByNameAndType(name, type);
			if (member) {
				result = CastMemberID(member->getID(), cast._key);
				break;
			}
		}
		if (result.member == -1 && _sharedCast) {
			CastMember *member = _sharedCast->getCastMemberByNameAndType(name, type);
			if (member)
				result = CastMemberID(member->getID(), DEFAULT_CAST_LIB);
		}
	} else {
		warning("Movie::getCastMemberIDByNameAndType: Unknown castLib %d", castLib);
	}
	if (result.member == -1) {
		warning("Movie::getCastMemberIDByNameAndType: No match found for member name %s and lib %d", name.c_str(), castLib);
	}
	return result;
}

CastMemberInfo *Movie::getCastMemberInfo(CastMemberID memberID) {
	CastMemberInfo *result = nullptr;
	if (_casts.contains(memberID.castLib)) {
		result = _casts.getVal(memberID.castLib)->getCastMemberInfo(memberID.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getCastMemberInfo(memberID.member);
		}
	} else {
		warning("Movie::getCastMemberInfo: Unknown castLib %d", memberID.castLib);
	}
	return result;
}

bool Movie::isValidCastMember(CastMemberID memberID, CastType type) {
	CastMember *test = getCastMember(memberID);
	return test && ((test->_type == type) || (type == kCastTypeAny));
}

const Stxt *Movie::getStxt(CastMemberID memberID) {
	const Stxt *result = nullptr;
	if (_casts.contains(memberID.castLib)) {
		result = _casts.getVal(memberID.castLib)->getStxt(memberID.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->getStxt(memberID.member);
		}
	} else {
		warning("Movie::getStxt: Unknown castLib %d", memberID.castLib);
	}
	return result;
}

LingoArchive *Movie::getMainLingoArch() {
	return _casts.getVal(DEFAULT_CAST_LIB)->_lingoArchive;
}

LingoArchive *Movie::getSharedLingoArch() {
	return _sharedCast ? _sharedCast->_lingoArchive : nullptr;
}

ScriptContext *Movie::getScriptContext(ScriptType type, CastMemberID id) {
	ScriptContext *result = nullptr;
	if (_casts.contains(id.castLib)) {
		result = _casts.getVal(id.castLib)->_lingoArchive->getScriptContext(type, id.member);
		if (result == nullptr && _sharedCast) {
			result = _sharedCast->_lingoArchive->getScriptContext(type, id.member);
		}
	} else if (!id.isNull()) {
		warning("Movie::getScriptContext: Unknown castLib %d", id.castLib);
	}
	return result;
}

Symbol Movie::getHandler(const Common::String &name) {
	for (auto &it : _casts) {
		if (it._value->_lingoArchive->functionHandlers.contains(name))
			return it._value->_lingoArchive->functionHandlers[name];
	}

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
