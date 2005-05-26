/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SAGA_H
#define SAGA_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/stream.h"
#include "common/rect.h"

#include "saga/text.h"
#include "saga/gfx.h"
#include "saga/list.h"

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

#define MIN_IMG_RLECODE    3
#define MODEX_SCANLINE_LIMIT 200

#define SAGA_IMAGE_DATA_OFFSET 776
#define SAGA_IMAGE_HEADER_LEN  8

#define MAXPATH 512 //TODO: remove

#define SAVE_TITLE_SIZE 28
#define MAX_SAVES 96
#define MAX_FILE_NAME 256


#define IS_BIG_ENDIAN ((_vm->getFeatures() & GF_BIG_ENDIAN_DATA) != 0)

#define ID_NOTHING 0
#define ID_PROTAG 1
#define OBJECT_TYPE_SHIFT 13
#define OBJECT_TYPE_MASK ((1 << OBJECT_TYPE_SHIFT) - 1)

#define OBJ_SPRITE_BASE 9

#define memoryError(Place) error("%s Memory allocation error.", Place)

struct RSCFILE_CONTEXT;
struct StringList;

enum ERRORCODE {
	MEM = -2,//todo: remove
	FAILURE = -1,
	SUCCESS = 0
};

enum SAGAGameType {
	GType_ITE,
	GType_IHNM
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
	kRepeatSpeed = 40,   // 25 frames/sec
	kNormalFadeDuration = 320, // 64 steps, 5 msec each
	kQuickFadeDuration = 64,  // 64 steps, 1 msec each
	kPuzzleHintTime = 30000  // 30 secs
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


enum PanelButtonType {
	kPanelButtonVerb = 1,
	kPanelButtonArrow = 2,
	kPanelButtonConverseText = 4,
	kPanelButtonInventory = 8,
	kPanelButtonOption = 0x10,
	kPanelButtonSlider = 0x20,
	kPanelAllButtons = 0xFFFFF
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
	kTextMax
};

struct IMAGE_HEADER {
	int width;
	int height;
};

struct StringsTable {
	byte *stringsPointer;
	int stringsCount;
	const char **strings;

