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

#ifndef IGOR_H
#define IGOR_H

#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"

#include "igor/detection.h"
#include "igor/font.h"
#include "igor/resource_ids.h"

namespace Igor {

struct IgorGameDescription;

enum {
	kFlagDemo = 1 << 0,
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
	kFastModeFactor = 4,
	kFastModeMaxFactor = 16,
	kQuickSaveSlot = 100,
	kMaxSaveStates = 10,
	kNoSpeechSound = 999
};

enum {
	MAX_DIALOGUE_TEXTS = 6,
	MAX_OBJECT_NAME_LENGTH = 31,
	MAX_DIALOGUE_TEXT_LENGTH = 51,
	MAX_VERB_NAME_LENGTH = 12,
	MAX_ROOM_OBJECT_AREAS = 256,
	MAX_DIALOGUE_QUESTIONS = 30,
	MAX_DIALOGUE_REPLIES = 100
};

enum FacingPosition {
	kFacingPositionBack = 1,
	kFacingPositionRight = 2,
	kFacingPositionFront = 3,
	kFacingPositionLeft = 4
};

enum {
	kTalkModeSpeechOnly = 0,
	kTalkModeSpeechAndText = 1,
	kTalkModeTextOnly = 2
};

enum InputVar {
	kInputSkipDialogue = 0,
	kInputCursorXPos,
	kInputCursorYPos,
	kInputClick,
	kInputEscape,
	kInputPause,
	kInputOptions,
	kInputRightClick,
	kInputRightRelease,
	kInputRightMoveLeft,
	kInputRightMoveRight,
	kInputVarCount
};

struct RoomWalkBounds {
	int x1, y1;
	int x2, y2;
	// Minimum y after clamping a click past the corresponding horizontal edge.
	int x1MinY, x2MinY;
};

struct DetectedGameVersion {
	int version;
	int flags;
	Common::Language language;
	const char *ovlFileName;
	const char *sfxFileName;
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
	uint8 dialogueData[583];
	uint8 dialogueChoiceStart;
	uint8 dialogueChoiceCount;
	// byte[2]
	uint8 shouldShowCutsceneCounter;
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

struct DialogueDataOffsets {
	int questionsOffset;
	int questionsSize;
	int repliesOffset;
	int repliesSize;
	int matSize;
	int replyDataOffset;
	int questionSoundsOffset;
	int replySoundsOffset;
	int replySoundsSize;
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
	DialogueDataOffsets dlg;
};

// The click fix of a room that clamps the clicked position: y is limited, x is limited to [xMin, xMax]
// (-1: no limit), then the first walkable row below is looked for (and above if scanUp).
struct RoomClickFix {
	int yMax;
	int xMin;
	int xMax;
	bool scanUp;
	bool enabled;
};

// The maze: a grid of locations, each with the location to the north, east, south and west (0: none)
// and the shape of the room that shows it (the part of the room is 50 + shape).
struct MazeNode {
	uint8 neighbors[4];
	uint8 shape;
};

enum MazeDirection {
	kMazeNorth,
	kMazeEast,
	kMazeSouth,
	kMazeWest
};

enum MazeEntryKind {
	kMazeEntryWalk,     // Igor appears at (x, y) and walks to (destX, destY)
	kMazeEntryStairs    // Igor walks into the picture at x, scaling down
};

struct MazeEntry {
	uint16 state;
	uint8 kind;
	int16 x, y;
	uint8 facing;
	int16 destX, destY;
};

enum MazeActionKind {
	kMazeActionExit,        // walk (x1, y1) -> (x2, y2), then to the neighbour location
	kMazeActionStairsExit,  // walk out of the picture at x, scaling up, then to the neighbour location
	kMazeActionExitFixed,   // walk, then to a fixed location and state
	kMazeActionDialogue
};

struct MazeDialogueLine {
	uint16 text;
	uint8 count;
	uint16 sound;
};

struct MazeAction {
	uint8 code;
	uint8 kind;
	int16 x1, y1, x2, y2;
	uint8 dir;
	int16 stairsX;
	int16 location;     // -1: unchanged
	int16 state;
	int8 objectState;   // -1: none
	int8 objectStateValue;
	const MazeDialogueLine *lines;
	uint8 numLines;
	uint8 dialogueStart;
	uint8 dialogueCount;
};

enum MazeFlameMode {
	kMazeFlameNone,
	kMazeFlameFixed,        // one flame at a fixed position, color 1 shows the first frame
	kMazeFlameFirstFrame,   // color 1 shows the first frame
	kMazeFlameLayer1        // color 1 shows the next pixel of the room
};

struct MazeFlame {
	int16 x, y;
};

struct MazeRoom {
	uint8 part;
	int dat, txt, img, pal, msk, box;
	int8 music;             // -1: depends on the location
	uint8 darkness;
	uint8 flameMode;
	uint8 numFlames;
	MazeFlame flames[2];
	uint8 flameFrames[2];   // frames drawn when the room starts
	uint8 secondFlameFrame; // the frame the second flame counts as having at the start
	bool saveLocation;      // the room ends when the location changes
	const RoomDataOffsets *offsets;
	int giveObjectSize;     // stride of the give matrix when it is not the one of the use matrix
	RoomClickFix clickFix;
	const MazeEntry *entries;
	uint8 numEntries;
	const MazeAction *actions;
	uint8 numActions;
	const char *objectName3;
};
enum {
	kUpdateDialogueAnimEndOfSentence = 1,
	kUpdateDialogueAnimMiddleOfSentence,
	kUpdateDialogueAnimStanding
};

enum ObjectType {
	kObjectTypeInventory = 1,
	kObjectTypeRoom = 2
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

enum DebugOverlay {
	kOverlayOff = 0,
	kOverlayWalkAreas,
	kOverlayHotspots,
	kOverlayY1Lum,
	kOverlayY2Lum,
	kOverlayDeltaLum
};

enum {
	kIdEngDemo100,
	kIdEngDemo110,
	kIdEngFloppy,
	kIdSpaFloppy,
	kIdEngCD,
	kIdSpaCD
};

struct DialogueText {
	int num;
	int count;
	int sound;
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

enum AnimBlend {
		kBlendCopy,
		kBlendLitSprite,
		kBlendLitSpriteNoShade,
		kBlendBehindIgor,
		kBlendBehindIgorAndText
};

class IgorEngine : public Engine {
public:
	typedef void (IgorEngine::*ExecuteActionProc)(int action);
	typedef void (IgorEngine::*UpdateRoomBackgroundProc)();


