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

#ifndef DIRECTOR_MOVIE_H
#define DIRECTOR_MOVIE_H

#define DEFAULT_CAST_LIB 1
#define SHARED_CAST_LIB -1337

namespace Common {
struct Event;
class ReadStreamEndian;
class SeekableReadStreamEndian;
}

namespace Director {

class Archive;
class Cast;
struct CastMemberInfo;
class CastMember;
class DirectorEngine;
class Lingo;
struct LingoArchive;
struct LingoEvent;
class ScriptContext;
class Window;
struct Symbol;

struct InfoEntry {
	uint32 len;
	byte *data;

	InfoEntry() { len = 0; data = nullptr; }

	InfoEntry(const InfoEntry &old) {
		len = old.len;
		data = (byte *)malloc(len);
		memcpy(data, old.data, len);
	}

	~InfoEntry() {
		free(data);
		data = nullptr;
	}

	InfoEntry &operator=(const InfoEntry &old) {
		free(data);
		len = old.len;
		data = (byte *)malloc(len);
		memcpy(data, old.data, len);
		return *this;
	}

	Common::String readString(bool pascal = true);
};

struct InfoEntries {
	uint32 unk1;
	uint32 unk2;
	uint32 flags;
	uint32 scriptId;
	Common::Array<InfoEntry> strings;

	InfoEntries() : unk1(0), unk2(0), flags(0), scriptId(0) {}
};

class Movie {
public:
	Movie(Window *window);
	~Movie();

	static Common::Rect readRect(Common::ReadStreamEndian &stream);
	static InfoEntries loadInfoEntries(Common::SeekableReadStreamEndian &stream, uint16 version);

	void loadCastLibMapping(Common::SeekableReadStreamEndian &stream);
	bool loadArchive();
	void setArchive(Archive *archive);
	Archive *getArchive() const { return _movieArchive; };
	Common::String getMacName() const { return _macName; }
	Window *getWindow() const { return _window; }
	DirectorEngine *getVM() const { return _vm; }
	Cast *getCast() const { return _casts.getValOrDefault(DEFAULT_CAST_LIB, nullptr); }
	Cast *getCast(CastMemberID memberID);
	Cast *getSharedCast() const { return _sharedCast; }
	const Common::HashMap<int, Cast *> *getCasts() const { return &_casts; }
	Score *getScore() const { return _score; }

	void clearSharedCast();
	void loadSharedCastsFrom(Common::Path &filename);
	Archive *loadExternalCastFrom(Common::Path &filename);

	CastMember *getCastMember(CastMemberID memberID);
	CastMember *createOrReplaceCastMember(CastMemberID memberID, CastMember *cast);
	bool eraseCastMember(CastMemberID memberID);
	bool duplicateCastMember(CastMemberID source, CastMemberID target);
	CastMemberID getCastMemberIDByMember(int memberID);
	int getCastLibIDByName(const Common::String &name);
	CastMemberID getCastMemberIDByName(const Common::String &name);
	CastMemberID getCastMemberIDByNameAndType(const Common::String &name, int castLib, CastType type);
	CastMemberInfo *getCastMemberInfo(CastMemberID memberID);
	bool isValidCastMember(CastMemberID memberID, CastType type);
	const Stxt *getStxt(CastMemberID memberID);

	LingoArchive *getMainLingoArch();
	LingoArchive *getSharedLingoArch();
	ScriptContext *getScriptContext(ScriptType type, CastMemberID id);
	Symbol getHandler(const Common::String &name);

	// events.cpp
	bool processEvent(Common::Event &event);

	// lingo/lingo-events.cpp
	void setPrimaryEventHandler(LEvent event, const Common::String &code);
	void resolveScriptEvent(LingoEvent &event);
	void processEvent(LEvent event, int targetId = 0);
	void queueInputEvent(LEvent event, int targetId = 0, Common::Point pos = Common::Point(-1, -1));

private:
	void loadFileInfo(Common::SeekableReadStreamEndian &stream);

	void queueEvent(Common::Queue<LingoEvent> &queue, LEvent event, int targetId = 0, Common::Point pos = Common::Point(-1, -1));
	void queueSpriteEvent(Common::Queue<LingoEvent> &queue, LEvent event, int eventId, int spriteId);

public:
	Archive *_movieArchive;
	uint16 _version;
	Common::Platform _platform;
	Common::Rect _movieRect;
	uint16 _currentActiveSpriteId;
	uint16 _currentMouseSpriteId;
	CastMemberID _currentMouseDownCastID;
	uint16 _currentEditableTextChannel;
	uint32 _lastEventTime;
	uint32 _lastRollTime;
	uint32 _lastClickTime;
	uint32 _lastClickTime2;
	Common::Point _lastClickPos;
	uint32 _lastKeyTime;
	uint32 _lastTimerReset;
	uint32 _stageColor;
	Cast *_sharedCast;
	bool _allowOutdatedLingo;
	bool _remapPalettesWhenNeeded;
	Common::String _createdBy;
	Common::String _changedBy;
	Common::String _origDirectory;
	CastMemberID _defaultPalette;

	bool _videoPlayback;

	int _nextEventId;
	Common::Queue<LingoEvent> _inputEventQueue;

	unsigned char _key;
	int _keyCode;
	byte _keyFlags;

	int _selStart;
	int _selEnd;

	int _checkBoxType;
	int _checkBoxAccess;

	uint16 _currentHiliteChannelId;

	uint _lastTimeOut;
	uint _timeOutLength;
	bool _timeOutKeyDown;
	bool _timeOutMouse;
	bool _timeOutPlay;

	bool _isBeepOn;

	Common::String _script;

private:
	Window *_window;
	DirectorEngine *_vm;
	Lingo *_lingo;
	Cast *_cast;
	Common::HashMap<int, Cast *> _casts;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _castNames;
	Score *_score;

	uint32 _flags;

	Common::String _macName;

	bool _mouseDownWasInButton;
	Channel *_currentDraggedChannel;
	Common::Point _draggingSpriteOffset;
};

} // End of namespace Director

#endif