	const char *getString(int index) const {
		if ((stringsCount <= index) || (index < 0))
			error("StringList::getString wrong index 0x%X", index);
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

struct CLICKAREA {
	int n_points;
	Point *points;
};

enum GameIds {
	// Dreamers Guild
	GID_ITE_DEMO_G = 0,
	GID_ITE_DISK_G,
	GID_ITE_CD_G,
	GID_ITE_MAC_G,  // TODO

	// Wyrmkeep
	GID_ITE_CD,       // data for Win rerelease is same as in old DOS
	GID_ITE_MACCD,
	GID_ITE_LINCD,
	GID_ITE_WINDEMO1, // older Wyrmkeep windows demo
	GID_ITE_MACDEMO1, // older Wyrmkeep mac demo
	GID_ITE_LINDEMO,
	GID_ITE_WINDEMO2,
	GID_ITE_MACDEMO2,

	// German
	GID_ITE_DISK_DE,
	GID_ITE_AMIGACD_DE, // TODO
	GID_ITE_OLDMAC_DE,  // TODO
	GID_ITE_AMIGA_FL_DE,// TODO
	GID_ITE_CD_DE,      // reported by mld. Bestsellergamers cover disk
	GID_ITE_AMIGA_AGA_DEMO, // TODO
	GID_ITE_AMIGA_ECS_DEMO, // TODO

	GID_IHNM_DEMO,
	GID_IHNM_CD,
	GID_IHNM_CD_DE   // reported by mld. German retail
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
	GAME_PATCHFILE    = 1 << 8
};

enum GameSoundTypes {
	GAME_SOUND_PCM = 0,
	GAME_SOUND_VOC,
	GAME_SOUND_WAV,
	GAME_SOUND_VOX
};

enum GameFeatures {
	GF_VOX_VOICES      = 1 << 0,
	GF_BIG_ENDIAN_DATA = 1 << 1,
	GF_MAC_RESOURCES   = 1 << 2,
	GF_LANG_DE         = 1 << 3,
	GF_WYRMKEEP        = 1 << 4,
	GF_CD_FX           = 1 << 5
};

struct GameSoundInfo {
	int res_type;
	long freq;
	int sample_size;
	int stereo;
};

struct GameFontDescription {
	uint32 fontResourceId;
};

struct GameResourceDescription {
	uint32 sceneLUTResourceId;
	uint32 moduleLUTResourceId;
	uint32 mainPanelResourceId;
	uint32 conversePanelResourceId;
	uint32 optionPanelResourceId;
	uint32 mainSpritesResourceId;
	uint32 mainPanelSpritesResourceId;
	uint32 defaultPortraitsResourceId;
	uint32 mainStringsResourceId;
	uint32 actorsStringsResourceId;
};

struct GameFileDescription {
	const char *fileName;
	uint16 fileType;
};

struct PanelButton {
	PanelButtonType type;
	int xOffset;
	int yOffset;
	int width;
	int height;
	int id;
	int keyChar;
	int state;
	int upSpriteNumber;
	int downSpriteNumber;
	int overSpriteNumber;
};

struct GameDisplayInfo {
	int logicalWidth;
	int logicalHeight;
	
	int pathStartY;
	int sceneHeight;

	int statusXOffset;
	int statusYOffset;
	int statusWidth;
	int statusHeight;
	int statusTextY;
	int statusTextColor;
	int statusBGColor;
	
	int saveReminderXOffset;
	int saveReminderYOffset;
	int saveReminderWidth;
	int saveReminderHeight;
	int saveReminderFirstSpriteNumber;
	int saveReminderSecondSpriteNumber;

	int verbTextColor;
	int verbTextShadowColor;
	int verbTextActiveColor;

	int leftPortraitXOffset;
	int leftPortraitYOffset;
	int rightPortraitXOffset;
	int rightPortraitYOffset;

	int inventoryUpButtonIndex;
	int inventoryDownButtonIndex;
	int inventoryRows;
	int inventoryColumns;

	int mainPanelXOffset;
	int mainPanelYOffset;
	int mainPanelButtonsCount;
	PanelButton *mainPanelButtons;

	int converseUpButtonIndex;
	int converseDownButtonIndex;

	int conversePanelXOffset;
	int conversePanelYOffset;
	int conversePanelButtonsCount;
	PanelButton *conversePanelButtons;

	int optionPanelXOffset;
	int optionPanelYOffset;
	int optionPanelButtonsCount;
	PanelButton *optionPanelButtons;
};


struct GameDescription {
	const char *name;
	SAGAGameType gameType;
	GameIds gameId;
	const char *title;
	GameDisplayInfo *gameDisplayInfo;
	int startSceneNumber;
	GameResourceDescription *resourceDescription;
	int filesCount;
	GameFileDescription *filesDescriptions;
	int fontsCount;
	GameFontDescription *fontDescriptions;
	GameSoundInfo *soundInfo;
	uint32 features;

	GameSettings toGameSettings() const {
		GameSettings dummy = { name, title, features };
		return dummy;
	}
};


inline int ticksToMSec(int tick) {
	return tick * 1000 / kScriptTimeTicksPerSecond;
}

inline int clamp(int minValue, int value, int maxValue) {
	if (value <= minValue) {
		return minValue;
	} else {
		if (value >= maxValue) {
			return maxValue;
		} else {
			return value;
		}
	}
}

inline int integerCompare(int i1, int i2) {
	return ((i1) > (i2) ? 1 : ((i1) < (i2) ? -1 : 0));
}

inline int objectTypeId(uint16 objectId) {
	return objectId >> OBJECT_TYPE_SHIFT;
}

inline int objectIdToIndex(uint16 objectId) {
	return OBJECT_TYPE_MASK & objectId;
}

inline uint16 objectIndexToId(int type, int index) {
	return (type << OBJECT_TYPE_SHIFT) | (OBJECT_TYPE_MASK & index);
}


DetectedGameList GAME_ProbeGame(const FSList &fslist);

class SagaEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);

protected:
	int go();
	int init(GameDetector &detector);
public:
	SagaEngine(GameDetector * detector, OSystem * syst);
	virtual ~SagaEngine();
	void shutDown() { _quit = true; }

