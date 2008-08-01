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

#ifndef SAGA_H
#define SAGA_H

#include "engines/engine.h"

#include "common/stream.h"
#include "sound/mididrv.h"

#include "saga/gfx.h"
#include "saga/list.h"

namespace Common {
struct ADGameFileDescription;
}

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

struct ResourceContext;
struct StringList;

using Common::MemoryReadStream;
using Common::MemoryReadStreamEndian;

#define SAGA_IMAGE_DATA_OFFSET 776
#define SAGA_IMAGE_HEADER_LEN  8

// Note that IHNM has a smaller save title size than ITE
// We allocate the ITE save title size in savegames, to
// preserve savegame backwards compatibility. We only check
// for IHNM's save title during text input
#define SAVE_TITLE_SIZE 28
#define IHNM_SAVE_TITLE_SIZE 22
#define MAX_SAVES 96
#define MAX_FILE_NAME 256

#define ID_NOTHING 0
#define ID_PROTAG 1
#define OBJECT_TYPE_SHIFT 13
#define OBJECT_TYPE_MASK ((1 << OBJECT_TYPE_SHIFT) - 1)

#define OBJ_SPRITE_BASE 9
#define IHNM_OBJ_PROFILE 0x4000

#define memoryError(Place) error("%s Memory allocation error.", Place)

enum ERRORCODE {
	FAILURE = -1,
	SUCCESS = 0
};

enum SAGAGameType {
	GType_ITE = 0,
	GType_IHNM = 1
};

enum GameIds {
	// Dreamers Guild
	GID_ITE_DEMO_G = 0,
	GID_ITE_DISK_G,
	GID_ITE_DISK_G2,
	GID_ITE_CD_G,
	GID_ITE_CD_G2,
	GID_ITE_MACCD_G,

	// Wyrmkeep
	GID_ITE_CD,       // data for Win rerelease is same as in old DOS
	GID_ITE_WINCD,    // but it has a bunch of patch files
	GID_ITE_MACCD,
	GID_ITE_LINCD,
	GID_ITE_MULTICD,  // Wyrmkeep combined Windows/Mac/Linux version
	GID_ITE_WINDEMO1, // older Wyrmkeep windows demo
	GID_ITE_MACDEMO1, // older Wyrmkeep mac demo
	GID_ITE_LINDEMO,
	GID_ITE_WINDEMO2,
	GID_ITE_WINDEMO3,
	GID_ITE_MACDEMO2,

	// German
	GID_ITE_DISK_DE,
	GID_ITE_DISK_DE2,
	GID_ITE_AMIGACD_DE, // TODO
	GID_ITE_OLDMAC_DE,  // TODO
	GID_ITE_AMIGA_FL_DE,// TODO
	GID_ITE_CD_DE,      // reported by mld. Bestsellergamers cover disk
	GID_ITE_CD_DE2,
	GID_ITE_AMIGA_AGA_DEMO, // TODO
	GID_ITE_AMIGA_ECS_DEMO, // TODO

	GID_IHNM_DEMO,
	GID_IHNM_CD,
	GID_IHNM_CD_DE,   // reported by mld. German retail
	GID_IHNM_CD_ES,
	GID_IHNM_CD_RU,
	GID_IHNM_CD_FR,
	GID_IHNM_CD_ITA,
	GID_IHNM_CD_MAC
};

enum GameFileTypes {
	GAME_RESOURCEFILE = 1 << 0,
	GAME_SCRIPTFILE   = 1 << 1,
	GAME_SOUNDFILE    = 1 << 2,
	GAME_VOICEFILE    = 1 << 3,
	GAME_DEMOFILE     = 1 << 4,
	GAME_MUSICFILE    = 1 << 5,
	GAME_MUSICFILE_GM = 1 << 6,
	GAME_MUSICFILE_FM = 1 << 7,
	GAME_PATCHFILE    = 1 << 8,
	GAME_MACBINARY    = 1 << 9,
	GAME_SWAPENDIAN   = 1 << 10
};

enum GameFeatures {
	GF_BIG_ENDIAN_DATA   = 1 << 0,
	GF_WYRMKEEP          = 1 << 1,
	GF_CD_FX             = 1 << 2,
	GF_SCENE_SUBSTITUTES = 1 << 3,
	GF_COMPRESSED_SOUNDS = 1 << 4,
	GF_NON_INTERACTIVE   = 1 << 5
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

enum GameSoundTypes {
	kSoundPCM = 0,
	kSoundVOX = 1,
	kSoundVOC = 2,
	kSoundWAV = 3,
	kSoundMacPCM = 4,
	kSoundMP3 = 5,
	kSoundOGG = 6,
	kSoundFLAC = 7,
	kSoundAIFF = 8
};

enum TextStringIds {
	kTextWalkTo,
	kTextLookAt,
	kTextPickUp,
	kTextTalkTo,
	kTextOpen,
	kTextClose,
	kTextUse,
	kTextGive,
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

struct GameSoundInfo {
	GameSoundTypes resourceType;
	int sampleBits;
	bool isBigEndian;
	bool isSigned;
};

struct GamePatchDescription {
	const char *fileName;
	uint16 fileType;
	uint32 resourceId;
	const GameSoundInfo *soundInfo;
};

struct SAGAGameDescription;

enum GameObjectTypes {
	kGameObjectNone = 0,
	kGameObjectActor = 1,
	kGameObjectObject = 2,
	kGameObjectHitZone = 3,
	kGameObjectStepZone = 4
};

enum ScriptTimings {
	kScriptTimeTicksPerSecond = (728L/10L),
	kScriptTimeTicksPerSecondIHNM = 72,
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
	byte *stringsPointer;
	int stringsCount;
	const char **strings;