	typedef void (IgorEngine::*UpdateDialogueProc)(int action);

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	// MidiPlayer *_midiPlayer;

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

	uint8 *_debugOverlayBuffer;
	int _debugOverlayMode;

	bool _eventQuitGame;
	bool _gameStateLoaded;
	GameStateData _gameState;
	uint8 _mazeLocation;
	uint8 _mazeSavedLocation;
	const MazeRoom *_mazeRoom;
	int _mazeFlameFrame[2];
	uint32 _nextTimer;

	// Speed multiplier, 1 = original timing. Divides the millisecond
	// deadlines in waitForTimer();
	// Toggled with Ctrl+F (1 <-> kFastModeFactor), or set to any 1..kFastModeMaxFactor
	// with the console's fast_mode command.
	int _fastMode;

	int _language;
	DetectedGameVersion _game;
	int _currentCursor;
	bool _roomCursorOn;
	bool _dialogueCursorOn;

	char _globalDialogueTexts[300][MAX_DIALOGUE_TEXT_LENGTH];
	uint8 _walkXScaleRoom[320];
	uint8 _walkYScaleRoom[144 * 3];
	RoomObjectArea _roomObjectAreasTable[MAX_ROOM_OBJECT_AREAS];
	uint8 _roomActionsTable[0x2000];

	ExecuteActionProc _executeMainAction;
	ExecuteActionProc _executeRoomAction;

	Action _currentAction;
	uint8 _actionCode;
	uint8 _actionWalkPoint;

	int16 _inputVars[kInputVarCount];

	bool _rightButtonSelecting;
	int _rightButtonSelectCursorX, _rightButtonSelectCursorY;

	bool _scrollInventory;
	int _scrollInventoryStartY, _scrollInventoryEndY, _scrollInventoryDy;
	WalkData _walkData[101];
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
	int _dialogueTextsBuildCount;
	int _dialogueTextsStart;
	int _dialogueTextsCount;
	int _dialogueDirtyRectY;
	int _dialogueDirtyRectSize;
	char _dialogueQuestions[MAX_DIALOGUE_QUESTIONS][2][41];
	uint16 _dialogueQuestionSounds[MAX_DIALOGUE_QUESTIONS];
	char _dialogueReplies[MAX_DIALOGUE_REPLIES][51];
	uint16 _dialogueReplySounds[MAX_DIALOGUE_REPLIES];
	bool _dialogueEnded;
	int _dialogueChoiceSelected;
	uint8 _dialogueInfo[6];

	uint8 _objectsState[112];
	bool _part07FirstVisitDone;
	// scratch of the room loops: the current ambient frame (a star in part 72, the screen in part 68)
	uint8 _roomAmbientIndex;
	// the two fireflies of room 69: the current frame of each and the steps of the first one since the second one started
	uint8 _part69FireflyFrameA;
	uint8 _part69FireflyFrameB;
	uint16 _part69FireflyStepCount;
	uint8 _part68QuestionCounter;
	uint8 _part68LastDialogueCode;
	bool _part68NpcPresent;
	uint8 _parkLadyIdleStep;
	uint8 _inventoryImages[36];
	uint8 _inventoryInfo[74];
	char _verbPrepositions[3][7];
	char _roomObjectNames[20][MAX_OBJECT_NAME_LENGTH];
	char _globalObjectNames[35][MAX_OBJECT_NAME_LENGTH];
	char _verbsName[9][MAX_VERB_NAME_LENGTH];

	uint8 _currentPalette[768];
	uint8 _paletteBuffer[768];
	uint8 _igorPalette[48];
	uint8 *_igorTempFrames;

	RoomWalkBounds _roomWalkBounds;
	RoomClickFix _roomClickFix;
	int _roomClickFixBottom; // the last row the click fix scans
	// stride of the give matrix of the room when it differs from the use matrix (0: same)
	int _roomGiveObjectSize;
	RoomDataOffsets _roomDataOffsets;
	UpdateDialogueProc _updateDialogue;
	// palette indices used to draw cutscene dialogue text and its outline
	uint8 _talkColorIndex;
	uint8 _talkShadowIndex;
	UpdateRoomBackgroundProc _updateRoomBackground;
	int _demoActionsCounter;
	int _gameTicks;
	int _resourceEntriesCount;
	int _soundOffsetsCount;
	uint32 *_soundOffsets;

	Font _font;

	ResourceEntry *_resourceEntries;
	Common::Array<StringEntry> _stringEntries;
	char _saveStateDescriptions[kMaxSaveStates][100];

	void restart();

	void setupDefaultPalette();

	void readTableFile();


