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

#ifndef SAGA_SAGA_H
#define SAGA_SAGA_H

#include "engines/engine.h"

#include "common/array.h"
#include "common/random.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "saga/gfx.h"
#include "saga/detection.h"

struct ADGameFileDescription;

/**
 * This is the namespace of the SAGA engine.
 *
 * Status of this engine:
 *
 * This engine contains 2 main engine generations, SAGA and SAGA2
 *
 * SAGA status: complete
 *
 * SAGA2 status: in early stages of development, no recent activity. Contact sev
 *  if you want to work on it, since we have some original source codes.
 *
 * Games using this engine:
 *
 * SAGA:
 * - Inherit the Earth
 * - I Have No Mouth And I Must Scream
 *
 * SAGA2:
 * - Dinotopia
 * - Faery Tale Adventure II: Halls of the Dead
 *
 */
namespace Saga {

class SndRes;
class Sound;
class Music;
class Anim;
class Render;
class IsoMap;
class Gfx;
class Script;
class Actor;
class Font;
class Sprite;
class Scene;
class Interface;
class Console;
class Events;
class PalAnim;
class Puzzle;
class Resource;

class ResourceContext;

// #define SAGA_DEBUG 1		// define for test functions
#define SAGA_IMAGE_DATA_OFFSET 776
#define SAGA_IMAGE_HEADER_LEN  8

// Note that IHNM has a smaller save title size than ITE
// We allocate the ITE save title size in savegames, to
// preserve savegame backwards compatibility. We only check
// for IHNM's save title during text input
#define SAVE_TITLE_SIZE 28
#define TITLESIZE 80
#define IHNM_SAVE_TITLE_SIZE 22
#define MAX_SAVES 96
#define MAX_FILE_NAME 256

#define ID_NOTHING 0
#define ID_PROTAG 1
#define OBJECT_TYPE_SHIFT 13
#define OBJECT_TYPE_MASK ((1 << OBJECT_TYPE_SHIFT) - 1)

#define IHNM_OBJ_PROFILE 0x4000

#define memoryError(Place) error("%s Memory allocation error.", Place)

enum ERRORCODE {
	FAILURE = -1,
	SUCCESS = 0
};

enum VerbTypeIds {
	kVerbITENone = 0,
	kVerbITEPickUp = 1,
	kVerbITELookAt = 2,
	kVerbITEWalkTo = 3,
	kVerbITETalkTo = 4,
	kVerbITEOpen = 5,
	kVerbITEClose = 6,
	kVerbITEGive = 7,
	kVerbITEUse = 8,
	kVerbITEOptions = 9,
	kVerbITEEnter = 10,
	kVerbITELeave = 11,
	kVerbITEBegin = 12,
	kVerbITEWalkOnly = 13,
	kVerbITELookOnly = 14,


	kVerbIHNMNone = 0,
	kVerbIHNMWalk = 1,
	kVerbIHNMLookAt = 2,
	kVerbIHNMTake = 3,
	kVerbIHNMUse = 4,
	kVerbIHNMTalkTo = 5,
	kVerbIHNMSwallow = 6,
	kVerbIHNMGive = 7,
	kVerbIHNMPush = 8,
	kVerbIHNMOptions = 9,
	kVerbIHNMEnter = 10,
	kVerbIHNMLeave = 11,
	kVerbIHNMBegin = 12,
	kVerbIHNMWalkOnly = 13,
	kVerbIHNMLookOnly = 14,

	kVerbTypeIdsMax = kVerbITELookOnly + 1
};

enum PanelButtonType {
	kPanelButtonVerb = 1 << 0,
	kPanelButtonArrow = 1 << 1,
	kPanelButtonConverseText = 1 << 2,
	kPanelButtonInventory = 1 << 3,

	kPanelButtonOption = 1 << 4,
	kPanelButtonOptionSlider = 1 << 5,
	kPanelButtonOptionSaveFiles = 1 << 6,
	kPanelButtonOptionText = 1 << 7,