	const char *getString(int index) const {
		if ((stringsCount <= index) || (index < 0)) {
			// This occurs at the end of Ted's chapter, right after the ending cutscene
			warning("StringList::getString wrong index 0x%X (%d)", index, stringsCount);
			return "";
		}
		return strings[index];
	}

	void freeMem() {
		free(strings);
		free(stringsPointer);
		memset(this, 0, sizeof(*this));
	}

	StringsTable() {
		memset(this, 0, sizeof(*this));
	}
	~StringsTable() {
		freeMem();
	}
};



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
	kITEColorGreen = 0xba,

	kIHNMColorPortrait = 0xfe
};

enum KnownColor {
	kKnownColorTransparent,
	kKnownColorBrightWhite,
	kKnownColorWhite,
	kKnownColorBlack,

	kKnownColorSubtitleTextColor,
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

class SagaEngine : public Engine {
	friend class Scene;

protected:
	int go();
	int init();
public:
	SagaEngine(OSystem *syst, const SAGAGameDescription *gameDesc);
	virtual ~SagaEngine();
	void shutDown() { _quit = true; }

	void save(const char *fileName, const char *saveName);
	void load(const char *fileName);
	uint32 getCurrentLoadVersion() {
		return _saveHeader.version;
	}
	void fillSaveList();
	char *calcSaveFileName(uint slotNumber);

	SaveFileData *getSaveFile(uint idx);
	uint getSaveSlotNumber(uint idx);
	uint getNewSaveSlotNumber();
	bool locateSaveFile(char *saveName, uint &titleNumber);
	bool isSaveListFull() const {
		return _saveFilesCount == MAX_SAVES;
	}
	uint getSaveFilesCount() const {
		return isSaveListFull() ? _saveFilesCount : _saveFilesCount + 1;
	}

	int16 _framesEsc;

	uint32 _globalFlags;
	int16 _ethicsPoints[8];
	int _spiritualBarometer;

	int _soundVolume;
	int _musicVolume;
	bool _subtitlesEnabled;
	bool _voicesEnabled;
	bool _voiceFilesExist;
	int _readingSpeed;

	bool _copyProtection;
	bool _gf_wyrmkeep;
	bool _gf_compressed_sounds;

	SndRes *_sndRes;
	Sound *_sound;
	Music *_music;
	MidiDriver *_driver;
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
	int decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len);
	int flipImage(byte *img_buf, int columns, int scanlines);
	int unbankBGImage(byte *dest_buf, const byte *src_buf, int columns, int scanlines);
	uint32 _previousTicks;

public:
	int decodeBGImage(const byte *image_data, size_t image_size,
			  byte **output_buf, size_t *output_buf_len, int *w, int *h, bool flip = false);
	const byte *getImagePal(const byte *image_data, size_t image_size);
	void loadStrings(StringsTable &stringsTable, const byte *stringsPointer, size_t stringsLength);

	const char *getObjectName(uint16 objectId);
public:
	int processInput(void);
	Point mousePos() const;

	int getMouseClickCount() {
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

	inline int ticksToMSec(int tick) {
		if (getGameType() == GType_ITE)
			return tick * 1000 / kScriptTimeTicksPerSecond;
		else
			return tick * 1000 / kScriptTimeTicksPerSecondIHNM;
	}

 private:
	uint _saveFilesCount;
	SaveFileData _saveFiles[MAX_SAVES];
	SaveGameHeader _saveHeader;

	bool _leftMouseButtonPressed;
	bool _rightMouseButtonPressed;
	int _mouseClickCount;

	bool _quit;

//current game description
	int _gameNumber;
	const SAGAGameDescription *_gameDescription;
	Common::String _gameTitle;
	Common::Rect _displayClip;

public:
	int32 _frameCount;

public:
	bool initGame(void);

	bool isBigEndian() const;
	bool isMacResources() const;
	const GameResourceDescription *getResourceDescription();
	const GameSoundInfo *getVoiceInfo() const;
	const GameSoundInfo *getSfxInfo() const;
	const GameSoundInfo *getMusicInfo() const;

	const GameFontDescription *getFontDescription(int index);
	int getFontsCount() const;

	int getGameId() const;
	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	int getGameNumber() const;
	int getStartSceneNumber() const;

	int getPatchesCount() const;
	const GamePatchDescription *getPatchDescriptions() const;

	const Common::ADGameFileDescription *getFilesDescriptions() const;

	const Common::Rect &getDisplayClip() const { return _displayClip;}
	int getDisplayWidth() const;
	int getDisplayHeight() const;
	const GameDisplayInfo &getDisplayInfo();

	const char *getTextString(int textStringId);
	void getExcuseInfo(int verb, const char *&textString, int &soundResourceId);

private:

public:
	ColorId KnownColor2ColorId(KnownColor knownColor);
	void setTalkspeed(int talkspeed);
	int getTalkspeed();
};

} // End of namespace Saga

#endif