	void copyArea(uint8 *dst, int dstOffset, int dstPitch, const uint8 *src, int srcPitch, int w, int h, bool transparent = false);
	void drawAnimRect(int dstOffset, int animOffset, int w, int h, bool alsoBackground = false, AnimBlend blend = kBlendCopy);
	void animateLitAnimFrames(int srcOffset, int firstFrame, int lastFrame, int frameSize, int width, int height, int dstOffset, int delay, int soundFrame, int sound);
	void updateIgorIdleAnimation(int fl);


	void loadDialogueData(int dlg);
	void loadMainTexts();
	const char *getString(int id) const;

	void startMusic(int cmf);
	void playMusic(int num);
	void updateMusic();
	void playSound(int num, int type);
	void stopSound();
	void loadIgorFrames();

	void ADD_DIALOGUE_TEXT(int num, int count, int sound = kNoSpeechSound);
	void SET_DIALOGUE_TEXT(int start, int count);
	void SET_EXEC_ACTION_FUNC(int i, ExecuteActionProc p);


	// One-call dialogue helpers. Each queues the given lines as consecutive pages and starts them.
	//   igorSay                         Igor speaks, returns once the first page has started
	//   igorSayAndWait                  Igor speaks, returns once all pages are finished
	//   cutsceneSayStart                text at (x, y) in color (r, g, b), returns once the first page has started
	//   cutsceneSay                     same, but returns once all pages are finished
	//   cutsceneSayStartWithCallback    like cutsceneSayStart, but first installs a talk-animation callback (see _updateDialogue); it stays installed for the caller to clear
	//   cutsceneSayWithCallback         like cutsceneSay, but installs the callback first and clears it afterwards
	void igorSay(const Common::Array<DialogueText> &lines);
	void igorSay(int num, int count = 1, int sound = kNoSpeechSound);
	void igorSayAndWait(const Common::Array<DialogueText> &lines);
	void igorSayAndWait(int num, int count = 1, int sound = kNoSpeechSound);
	void cutsceneSayStart(int x, int y, int r, int g, int b, const Common::Array<DialogueText> &lines);
	void cutsceneSayStart(int x, int y, int r, int g, int b, int num, int count = 1, int sound = kNoSpeechSound);
	void cutsceneSay(int x, int y, int r, int g, int b, const Common::Array<DialogueText> &lines);
	void cutsceneSay(int x, int y, int r, int g, int b, int num, int count = 1, int sound = kNoSpeechSound);
	void cutsceneSayStartWithCallback(int x, int y, int r, int g, int b, const Common::Array<DialogueText> &lines, UpdateDialogueProc update);
	void cutsceneSayStartWithCallback(int x, int y, int r, int g, int b, int num, int count, int sound, UpdateDialogueProc update);
	void cutsceneSayWithCallback(int x, int y, int r, int g, int b, const Common::Array<DialogueText> &lines, UpdateDialogueProc update);
	void cutsceneSayWithCallback(int x, int y, int r, int g, int b, int num, int count, int sound, UpdateDialogueProc update);

	void queueDialogueLines(const Common::Array<DialogueText> &lines);
	void animateIgorTalking(int frame);
	void fixIgorDialogueTextPosition(int num, int count, int *x, int *y);
	void startIgorDialogue();
	void waitForEndOfIgorDialogue(bool animateHead = true);
	void fixDialogueTextPosition(int num, int count, int *x, int *y);
	void startCutsceneDialogue(int x, int y, int r, int g, int b);
	void waitForEndOfCutsceneDialogue(int x, int y, int r, int g, int b);

	bool isDialogueSpeechPlaying() const;
	void stopDialogueSpeech();

	void PART_MAIN();
	void moveScreenUp(int offset);
	void lookAtPapyrus(bool reveal);
	void EXEC_MAIN_ACTION(int action);
	void EXEC_MAIN_ACTION_38_lookAtNewspaper();
	void EXEC_MAIN_ACTION_43_lookAtPhoto();

	// map
	void PART_04_EXEC_ACTION(int action);
	void PART_04();

	// Igors room
	void PART_00();
	void PART_00_EXEC_ACTION(int action);
	void PART_00_APPLY_OBJECT_STATE(int num);
	void PART_00_ENTRY_ANIMATION();
	void PART_00_WALK_IN_FROM_CLOSET();
	void PART_00_ENTER_FROM_BELOW();
	// Rooftop
	void PART_01();
	void PART_01_EXEC_ACTION(int action);
	void PART_01_CLOSE_WINDOW();
	void PART_01_STATE_11_BLIT_blitIgor();
	void PART_01_STATE_11_BLIT_blitRoof();
	void PART_01_STATE_11_BLIT_drawIgorsEyes(int frame);
	void PART_01_STATE_11_DRAW_drawPigeons(int index);
	void PART_01_STATE_11_pigeonsCutscene();
	void PART_01_STATE_12_explosion();
	// junk room
	void PART_02();
	void PART_02_EXEC_ACTION(int action);
	void lightUpDynamite();
	void PART_02_APPLY_OBJECT_STATE(int num);
	void PART_02_SEARCH_TRUNK();
	void PART_02_DRAW_FUSE_SPARK(int frame);
	void PART_02_UPDATE_FUSE();
	void PART_02_WALK_WHILE_FUSE_BURNS(int srcX, int srcY, int dstX, int dstY);

	// spring bridge
	void PART_05_HELPER_4_drawPaperOrNot(int num);
	void PART_05_HELPER_6_walkIgorToScene();
	void PART_05_06_drawPaperFrame(int frame);
	void PART_05_06_SAVE_PHOTOGRAPHER_BACKGROUND();
	void PART_05_06_DRAW_PHOTOGRAPHER();
	void PART_05_06_DRAW_CAMERA(int frame);
	void PART_05_06_DRAW_TRIPOD(bool drawToScreen);
	void PART_05();
	void PART_05_EXEC_ACTION(int action);
	void PART_05_ACTION_103_pickPaper();
	void PART_05_ACTION_102_scrollRight();
	void PART_05_UPDATE_ROOM_BACKGROUND();

