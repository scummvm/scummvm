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

#ifndef TOON_H
#define TOON_H

#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "common/random.h"
#include "toon/resource.h"
#include "toon/script.h"
#include "toon/script_func.h"
#include "toon/state.h"
#include "toon/picture.h"
#include "toon/anim.h"
#include "toon/movie.h"
#include "toon/font.h"
#include "toon/text.h"
#include "toon/audio.h"

#define TOON_DAT_VER_MAJ 0  // 1 byte
#define TOON_DAT_VER_MIN 3  // 1 byte
#define TOON_SAVEGAME_VERSION 4
#define DATAALIGNMENT 4

namespace Toon {
enum ToonGameType {
	GType_TOON = 1
};

enum ToonDebugChannels {
	kDebugAnim      = 1 <<  0,
	kDebugCharacter = 1 <<  1,
	kDebugAudio     = 1 <<  2,
	kDebugHotspot   = 1 <<  3,
	kDebugFont      = 1 <<  4,
	kDebugPath      = 1 <<  5,
	kDebugMovie     = 1 <<  6,
	kDebugPicture   = 1 <<  7,
	kDebugResource  = 1 <<  8,
	kDebugState     = 1 <<  9,
	kDebugTools     = 1 << 10,
	kDebugText      = 1 << 11
};

class Picture;
class Movie;
class Hotspots;
class Character;
class CharacterDrew;
class CharacterFlux;
class FontRenderer;
class TextResource;
class AudioManager;
class PathFinding;

class ToonEngine : public Engine {
public:
	ToonEngine(OSystem *syst, const ADGameDescription *gameDescription);
	~ToonEngine();

	const ADGameDescription *_gameDescription;
	Common::Language _language;
	byte   _numVariant;
	byte   _gameVariant;
	char **_locationDirNotVisited;
	char **_locationDirVisited;
	char **_specialInfoLine;

	virtual Common::Error run();
	virtual bool showMainmenu(bool &loadedGame);
	virtual void init();
	bool loadToonDat();
	char **loadTextsVariante(Common::File &in);
	virtual void setPaletteEntries(uint8 *palette, int32 offset, int32 num);
	virtual void fixPaletteEntries(uint8 *palette, int num);
	virtual void flushPalette();
	virtual void parseInput();
	virtual void initChapter();
	virtual void initFonts();
	virtual void loadScene(int32 SceneId, bool forGameLoad = false);
	virtual void exitScene();
	virtual void loadCursor();
	virtual void setCursor(int32 type, bool inventory = false, int32 offsetX = 0, int offsetY = 0);
	virtual void loadAdditionalPalette(Common::String fileName, int32 mode);
	virtual void setupGeneralPalette();
	virtual void render();
	virtual void update(int32 timeIncrement);
	virtual void doFrame();
	virtual void updateAnimationSceneScripts(int32 timeElapsed);
	virtual void updateCharacters(int32 timeElapsed);
	virtual void setSceneAnimationScriptUpdate(bool enable);
	virtual bool isUpdatingSceneAnimation();
	virtual int32 getCurrentUpdatingSceneAnimation();
	virtual int32 randRange(int32 minStart, int32 maxStart);
	virtual void selectHotspot();
	virtual void clickEvent();
	virtual int32 runEventScript(int32 x, int32 y, int32 mode, int32 id, int32 scriptId);
	virtual void flipScreens();
	virtual void drawInfoLine();
	virtual void drawConversationLine();
	virtual const char *getLocationString(int32 locationId, bool alreadyVisited);
	virtual int32 getScaleAtPoint(int32 x, int32 y);
	virtual int32 getZAtPoint(int32 x, int32 y);
	virtual int32 getLayerAtPoint(int32 x, int32 y);
	virtual int32 characterTalk(int32 dialogid, bool blocking = true);
	virtual int32 simpleCharacterTalk(int32 dialogid);
	virtual void sayLines(int numLines, int dialogId);
	virtual void haveAConversation(int32 convId);
	virtual void processConversationClick(Conversation *conv, int32 status);
	virtual int32 runConversationCommand(int16 **command);
	virtual void prepareConversations();
	virtual void drawConversationIcons();
	virtual void simpleUpdate();
	virtual int32 waitTicks(int32 numTicks, bool breakOnMouseClick);
	virtual void copyToVirtualScreen(bool updateScreen = true);
	virtual void getMouseEvent();
	virtual int32 showInventory();
	virtual void drawSack();
	virtual void addItemToInventory(int32 item);
	virtual void deleteItemFromInventory(int32 item);
	virtual void replaceItemFromInventory(int32 item, int32 destItem);
	virtual void rearrangeInventory();
	virtual void createMouseItem(int32 item);
	virtual void deleteMouseItem();
	virtual void showCutaway(Common::String cutawayPicture);
	virtual void hideCutaway();
	virtual void drawPalette();
	virtual void newGame();
	virtual void playSoundWrong();
	virtual void playSFX(int32 id, int32 volume);
	virtual void storeRifFlags(int32 location);
	virtual void restoreRifFlags(int32 location);
	virtual void getTextPosition(int32 characterId, int32 *retX, int32 *retY);
	virtual int32 getConversationFlag(int32 locationId, int32 param);
	virtual int32 getSpecialInventoryItem(int32 item);
	virtual Character *getCharacterById(int32 charId);
	virtual Common::String getSavegameName(int nr);
	virtual bool loadGame(int32 slot);
	virtual bool saveGame(int32 slot);
	virtual void fadeIn(int32 numFrames) ;
	virtual void fadeOut(int32 numFrames) ;
	virtual void initCharacter(int32 characterId, int32 animScriptId, int32 animToPlayId, int32 sceneAnimationId);
	virtual int32 handleInventoryOnFlux(int32 itemId);
	virtual int32 handleInventoryOnInventory(int32 itemDest, int32 itemSrc);
	virtual int32 handleInventoryOnDrew(int32 itemId);
	virtual int32 pauseSceneAnimationScript(int32 animScriptId, int32 tickToWait);
	virtual void updateTimer(int32 timeIncrement);
	virtual Common::String createRoomFilename(Common::String name);
	virtual void createShadowLUT();
	virtual void playTalkAnimOnCharacter(int32 animID, int32 characterId, bool talker);
	virtual void updateScrolling(bool force, int32 timeIncrement);
	virtual void enableTimer(int32 timerId);
	virtual void setTimer(int32 timerId, int32 timerWait);
	virtual void disableTimer(int32 timerId);
	virtual void updateTimers();
	virtual void makeLineNonWalkable(int32 x, int32 y, int32 x2, int32 y2);
	virtual void makeLineWalkable(int32 x, int32 y, int32 x2, int32 y2);
	virtual void renderInventory();
	virtual void viewInventoryItem(Common::String str, int32 lineId, int32 itemDest);
	virtual void storePalette();
	virtual void restorePalette();
	virtual const char *getSpecialConversationMusic(int32 locationId); 
	virtual void playRoomMusic();