	void save() { //TODO: remove
		save("iteSCUMMVM.sav", "default");
	}
	void load() { //TODO: remove
		load("iteSCUMMVM.sav");
	}
	void save(const char *fileName, const char *saveName);
	void load(const char *fileName);
	void fillSaveList();
	char *calcSaveFileName(uint slotNumber);
	char *getSaveFileName(uint idx);

	int _soundEnabled;
	int _musicEnabled;

	
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


	/** Random number generator */
	Common::RandomSource _rnd;

private:
	int decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len);
	int flipImage(byte *img_buf, int columns, int scanlines);
	int unbankBGImage(byte *dest_buf, const byte *src_buf, int columns, int scanlines);
	uint32 _previousTicks;

public:
	int decodeBGImage(const byte *image_data, size_t image_size,
						byte **output_buf, size_t *output_buf_len, int *w, int *h);
	const byte *getImagePal(const byte *image_data, size_t image_size);
	void loadStrings(StringsTable &stringsTable, const byte *stringsPointer, size_t stringsLength);

	const char *getObjectName(uint16 objectId);
public:
	TEXTLIST *textCreateList();
	void textDestroyList(TEXTLIST *textlist);
	void textClearList(TEXTLIST *textlist);
	int textDrawList(TEXTLIST *textlist, SURFACE *ds);
	TEXTLIST_ENTRY *textAddEntry(TEXTLIST *textlist, TEXTLIST_ENTRY *entry);
	int textDeleteEntry(TEXTLIST *textlist, TEXTLIST_ENTRY *entry);
	int textSetDisplay(TEXTLIST_ENTRY *entry, int val);
	int textDraw(int font_id, SURFACE *ds, const char *string, int text_x, int text_y, int color,
				  int effect_color, int flags);
	int textProcessList(TEXTLIST *textlist, long ms);

	int transitionDissolve(byte *dst_img, int dst_w, int dst_h, int dst_p, 
			const byte *src_img, int src_w, int src_h, int src_p, int flags, int x, int y, 
			double percent);

	int processInput(void);
	const Point &mousePos() const {
		return _mousePos;
	}

	const bool leftMouseButtonPressed() const {
		return _leftMouseButtonPressed;
	}

	const bool rightMouseButtonPressed() const {
		return _rightMouseButtonPressed;
	}

	const bool mouseButtonPressed() const {
		return _leftMouseButtonPressed || _rightMouseButtonPressed;
	}

 private:
	Common::String _targetName;
	uint _saveFileNamesCount;
	char _saveFileNames[MAX_SAVES][SAVE_TITLE_SIZE];
	Point _mousePos;
	bool _leftMouseButtonPressed;
	bool _rightMouseButtonPressed;

	bool _quit;
	RSCFILE_CONTEXT **_gameFileContexts;

//current game description
	int _gameNumber;
	GameDescription *_gameDescription;
	GameDisplayInfo _gameDisplayInfo;

public:
	int initGame(void);
	RSCFILE_CONTEXT *getFileContext(uint16 type, int param);
public:
	const GameResourceDescription *getResourceDescription() { return _gameDescription->resourceDescription; }
	const GameSoundInfo *getSoundInfo() { return _gameDescription->soundInfo; }

	const GameFontDescription *getFontDescription(int index) { 
		assert(index < _gameDescription->fontsCount);
		return &_gameDescription->fontDescriptions[index];
	}
	int getFontsCount() const { return _gameDescription->fontsCount; }

	int getGameId() const { return _gameDescription->gameId; }
	int getGameType() const { return _gameDescription->gameType; }
	uint32 getFeatures() const { return _gameDescription->features; }
	int getGameNumber() const { return _gameNumber; }
	int getStartSceneNumber() const { return _gameDescription->startSceneNumber; }


	int getDisplayWidth() const { return _gameDisplayInfo.logicalWidth; }
	int getDisplayHeight() const { return _gameDisplayInfo.logicalHeight;}
	int getSceneHeight() const { return _gameDisplayInfo.sceneHeight; }
	const GameDisplayInfo & getDisplayInfo() { return _gameDisplayInfo; }
	
	const char *getTextString(int textStringId);
	void getExcuseInfo(int verb, const char *&textString, int &soundResourceId);
private:
	int loadGame(int gameNumber);
};

// FIXME: Global var. We use it until everything will be turned into objects
extern SagaEngine *_vm;

} // End of namespace Saga

#endif