	// spring rock
	void PART_06();
	void PART_06_UPDATE_ROOM_BACKGROUND();
	void PART_06_EXEC_ACTION(int action);
	void PART_06_ACTION_103_talkToPhotographer();
	void PART_06_HANDLE_DIALOGUE_PHOTOGRAPHER();
	void PART_06_UPDATE_DIALOGUE_PHOTOGRAPHER(int action);
	void PART_06_HELPER_7_decodePhotographerTalkingFrame(int frame);
	void PART_06_ACTION_105_pickCamera();
	void PART_06_ACTION_107_giveAnythingToPhotographer();
	void PART_06_ACTION_108_giveRocketToPhotographer();
	void PART_06_ACTION_102_scrollLeft();
	void PART_06_HELPER_6_setPhotographerState(int num);
	void PART_06_HELPER_8_animatePhotographer(int frame);
	void PART_06_HELPER_12_drawBackgroundOverPhotographer();

	void PART_07();
	void PART_07_EXEC_ACTION(int action);
	void PART_07_DRAW_DOOR_STATE(int num);
	void PART_07_openCloseDoor(int door, bool open);
	void PART_07_DRAW_SCALED_IGOR(int scaleStep, int facing, int frame, int dyPos);
	void PART_07_enterFromOutside();
	void PART_07_exitToOutside();

	// Dean's office
	void PART_08();
	void PART_08_EXEC_ACTION(int action);
	void PART_08_APPLY_OBJECT_STATE(int num);
	void drawDoor(bool open);
	void PART_08_ACTION_pickNewspaper();
	void PART_08_ACTION_108_deanCallsSecretary();
	void PART_08_ACTION_pickBook();
	void PART_08_ACTION_103_talkToDean();
	void giveBottleToDean();
	void PART_08_deanDrinksBottle();
	void PART_08_deanPassesOut();
	void PART_08_HANDLE_DIALOGUE_DEAN();
	void drawDean();
	void drawDeanTalkingFrame(int frame);
	void PART_08_UPDATE_DIALOGUE_DEAN(int action);
	void drawSecretaryTalkingFrame(int frame);
	void PART_08_UPDATE_DIALOGUE_SECRETARY(int action);
	void PART_09();
	void PART_09_EXEC_ACTION(int action);
	void PART_09_APPLY_OBJECT_STATE(int num);
	void PART_09_ANIMATE_DOOR(bool open);
	void PART_09_drawSecretaryFrame(int frame, bool background);
	void PART_09_DRAW_openClosetFrame(int frame);
	void PART_09_ACTION_101_openFileCabinet();
	void PART_09_ACTION_106_openCloset(bool search);
	void PART_09_ACTION_110_pickCostumeFromCloset();
	void PART_09_UPDATE_DIALOGUE_SECRETARY(int action);
	void PART_09_DRAW_SECRETARY_MOUTH(int frame);
	void PART_09_secretarySearchesFile();
	void PART_09_UPDATE_ROOM_BACKGROUND();

	// Decanato right
	void PART_10();
	void PART_10_EXEC_ACTION(int action);
	void PART_10_ACTION_104_pickHamburger();
	void PART_10_ACTION_108_scrollLeft();
	void PART_10_11_DRAW_OBJECT_STATE(int num);

	// Decanato left
	void PART_11();
	void PART_11_EXEC_ACTION(int action);
	void PART_11_ACTION_105_pickSlug();
	void PART_11_ACTION_107_useButterflyNetWithHole();
	void PART_11_ACTION_108_scrollRight();
	void PART_11_ACTION_112_pickBottle();
	void PART_11_APPLY_OBJECT_STATE(int num);

	// outside church
	void PART_12_EXEC_ACTION(int action);
	void PART_12_ACTION_goToPath();
	void PART_12_ACTION_104_tryEnterChurch();
	void PART_12_ACTION_105_useResinWithStone();
	void PART_12_ACTION_108_enterChurch();
	void PART_12_UPDATE_ROOM_BACKGROUND();
	void PART_12_UPDATE_DIALOGUE_PRIEST(int action);
	void PART_12_HANDLE_DIALOGUE_PRIEST();
	void PART_12_OBJECT_STATE(int num);
	void PART_12_HELPER_2();
	void PART_12_HELPER_3_paintOverLizard();
	void PART_12_HELPER_4_exorcismCutscene();
	void PART_12_HELPER_5_enterFromPath();
	void PART_12_HELPER_6_enterFromChurch();
	void PART_12_HELPER_8_drawIdlePriest();
	void PART_12_HELPER_9_drawEntrance();
	void PART_12_HELPER_10_drawPriestTalkingFrame(int frame);
	void PART_12();

	// inside church
	void PART_13_EXEC_ACTION(int action);
	void PART_13_ACTION_101_103_goToPlatform();
	void PART_13_ACTION_104_exitChurch();
	void PART_13_HELPER_OBJECT_STATE(int num);
	void PART_13_HELPER_2_enterFromRight();
	void PART_13_HELPER_3_enterFromBelow();
	void PART_13();