	Resources *resources() {
		return _resources;
	}

	State *state() {
		return _gameState;
	}

	Graphics::Surface &getMainSurface() {
		return *_mainSurface;
	}

	Picture *getMask() {
		return _currentMask;
	}

	Picture *getPicture() {
		return _currentPicture;
	}

	AnimationManager *getAnimationManager() {
		return _animationManager;
	}

	Movie *getMoviePlayer() {
		return _moviePlayer;
	}

	SceneAnimation *getSceneAnimation(int32 id) {
		return &_sceneAnimations[id];
	}

	SceneAnimationScript *getSceneAnimationScript(int32 id) {
		return &_sceneAnimationScripts[id];
	}

	EMCInterpreter *getScript() {
		return _script;
	}

	Hotspots *getHotspots() {
		return _hotspots;
	}

	Character *getCharacter(int32 charId) {
		return _characters[charId];
	}

	uint8 *getShadowLUT() {
		return _shadowLUT;
	}

	int32 getCurrentLineToSay() {
		return _currentTextLineId;
	}

	CharacterDrew *getDrew() {
		return (CharacterDrew *)_drew;
	}

	CharacterFlux *getFlux() {
		return (CharacterFlux *)_flux;
	}

	int32 getTickLength() {
		return _tickLength;
	}

	int32 getOldMilli() {
		return _oldTimer2;
	}

	OSystem *getSystem() {
		return _system;
	}

	AudioManager *getAudioManager() {
		return _audioManager;
	}

	int32 getScriptRegionNested() {
		return _currentScriptRegion;
	}

	int32 getMouseX() {
		return _mouseX;
	}

	int32 getMouseY() {
		return _mouseY;
	}

	PathFinding *getPathFinding() {
		return _pathFinding;
	}

	Common::WriteStream *getSaveBufferStream() {
		return _saveBufferStream;
	}

protected:
	OSystem *_system;
	int32 _tickLength;
	Resources *_resources;
	TextResource *_genericTexts;
	TextResource *_roomTexts;
	State *_gameState;
	uint8 *_finalPalette;
	uint8 *_backupPalette;
	uint8 *_additionalPalette1;
	uint8 *_additionalPalette2;
	uint8 *_cutawayPalette;
	uint8 *_universalPalette;
	uint8 *_fluxPalette;
	uint8 *_roomScaleData;
	uint8 *_shadowLUT;

	Picture *_currentPicture;
	Picture *_currentMask;
	Picture *_currentCutaway;
	Picture *_inventoryPicture;
	PathFinding *_pathFinding;

	EMCInterpreter *_script;
	EMCData _scriptData;
	EMCState _scriptState[4];
	int32 _currentScriptRegion; // script region ( nested script run )

	ScriptFunc *_script_func;

	SceneAnimation _sceneAnimations[64];
	SceneAnimationScript _sceneAnimationScripts[64];
	int32 _lastProcessedSceneScript;
	bool _animationSceneScriptRunFlag;
	bool _updatingSceneScriptRunFlag;

	Graphics::Surface *_mainSurface;

	AnimationInstance *_cursorAnimationInstance;
	Animation *_cursorAnimation;
	Animation *_dialogIcons;
	Animation *_inventoryIcons;
	Animation *_inventoryIconSlots;
	int32 _cursorOffsetX;
	int32 _cursorOffsetY;

	char *_currentTextLine;
	int32 _currentTextLineId;
	int32 _currentTextLineX;
	int32 _currentTextLineY;
	int32 _currentTextLineCharacterId;

	int32 _oldScrollValue;

	AnimationManager *_animationManager;

	Character *_characters[32];
	Character *_drew;
	Character *_flux;

	Hotspots *_hotspots;
	int32 _currentHotspotItem;


	int32 _mouseX;
	int32 _mouseY;
	int32 _mouseButton;
	int32 _lastMouseButton;

	int32 _oldTimer;
	int32 _oldTimer2;

	Movie *_moviePlayer;

	Common::RandomSource _rnd;

	FontRenderer *_fontRenderer;
	Animation *_fontToon;
	Animation *_fontEZ;

	AudioManager *_audioManager;

	Common::MemoryWriteStreamDynamic *_saveBufferStream;

	int16 *_conversationData;

	bool _firstFrame;
	bool _isDemo;
};

} // End of namespace Toon

#endif