	kPanelButtonQuit = 1 << 8,
	kPanelButtonQuitText = 1 << 9,

	kPanelButtonLoad = 1 << 10,
	kPanelButtonLoadText = 1 << 11,

	kPanelButtonSave = 1 << 12,
	kPanelButtonSaveText = 1 << 13,
	kPanelButtonSaveEdit = 1 << 14,

	kPanelButtonProtectText = 1 << 15,
	kPanelButtonProtectEdit = 1 << 16,

	kPanelAllButtons = 0xFFFFF
};

enum TextStringIds {
	kTextPickUp,
	kTextLookAt,
	kTextWalkTo,
	kTextTalkTo,
	kTextOpen,
	kTextClose,
	kTextGive,
	kTextUse,

	kTextOptions,
	kTextTest,
	kTextDemo,
	kTextHelp,
	kTextQuitGame,
	kTextFast,
	kTextSlow,
	kTextOn,
	kTextOff,
	kTextContinuePlaying,
	kTextLoad,
	kTextSave,
	kTextGameOptions,
	kTextReadingSpeed,
	kTextMusic,
	kTextSound,
	kTextCancel,
	kTextQuit,
	kTextOK,
	kTextMid,
	kTextClick,
	kText10Percent,
	kText20Percent,
	kText30Percent,
	kText40Percent,
	kText50Percent,
	kText60Percent,
	kText70Percent,
	kText80Percent,
	kText90Percent,
	kTextMax,
	kTextQuitTheGameQuestion,
	kTextLoadSuccessful,
	kTextEnterSaveGameName,
	kTextGiveTo,
	kTextUseWidth,
	kTextNewSave,
	kTextICantPickup,
	kTextNothingSpecial,
	kTextNoPlaceToOpen,
	kTextNoOpening,
	kTextDontKnow,
	kTextShowDialog,
	kTextEnterProtectAnswer,
	kTextVoices,
	kTextText,
	kTextAudio,
	kTextBoth,
	kTextLoadSavedGame
};

struct GameDisplayInfo;

enum GameObjectTypes {
	kGameObjectNone = 0,
	kGameObjectActor = 1,
	kGameObjectObject = 2,
	kGameObjectHitZone = 3,
	kGameObjectStepZone = 4
};

enum ScriptTimings {
	kScriptTimeTicksPerSecond = (728L/10L),
	kRepeatSpeedTicks = (728L/10L)/3,
	kNormalFadeDuration = 320, // 64 steps, 5 msec each
	kQuickFadeDuration = 64,  // 64 steps, 1 msec each
	kPuzzleHintTime = 30000000L  // 30 secs. used in timer
};

enum Directions {
	kDirUp = 0,
	kDirUpRight = 1,
	kDirRight = 2,
	kDirDownRight = 3,
	kDirDown = 4,
	kDirDownLeft = 5,
	kDirLeft = 6,
	kDirUpLeft = 7
};

enum HitZoneFlags {
	kHitZoneEnabled = (1 << 0),   // Zone is enabled
	kHitZoneExit = (1 << 1),      // Causes char to exit

	//	The following flag causes the zone to act differently.
	//	When the actor hits the zone, it will immediately begin walking
	//	in the specified direction, and the actual specified effect of
	//	the zone will be delayed until the actor leaves the zone.
	kHitZoneAutoWalk = (1 << 2),

	//      When set on a hit zone, this causes the character not to walk
	//      to the object (but they will look at it).
	kHitZoneNoWalk = (1 << 2),

	//	zone activates only when character stops walking
	kHitZoneTerminus = (1 << 3),

	//      Hit zones only - when the zone is clicked on it projects the
	//      click point downwards from the middle of the zone until it
	//      reaches the lowest point in the zone.
	kHitZoneProject = (1 << 3)
};

struct ImageHeader {
	int width;
	int height;
};

struct StringsTable {
	Common::Array<char> buffer;
	Common::Array<char *> strings;

