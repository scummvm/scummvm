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

#ifndef IGOR_ENGINE_H
#define IGOR_ENGINE_H

#include "common/array.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/str.h"
#include "common/util.h"

#include "sound/mixer.h"

#include "engines/engine.h"

#include "igor/resource_ids.h"

namespace Igor {

enum {
	kDebugEngine   = 1 << 0,
	kDebugResource = 1 << 1,
	kDebugScreen   = 1 << 2,
	kDebugWalk     = 1 << 3,
	kDebugGame     = 1 << 4
};

enum {
	kIdEngDemo100,
	kIdEngDemo110,
	kIdEngFloppy,
	kIdSpaFloppy,
	kIdEngCD,
	kIdSpaCD
};

enum {
	kFlagDemo   = 1 << 0,
	kFlagFloppy = 1 << 1,
	kFlagTalkie = 1 << 2
};

enum {
	kStartupPart = 900,
	kInvalidPart = 255,
	kSharewarePart = 950,
	kTalkColor = 240,
	kTalkShadowColor = 241,
	kTickDelay = 1193180 / 4096,
	kTimerTicksCount = 8,
	kQuickSaveSlot = 0,
	kMaxSaveStates = 10,
	kNoSpeechSound = 999
};

enum {
	kLanguageEnglish = 0,
	kLanguageSpanish = 1
};

enum {
	kTalkModeSpeechOnly = 0,
	kTalkModeSpeechAndText = 1,
	kTalkModeTextOnly = 2
};

enum Verb {
	kVerbWalk = 1,
	kVerbTalk,
	kVerbTake,
	kVerbLook,
	kVerbUse,
	kVerbOpen,
	kVerbClose,
	kVerbGive
};

enum ObjectType {
	kObjectTypeInventory = 1,
	kObjectTypeRoom = 2
};

enum FacingPosition {
	kFacingPositionBack = 1,
	kFacingPositionRight = 2,
	kFacingPositionFront = 3,
	kFacingPositionLeft = 4
};

enum {
	kUpdateDialogueAnimEndOfSentence = 1,
	kUpdateDialogueAnimMiddleOfSentence,
	kUpdateDialogueAnimStanding
};

enum InputVar {
	kInputSkipDialogue = 0,
	kInputCursorXPos,
	kInputCursorYPos,
	kInputClick,
	kInputEscape,
	kInputPause,
	kInputOptions,
	kInputVarCount
};

struct DetectedGameVersion {
	int version;
	int flags;
	Common::Language language;
	const char *ovlFileName;
	const char *sfxFileName;
};

struct DialogueText {
	int num;
	int count;
	int sound;
};

struct ResourceEntry {
	int id;
	uint32 offs;
	uint32 size;
};

struct StringEntry {
	int id;
	Common::String str;

	StringEntry() : id(0) {}
	StringEntry(int i, const char *s) : id(i), str(s) {}
};

struct RoomObjectArea {
	uint8 area;
	uint8 object;
	uint8 y1Lum;
	uint8 y2Lum;
	uint8 deltaLum;
};

struct RoomDataOffsets {
	struct {
		int box;
		int boxSize;
		int boxSrcSize;
		int boxDstSize;
	} area;
	struct {
		int walkPoints;
		int walkFacingPosition;
	} obj;
	struct {
		int defaultVerb;
		int useVerb;
		int giveVerb;
		int object2;
		int object1;
		int objectSize;
	} action;
	struct {
		int questionsOffset;
		int questionsSize;
		int repliesOffset;
		int repliesSize;
		int matSize;
	} dlg;
};

struct RoomWalkBounds {
	int x1, y1;
	int x2, y2;
};

struct WalkData {
	int16 x, y;
	uint8 posNum;
	uint8 frameNum;
	uint8 clipSkipX;
	int16 clipWidth;
	int16 scaleWidth;
	uint8 xPosChanged;
	int16 dxPos;
	uint8 yPosChanged;
	int16 dyPos;
	uint8 scaleHeight;

	void setPos(int xPos, int yPos, uint8 facingPos, uint8 frame) {
		x = xPos;
		y = yPos;
		posNum = facingPos;
		frameNum = frame;
	}