	// church puzzle
	void PART_14_EXEC_ACTION(int action);
	void PART_14_UPDATE_ROOM_BACKGROUND_ACTION_108();
	void PART_14_ACTION_101_goToChurchBell();
	void PART_14_ACTION_103_lookAtPuzzle();
	void PART_14_ACTION_105_goBackToChurch();
	void PART_14_ACTION_106_enterMaze();
	void PART_14_ACTION_108_useMatchesOnCandles();
	void PART_14_HELPER_1_OBJECT_STATE(int num);
	void PART_14_HELPER_2_enterFromChurch();
	void PART_14_HELPER_3_enterFromChurchBell();
	void PART_14_HELPER_4_enterFromMaze();
	void PART_14_HELPER_6_drawEntrance();
	void PART_14_HELPER_7_candleFlicker(int frame);
	void PART_14_HELPER_8_brotherReveal(int start, int end);
	void PART_14_HELPER_9_showPuzzle();
	void PART_14_HELPER_10_loadChurchMosaicData();
	void PART_14_pushStone(int screenOffset, int w, int h, int animOffset);
	void PART_14();
	void loadResourceData__ROOM_ChurchPuzzle();
	void loadResourceData__ANIM_ChurchPuzzle();

	// tobias office
	void PART_15_EXEC_ACTION(int action);
	void PART_15_ACTION_101_leaveRoom();
	void PART_15_ACTION_107_talkToTobias();
	void PART_15_ACTION_115_giveProjectToTobias();
	void PART_15_ACTION_116_giveMoneyToTobias();
	void PART_15_UPDATE_ROOM_BACKGROUND();
	void PART_15_UPDATE_DIALOGUE_TOBIAS(int action);
	void PART_15_HANDLE_DIALOGUE_TOBIAS();
	void PART_15_HELPER_1_OBJECT_STATE(int num);
	void PART_15_HELPER_2_walkIn();
	void PART_15_HELPER_3_updateCuckooClock();
	void PART_15_waitForCuckooClock();
	void PART_15_HELPER_5_animateTobiasIdle();
	void PART_15_HELPER_6_drawCuckooFrame(int frame);
	void PART_15_HELPER_7_drawIgorAndTobiasScene(int frame);
	void PART_15_HELPER_8_drawTobiasTalking(int frame);
	void PART_15_PART_15_HELPER_9_drawTobiasIdleFrame(int frame);
	void PART_15();

	// laboratory
	void PART_16_EXEC_ACTION(int action);
	void PART_16_ACTION_101();
	void PART_16_UPDATE_DIALOGUE_MARGARET_HARRISON(int action);
	void PART_16_UPDATE_DIALOGUE_MARGARET(int action);
	void PART_16_HELPER_1(int num);
	void PART_16_HELPER_2();
	void PART_16_HELPER_3_photoCutscene();
	void PART_16_HELPER_5_displayPhoto();
	void PART_16_HELPER_6(int frame);
	void PART_16();
	void loadResourceData__ROOM_Laboratory();
	void loadResourceData__ANIM_Laboratory();

	// outside college
	void PART_17_EXEC_ACTION(int action);
	void PART_17_ACTION_101_walkIn();
	void PART_17_ACTION_103_talkPhilipJimmy();
	void PART_17_ACTION_105();
	void PART_17_ACTION_106_swapFolders();
	void PART_17_HANDLE_DIALOGUE_PHILIP();
	void PART_17_UPDATE_DIALOGUE_Jimmy(int action);
	void PART_17_UPDATE_DIALOGUE_Philip(int action);
	void PART_17_UPDATE_ROOM_BACKGROUND();
	void PART_17_HELPER_1(int num);
	void PART_17_HELPER_2_walkFromMap();
	void PART_17_HELPER_3(int lum);
	void PART_17_HELPER_4_paintFirsFrameOfPhilipAndJimmy();
	void PART_17_HELPER_5_changeZindexOfPath(int lum);
	void PART_17_HELPER_6_walkFromCollege();
	void PART_17_HELPER_8_PhillipToJimmyAnimFrame(int num);
	void PART_17_HELPER_9_JimmyTalkingAnimFrame(int num);
	void PART_17_HELPER_10();
	void PART_17_HELPER_11_PhillipToIgor(int frame);
	void PART_17();

	// men toilets
	void PART_18_EXEC_ACTION(int action);
	void PART_18_ACTION_109_useSlugOnGrating();
	void PART_18_ACTION_111();
	void PART_18_HELPER_1(int num);
	void PART_18_HELPER_2_walkIn();
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
	void PART_19_HELPER_2_slugCutscene();
	void PART_19_HELPER_3();
	void PART_19_HELPER_4();
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
	void PART_21_HELPER_5();
	void PART_21_HELPER_6(int frame);
	void PART_21_HELPER_7();
	void PART_21_HELPER_8();
	void PART_21_HELPER_9();
	void PART_21_HELPER_10();
	void PART_21_HELPER_11(int frame);
	void PART_21();

	// church bell tower
	void PART_22_EXEC_ACTION(int action);
	void PART_22_ACTION_101();
	void PART_22_ACTION_102();
	void PART_22_APPLY_OBJECT_STATE(int num);
	void PART_22_IGOR_STEP(int facing, int step);
	void PART_22_SCROLL_STEP(int step);
	void PART_22_ENTER();
	void PART_22();