	const char *getString(uint index) const {
		if (strings.size() <= index) {
			// This occurs at the end of Ted's chapter, right after the ending cutscene
			warning("StringsTable::getString wrong index 0x%X (%d)", index, strings.size());
			return "";
		}
		return strings[index];
	}

	void clear() {
		strings.clear();
		buffer.clear();
	}
};

typedef Common::Array<Point> PointList;

enum ColorId {
	kITEColorTransBlack = 0x00,
	kITEColorBrightWhite = 0x01,
	kITEColorWhite = 0x02,
	kITEColorLightGrey = 0x04,
	kITEColorGrey = 0x0a,
	kITEColorDarkGrey = 0x0b,
	kITEColorDarkGrey0C = 0x0C,
	kITEColorBlack = 0x0f,
	kITEColorRed = 0x65,
	kITEColorDarkBlue8a = 0x8a,
	kITEColorBlue89 = 0x89,
	kITEColorLightBlue92 = 0x92,
	kITEColorBlue = 0x93,
	kITEColorLightBlue94 = 0x94,
	kITEColorLightBlue96 = 0x96,
	kITEColorGreen = 0xba
};

enum KnownColor {
	kKnownColorTransparent,
	kKnownColorBrightWhite,
	kKnownColorWhite,
	kKnownColorBlack,

	kKnownColorSubtitleTextColor,
	kKnownColorSubtitleEffectColorPC98,
	kKnownColorVerbText,
	kKnownColorVerbTextShadow,
	kKnownColorVerbTextActive
};

struct SaveFileData {
	char name[SAVE_TITLE_SIZE];
	uint slotNumber;
};

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 version;
	char name[SAVE_TITLE_SIZE];
};

inline int objectTypeId(uint16 objectId) {
	return objectId >> OBJECT_TYPE_SHIFT;
}

inline int objectIdToIndex(uint16 objectId) {
	return OBJECT_TYPE_MASK & objectId;
}

inline uint16 objectIndexToId(int type, int index) {
	return (type << OBJECT_TYPE_SHIFT) | (OBJECT_TYPE_MASK & index);
}

class ByteArray : public Common::Array<byte> {
public:
	/**
	 * Return a pointer to the start of the buffer underlying this byte array,
	 * or NULL if the buffer is empty.
	 */
	byte *getBuffer() {
		return empty() ? NULL : &front();
	}

	const byte *getBuffer() const {
		return empty() ? NULL : &front();
	}

	void assign(const ByteArray &src) {
		resize(src.size());
		if (!empty()) {
			memcpy(&front(), &src.front(), size());
		}
	}
};

class ByteArrayReadStreamEndian : public Common::MemoryReadStreamEndian {
public:
	ByteArrayReadStreamEndian(const ByteArray & byteArray, bool bigEndian = false)
		: Common::MemoryReadStreamEndian(byteArray.getBuffer(), byteArray.size(), bigEndian),
		ReadStreamEndian(bigEndian) {
	}
};

class SagaEngine : public Engine {
	friend class Scene;

public:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	void syncSoundSettings() override;
	void pauseEngineIntern(bool pause) override;

	SagaEngine(OSystem *syst, const SAGAGameDescription *gameDesc);
	~SagaEngine() override;

	void save(const char *fileName, const char *saveName);
	void load(const char *fileName);
	uint32 getCurrentLoadVersion() const {
		return _saveHeader.version;
	}
	void fillSaveList();
	char *calcSaveFileName(uint slotNumber);
	virtual Common::String getSaveStateName(int slot) const override {
		return Common::String::format("%s.s%02u", _targetName.c_str(), slot);
	}

	SaveFileData *getSaveFile(uint idx);
	uint getNewSaveSlotNumber() const;
	bool locateSaveFile(char *saveName, uint &titleNumber);
	bool isSaveListFull() const {
		return _saveFilesCount == MAX_SAVES;
	}
	uint getSaveFilesCount() const {
		return isSaveListFull() ? _saveFilesCount : _saveFilesCount + 1;
	}

	bool isIHNMDemo() const { return _isIHNMDemo; }