	void setDefaultScale() {
		clipSkipX = 1;
		clipWidth = 30;
		scaleWidth = 50;
		xPosChanged = 1;
		dxPos = 0;
		yPosChanged = 1;
		dyPos = 0;
		scaleHeight = 50;
	}

	void setScale(int w, int h) {
		scaleWidth = w;
		scaleHeight = h;
	}

	static void setNextFrame(uint8 pos, uint8 &frame) {
		switch (pos) {
		case kFacingPositionBack:
		case kFacingPositionFront:
			if (frame == 6) {
				frame = 1;
			} else {
				++frame;
			}
			break;
		case kFacingPositionLeft:
		case kFacingPositionRight:
			if (frame == 8) {
				frame = 1;
			} else {
				++frame;
			}
			break;
		}
	}
};

struct GameStateData {
	uint8 enableLight;
	int8 colorLum;
	int16 counter[5];
	bool igorMoving;
	bool dialogueTextRunning;
	bool updateLight;
	bool unkF;
	uint8 unk10;
	uint8 unk11;
	bool dialogueStarted;
	// byte[1]
	uint8 dialogueData[500];
	uint8 dialogueChoiceStart;
	uint8 dialogueChoiceCount;
	// byte[2]
	uint8 nextMusicCounter;
	bool jumpToNextMusic;
	uint8 configSoundEnabled;
	uint8 talkSpeed;
	uint8 talkMode;
	// byte[3]
	uint8 musicNum;
	uint8 musicSequenceIndex;
};

struct Action {
	uint8 verb;
	uint8 object1Num;
	uint8 object1Type;
	uint8 verbType; // 1:use,2:give
	uint8 object2Num;
	uint8 object2Type;
};

class TypeSerializer;
class MidiPlayer;

class IgorEngine: public Engine {
public:

	enum {
		MAX_DIALOGUE_TEXTS = 6,
		MAX_OBJECT_NAME_LENGTH = 31,
		MAX_DIALOGUE_TEXT_LENGTH = 51,
		MAX_VERB_NAME_LENGTH = 12,
		MAX_ROOM_OBJECT_AREAS = 256,
		MAX_DIALOGUE_QUESTIONS = 30,
		MAX_DIALOGUE_REPLIES = 70
	};

	typedef void (IgorEngine::*ExecuteActionProc)(int action);
	typedef void (IgorEngine::*UpdateDialogueProc)(int action);
	typedef void (IgorEngine::*UpdateRoomBackgroundProc)();

	IgorEngine(OSystem *system, const DetectedGameVersion *dgv);
	virtual ~IgorEngine();

	virtual Common::Error init();
	virtual Common::Error go();

	void handleOptionsMenu_paintSave();
	bool handleOptionsMenu_handleKeyDownSave(int key);
	void handleOptionsMenu_paintLoad();
	bool handleOptionsMenu_handleKeyDownLoad(int key);
	void handleOptionsMenu_paintQuit();
	bool handleOptionsMenu_handleKeyDownQuit(int key);
	void handleOptionsMenu_paintCtrl();
	bool handleOptionsMenu_handleKeyDownCtrl(int key);

protected:

	bool compareGameTick(int add, int mod) const { return ((_gameTicks + (add & ~7)) % mod) == 0; } // { return ((_gameTicks + add) % mod) == 0; }
	bool compareGameTick(int eq) const { return _gameTicks == (eq & ~7); } // { return _gameTicks == eq; }
	int getPart() const { return _currentPart / 10; }
	void readTableFile();
	const char *getString(int id) const;
	void restart();
	void waitForTimer(int ticks = -1);
	void copyArea(uint8 *dst, int dstOffset, int dstPitch, const uint8 *src, int srcPitch, int w, int h, bool transparent = false);
	int getRandomNumber(int m);
	void handleOptionsMenu();
	void handlePause();
	void startMusic(int cmf);
	void playMusic(int num);
	void updateMusic();
	void playSound(int num, int type);
	void stopSound();
	void loadIgorFrames();
	void loadIgorFrames2();
	void fixDialogueTextPosition(int num, int count, int *x, int *y);
	void startCutsceneDialogue(int x, int y, int r, int g, int b);
	void waitForEndOfCutsceneDialogue(int x, int y, int r, int g, int b);
	void fixIgorDialogueTextPosition(int num, int count, int *x, int *y);
	void startIgorDialogue();
	void waitForEndOfIgorDialogue();
	int getObjectFromInventory(int x) const;
	ResourceEntry *findData(int num);
	uint8 *loadData(int num, uint8 *dst = 0, int *size = 0);
	void decodeMainText(const uint8 *p);
	void decodeRoomStrings(const uint8 *p, bool skipObjectNames = false);
	void decodeRoomText(const uint8 *p);
	void decodeRoomAreas(const uint8 *p, int count);
	void decodeRoomMask(const uint8 *p);
	void loadRoomData(int pal, int img, int box, int msk, int txt);
	void loadAnimData(const int *anm, int loadOffset = 0);
	void loadActionData(int act);
	void loadDialogueData(int dlg);
	void loadMainTexts();
	void setupDefaultPalette();
	void updatePalette(int count);
	void clearPalette();
	void setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b);
	void setPaletteRange(int startColor, int endColor);
	void fadeInPalette(int count);
	void fadeOutPalette(int count);
	void scrollPalette(int startColor, int endColor);
	void drawChar(uint8 *dst, int chr, int x, int y, int color1, int color2, int color3);
	void drawString(uint8 *dst, const char *s, int x, int y, int color1, int color2 = -1, int color3 = -1);
	int getStringWidth(const char *s) const;
	void drawActionSentence(const char *sentence, uint8 color);
	void formatActionSentence(uint8 color);
	const uint8 *getAnimFrame(int baseOffset, int tableOffset, int frame);
	void decodeAnimFrame(const uint8 *src, uint8 *dst, bool preserveText = false);
	void copyAnimFrame(int srcOffset, int frame, int frameSize, int w, int h, int dstOffset);
	void setCursor(int num);
	void showCursor();
	void hideCursor();
	void updateRoomLight(int fl);
	void drawVerbsPanel();
	void redrawVerb(uint8 verb, bool highlight);
	int getVerbUnderCursor(int x) const { return ((x % 46) < 44) ? (kVerbTalk + x / 46) : 0; }
	void drawInventory(int start, int mode);
	void packInventory();
	void scrollInventory();
	void addObjectToInventory(int object, int index);
	void removeObjectFromInventory(int index);
	void executeAction(int action);
	void clearAction();
	void handleRoomInput();
	void animateIgorTalking(int frame);
	void handleRoomDialogue();
	void handleRoomIgorWalk();
	void handleRoomInventoryScroll();
	void handleRoomLight();
	void enterPartLoop();
	void leavePartLoop();
	void runPartLoop();
	int lookupScale(int xOffset, int yOffset, int h) const;
	void moveIgor(int pos, int frame);
	void buildWalkPathSimple(int srcX, int srcY, int dstX, int dstY);
	void getClosestAreaTrianglePoint(int dstArea, int srcArea, int *dstY, int *dstX, int srcY, int srcX);
	void getClosestAreaTrianglePoint2(int dstArea, int srcArea, int *dstY, int *dstX, int srcY1, int srcX1, int srcY2, int srcX2);
	void buildWalkPath(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathArea(int srcX, int srcY, int dstX, int dstY);
	int getVerticalStepsCount(int minX, int minY, int maxX, int maxY);
	int getHorizontalStepsCount(int minX, int minY, int maxX, int maxY);
	void lookupScale(int curX, int curY, uint8 &scale, uint8 &xScale, uint8 &yScale) const;
	void buildWalkPathAreaUpDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaDownDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaRightDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaLeftDirection(int srcX, int srcY, int dstX, int dstY);
	void waitForIgorMove();
	void setRoomWalkBounds(int x1, int y1, int x2, int y2);
	void fixWalkPosition(int *x, int *y);
	void recolorDialogueChoice(int num, bool highlight);
	void handleDialogue(int x, int y, int r, int g, int b);
	void drawDialogueChoices();
	int selectDialogue();
	void dialogueAskQuestion();
	void dialogueReplyToQuestion(int x, int y, int r, int g, int b, int reply = 0);

	void saveOrLoadGameState(TypeSerializer &typeSerializer);
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot);
	void generateGameStateFileName(int num, char *dst, int len) const;

	MidiPlayer *_midiPlayer;

	Common::RandomSource _rnd;

	Common::File _ovlFile;
	Common::File _sndFile;

	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _speechHandle;