	// college corridor lucas
	void PART_23_EXEC_ACTION(int action);
	void PART_23_ACTION_105_enterLadiesRoom();
	void PART_23_ACTION_107_openDoor();
	void PART_23_ACTION_108_closeDoor();
	void PART_23_UPDATE_ROOM_BACKGROUND();
	void PART_23_HELPER_1(int num);
	void PART_23_HELPER_2(int frame);
	void PART_23_HELPER_3();
	void PART_23_HELPER_4_walkFromDoor();
	void PART_23_HELPER_5_walkFromLeft();
	void PART_23_HELPER_6_walkFromRight();
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
	void PART_27_ACTION_106_openPhilipLocker();
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
	void PART_30_ACTION_102_goUpstairs();
	void PART_30_ACTION_104_goDownstairs();
	void PART_30_UPDATE_DIALOGUE_LAURA(int action);
	void PART_30_HANDLE_DIALOGUE_LAURA();
	void PART_30_HELPER_1(int num);
	void PART_30_HELPER_2_walkInFromLeft();
	void PART_30_HELPER_3_walkInFromUpstairs();
	void PART_30_HELPER_2_walkInFromOutside();
	void PART_30_HELPER_5_walkInFromRight();
	void PART_30_HELPER_8_LauraCutscene();
	void lauraAndIgorBumpIntoEachOther();
	void igorCrossesArms();
	void lauraLeaves();
	void PART_30_HELPER_9_setLauraFrame(int frame);
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
	void PART_33_HARRISON_SPEAKS(const Common::Array<DialogueText> &lines);
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

	// park
	void PARK_DRAW_LADY_FRAME(uint8 *dst, int frame);
	void PARK_DRAW_LAURA_FRAME(int frame);
	void PARK_PICK_UP_ANIMATION(int screenOffset, int framesOffset);
	void PARK_UPDATE_AMBIENT_SOUND();
	void PARK_UPDATE_DIALOGUE_LADY(int action);
	void PARK_UPDATE_DIALOGUE_LAURA(int action);
	void PARK_QUEUE_REPLY_TEXT(int reply, int count, int sound, int &textIndex);
	void PARK_WAIT_FOR_IGOR_DIALOGUE();
	void PARK_WAIT_FOR_LADY_DIALOGUE();
	void PARK_WAIT_FOR_LAURA_DIALOGUE();
	void PART_34();
	void PART_34_EXEC_ACTION(int action);
	void PART_34_ACTION_103_TAKE();
	void PART_34_ACTION_105_TALK();
	void PART_34_ACTION_108_OLD_LADY();
	void PART_34_ACTION_109_SCROLL_RIGHT();
	void PART_34_APPLY_OBJECT_STATE(int num);
	void PART_34_LADY_IDLE(int step);
	void PART_34_LAURA_CONVERSATION();
	void PART_34_OLD_LADY_CONVERSATION();
	void PART_34_UPDATE_ROOM_BACKGROUND();
	void PART_35();
	void PART_35_EXEC_ACTION(int action);
	void PART_35_ACTION_102_TAKE();
	void PART_35_ACTION_106_EXIT_TO_MAP();
	void PART_35_ACTION_107_SCROLL_LEFT();
	void PART_35_APPLY_OBJECT_STATE(int num);

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

	// outside the maze
	void PART_50_EXEC_ACTION(int action);
	void PART_50_ACTION_101_enterMaze();
	void PART_50_ENTER_FROM_DOOR();
	void PART_50_ENTER_FROM_HILL();
	void PART_50_UPDATE_ROOM_BACKGROUND();
	void PART_50();

	void PART_69_EXEC_ACTION(int action);
	void PART_69_APPLY_OBJECT_STATE();
	void PART_69_DRAW_FIREFLY_A(int frame);
	void PART_69_DRAW_FIREFLY_B(int frame);
	void PART_69_ANIMATE_FIREFLIES();
	void PART_69_ACTION_107_watch();
	void PART_69_FINALE(int num, int sound);
	void PART_69_ENTER_FROM_RIGHT();
	void PART_69_UPDATE_ROOM_BACKGROUND();
	void PART_69();

	void PART_70_EXEC_ACTION(int action);
	void PART_70_DRAW_OBJECT_STATE();
	void PART_70_UPDATE_ROOM_BACKGROUND();
	void PART_70();

	void PART_74_CYCLE_COLORS();
	void PART_74_DRAW_FRAME(int frame);
	void PART_74_UPDATE_DIALOGUE_FIRST(int action);
	void PART_74_UPDATE_DIALOGUE_SECOND_A(int action);
	void PART_74_UPDATE_DIALOGUE_SECOND_B(int action);
	void PART_74_SAY(int who, const Common::Array<DialogueText> &lines);
	void PART_74_CUTSCENE();

	void PART_81_EXEC_ACTION(int action);
	void PART_81_LOAD_ROOM();
	void PART_81_LOAD_ANIMATION();
	void PART_81_DRAW_HEAD(int frame);
	void PART_81_DRAW_STORY_HEAD(int frame);
	void PART_81_DRAW_FIGURE(int offset);
	void PART_81_DRAW_END_FRAME(int frame);
	void PART_81_UPDATE_ROOM_BACKGROUND();
	void PART_81_UPDATE_DIALOGUE_NPC(int action);
	void PART_81_NPC_SAY(const Common::Array<DialogueText> &lines);
	void PART_81_IGOR_SAY(int num, int count, int sound);
	void PART_81_APPLY_OBJECT_STATE(int num);
	void PART_81_SCROLL_IN();
	void PART_81_TRANSITION_IN();
	void PART_81_TRANSITION_OUT();
	void PART_81_END_SCENE();
	void PART_81_ACTION_109();
	void PART_81_ACTION_110();
	void PART_81_ACTION_102_talk();
	void PART_81_DIALOGUE_CODE(int code);
	void PART_81_CONVERSATION();
	void PART_81_ENTER();
	void PART_81();


