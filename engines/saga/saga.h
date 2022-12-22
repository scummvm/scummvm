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
 * Status of this engine: complete
 *
 * Games using this engine:
 * - Inherit the Earth
 * - I Have No Mouth And I Must Scream
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

struct GameResourceDescription {
	uint32 sceneLUTResourceId;
	uint32 moduleLUTResourceId;
	uint32 mainPanelResourceId;
	uint32 conversePanelResourceId;
	uint32 optionPanelResourceId;
	uint32 mainSpritesResourceId;
	uint32 mainPanelSpritesResourceId;
	uint32 mainStringsResourceId;
	// ITE specific resources
	uint32 actorsStringsResourceId;
	uint32 defaultPortraitsResourceId;
	// IHNM specific resources
	uint32 optionPanelSpritesResourceId;
	uint32 warningPanelResourceId;
	uint32 warningPanelSpritesResourceId;
	uint32 psychicProfileResourceId;
};

struct GameFontDescription {
	uint32 fontResourceId;
};

struct GameDisplayInfo;

struct GamePatchDescription {
	const char *fileName;
	uint16 fileType;
	uint32 resourceId;
};

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
	// DOS and AGA palettes
	kITEDOSColorTransBlack = 0x00,
	kITEDOSColorBrightWhite = 0x01,
	kITEDOSColorWhite = 0x02,
	kITEDOSColorLightGrey = 0x04,
	kITEDOSColorGrey = 0x0a,
	kITEDOSColorDarkGrey = 0x0b,
	kITEDOSColorDarkGrey0C = 0x0C,
	kITEDOSColorBlack = 0x0f,
	kITEDOSColorYellow60 = 0x60,
	kITEDOSColorRed = 0x65,
	kITEDOSColorDarkBlue8a = 0x8a,
	kITEDOSColorBlue89 = 0x89,
	kITEDOSColorLightBlue92 = 0x92,
	kITEDOSColorBlue = 0x93,
	kITEDOSColorLightBlue94 = 0x94,
	kITEDOSColorLightBlue96 = 0x96,
	kITEDOSColorGreen = 0xba,

        // ECS palette

	// Constant colors
	kITEECSColorTransBlack = 0x00,
	kITEECSColorBrightWhite = 0x4f,
	kITEECSColorWhite = kITEECSColorBrightWhite,
	kITEECSColorBlack = 0x50,

	// ECS palette after the palette switch
	kITEECSBottomColorGreen = 0x25,
	kITEECSBottomColorLightBlue96 = 0x28,
	kITEECSBottomColorWhite = 0x2f,
	kITEECSBottomColorBrightWhite = 0x2f,
	kITEECSBottomColorDarkGrey = 0x32,
	kITEECSBottomColorGrey = 0x36,
	kITEECSBottomColorBlue = 0x3b,
	kITEECSBottomColorYellow60 = 0x3e,

	// ECS palette for options
	kITEECSOptionsColorLightBlue94 = 0x48,
	kITEECSOptionsColorBlue = 0x48,
	kITEECSOptionsColorDarkBlue8a = 0x48,
	kITEECSOptionsColorLightBlue92 = 0x48,
	kITEECSOptionsColorLightBlue96 = 0x48,
	kITEECSOptionsColorDarkGrey0C = 0x49,
	kITEECSOptionsColorBlack = kITEECSColorBlack,
	kITEECSOptionsColorBrightWhite = kITEECSColorBrightWhite,
	kITEECSOptionsColorDarkGrey = 0x52,
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

	ByteArray() : Common::Array<byte>() {}
	ByteArray(const byte *array, size_type n) : Common::Array<byte>(array, n) {}
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
	Common::String getSaveStateName(int slot) const override {
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

	bool isITEAmiga() const { return getPlatform() == Common::kPlatformAmiga && getGameId() == GID_ITE; }
	bool isAGA() const { return _gameDescription->features & GF_AGA_GRAPHICS; }
	bool isECS() const { return _gameDescription->features & GF_ECS_GRAPHICS; }
	unsigned getPalNumEntries() const { return isECS() ? 32 : 256; }
	GameIntroList getIntroList() const { return _gameDescription->introList; }

	int16 _framesEsc;

	uint32 _globalFlags;
	int16 _ethicsPoints[8];
	int _spiritualBarometer;

	int _soundVolume;
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
  	bool decodeBGImageMask(const ByteArray &imageData, ByteArray &outputBuffer, int *w, int *h, bool flip = false);
	const byte *getImagePal(const ByteArray &imageData) {
		if (imageData.size() <= SAGA_IMAGE_HEADER_LEN) {
			return NULL;
		}

		return &imageData.front() + SAGA_IMAGE_HEADER_LEN;
	}
	void loadStrings(StringsTable &stringsTable, const ByteArray &stringsData, bool isBigEndian);

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
	const GameResourceDescription *getResourceDescription() const;

	GameResourceList getResourceList() const;
	GameFontList getFontList() const;
	GamePatchList getPatchList() const;

	int getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	int getGameNumber() const;
	int getStartSceneNumber() const;

	const ADGameFileDescription *getFilesDescriptions() const;
	const ADGameFileDescription *getArchivesDescriptions() const;

	const Common::Rect &getDisplayClip() const { return _displayClip;}
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	const GameDisplayInfo &getDisplayInfo();

	int getLanguageIndex();
	const char *getTextString(int textStringId);
	void getExcuseInfo(int verb, const char *&textString, int &soundResourceId);

private:

public:
	ColorId KnownColor2ColorId(KnownColor knownColor);
	void setTalkspeed(int talkspeed);
	int getTalkspeed() const;

#define ITE_COLOR_DISPATCHER_TYPE(NAME, TYPE)				\
	ColorId iteColor ## TYPE ## NAME() const { return isECS() ? kITEECS ## TYPE ## Color ## NAME : kITEDOSColor ## NAME; }
#define ITE_COLOR_DISPATCHER_BOTTOM(NAME) ITE_COLOR_DISPATCHER_TYPE(NAME, Bottom)
#define ITE_COLOR_DISPATCHER_OPTIONS(NAME) ITE_COLOR_DISPATCHER_TYPE(NAME, Options)
#define ITE_COLOR_DISPATCHER(NAME) ITE_COLOR_DISPATCHER_TYPE(NAME, )

	ITE_COLOR_DISPATCHER(Black)
	ITE_COLOR_DISPATCHER(TransBlack)
	ITE_COLOR_DISPATCHER(BrightWhite)
	ITE_COLOR_DISPATCHER(White)

	ITE_COLOR_DISPATCHER_BOTTOM(DarkGrey)
	ITE_COLOR_DISPATCHER_BOTTOM(Blue)
	ITE_COLOR_DISPATCHER_BOTTOM(Grey)
	ITE_COLOR_DISPATCHER_BOTTOM(White)
	ITE_COLOR_DISPATCHER_BOTTOM(BrightWhite)
	ITE_COLOR_DISPATCHER_BOTTOM(Green)

	ITE_COLOR_DISPATCHER_OPTIONS(DarkGrey)
	ITE_COLOR_DISPATCHER_OPTIONS(LightBlue92)
	ITE_COLOR_DISPATCHER_OPTIONS(LightBlue94)
	ITE_COLOR_DISPATCHER_OPTIONS(LightBlue96)
	ITE_COLOR_DISPATCHER_OPTIONS(DarkBlue8a)
	ITE_COLOR_DISPATCHER_OPTIONS(DarkGrey0C)
	ITE_COLOR_DISPATCHER_OPTIONS(Blue)
	ITE_COLOR_DISPATCHER_OPTIONS(BrightWhite)
#undef ITE_COLOR_DISPATCHER
#undef ITE_COLOR_DISPATCHER_BOTTOM
#undef ITE_COLOR_DISPATCHER_OPTIONS
#undef ITE_COLOR_DISPATCHER_TYPE
};


} // End of namespace Saga

#endif