	uint8 *_screenVGA;
	uint8 *_facingIgorFrames[4];
	uint8 *_screenLayer1;
	uint8 *_screenLayer2;
	uint8 *_screenTextLayer;
	uint8 *_screenTempLayer;
	uint8 *_igorHeadFrames;
	uint8 *_animFramesBuffer;
	uint8 *_inventoryPanelBuffer;
	uint8 *_inventoryImagesBuffer;
	uint8 *_verbsPanelBuffer;
	int _screenVGAVOffset;

	bool _eventQuitGame;
	GameStateData _gameState;
	uint32 _nextTimer;
	bool _fastMode;
	int _language;
	DetectedGameVersion _game;

	WalkData _walkData[100];
	uint8 _walkCurrentPos;
	uint8 _walkDataLastIndex;
	uint8 _walkDataCurrentIndex;
	uint8 _walkCurrentFrame;
	int _walkDataCurrentPosX, _walkDataCurrentPosY;
	int _walkToObjectPosX, _walkToObjectPosY;

	int16 _currentPart;
	int _talkDelay;
	int _talkSpeechCounter;
	int _talkDelayCounter;
	DialogueText _dialogueTextsTable[MAX_DIALOGUE_TEXTS];
	int _dialogueTextsStart;
	int _dialogueTextsCount;
	int _dialogueDirtyRectY;
	int _dialogueDirtyRectSize;
	char _dialogueQuestions[MAX_DIALOGUE_QUESTIONS][2][41];
	char _dialogueReplies[MAX_DIALOGUE_REPLIES][51];
	bool _dialogueEnded;
	int _dialogueChoiceSelected;
	uint8 _dialogueInfo[6];

	uint8 _objectsState[112];
	uint8 _inventoryImages[36];
	uint8 _inventoryInfo[74];
	char _verbPrepositions[3][7];
	char _globalObjectNames[35][MAX_OBJECT_NAME_LENGTH];
	char _globalDialogueTexts[300][MAX_DIALOGUE_TEXT_LENGTH];
	char _verbsName[9][MAX_VERB_NAME_LENGTH];
	char _roomObjectNames[20][MAX_OBJECT_NAME_LENGTH];
	uint8 _currentPalette[768];
	uint8 _paletteBuffer[768];
	uint8 _igorPalette[48];
	uint8 *_igorTempFrames;
	uint8 _walkXScaleRoom[320];
	uint8 _walkYScaleRoom[144 * 3];
	RoomObjectArea _roomObjectAreasTable[MAX_ROOM_OBJECT_AREAS];
	uint8 _roomActionsTable[0x2000];
	ExecuteActionProc _executeMainAction;
	ExecuteActionProc _executeRoomAction;
	uint8 _previousMusic;
	uint8 *_musicData;
	Action _currentAction;
	uint8 _actionCode;
	uint8 _actionWalkPoint;
	int16 _inputVars[kInputVarCount];
	bool _scrollInventory;
	int _scrollInventoryStartY, _scrollInventoryEndY, _scrollInventoryDy;
	bool _roomCursorOn;
	int _currentCursor;
	bool _dialogueCursorOn;
	RoomDataOffsets _roomDataOffsets;
	RoomWalkBounds _roomWalkBounds;
	UpdateDialogueProc _updateDialogue;
	UpdateRoomBackgroundProc _updateRoomBackground;
	int _demoActionsCounter;
	int _gameTicks;
	int _resourceEntriesCount;
	ResourceEntry *_resourceEntries;
	int _soundOffsetsCount;
	uint32 *_soundOffsets;
	Common::Array<StringEntry> _stringEntries;
	char _saveStateDescriptions[kMaxSaveStates][100];

	static const uint8 _dialogueColor[];
	static const uint8 _sentenceColorIndex[];
	static const uint8 _fontCharIndex[];
	static const uint8 _fontCharWidth[];
	static const uint32 _fontData[];
	static const uint8 _talkDelays[];
	static const uint8 _inventoryOffsetTable[];
	static const uint8 _inventoryActionsTable[];
	static const uint8 _walkWidthScaleTable[];
	static const float _walkScaleSpeedTable[];
	static const uint8 _walkScaleTable[];
	static const uint8 _mouseCursorMask[];
	static const uint8 _mouseCursorData[];