	void PART_68_EXEC_ACTION(int action);
	void PART_68_LOAD_ROOM();
	void PART_68_APPLY_OBJECT_STATE(int num);
	void PART_68_DRAW_NPC(int frame);
	void PART_68_DRAW_AMBIENT(int frame);
	void PART_68_DRAW_OBJECT(int frame);
	void PART_68_DRAW_POSE(int frame);
	void PART_68_UPDATE_AMBIENT();
	void PART_68_UPDATE_ROOM_BACKGROUND();
	void PART_68_UPDATE_ROOM_BACKGROUND_TALK();
	void PART_68_UPDATE_DIALOGUE_NPC(int action);
	void PART_68_NPC_SAY(const Common::Array<DialogueText> &lines);
	void PART_68_NPC_SAY_REPLIES(const int *replies, const int *sounds, int count);
	void PART_68_ENTER_FROM_RIGHT();
	void PART_68_ENTER_FROM_LEFT();
	void PART_68_ENTER_INSIDE();
	void PART_68_ACTION_102_takeObject();
	void PART_68_ACTION_104_talk();
	void PART_68_ACTION_106();
	void PART_68_ACTION_108_giveObject();
	void PART_68_CONVERSATION();
	void PART_68_DIALOGUE_CODE(int code);
	void PART_68_NPC_LEAVES();
	void PART_68();

	void PART_71_EXEC_ACTION(int action);
	void PART_71_APPLY_OBJECT_STATE(int what);
	void PART_71_DRAW_OBJECT(int frame);
	void PART_71_ACTION_102_takeObject();
	void PART_71_ACTION_103_look();
	void PART_71_ENTER_FROM_RIGHT();
	void PART_71_LIGHTNING();
	void PART_71_UPDATE_ROOM_BACKGROUND();
	void PART_71();

	void PART_72_EXEC_ACTION(int action);
	void PART_72_APPLY_OBJECT_STATE();
	void PART_72_ACTION_102_takeObject();
	void PART_72_ENTER_FROM_RIGHT();
	void PART_72_ENTER_FROM_LEFT();
	void PART_72_UPDATE_STAR();
	void PART_72_UPDATE_ROOM_BACKGROUND();
	void PART_72();

	// maze
	static const MazeNode MAZE_NODES[108];
	static const MazeRoom *getMazeRoom(int part);
	void PART_MAZE();
	void PART_MAZE_EXEC_ACTION(int action);
	void PART_MAZE_UPDATE_ROOM_BACKGROUND();
	void maybeUpdateFlicker();
	void mazeFlicker();
	void mazeDrawFlameFrame(int x, int y, int frame);
	void MAZE_ENTER_WALK(const MazeEntry &entry);
	void enterFromStairs(const MazeEntry &entry);
	void mazeWalkToExit(const MazeAction &action);
	void mazeExitThroughStairs(const MazeAction &action);
	void mazeGoToNeighbor(int dir);
	int MAZE_MUSIC_TRACK() const;
	void setRoomClickFix(int yMax, int xMin, int xMax, bool scanUp, int yBottom = 143);

	void UPDATE_OBJECT_STATE(int num);
	void PART_UPDATE_FIGURES_ON_PAPER(int delay);

	void PART_MEANWHILE();

	// Margaret cutscenes
	void PART_MARGARET_ROOM_CUTSCENE_HELPER_1();
	void PART_MARGARET_ROOM_CUTSCENE_HELPER_2(int frame);
	void PART_MARGARET_ROOM_CUTSCENE_UPDATE_DIALOGUE_MARGARET(int action);
	void PART_MARGARET_ROOM_CUTSCENE();

	// philip vodka cutscene
	void PART_75_UPDATE_DIALOGUE_PHILIP(int action);
	void PART_75_HELPER_1(int frame);
	void PART_75();

	// Intro
	void PART_85();
	void PART_85_HELPER_1_PLAY_ANIM(int frameOffset2, int frameOffset1, int firstFrame, int lastFrame, int delay);
	void PART_85_HELPER_2_SCROLL_RIGHT();
	void displayLogo();
	void PART_85_UPDATE_DIALOGUE_PHILIP_LAURA(int action);
	void PART_85_UPDATE_ROOM_BACKGROUND();

	void PART_85_HELPER_6_animateIgorHead(int frame);

	// Splash screens
	void PART_90();

	void handleRoomInput();

	void executeAction(int action);
	void clearAction(bool redraw = true);
	void formatActionSentence(uint8 color);
	void drawActionSentence(const char *sentence, uint8 color);
	void handleRoomIgorWalk();

	void drawVerbsPanel();
	void redrawVerb(uint8 verb, bool highlight);
	void beginRightButtonVerbPick();
	void updateRightButtonVerbPick();
	void endRightButtonVerbPick();
	void formatRightButtonSentence();
	int getVerbUnderCursor(int x) const { return ((x % 46) < 44) ? (kVerbTalk + x / 46) : 0; }
	void handleRoomInventoryScroll();
	void handleOptionsMenu();
	void scrollInventory();
	void drawInventory(int start, int mode);
	void addObjectToInventory(int object, int index);
	void removeInventoryEntry(int index);
	void removeObjectFromInventory(int index);
	int getObjectFromInventory(int x) const;

	void packInventory();

	void enterPartLoop();
	void leavePartLoop();
	void runPartLoop();
	bool restoreRoomAfterLoad(bool drawIgor = true);

	void handleRoomDialogue();

	void handleIgorIdleAnimation();


	void getClosestAreaTrianglePoint(int dstArea, int srcArea, int *dstY, int *dstX, int srcY, int srcX);
	void getClosestAreaTrianglePoint2(int dstArea, int srcArea, int *dstY, int *dstX, int srcY1, int srcX1, int srcY2, int srcX2);

	int lookupScale(int xOffset, int yOffset, int h) const;
	void lookupScale(int curX, int curY, uint8 &scale, uint8 &xScale, uint8 &yScale) const;