	int16 _framesEsc;

	uint32 _globalFlags;
	int16 _ethicsPoints[8];
	int _spiritualBarometer;

	int _soundVolume;
	int _musicVolume;
	int _speechVolume;
	bool _subtitlesEnabled;
	bool _voicesEnabled;
	bool _voiceFilesExist;
	int _readingSpeed;

	bool _copyProtection;
	bool _musicWasPlaying;
	bool _isIHNMDemo;
	bool _hasITESceneSubstitutes;

	SndRes *_sndRes;
	Sound *_sound;
	Music *_music;
	Anim *_anim;
	Render *_render;
	IsoMap *_isoMap;
	Gfx *_gfx;
	Script *_script;
	Actor *_actor;
	Font *_font;
	Sprite *_sprite;
	Scene *_scene;
	Interface *_interface;
	Console *_console;
	Events *_events;
	PalAnim *_palanim;
	Puzzle *_puzzle;
	Resource *_resource;


	// Random number generator
	Common::RandomSource _rnd;

private:
	bool decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, ByteArray &outbuf);
	void flipImage(byte *imageBuffer, int columns, int scanlines);
	void unbankBGImage(byte *dest_buf, const byte *src_buf, int columns, int scanlines);
	uint32 _previousTicks;

public:
	bool decodeBGImage(const ByteArray &imageData, ByteArray &outputBuffer, int *w, int *h, bool flip = false);
	const byte *getImagePal(const ByteArray &imageData) {
		if (imageData.size() <= SAGA_IMAGE_HEADER_LEN) {
			return NULL;
		}

		return &imageData.front() + SAGA_IMAGE_HEADER_LEN;
	}
	void loadStrings(StringsTable &stringsTable, const ByteArray &stringsData);

	const char *getObjectName(uint16 objectId) const;
public:
	int processInput();
	Point mousePos() const;

	int getMouseClickCount() const {
		return _mouseClickCount;
	}

	void incrementMouseClickCount() {
		_mouseClickCount++;
	}

	void resetMouseClickCount() {
		_mouseClickCount = 0;
	}

	bool leftMouseButtonPressed() const {
		return _leftMouseButtonPressed;
	}

	bool rightMouseButtonPressed() const {
		return _rightMouseButtonPressed;
	}

	bool mouseButtonPressed() const {
		return _leftMouseButtonPressed || _rightMouseButtonPressed;
	}

	inline int ticksToMSec(int tick) const {
		return tick * 1000 / kScriptTimeTicksPerSecond;
	}

 private:
	uint _saveFilesCount;
	SaveFileData _saveFiles[MAX_SAVES];
	SaveGameHeader _saveHeader;

	bool _leftMouseButtonPressed;
	bool _rightMouseButtonPressed;
	int _mouseClickCount;

//current game description
	int _gameNumber;
	const SAGAGameDescription *_gameDescription;
	Common::String _gameTitle;
	Common::Rect _displayClip;

public:
	int32 _frameCount;

public:
	bool initGame();

	bool isBigEndian() const;
	bool isMacResources() const;
	bool isSaga2() const { return getGameId() == GID_DINO || getGameId() == GID_FTA2; }
	const GameResourceDescription *getResourceDescription() const;

	const GameFontDescription *getFontDescription(int index) const;
	int getFontsCount() const;

	int getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	int getGameNumber() const;
	int getStartSceneNumber() const;

	const GamePatchDescription *getPatchDescriptions() const;

	const ADGameFileDescription *getFilesDescriptions() const;

	const Common::Rect &getDisplayClip() const { return _displayClip;}
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	const GameDisplayInfo &getDisplayInfo();

	const char *getTextString(int textStringId);
	void getExcuseInfo(int verb, const char *&textString, int &soundResourceId);

private:

public:
	ColorId KnownColor2ColorId(KnownColor knownColor);
	void setTalkspeed(int talkspeed);
	int getTalkspeed() const;
};

} // End of namespace Saga

#endif