	//
	// game logic
	//

	// main loop
	void ADD_DIALOGUE_TEXT(int num, int count, int sound = kNoSpeechSound);
	void SET_DIALOGUE_TEXT(int start, int count);
	void SET_EXEC_ACTION_FUNC(int i, ExecuteActionProc p);
	void EXEC_MAIN_ACTION(int action);
	void EXEC_MAIN_ACTION_38();
	void EXEC_MAIN_ACTION_43();
	void EXEC_MAIN_ACTION_54();
	void CHECK_FOR_END_OF_DEMO();
	void SET_PAL_208_96_1();
	void SET_PAL_240_48_1();
	void UPDATE_OBJECT_STATE(int num);
	void PART_MEANWHILE();
	void PART_MARGARET_ROOM_CUTSCENE_HELPER_1();
	void PART_MARGARET_ROOM_CUTSCENE_HELPER_2(int frame);
	void PART_MARGARET_ROOM_CUTSCENE_UPDATE_DIALOGUE_MARGARET(int action);
	void PART_MARGARET_ROOM_CUTSCENE();
	void PART_UPDATE_FIGURES_ON_PAPER(int delay);
	void PART_MAIN();

	// college map
	void PART_04_EXEC_ACTION(int action);
	void PART_04_CLEAR_OBJECT_STATE_84(int num);
	void PART_04();

	// bridge
	void PART_05_EXEC_ACTION(int action);
	void PART_05_ACTION_102();
	void PART_05_ACTION_103();
	void PART_05_UPDATE_ROOM_BACKGROUND();
	void PART_05_HELPER_1();
	void PART_05_HELPER_2();
	void PART_05_HELPER_3(int frame);
	void PART_05_HELPER_4(int num);
	void PART_05_HELPER_5(int frame);
	void PART_05_HELPER_6();
	void PART_05();

	// bridge (rock)
	void PART_06_EXEC_ACTION(int action);
	void PART_06_ACTION_102();
	void PART_06_ACTION_103();
	void PART_06_ACTION_105();
	void PART_06_ACTION_107();
	void PART_06_ACTION_108();
	void PART_06_UPDATE_DIALOGUE_PHOTOGRAPHER(int action);
	void PART_06_HANDLE_DIALOGUE_PHOTOGRAPHER();
	void PART_06_UPDATE_ROOM_BACKGROUND();
	void PART_06_HELPER_1(int frame);
	void PART_06_HELPER_2();
	void PART_06_HELPER_3();
	void PART_06_HELPER_6(int num);
	void PART_06_HELPER_7(int frame);
	void PART_06_HELPER_8(int frame);
	void PART_06_HELPER_12();
	void PART_06_HELPER_13(int frame);
	void PART_06_HELPER_14();
	void PART_06_HELPER_15(int frame);
	void PART_06();

	// outside church
	void PART_12_EXEC_ACTION(int action);
	void PART_12_ACTION_101();
	void PART_12_ACTION_104();
	void PART_12_ACTION_105();
	void PART_12_ACTION_108();
	void PART_12_UPDATE_ROOM_BACKGROUND();
	void PART_12_UPDATE_DIALOGUE_CHURCHMAN(int action);
	void PART_12_HANDLE_DIALOGUE_CHURCHMAN();
	void PART_12_HELPER_1(int num);
	void PART_12_HELPER_2();
	void PART_12_HELPER_3();
	void PART_12_HELPER_4();
	void PART_12_HELPER_5();
	void PART_12_HELPER_6();
	void PART_12_HELPER_8();
	void PART_12_HELPER_9();
	void PART_12_HELPER_10(int frame);
	void PART_12();

	// inside church
	void PART_13_EXEC_ACTION(int action);
	void PART_13_ACTION_101_103();
	void PART_13_ACTION_104();
	void PART_13_HELPER_1(int num);
	void PART_13_HELPER_2();
	void PART_13_HELPER_3();
	void PART_13();

