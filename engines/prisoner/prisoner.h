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

#ifndef PRISONER_H
#define PRISONER_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"

#include "audio/mixer.h"

#include "graphics/surface.h"

#include "engines/engine.h"

#include "prisoner/objectstorage.h"

namespace Prisoner {

enum PrisonerGameFeatures {
};

struct PrisonerGameDescription;

class PrisonerResourceLoader;
class Screen;
class ResourceManager;
class AnimationResource;
struct AnimationFrameList;
struct AnimationCel;
class FontResource;
class PictureResource;
class ScriptResource;
class TextResource;
struct TextItem;
class ScriptOpcodes;
class MidiPlayer;
class PathSystem;
class PrisonerEngine;

/* Script */

const int16 kSceneScriptProgram = 0;
const int16 kModuleScriptProgram = 1;
const int16 kMaxScripts = 50;

enum {
	kScriptStatusPaused		= 0,
	kScriptStatusRunCode	= 1,
	kScriptStatusSleeping	= 2,
	kScriptStatusSync		= 3,
	kScriptStatusAnimation	= 4,
	kScriptStatusText		= 5,
	kScriptStatusDialog		= 6,
	kScriptStatusWalking	= 7,
	kScriptStatusSyncResume	= 8,
	kScriptStatusActorZone	= 9,
	kScriptStatus10			= 10,
	kScriptStatusSound		= 11,
	kScriptStatus12			= 12,
	kScriptStatus13			= 13
};

struct Script {
	byte status;
	byte *ip, *code;
	int16 soundIndex;
	int16 zoneIndex;
	bool zoneEnterLeaveFlag;
	int16 screenTextIndex;
	int32 sleepCounter;
	int16 syncScriptNumber;
	int16 actorIndex;
	int16 actorIndex2;
	int16 altAnimationIndex;
	int16 frameIndex;
	byte readByte();
	int16 readInt16();
	Common::String readString();
	void clear() {
		status = kScriptStatusPaused;
		ip = NULL;
		soundIndex = -1;
		zoneIndex = -1;
		screenTextIndex = -1;
		syncScriptNumber = -1;
		actorIndex = -1;
		actorIndex2 = -1;
	}
};

struct ScriptProgram {
	int16 resourceCacheSlot;
	ScriptResource *scriptResource;
	int16 scriptCount;
	Script scripts[kMaxScripts];
	ScriptProgram() : resourceCacheSlot(-1), scriptCount(0), scriptResource(NULL) {}
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

/* Zones */

struct Zone {
	byte used;
	int16 x1, y1, x2, y2;
	int8 type;
	int16 actorIndex;
	int16 mouseCursor;
	byte hasText;
	int16 resourceCacheSlot;
	uint textIndex;
	Common::String identifier;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

struct ZoneAction {
	int16 used;
	int16 zoneActionIndex;
	int16 zoneIndex;
	int16 type;
	int16 inventoryItemIndex;
	int16 pathNodeIndex;
	int16 scriptIndex1;
	int16 scriptProgIndex;
	int16 scriptIndex2;
	int16 moduleIndex;
	int16 sceneIndex;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxZones = 50;
const int16 kMaxZoneActions = 70;

/* Dialog */

struct DialogKeyword {
	Common::String _keyword;
	byte _used;
	DialogKeyword() : _used(0) {}
	DialogKeyword(const Common::String &keyword) : _keyword(keyword), _used(0) {
		debug(1, "keyword = [%s]", keyword.c_str());
	}
};

struct Dialog {
	byte used;
	Common::String pakName;
	int16 pakSlot;
	Common::Array<DialogKeyword> keywords;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxDialogs = 3;

/* Inventory */

struct InventoryItem {
	int16 resourceCacheSlot;
	int16 id;
	int8 status;
	Common::String name;
	int16 combinationIndex;
	bool isEmpty() const { return resourceCacheSlot == -1; }
	void clear() {
		resourceCacheSlot = -1;
		id = -1;
		status = 0;
		name.clear();
		combinationIndex = -1;
	}
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

struct InventoryItemCombination {
	byte used;
	int16 inventoryItem1, inventoryItem2;
	int16 scriptIndex;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxInventoryItems = 50;
const int16 kMaxInventoryItemCombinations = 10;

/* Actor */

struct ActorSprite {
	byte used;
	int16 actorIndex;
	int16 x, y;
	int16 xsub, ysub;
	int16 xadd, yadd;
	int16 xoffs, yoffs;
	int16 scale;
	int16 boundsX1, boundsY1;
	int16 boundsX2, boundsY2;
	int16 zoneX1, zoneY1, zoneX2, zoneY2;
	byte flag;
	AnimationResource *animationResource;
	int16 elementIndex;
	int16 prevFrameIndex, frameIndex, frameCount;
	int16 ticks;
	int16 frameListIndex, frameListCount;
	AnimationFrameList *frameList;
	void clear() { used = 0; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

struct CompareActorSpriteByY {
	bool operator()(const ActorSprite *left, const ActorSprite *right) const {
		return left->y < right->y;
	}
};

struct PathResult;

const int16 kMaxActors = 20;

struct Actor {
	int16 resourceCacheSlot;
	int16 altAnimationIndex;
	int16 status;
	int16 pathWalkerIndex;
	int16 pathResultIndex;
	PathResult *pathWalker1;
	int16 pathResultCount;
	PathResult *pathWalker2;
	int16 pathNodeIndex;
	int16 pathEdgeIndex;
	int16 pathPolyIndex;
	int16 firstFrameListIndex, lastFrameListIndex;
	int16 minTicks, maxTicks;
	int32 ticks;
	byte ticksFlag;
	Common::String pakName;
	int16 pakSlot;
	int16 frameListIndex;
	int16 walkDestX, walkDestY;
	int16 x2, y2;
	int16 textFontNumber;
	int16 fontOutlineColor, fontInkColor;
	ActorSprite *actorSprite;
	int16 x, y;
	bool isEmpty() const { return resourceCacheSlot == -1; }
	void clear() { resourceCacheSlot = -1; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxAltActorAnimations = 5;

struct AltActorAnimation {
	int16 resourceCacheSlot;
	byte value;
	bool isEmpty() const { return resourceCacheSlot == -1; }
	void clear() { resourceCacheSlot = -1; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

/* BackgroundObject */

struct BackgroundObject {
	byte used;
	int16 x, y;
	byte va2Command;
	int16 celIndex;
	int16 boundsX1, boundsY1, boundsX2, boundsY2;
	int16 x1, y1, x2, y2;
	AnimationResource *animationResource;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
};

struct CompareBackgroundObjectByY {
	bool operator()(const BackgroundObject *left, const BackgroundObject *right) const {
		return left->y < right->y;
	}
};

const int16 kMaxBackgroundObjects = 20;

struct PathResult {
	int16 x, y;
	int16 edgeIndex;
	int16 nodeIndex;
	int16 polyIndex;
	int16 direction;
	int16 scale;
};

const int16 kMaxPathWalkers = 10;

struct PathWalker {
	byte used;
	PathResult items[452];
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
};


/* Scene items */

struct SceneItem {
	int16 actorIndex;
	int16 inventoryItemIndex;
	bool isEmpty() const { return actorIndex == -1; }
	void clear() {
		actorIndex = -1;
		inventoryItemIndex = -1;
	}
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxSceneItems = 10;

/* ActorFrameSounds */

struct ActorFrameSound {
	int16 actorIndex;
	int16 soundIndex;
	int16 frameListIndex;
	int16 frameIndex;
	int16 volume;
	bool flag;
	bool isEmpty() const { return actorIndex == -1; }
	void clear() { actorIndex = -1; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxActorFrameSounds = 30;

/* Fonts */

struct FontColorDef {
	byte inkColor, outlineColor;
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

struct Font {
	int16 resourceCacheSlot;
	FontResource *fontResource;
	int16 unk1;
	int16 height;
	int16 unk2;
	int16 interletter;
	int16 outlineColorIdx;
	int16 inkColorIdx;
	int16 outlineColor;
	int16 inkColor;
	bool isEmpty() const { return resourceCacheSlot == -1; }
	void clear() { resourceCacheSlot = -1; }
};

/* Screen text */

struct ScreenText {
	byte used;
	bool screenText;
	Common::String identifier;
	int16 x0, y0;
	bool screenCenter;
	int16 resourceCacheSlot;
	int16 actorIndex;
	uint32 finishedTime;
	int16 speechPakSlot;
	int16 chunkCount;
	int16 chunkIndex;
	bool inventoryActive;
	int16 textIndex;
	int16 fontIndex;
	int16 fontOutlineColor;
	int16 fontInkColor;
	int16 x, y;
	int16 lineCount;
	int16 width, height;
	int16 fontHeight;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxScreenTexts = 3;

/* LipSync */

struct LipSyncChannelStatus {
	int16 ticks;
	int16 index;
	LipSyncChannelStatus() : ticks(-1), index(0) {}
};

/* Mouse cursors */

struct MouseCursor {
	Common::String key;
	int16 pakSlot;
	int16 resourceCacheSlot;
	int16 frameListIndex;
};

const int16 kMouseCursors = 14;

enum {
	kCursorDefault		= 0,
	kCursorLook			= 1,
	kCursorMove			= 2,
	kCursorDoor			= 3,
	kCursorRead			= 4,
	kCursorAct			= 5,
	kCursorTalk			= 6,
	kCursorDisk			= 7,
	kCursorDialog		= 8,
	kCursorAutomatic	= 9,
	// 10 and 11 are not used
	kCursorItem			= 12,
	kCursorMenu			= 13
};

/* ClickBox */

struct ClickBox {
	byte used;
	byte flag1;
	byte flag2;
	int16 x1, y1, x2, y2;
	int16 tag;
	int32 unk1;
	int16 unk2;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
};

const int16 kMaxClickBoxes = 110;

/* Palette task */

struct PaletteTask {
	bool active;
	int16 value1;
	int16 value2;
	int16 value3;
	int16 positionIncr;
	int16 updateTicks;
	void save(PrisonerEngine *vm, Common::WriteStream *out);
	void load(PrisonerEngine *vm, Common::ReadStream *in);
};

const int16 kMaxPaletteTasks = 5;

/* Sounds */

struct SoundSlot {
	Audio::SoundHandle handle;
	int16 resourceCacheSlot;
	bool volumeFlag;
	uint volume;
	bool shouldResume;
	bool moduleWide;
	bool isEmpty() const { return resourceCacheSlot == -1; }
	void clear() { resourceCacheSlot = -1; }
};

const int16 kMaxSounds = 25;

/* Input low-level */

enum {
	kLeftButton			= 1 << 0,
	kRightButton		= 1 << 1,
	kLeftButtonDbl		= 1 << 2,
	kRightButtonDbl		= 1 << 3
};

/* PrisonerEngine */

class PrisonerEngine : public ::Engine {
protected:

	Common::Error run();

public:
	PrisonerEngine(OSystem *syst, const PrisonerGameDescription *gameDesc);
	virtual ~PrisonerEngine();

	// Detection related functions
	const PrisonerGameDescription *_gameDescription;
	const char *getGameId() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	bool hasFeature(EngineFeature f) const;

	Common::RandomSource *_rnd;

	Screen *_screen;
	PrisonerResourceLoader *_resLoader;
	ResourceManager *_res;

	int _mouseX, _mouseY;

	Common::KeyCode _keyState;
	uint16 _buttonState;

	char _languageChar;
	int16 _animationSpeed;
	int16 _lockUserInputRefCounter;
	bool _clearBackgroundFlag;
	bool _sceneFlag;
	bool _mainMenuRequested;

	Common::String _muxFilename;
	bool _muxClearScreenAfter, _muxClearScreenBefore;
	Common::KeyCode _muxEasterEggKey;
	uint _muxEasterEggCount;

	int16 _textFont, _menuFont;

	int16 _inventoryItemCursor;
	int16 _currInventoryItemCursor;

	int16 _currMouseCursor;

	bool _menuMouseCursorActive;
	int16 _menuMouseCursor;

	bool _updateDirtyRectsFlag;

	bool _talkieEnabled;

	bool _loadingSavegame;

	/* AutoSave */
	bool _autoSaveRequested;
	Common::String _autoSavePakName, _autoSaveIdentifier;
	int16 _autoSavePakSlot;

	int16 _globalTextResourceCacheSlot;

	/* Screensaver */
	bool _screensaverRunning;
	bool _screensaverAborted;

	/* Frame time */
	uint32 _lastFrameTime;
	int16 _frameTicks;
	bool _animationFrameTimeFlag;
	int16 _animationFrameTicks;

	/* Input low-level */
	bool _inpMouseWaitRelease, _inpMousePulseRelease;
	uint32 _inpMousePulseTime;
	int16 _inpMousePulseTicks;
	uint32 _inpMouseDblClickTime, _inpMouseDblClickLastClickTime;
	bool _inpKeybWaitRelease, _inpKeybPulseRelease;
	uint32 _inpKeybPulseTime;
	uint16 _inpKeybPulseTicks;

	/* Input */
	int16 _optionsTextTicksCounter;
	uint32 _optionsTextTicks;
	bool _inputFlag;

	/* Scene control */
	int16 _currModuleIndex, _currSceneIndex;
	int16 _newModuleIndex, _newSceneIndex;
	int16 _prevModuleIndex, _prevSceneIndex;
	int16 _enterSceneScriptProgramIndex, _enterSceneScriptIndex;
	int16 _leaveSceneScriptProgramIndex, _leaveSceneScriptIndex;
	bool _inScene;

	/* Script */
	ScriptProgram _scriptPrograms[2];
	ScriptOpcodes *_scriptOpcodes;
	int16 _currScriptProgramIndex;
	Script *_currScript;
	int16 _currScriptIndex;
	bool _scriptContinueFlag;
	bool _needToPlayMux;
	bool _moduleScriptCalled;
	int16 _globalScriptVars[250];
	int16 _moduleScriptVars[300];

	/* Background */
	int16 _backgroundResourceCacheSlot;
	PictureResource *_backgroundResource;
	int16 _backgroundWidth, _backgroundHeight;
	bool _backgroundNoScrollFlag1, _backgroundNoScrollFlag2;
	int16 _backgroundDrawX, _backgroundDrawY;
	int16 _backgroundDrawWidth, _backgroundDrawHeight;
	bool _updateScreenValue; // always true
	int16 _cameraX, _cameraY;
	int16 _cameraDeltaX, _cameraDeltaY;
	bool _cameraFocusActor;
	int16 _cameraFollowsActorIndex;
	bool _backgroundCameraLocked;
	int16 _backgroundFlag;

	/* Palette */
	bool _needToUpdatePalette;
	bool _scenePaletteOk, _effectPaletteOk;
	byte _effectPalette[768], _scenePalette[768];

	Graphics::Surface *_screenBackupSurface;
	bool _screenBackedup;

	/* Actors */
	ObjectStorage<Actor, kMaxActors> _actors;
	ObjectStorage<ActorSprite, kMaxActors> _actorSprites;
	ObjectStorage<AltActorAnimation, kMaxAltActorAnimations> _altActorAnimations;
	int16 _mainActorIndex;
	bool _mainActorValid;
	bool _actorsCleared;

	bool _actorPathFlag;
	ZoneAction _zoneActionItem;
	int16 _actorPathX, _actorPathY;
	int16 _actorPathDeltaX, _actorPathDeltaY;

	/* BackgroundObject */
	int16 _backgroundObjectsResourceCacheSlot;
	ObjectStorage<BackgroundObject, kMaxBackgroundObjects> _backgroundObjects;
	Common::Array<BackgroundObject*> _backgroundObjectsDrawQueue;
	Common::Array<ActorSprite*> _actorSpriteDrawQueue;

	/* Path */
	PathSystem *_pathSystem;
	ObjectStorage<PathWalker, kMaxPathWalkers> _pathWalkers;

	/* Scene items */
	ObjectStorage<SceneItem, kMaxSceneItems> _sceneItems;

	/* Zones */
	ObjectStorage<Zone, kMaxZones> _zones;
	bool _zoneTextActive;
	bool _zoneMouseCursorActive;
	int16 _zoneMouseCursor;
	bool _exitZoneActionFlag;

	/* ZoneActions */
	ObjectStorage<ZoneAction, kMaxZoneActions> _zoneActions;
	int16 _currZoneActionIndex;
	int16 _inventoryItemIndex2;
	int16 _zoneIndexAtMouse;
	ZoneAction _queuedZoneAction;

	/* Dialog */
	ObjectStorage<Dialog, kMaxDialogs> _dialogs;
	int16 _currDialogIndex;
	int16 _selectedDialogIndex; // UNUSED?
	int16 _dialogXAdd, _dialogYAdd;
	int16 _dialogRectX1, _dialogRectY1, _dialogRectX2, _dialogRectY2;
	int16 _dialogPanelResourceCacheSlot;

	/* Active dialog */
	int16 _currDialogKeywordIndices[50];
	int16 _dialogActiveKeywordsCount;
	int16 _dialogMaxKeywordTextWidth;
	int16 _dialogFontHeight;
	bool _dialogRunning;
	int16 _selectedDialogKeywordIndex;
	bool _dialogFlag;

	/* ActorFrameSounds */
	int16 _actorFrameSoundItemsCount;
	ObjectStorage<ActorFrameSound, kMaxActorFrameSounds> _actorFrameSounds;

	/* Inventory */
	ObjectStorage<InventoryItem, kMaxInventoryItems> _inventoryItems;
	ObjectStorage<InventoryItemCombination, kMaxInventoryItemCombinations> _inventoryItemCombinations;
	int16 _inventoryItemsCount;
	int16 _inventoryItemSlotBaseIndex;
	int16 _inventoryItemsResourceCacheIndex;

	/* Inventory bar */
	bool _inventoryBarEnabled, _inventoryBarFlag;
	bool _buildInventoryClickBoxes;
	bool _inventoryWarpMouse;
	int16 _selectedInventoryItemIndex;
	int16 _inventoryClickBoxIndex;
	int16 _currInventoryItemSlotBaseIndex;
	bool _inventoryActive;
	int16 _inventoryBoxResourceCacheSlot;

	/* Fonts */
	ObjectStorage<Font, 4> _fonts;
	int16 _activeFontIndex;

	/* Screen text */
	ObjectStorage<ScreenText, kMaxScreenTexts> _screenTexts;
	bool _screenTextShowing;
	bool _screenTextHasSpeech;
	bool _autoAdvanceScreenTexts;
	bool _screenTextActive;

	/* Talkie */
	bool _talkieSpeechActive;
	Audio::SoundHandle _talkieSoundHandle;
	int16 _talkieDataResourceCacheSlot;
	bool _talkieSpeechPlayNow;
	Audio::AudioStream *_talkieSpeechAudioStream;

	/* LipSync */
	int16 _lipSyncScriptNumber;
	int16 _lipSyncResourceCacheSlot;
	int16 _lipSyncX, _lipSyncY;
	int16 _lipSyncActorIndex;
	uint32 _lipSyncTicks;
	int16 _lipSyncTime;
	AnimationResource *_lipSyncAnimationResource;
	Common::Array<LipSyncChannelStatus> _lipSyncChannelStatus;
	bool _lipSyncChannelStatusRestored;

	/* Mouse cursors */
	MouseCursor _mouseCursors[kMouseCursors];
	int16 _mouseCursorAnimationResourceCacheSlot;
	int16 _mouseCursorAnimationFrameListIndex;
	int16 _currAnimatedMouseCursor;
	int16 _mouseCursorAnimationFrameCount;
	int16 _mouseCursorAnimationCurrFrame;
	uint32 _mouseCursorAnimationLastUpdate;
	int16 _mouseCursorAnimationUpdateTicks;

	/* ClickBox */
	ObjectStorage<ClickBox, kMaxClickBoxes> _clickBoxes;

	/* Palette task */
	PaletteTask _paletteTasks[kMaxPaletteTasks];
	int16 _fadeInOutColorUpdateTicks, _fadeInOutColorPosition;
	int16 _alarmPaletteSub, _alarmPaletteSubDelta;

	/* Music */
	MidiPlayer *_midi;

	/* Font colors */
	FontColorDef _dialogFontColor, _dialogHoverFontColor, _inventoryFontColor,
		_zoneFontColor, _screenTextFontColor, _inventoryScreenTextFontColor;

public:
	int16 loadTextResource(Common::String &pakName, int16 pakSlot);
	void makeLanguageString(Common::String &value);
	int16 getGlobalScriptVar(int16 varIndex);
	void setGlobalScriptVar(int16 varIndex, int16 value);
	int16 getModuleScriptVar(int16 varIndex);
	void setModuleScriptVar(int16 varIndex, int16 value);
	int16 getSysVar(int16 varIndex);

	void playIntroVideos();
	void death();
	bool waitForInput();

	const Common::String getGlobalText(Common::String &identifier);
	void mainLoop();

	/* Frame time */
	void resetFrameValues();
	void initFrameTime();
	void updateFrameTime();
	void resetAnimationFrameTicks();
	void updateAnimationFrameTicks();

	/* Input low-level */
	void initInput();
	void getInputStatus(Common::KeyCode &keyState, uint16 &buttonState);
	void inpSetWaitRelease(bool value);
	void inpMouseSetWaitRelease(bool value);
	void inpKeybSetWaitRelease(bool value);

	/* Input */
	int16 handleInput(int16 x, int16 y);
	void setUserInput(bool enabled);
	void addDirtyRect(int16 x1, int16 y1, int16 x2, int16 y2, int16 flag);

	/* Scene control */
	void setEnterSceneScript(int16 programIndex, int16 scriptIndex);
	void setLeaveSceneScript(int16 programIndex, int16 scriptIndex);
	bool isEnterSceneScriptFinished();
	bool isLeaveSceneScriptFinished();
	void startModuleScript(Common::String &pakName, int16 pakSlot);
	void stopModuleScript();
	void updateModuleScript();
	void checkForSceneChange();
	void enterScene(int16 moduleIndex, int16 sceneIndex);
	void leaveScene();
	void gotoScene(int16 moduleIndex, int16 sceneIndex);
	void actorEntersScene(int16 prevModuleIndex, int16 prevSceneIndex, int16 nodeIndex,
		Common::String &pakName, int16 pakSlot, int16 frameListIndex, int16 scriptIndex);

	/* Script */
	void clearScriptPrograms();
	void loadScriptProgram(Common::String &pakName, int16 pakSlot, int16 programIndex);
	void clearScriptProgram(int16 programIndex);
	void unloadScriptProgram(int16 programIndex);
	void startScript(int16 programIndex, int16 scriptIndex);
	void stopScript(int16 programIndex, int16 scriptIndex);
	void stopScriptProgram(int16 programIndex);
	void runCurrentScript();
	void runInitScript(int16 programIndex);
	void runScripts(int16 programIndex);
	void startLocalScript(int16 scriptIndex);
	void stopLocalScript(int16 scriptIndex);

	/* Background */
	void loadBackground(Common::String &pakName, int16 pakSlot);
	void unloadBackground();
	void setBackground(Common::String &pakName, int16 pakSlot, int16 backgroundFlag);
	void clearBackground(int16 backgroundFlag);
	void initBackgroundDrawDimensions();
	void updateCameraPosition(int16 x, int16 y);
	void setBackgroundCameraLocked(bool value);
	void updateBackground(bool fullRedraw);
	void resetBackgroundValues();
	void updateCameraFollowing();
	void setCameraFollowsActor(int16 actorIndex);
	void updateScreen(bool fullRedraw, int16 x, int16 y);
	void updateCurrZone(int16 x, int16 y);

	/* Palette */
	void setPalette(Common::String &pakName, int16 pakSlot);
	void fadeInOutColor(byte *source, int16 fadeDirection, int16 fadeR, int16 fadeG, int16 fadeB, int16 fadePosition);
	void alarmPalette(byte *source);

	bool backupScreen();
	void restoreScreen(bool ignorePalette);

	/* Actors */
	int16 addActor(Common::String &pakName, int16 pakSlot, int16 frameListIndex, int16 nodeIndex,
		int16 x, int16 y);
	void clearActor(int16 actorIndex);
	void clearActors();
	void unloadActors();
	void restoreActorSprites();
	void setActorFontColors(int16 actorIndex, int16 outlineColor, int16 inkColor);
	void setMainActor(int16 actorIndex);
	void actorAssignPathWalker(int16 actorIndex);
	void actorWalkToPoint(int16 actorIndex, int16 x, int16 y);
	void actorWalkToPathNode(int16 actorIndex, int16 nodeIndex);
	void resetActorPathWalk(int16 actorIndex);
	void actor21C78();
	void actor21C89();
	void setActorDirection(int16 actorIndex, int16 direction);
	void setActorLookInDirection(int16 actorIndex);
	int16 getActorX(int16 actorIndex);
	int16 getActorY(int16 actorIndex);
	void setActorX(int16 actorIndex, int16 x);
	void setActorY(int16 actorIndex, int16 y);
	void actorPutAtPos(int16 actorIndex, int16 x, int16 y);
	void actorPutAtPathNode(int16 actorIndex, int16 nodeIndex);
	void actorAnimation218A1(int16 actorIndex, Common::String &pakName, int16 pakSlot,
		int16 firstFrameListIndex, int16 lastFrameListIndex, int16 minTicks, int16 maxTicks);
	void setActorRandomFrameListIndex(Actor *actor);
	void setActorAnimation(int16 actorIndex, Common::String &pakName, int16 pakSlot,
		int16 frameListIndex, int16 nodeIndex);
	void setActorAnimationAtPos(int16 actorIndex, Common::String &pakName, int16 pakSlot,
		int16 frameListIndex, int16 x, int16 y);
	void setActorSpriteFrameListIndexIfIdle(int16 actorIndex, int16 frameListIndex);
	void updateActors();
	void resetPathWalker(int16 pathWalkerIndex);
	bool updateActorSpriteWalking(int16 actorIndex, int16 prevX, int16 prevY, int16 newX, int16 newY);
	void backupActorAnimation(int16 actorIndex);
	void restoreActorAnimation(int16 actorIndex);

	/* Actor sprites */
	void assignActorSprite(int16 actorIndex, ActorSprite &actorSprite);
	void setActorSpriteFrameListIndex(ActorSprite *actorSprite, int16 frameListIndex, bool firstFrame);
	void calcAnimationFrameBounds(AnimationResource *animationResource, int16 elementIndex,
		int16 &boundsX1, int16 &boundsY1, int16 &boundsX2, int16 &boundsY2);
	void clearActorSprites();
	bool updateActorSpriteAnimation(int16 actorIndex);
	bool updateActorSpriteWalkingPosition(ActorSprite *actorSprite, int16 prevX, int16 prevY, int16 x, int16 y);
	void resetActorSpriteAnimationTicks(ActorSprite *actorSprite);

	/* Actor alt animations */
	int16 addActorAltAnimation(Common::String &pakName, int16 pakSlot);
	void unloadActorAltAnimation(int16 altActorAnimationIndex);
	void setActorAltAnimationAtPos(int16 actorIndex, int16 altAnimationIndex, int16 frameListIndex, int16 x, int16 y);

	/* BackgroundObject */
	void setBackgroundObjects(Common::String &pakName, int16 pakSlot);
	void unloadBackgroundObjects();

	void drawSprites(int16 xOffs, int16 yOffs);
	void buildActorSpriteDrawQueue();
	void drawBackgroundObject(int16 xOffs, int16 yOffs, BackgroundObject *backgroundObject);
	void drawActorSprite(int16 xOffs, int16 yOffs, ActorSprite *actorSprite);

	/* Path */
 	int16 calcDirection(int16 x1, int16 y1, int16 x2, int16 y2);

	/* Scene items */
	int16 addSceneItem(int16 inventoryItemIndex, int16 pathNodeIndex);
	void removeSceneItem(int16 inventoryItemIndex);
	// NOTE: clearSceneItems() -> _sceneItems.clear();
	void clearSceneItemActors();

	/* Zones */
	void clearActorZone(int16 actorIndex);
	void clearZone(int16 zoneIndex);
	void removeZone(int16 zoneIndex);
	void clearZones();
	int16 getFreeZoneIndex();
	int16 addZone(int16 x1, int16 y1, int16 x2, int16 y2, int16 mouseCursor, Common::String *pakName,
		int16 pakSlot, Common::String *identifier);
	int16 addActorZone(int16 actorIndex, int16 mouseCursor, Common::String &pakName,
		int16 pakSlot, Common::String &identifier);
	int16 addItemZone(int16 sceneItemIndex, int16 mouseCursor, Common::String &pakName,
		int16 pakSlot, Common::String &identifier);
	void drawZoneDescription(int16 zoneIndex);
	void updateZones(int16 x, int16 y);
	void checkQueuedZoneAction();
	bool isPointInZone(int16 x, int16 y, int16 zoneIndex);

	/* ZoneActions */
	int16 updateZoneActions(int16 zoneActionType);
	int16 addZoneAction(int16 zoneIndex, int16 type, int16 pathNodeIndex, int16 scriptIndex,
		int16 scriptProgIndex, int16 moduleIndex, int16 sceneIndex, int16 inventoryItemIndex);
	void clearZoneZoneActions(int16 zoneIndex);
	void clearZoneAction(int16 zoneActionIndex);
	void clearZoneActions();
	void setZoneActionScript(int16 zoneActionIndex, int16 scriptIndex);
	void initZonesAndZoneActions();
	int16 checkZoneAction(int16 zoneActionType);

	/* Dialog */
	void loadDialogKeywords(Common::String &pakName, int16 pakSlot, Dialog *dialog, bool init);
	int16 loadDialog(Common::String &pakName, int16 pakSlot);
	void unloadDialog(int16 dialogIndex);
	void refreshDialogKeywords(int16 dialogIndex);
	void resetDialogValues();
	void enableDialogKeyword(int16 dialogIndex, int16 keywordIndex);
	void disableDialogKeyword(int16 dialogIndex, int16 keywordIndex);
	void startDialog(int16 dialogIndex);
	void initDialog();
	void unloadDialogPanel();
	void updateDialog(int16 x, int16 y);
	bool isPointInDialogRect(int16 x, int16 y);

	/* ActorFrameSounds */
	void updateActorFrameSounds();
	int16 addActorFrameSound(int16 actorIndex, int16 soundIndex, int16 volume, int16 frameListIndex, int16 frameIndex);
	void removeActorFrameSound(int16 actorFrameSoundIndex);
	void clearActorFrameSoundsBySoundIndex(int16 soundIndex);
	void clearActorFrameSoundsByActorIndex(int16 actorIndex);
	void setActorFrameSound(int16 actorFrameSoundIndex, int16 soundIndex, int16 volume);
	void clearActorFrameSounds();

	/* Inventory */
	int16 registerInventoryItem(Common::String &pakName, int16 pakSlot, int16 id);
	void loadInventoryItemText(int16 inventoryItemIndex);
	int16 addInventoryItemCombination(int16 inventoryItem1, int16 inventoryItem2, int16 scriptIndex);
	void removeInventoryItemCombination(int16 combinationIndex);
	int16 getInventoryItemCombinationScript(int16 inventoryItem1, int16 inventoryItem2);
	void addItemToInventory(int16 inventoryItemIndex);
	void removeItemFromInventory(int16 inventoryItemIndex);
	void clearInventoryItems();
	void unloadInventoryItems();

	/* Inventory bar */
	void loadInventoryItemsAnimation(Common::String &pakName, int16 pakSlot, int16 slotBaseIndex);
	void updateInventoryItems();
	void handleInventoryInput();

	/* Fonts */
	int16 addFont(Common::String &pakName, int16 pakSlot, int16 outlineIndex, int16 inkIndex);
	void unloadFont(int16 fontIndex);
	void unloadFonts();
	void clearFonts();
	void getFontColors(int16 fontIndex, int16 &outlineColor, int16 &inkColor);
	void setFontColors(int16 fontIndex, int16 outlineColor, int16 inkColor);
	void setActiveFont(int16 fontIndex);
	// These work with the active font
	void drawText(int16 x, int16 y, const Common::String &text);
	void drawTextEx(int16 x1, int16 x2, int16 y1, int16 y2, const Common::String &text);
	int16 getTextWidth(const Common::String &text);
	int16 getActiveFontUnk1();
	int16 getActiveFontUnk2();
	int16 getActiveFontHeight();

	/* Screen text items */
	void setupScreenText(ScreenText *screenText, TextResource *textResource, int16 chunkIndex);
	void addScreenText(ScreenText *screenText, int16 resourceCacheSlot, Common::String &identifier);
	void unloadScreenText(int16 screenTextIndex);
	void unloadScreenTexts();
	int16 addActorScreenText(int16 actorIndex, int16 resourceCacheSlot, Common::String &identifier);
	int16 addLooseScreenText(int16 x, int16 y, int16 resourceCacheSlot, Common::String &identifier);
	int16 addCenteredScreenText(int16 resourceCacheSlot, Common::String &identifier);
	void updateScreenTexts();
	void advanceScreenTexts();
	bool isScreenTextShowing() const { return _screenTextShowing; }

	/* Talkie */
	void startTalkieSpeech(int16 pakSlot, uint32 &finishedTime, int16 resourceCacheSlot);
	void stopTalkieSpeech();
	void updateTalkieSpeech(int16 &pakSlot, uint32 &finishedTime, int16 resourceCacheSlot);
	void startLipSync(Common::String &pakName, int16 pakSlot, int16 scriptIndex, int16 actorIndex,
		int16 lipSyncX, int16 lipSyncY);
	void stopLipSync();
	void drawLipSyncFrame(int16 frameListIndex);
	bool isSoundEnabled() const { return true; } // Just a placeholder for now

	/* Mouse cursors */
	void loadMouseCursors();
	Graphics::Surface *decompressAnimationCel(AnimationCel *animationCel);
	void setMouseCursor(int16 elementIndex, AnimationResource *animationResource);
	void updateMouseCursor();
	void updateMouseCursorAnimation();
	void resetMouseCursorValues();

	/* ClickBox */
	int16 addClickBox(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag);
	int16 findClickBoxAtPos(int16 x, int16 y, int16 tag);
	bool isPointInClickBox(int16 clickBoxIndex, int16 x, int16 y);
	int16 getClickBoxTag(int16 clickBoxIndex);

	/* Palette task */
	void startPaletteTask(int16 type, int16 value1, int16 value2, int16 value3);
	void updatePaletteTasks();
	void clearPaletteTasks();

	void playMux(Common::String filename);
	bool handleMuxInput();
	void playMuxSoon(Common::String &filename, bool clearScreenAfter, bool clearScreenBefore);

	void requestAutoSave(Common::String &pakName, int16 pakSlot, Common::String &identifier);
	void performAutoSave();

	void getInteractMessage(Common::String &pakName, int16 pakSlot, Common::String &identifier,
		int16 &outResourceCacheSlot, Common::String &outIdentifier);

	void updateEvents();
	uint32 getTicks();

	/* Sounds */
	ObjectStorage<SoundSlot, kMaxSounds> _sounds;
	void playSound(int16 soundIndex);
	void playLoopingSound(int16 soundIndex, int16 loops);
	void stopSound(int16 soundIndex);
	bool isSoundPlaying(int16 soundIndex);
	void setSoundVolume(int16 soundIndex, uint volume);
	int16 loadSound(Common::String &pakName, int16 pakSlot, bool moduleWide);
	void unloadSound(int16 soundIndex);
	void unloadSounds(bool all);

	/* Music */
	void initializeMidi();
	void shutdownMidi();

	/* Font colors */
	void setDefaultTextDisplayColors();
	void setTextDisplayColor(int16 textDisplayNum, int16 outlineColor, int16 inkColor);

	/* Save/load */

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *description);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

public:

};

} // End of namespace Prisoner

#endif /* PRISONER_H */