	void buildWalkPathSimple(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathArea(int srcX, int srcY, int dstX, int dstY);
	int getVerticalStepsCount(int minX, int minY, int maxX, int maxY);
	int getHorizontalStepsCount(int minX, int minY, int maxX, int maxY);
	void buildWalkPathAreaUpDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaDownDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaRightDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaLeftDirection(int srcX, int srcY, int dstX, int dstY);
	typedef void (IgorEngine::*IgorMoveTick)();
	// tick: room effect that runs on every iteration while Igor walks (rooms with their own wait loop)
	bool waitForIgorMove(IgorMoveTick tick = 0, bool escapeSkips = false, bool forceSkip = false);

	void moveIgor(int pos, int frame);

	void setCursor(int num);
	void showCursor();
	void hideCursor();

	void scrollPalette(int startColor, int endColor);
	void setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b);
	void setPaletteRange(int startColor, int endColor);
	void darkenPalette(uint8 *palette, int firstColor, int lastColor, int amount);
	void updatePalette(int count);
	void SET_PAL_208_96_1();
	void SET_PAL_240_48_1();

	void fadeIn(int count);
	void fadeOut(int count);

	void decodeRoomStrings(const uint8 *p, bool skipObjectNames = false);
	void decodeRoomText(const uint8 *p);
	void decodeRoomAreas(const uint8 *p, int count);
	void decodeRoomMask(const uint8 *p);

	void debugApplyOverlay();

	int getPart() const { return _currentPart / 10; }
	bool compareGameTick(int add, int mod) const { return ((_gameTicks + (add & ~7)) % mod) == 0; } // { return ((_gameTicks + add) % mod) == 0; }
	bool compareGameTick(int eq) const { return _gameTicks == (eq & ~7); }                          // { return _gameTicks == eq; }

	void waitForTimer(int ticks = -1);

	ResourceEntry *findData(int num);
	uint8 *loadData(int num, uint8 *dst = 0, int *size = 0);
	void loadAnimData(const int *anm, int loadOffset = 0);
	void loadActionData(int act);
	void loadRoomData(int pal, int img, int box, int msk, int txt);
	const uint8 *getAnimFrame(int baseOffset, int tableOffset, int frame);
	void decodeAnimFrame(const uint8 *src, uint8 *dst, bool preserveText = false);

	void setRoomWalkBounds(int x1, int y1, int x2, int y2,
			int x1MinY = -1, int x2MinY = -1);
	void fixWalkPosition(int *x, int *y);
	void recolorDialogueChoice(int num, bool highlight);
	void handleDialogue(int x, int y, int r, int g, int b, bool restoreUI = true);
	void drawDialogueChoices();
	int selectDialogue();
	void dialogueAskQuestion();
	void dialogueReplyToQuestion(int x, int y, int r, int g, int b, int reply = 0);

	void buildWalkPath(int srcX, int srcY, int dstX, int dstY);

	static const uint8 _dialogueColor[];
	static const uint8 _sentenceColorIndex[];

public:
	Graphics::Screen *_screen = nullptr;

	void handleOptionsMenu_paintSave();
	bool handleOptionsMenu_handleKeyDownSave(int key);
	void handleOptionsMenu_paintLoad();
	bool handleOptionsMenu_handleKeyDownLoad(int key);
	void handleOptionsMenu_paintQuit();
	bool handleOptionsMenu_handleKeyDownQuit(int key);
	void handleOptionsMenu_paintCtrl();
	bool handleOptionsMenu_handleKeyDownCtrl(int key);

public:
	IgorEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~IgorEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	void handlePause();

	// Debug overlays (console: paint_walk / paint_hotspots / paint_y1lum /
	// paint_y2lum / paint_deltalum / paint_off).
	// Paint the mask regions whose BOX record has a non-zero walk area or
	// object, or whose y1Lum / y2Lum / deltaLum threshold is set; applied on
	// top of the presented frame until cleared.
	void debugPaintWalkAreas();
	void debugPaintHotspots();
	void debugPaintY1Lum();
	void debugPaintY2Lum();
	void debugPaintDeltaLum();
	void debugClearOverlay();
	void debugSetFastMode(int factor) { _fastMode = factor; }
	int debugGetFastMode() const { return _fastMode; }
	void debugChangePart(int state);
	bool debugAddObjectToInventory(int object);
	void debugExecuteAction(int action) { executeAction(action); }

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

protected:
	// Engine APIs
	Common::Error run() override;
	static const RoomDataOffsets PART_04_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_00_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_01_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_02_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_05_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_06_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_07_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_08_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_09_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_10_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_11_ROOM_DATA_OFFSETS;
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
	static const RoomDataOffsets PART_34_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_35_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_36_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_37_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_50_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_68_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_69_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_70_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_71_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_72_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_81_ROOM_DATA_OFFSETS;
	static const uint8 INVENTORY_IMG_INIT[];
	static const uint8 _inventoryOffsetTable[];
	static const uint8 _inventoryActionsTable[];

	static const uint8 _walkWidthScaleTable[];
	static const uint8 _walkScaleTable[];
	static const float _walkScaleSpeedTable[];
	static const uint8 _talkDelays[];
	static const uint8 _mouseCursorMask[];
	static const uint8 _mouseCursorData[];
	static const uint8 PAL_IGOR_1[];
	static const uint8 PAL_48_1[];
	static const uint8 PAL_96_1[];
};

extern IgorEngine *g_engine;
#define SHOULD_QUIT ::Igor::g_engine->shouldQuit()

// Static data tables (defined in static_walk.cpp, static_cursor.cpp)

} // End of namespace Igor

#endif // IGOR_H