	// church puzzle
	void PART_14_EXEC_ACTION(int action);
	void PART_14_UPDATE_ROOM_BACKGROUND_ACTION_108();
	void PART_14_ACTION_101();
	void PART_14_ACTION_103();
	void PART_14_ACTION_105();
	void PART_14_ACTION_106();
	void PART_14_ACTION_108();
	void PART_14_HELPER_1(int num);
	void PART_14_HELPER_2();
	void PART_14_HELPER_3();
	void PART_14_HELPER_4();
	void PART_14_HELPER_6();
	void PART_14_HELPER_7(int frame);
	void PART_14_HELPER_8(int start, int end);
	void PART_14_HELPER_9();
	void PART_14_HELPER_10();
	void PART_14_PUSH_STONE(int screenOffset, int w, int h, int animOffset);
	void PART_14();
	void loadResourceData__ROOM_ChurchPuzzle();
	void loadResourceData__ANIM_ChurchPuzzle();

	// tobias office
	void PART_15_EXEC_ACTION(int action);
	void PART_15_ACTION_101();
	void PART_15_ACTION_107();
	void PART_15_ACTION_115();
	void PART_15_ACTION_116();
	void PART_15_UPDATE_ROOM_BACKGROUND();
	void PART_15_UPDATE_DIALOGUE_TOBIAS(int action);
	void PART_15_HANDLE_DIALOGUE_TOBIAS();
	void PART_15_HELPER_1(int num);
	void PART_15_HELPER_2();
	void PART_15_HELPER_3();
	void PART_15_HELPER_5();
	void PART_15_HELPER_6(int frame);
	void PART_15_HELPER_7(int frame);
	void PART_15_HELPER_8(int frame);
	void PART_15_HELPER_9(int frame);
	void PART_15_HELPER_10(int frame);
	void PART_15();

	// laboratory
	void PART_16_EXEC_ACTION(int action);
	void PART_16_ACTION_101();
	void PART_16_UPDATE_DIALOGUE_MARGARET_HARRISON(int action);
	void PART_16_UPDATE_DIALOGUE_MARGARET(int action);
	void PART_16_HELPER_1(int num);
	void PART_16_HELPER_2();
	void PART_16_HELPER_3();
	void PART_16_HELPER_5();
	void PART_16_HELPER_6(int frame);
	void PART_16();
	void loadResourceData__ROOM_Laboratory();
	void loadResourceData__ANIM_Laboratory();

	// outside college
	void PART_17_EXEC_ACTION(int action);
	void PART_17_ACTION_101();
	void PART_17_ACTION_103();
	void PART_17_ACTION_105();
	void PART_17_ACTION_106();
	void PART_17_HANDLE_DIALOGUE_PHILIP();
	void PART_17_UPDATE_DIALOGUE_PHILIP(int action);
	void PART_17_UPDATE_DIALOGUE_PHILIP_JIMMY(int action);
	void PART_17_UPDATE_ROOM_BACKGROUND();
	void PART_17_HELPER_1(int num);
	void PART_17_HELPER_2();
	void PART_17_HELPER_3(int lum);
	void PART_17_HELPER_4();
	void PART_17_HELPER_5(int lum);
	void PART_17_HELPER_6();
	void PART_17_HELPER_8(int num);
	void PART_17_HELPER_9(int num);
	void PART_17_HELPER_10();
	void PART_17_HELPER_11(int frame);
	void PART_17();

	// men toilets
	void PART_18_EXEC_ACTION(int action);
	void PART_18_ACTION_109();
	void PART_18_ACTION_111();
	void PART_18_HELPER_1(int num);
	void PART_18_HELPER_2();
	void PART_18();
	void loadResourceData__ROOM_MenToilets();
	void loadResourceData__ANIM_MenToilets();

	// women toilets
	void PART_19_EXEC_ACTION(int action);
	void PART_19_ACTION_107();
	void PART_19_ACTION_109();
	void PART_19_UPDATE_DIALOGUE_WOMEN(int action);
	void PART_19_UPDATE_BACKGROUND_HELPER_9();
	void PART_19_HELPER_1(int num);
	void PART_19_HELPER_2();
	void PART_19_HELPER_3();
	void PART_19_HELPER_4();
	void PART_19_HELPER_5();
	void PART_19_HELPER_7(int frame);
	void PART_19();

	// college corridor margaret
	void PART_21_EXEC_ACTION(int action);
	void PART_21_ACTION_101();
	void PART_21_ACTION_102();
	void PART_21_ACTION_107();
	void PART_21_ACTION_108();
	void PART_21_ACTION_110();
	void PART_21_ACTION_111();
	void PART_21_ACTION_113();
	void PART_21_UPDATE_ROOM_BACKGROUND();
	void PART_21_UPDATE_DIALOGUE_MARGARET_1(int action);
	void PART_21_UPDATE_DIALOGUE_MARGARET_2(int action);
	void PART_21_UPDATE_DIALOGUE_MARGARET_3(int action);
	void PART_21_HANDLE_DIALOGUE_MARGARET();
	void PART_21_HELPER_1(int num);
	void PART_21_HELPER_2();
	void PART_21_HELPER_3();
	void PART_21_HELPER_4();
	void PART_21_HELPER_6(int frame);
	void PART_21_HELPER_7();
	void PART_21_HELPER_8();
	void PART_21_HELPER_9();
	void PART_21_HELPER_10();
	void PART_21_HELPER_11(int frame);
	void PART_21();

	// bell church
	void PART_22_EXEC_ACTION(int action);
	void PART_22_ACTION_101();
	void PART_22_ACTION_102();
	void PART_22_HELPER_1(int num);
	void PART_22_HELPER_2();
	void PART_22();

	// college corridor lucas
	void PART_23_EXEC_ACTION(int action);
	void PART_23_ACTION_105();
	void PART_23_ACTION_107();
	void PART_23_ACTION_108();
	void PART_23_UPDATE_ROOM_BACKGROUND();
	void PART_23_HELPER_1(int num);
	void PART_23_HELPER_2(int frame);
	void PART_23_HELPER_3();
	void PART_23_HELPER_4();
	void PART_23_HELPER_5();
	void PART_23_HELPER_6();
	void PART_23_HELPER_7(int frame);
	void PART_23_HELPER_8(int frame);
	void PART_23();

	// college corridor sharon michael
	void PART_24_EXEC_ACTION(int action);
	void PART_24_ACTION_102();
	void PART_24_ACTION_104();
	void PART_24_ACTION_105();
	void PART_24_ACTION_107();
	void PART_24_UPDATE_ROOM_BACKGROUND();
	void PART_24_HELPER_1(int num);
	void PART_24_HELPER_2(int frame);
	void PART_24_HELPER_3(int frame);
	void PART_24_HELPER_4();
	void PART_24_HELPER_5();
	void PART_24_HELPER_7();
	void PART_24_HELPER_8();
	void PART_24_HELPER_9();
	void PART_24();

	// college corridor announcement board
	void PART_25_EXEC_ACTION(int action);
	void PART_25_ACTION_105();
	void PART_25_ACTION_107();
	void PART_25_ACTION_108();
	void PART_25_HELPER_1(int num);
	void PART_25_HELPER_2();
	void PART_25_HELPER_3();
	void PART_25_HELPER_4();
	void PART_25_HELPER_5();
	void PART_25_HELPER_7();
	void PART_25();

	// college corridor miss barrymore
	void PART_26_EXEC_ACTION(int action);
	void PART_26_ACTION_103();
	void PART_26_ACTION_104();
	void PART_26_ACTION_107();
	void PART_26_UPDATE_ROOM_BACKGROUND();
	void PART_26_HELPER_1(int num);
	void PART_26_HELPER_2();
	void PART_26_HELPER_3();
	void PART_26_HELPER_4();
	void PART_26_HELPER_5();
	void PART_26_HELPER_7(int frame);
	void PART_26();

	// college lockers
	void PART_27_EXEC_ACTION(int action);
	void PART_27_ACTION_106();
	void PART_27_ACTION_107();
	void PART_27_ACTION_108();
	void PART_27_ACTION_110();
	void PART_27_HELPER_1(int num);
	void PART_27_HELPER_2();
	void PART_27_HELPER_3();
	void PART_27_HELPER_4();
	void PART_27_HELPER_5();
	void PART_27();

	// college corridor caroline
	void PART_28_EXEC_ACTION(int action);
	void PART_28_ACTION_108();
	void PART_28_ACTION_109();
	void PART_28_UPDATE_DIALOGUE_CAROLINE(int action);
	void PART_28_UPDATE_ROOM_BACKGROUND();
	void PART_28_HELPER_1(int num);
	void PART_28_HELPER_2();
	void PART_28_HELPER_3();
	void PART_28_HELPER_5(int frame);
	void PART_28_HELPER_6();
	void PART_28_HELPER_8(int frame);
	void PART_28();

	// college corridor stairs first floor
	void PART_30_EXEC_ACTION(int action);
	void PART_30_ACTION_102();
	void PART_30_ACTION_104();
	void PART_30_UPDATE_DIALOGUE_LAURA(int action);
	void PART_30_HANDLE_DIALOGUE_LAURA();
	void PART_30_HELPER_1(int num);
	void PART_30_HELPER_2();
	void PART_30_HELPER_3();
	void PART_30_HELPER_4();
	void PART_30_HELPER_5();
	void PART_30_HELPER_8();
	void PART_30_HELPER_9(int frame);
	void PART_30();

	// college corridor stairs second floor
	void PART_31_EXEC_ACTION(int action);
	void PART_31_ACTION_102();
	void PART_31_ACTION_103();
	void PART_31_ACTION_106();
	void PART_31_ACTION_110();
	void PART_31_UPDATE_ROOM_BACKGROUND();
	void PART_31_HELPER_1(int num);
	void PART_31_HELPER_2(int frame);
	void PART_31_HELPER_3();
	void PART_31_HELPER_4();
	void PART_31_HELPER_5();
	void PART_31_HELPER_6();
	void PART_31_HELPER_9();
	void PART_31();

	// library
	void PART_33_EXEC_ACTION(int action);
	void PART_33_ACTION_109();
	void PART_33_ACTION_111();
	void PART_33_ACTION_113();
	void PART_33_ACTION_114();
	void PART_33_ACTION_115();
	void PART_33_UPDATE_DIALOGUE_HARRISON_1(int action);
	void PART_33_UPDATE_DIALOGUE_HARRISON_2(int action);
	void PART_33_UPDATE_DIALOGUE_HARRISON_3(int action);
	void PART_33_HANDLE_DIALOGUE_HARRISON();
	void PART_33_UPDATE_ROOM_BACKGROUND();
	void PART_33_HELPER_1(int num);
	void PART_33_HELPER_2();
	void PART_33_HELPER_3();
	void PART_33_HELPER_4(int frame);
	void PART_33_HELPER_5(int frame);
	void PART_33_HELPER_7();
	void PART_33_HELPER_8(int frame);
	void PART_33_HELPER_9();
	void PART_33();
	void loadResourceData__ROOM_Library();
	void loadResourceData__ANIM_Library();

	// chemistry classroom
	void PART_36_EXEC_ACTION(int action);
	void PART_36_ACTION_102();
	void PART_36_HELPER_1(int num);
	void PART_36_HELPER_2();
	void PART_36_HELPER_4(int frame);
	void PART_36_HELPER_5(int *x, int *y);
	void PART_36();

	// physics classroom
	void PART_37_EXEC_ACTION(int action);
	void PART_37_ACTION_102();
	void PART_37_HELPER_1(int num);
	void PART_37_HELPER_2();
	void PART_37();

	// philip vodka cutscene
	void PART_75_UPDATE_DIALOGUE_PHILIP(int action);
	void PART_75_HELPER_1(int frame);
	void PART_75();

	// introduction cutscene
	void PART_85_UPDATE_ROOM_BACKGROUND();
	void PART_85_UPDATE_DIALOGUE_PHILIP_LAURA(int action);
	void PART_85_HELPER_1(int frameOffset2, int frameOffset1, int firstFrame, int lastFrame, int delay);
	void PART_85_HELPER_2();
	void PART_85_HELPER_4();
	void PART_85_HELPER_6(int frame);
	void PART_85();

	// startup screens
	void PART_90();

	// shareware screens
	void PART_95();


	static const uint8 PAL_48_1[];
	static const uint8 PAL_96_1[];
	static const uint8 PAL_IGOR_1[];
	static const uint8 PAL_IGOR_2[];
	static const uint8 INVENTORY_IMG_INIT[];
	static const RoomDataOffsets PART_04_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_05_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_06_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_12_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_13_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_14_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_15_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_16_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_17_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_18_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_19_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_21_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_22_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_23_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_24_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_25_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_26_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_27_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_28_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_30_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_31_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_33_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_36_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_37_ROOM_DATA_OFFSETS;
};

} // namespace Igor

#endif
