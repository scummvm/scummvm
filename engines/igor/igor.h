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
#include "common/keyboard.h"
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
#include "igor/floppy_font.h"
#include "igor/resource_ids.h"

namespace Igor {

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
	kMaxSaveStates = 10,
	kNoSpeechSound = 0
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

// Effects that the Spanish floppy plays with another number than the other versions (see playRoomSound)
enum RoomSound {
	kSoundPart0Nightstand = 6,
	kSoundPart0ClosetClose = 4,
	kSoundPart1Window = 12,
	kSoundPart2Fuse = 11,
	kSoundPart8Shards = 16,
	kSoundPart18Tap = 47,
	kSoundPart19Dryer = 45,
	kSoundPart50Ambient = 44
};

// Numbers of the Spanish floppy sound table
enum SpaFloppySound {
	kSpaFloppySoundPart0Nightstand = 5,
	kSpaFloppySoundPart0ClosetClose = 14,
	kSpaFloppySoundPlaneEngine = 66,
	kSpaFloppySoundPart50Door = 68,
	kSpaFloppySoundPart68AmbientA = 69,
	kSpaFloppySoundPart68AmbientB = 70,
	kSpaFloppySoundPart18Tap = 71,
	kSpaFloppySoundPart19Dryer = 72,
	kSpaFloppySoundParkAmbient = 75,
	kSpaFloppySoundPart6Ambient = 76,
	kSpaFloppySoundPart8Shards = 77,
	kSpaFloppySoundPart1Window = 79,
	kSpaFloppySoundPart2Fuse = 80,
	kSpaFloppySoundPart14StonePush = 34,
	kSpaFloppySoundPart14Solved = 74,
	kSpaFloppySoundPart32Key = 31,
	kSpaFloppySoundPart32Animation = 73,
	kSpaFloppySoundPart32Girl = 32
};

// Value of _hoverVerbState when the verb panel shows the selected verb
static const int kHoverVerbSelected = -1;

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
	kInputConfirm,
	kInputVarCount
};

struct RoomWalkBounds {
	int x1, y1;
	int x2, y2;
	// Minimum y after clamping a click past the corresponding horizontal edge.
	// e.g. clamped to x1? then x1MinY is used to determine the minimum y.
	int x1MinY, x2MinY;
};

struct DetectedGameVersion {
	int version;
	int flags;
	Common::Language language;
	const char *ovlFileName;
	const char *sfxFileName;
};

enum ResourceSource {
	kResourceSourceContainer, // the game's resource container (IGOR.DAT, IGOR.EXE)
	kResourceSourceExecutable // the unpacked executable image of the Spanish floppy version
};

struct ResourceEntry {
	int id;
	uint32 offs;
	uint32 size;
	ResourceSource source;
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
	int questionsOffset; // Base offset used to locate question text
	int questionsSize;
	int repliesOffset; // Base offset used to locate reply text
	int repliesSize;
	int matSize; // Stride used to index the conversation choices matrix
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
struct RoomWalkTargetRules {
	int yMax;
	int xMin;
	int xMax;
	bool scanUp;
	bool enabled;
};

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
	int8 music;
	uint8 darkness;
	uint8 flameMode;
	uint8 numFlames;
	MazeFlame flames[2];
	uint8 flameFrames[2];
	uint8 secondFlameFrame;
	bool saveLocation;      // the room ends when the location changes
	const RoomDataOffsets *offsets;
	int giveObjectSize;
	RoomWalkTargetRules clickFix;
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

// One-based inventory object numbers, separate from object state and inventory lookup indices.
enum InventoryObjectNumber {
	kInventoryObjectBoomerang = 1,
	kInventoryObjectWallet = 2,
	kInventoryObjectParachute = 3,
	kInventoryObjectTplTickets = 4,
	kInventoryObjectRecords = 5,
	kInventoryObjectAlarmClock = 6,
	kInventoryObjectButterflyNet = 7,
	kInventoryObjectMatches = 8,
	kInventoryObjectHammer = 9,
	kInventoryObjectPickaxe = 10,
	kInventoryObjectDynamite = 11,
	kInventoryObjectHairpin = 12,
	kInventoryObjectDeansBook = 13,
	kInventoryObjectMonkeySuit = 14,
	kInventoryObjectNewspaper = 15,
	kInventoryObjectLabKeys = 16,
	kInventoryObjectBurger = 17,
	kInventoryObjectSlug = 18,
	kInventoryObjectPhoto = 19,
	kInventoryObjectFlask = 20,
	kInventoryObjectLoveNote = 21,
	kInventoryObjectFolder = 22,
	kInventoryObjectBottle = 23,
	kInventoryObjectMoney = 24,
	kInventoryObjectLizard = 25,
	kInventoryObjectRocket = 26,
	kInventoryObjectResin = 27,
	kInventoryObjectPapyrus = 28,
	kInventoryObjectPassword = 29,
	kInventoryObjectPen = 30,
	kInventoryObjectAstronomyBook = 31,
	kInventoryObjectLabNote = 32,
	kInventoryObjectFlower = 33,
	kInventoryObjectVase = 34,
	kInventoryObjectBiologyProject = 35,
	kInventoryObjectCamera = 36
};

/**
 * Indices for global game objects state, for better readability
 */
enum ObjectStateIndex {
	kBottleStateIndex = 0,
	kLizardFedStateIndex = 1,
	kPapyrusFiguresVisibleStateIndex = 2,
	kFolderSwappedStateIndex = 3,
	kAstronomyBookStateIndex = 4,
	kPhillipLockerCombinationLearnedStateIndex = 5,
	kVaseStateIndex = 6,
	kStatueDeliveredStateIndex = 7,
	kBedroomNightstandOpenStateIndex = 8,
	kAlarmClockTakenStateIndex = 9,
	kBedroomClosetOpenStateIndex = 10,
	kBedroomHoleOpenStateIndex = 11,
	kRecordTakenStateIndex = 12,
	kBedroomWindowStateIndex = 13,
	kRatSkinHatSeenStateIndex = 14,
	kBedroomWindowExitUsedStateIndex = 15,
	kAtticTrunkOpenStateIndex = 16,
	kButterflyNetTakenStateIndex = 17,
	kAtticWallHolePresentStateIndex = 18,
	kAtticLargeCrackPresentStateIndex = 19,
	kAtticSmallCrackPresentStateIndex = 20,
	kAtticNailHotspotActiveStateIndex = 21,
	kAtticNailExaminedStateIndex = 22,
	kDynamitePlacedStateIndex = 23,
	kAtticWindowExitTriedStateIndex = 24,
	kMatchesTakenStateIndex = 25,
	kDeanDoorOpenStateIndex = 26,
	kSecretaryDoorOpenStateIndex = 27,
	kDeanBookTakenStateIndex = 28,
	kDeanPassedOutStateIndex = 29,
	kPendingDeanDrinkStateIndex = 30,
	kNewspaperTakenStateIndex = 31,
	kIntercomUsedStateIndex = 32,
	kSecretaryClosetOpenStateIndex = 34,
	kSecretaryDismissedStateIndex = 35,
	kSecretaryClosetStateIndex = 36,
	kPersonalFileChangedStateIndex = 37,
	kHamburgerTakenStateIndex = 38,
	kSlugTakenStateIndex = 40,
	kGlassShardsPresentStateIndex = 41,
	kButterflyNetStateIndex = 42,
	kLizardTakenStateIndex = 44,
	kChurchDoorOpenStateIndex = 45,
	kProjectHandedToTobiasStateIndex = 46,
	kMoneyHandedToTobiasStateIndex = 47,
	kCuckooClockTriggeredStateIndex = 48,
	kCuckooAnimationControlStateIndex = 49,
	kSecretPassageEntranceOpenStateIndex = 50,
	kFlaskTakenStateIndex = 52,
	kPhotoCutsceneSeenStateIndex = 53,
	kPhillipAndJimmyPresentStateIndex = 54,
	kCollegeEntranceFoldersSwappedStateIndex = 55,
	kPhillipAndJimmyHaveLeftStateIndex = 56,
	kHairpinTakenStateIndex = 58,
	kToiletGirlsHaveLeftStateIndex = 59,
	kSpringBridgePaperStateIndex = 60,
	kPhotographerGoneStateIndex = 61,
	kTripodStateIndex = 62,
	kCameraStateIndex = 63,
	kPhysicsClassDoorStateIndex = 64,
	kMargaretStateIndex = 65,
	kLadiesRoomDoorStateIndex = 66,
	kChemistryDoorStateIndex = 67,
	kMensRoomDoorStateIndex = 68,
	kLaboratoryDoorOpenStateIndex = 69,
	kMazeBeatenStateIndex = 70,
	kMazeEntryDirectionStateIndex = 71,
	kAstronomyLabDoorStateIndex = 72,
	kLauraMetAtCollegeStateIndex = 73,
	kAstronomyBookTakenStateIndex = 74,
	kHarrisonPhotoStateIndex = 75,
	kHarrisonNoteShownStateIndex = 76,
	kBellTowerConversationSeenStateIndex = 78,
	kSeenChickThroughTelescopeStateIndex = 79,
	kLizardGivenStateIndex = 80,
	kOldLadyConversationSeenStateIndex = 81,
	kTreeResinDiscoveredStateIndex = 82,
	kPhillipLockerStateIndex = 84,
	kLockersExaminedStateIndex = 85,
	kCarolineFolderStateIndex = 87,
	kPenTakenStateIndex = 88,
	kPhysicsClassPaperTakenStateIndex = 89,
	kCemeteryEntrySideStateIndex = 90,
	kVaseTakenStateIndex = 91,
	kUndertakerLeftStateIndex = 92,
	kRecordGivenToUndertakerStateIndex = 93,
	kBorisIntroducedStateIndex = 94,
	kPart69FireflyCapturedStateIndex = 96,
	kPart69LeaderFireflyIdentifiedStateIndex = 97,
	kPart69SwarmAnimationPhaseStateIndex = 98,
	kPart69LeaderFireflyPathStateIndex = 99,
	kStatuetteTakenStateIndex = 100,
	kStatuetteExaminedStateIndex = 101,
	kFlowerTakenStateIndex = 102,
	kLighthouseMessageSentStateIndex = 104,
	kChurchBellMessageSeenStateIndex = 105,
	kStatuetteTaken2StateIndex = 106,
	kFlaskLeftInPhillipLockerStateIndex = 107,
	kPlaneDoorOpenStateIndex = 108,
	kParachuteTakenStateIndex = 109,
	kMargaretSceneStateIndex = 110,
	kExamCompletedStateIndex = 111
};

enum MargaretState {
	kMargaretStateInitial = 0,
	kMargaretStateNoteAltered,
	kMargaretStateReadNote,
	kMargaretStatePhotoTaken,
	kMargaretStatePhotoSequenceComplete
};

// Special Entries of _inventoryInfo after the objects: the number of the first
// visible object of the inventory and the number of objects held.
enum {
	kInventoryFirstVisible = 72,
	kInventoryCount = 73,
	kInventoryInfoSize = 74
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
	kBlendCopy, // blit the entire rectangle
	kBlendLitSprite, // Igor colors follow the room’s masking and shading rules.
	kBlendLitSpriteNoShade, // same as above, but no shading on igor
	kBlendBehindIgor, // igor colors survive, the rest is replaced
	kBlendBehindIgorAndText // igor colors AND text survive
};

class MidiPlayer;

class IgorEngine : public Engine {
public:
	typedef void (IgorEngine::*ExecuteActionProc)(int action); // pointer to room action function
	typedef void (IgorEngine::*UpdateRoomBackgroundProc)(); // pointer to function that executes on every frame

	typedef void (IgorEngine::*UpdateDialogueProc)(int action); // pointer to function that executes on every frame during dialogue
	void syncSoundSettings() override;
	void pauseEngineIntern(bool pause) override;

private:
	const IgorGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	MidiPlayer *_midiPlayer;
	uint32 _musicUpdateMillis;
	uint32 _musicTimerRemainder;
	uint32 _musicTimerDivisor;
	uint8 _musicRestartCounter;
	uint8 _musicRestartDelay;
	bool _musicSuspended;
	uint8 *_musicData;
	int _previousMusic;

	Common::File _ovlFile;
	Common::File _sndFile;
	Common::Array<byte> _unpackedExe;

	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _speechHandle;
	int _optionsControlRow;

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
	int _verbsPanelSize;
	int _screenVGAVOffset;

	bool _eventQuitGame;
	bool _gameStateLoaded;

	// pass of the end credits: 0 for the first one (set when the ending is over), the same "pages" are shown again after it
	uint8 _creditsPass;
	GameStateData _gameState;

	uint8 _mazeLocation;
	uint8 _mazeSavedLocation;
	const MazeRoom *_mazeRoom;
	int _mazeFlameFrame[2];
	uint32 _nextTimer;

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
	// Verb highlighted while the cursor is over a hotspot (Spanish floppy): kHoverVerbSelected = the selected verb
	int _hoverVerbState;
	int _keyboardCursorStep;
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
	bool _canSaveGame = true;
	Graphics::Surface _optionsMenuThumbnail;
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
	char _dialogueReplies[MAX_DIALOGUE_REPLIES][51];
	uint16 _dialogueQuestionSounds[MAX_DIALOGUE_QUESTIONS];
	uint16 _dialogueReplySounds[MAX_DIALOGUE_REPLIES];
	bool _dialogueEnded;
	int _dialogueChoiceSelected;
	uint8 _dialogueInfo[6];

	uint8 _objectsState[112];
	uint8 _inventoryImages[36];
	uint8 _inventoryInfo[kInventoryInfoSize];
	char _verbPrepositions[3][7];
	char _roomObjectNames[20][MAX_OBJECT_NAME_LENGTH];
	char _globalObjectNames[37][MAX_OBJECT_NAME_LENGTH];
	char _verbsName[9][MAX_VERB_NAME_LENGTH];

	bool _part07FirstVisitDone;

	uint8 _roomAmbientIndex;

	// the two fireflies of room 69: the current frame of each and the steps of the first one since the second one started
	uint8 *_part78BandFrames = nullptr;
	uint8 _part79EndFrame = 0;
	uint8 _part79BaseFrame = 0;
	uint8 _part69FireflyFrameA;
	uint8 _part69FireflyFrameB;
	uint16 _part69FireflyStepCount;
	uint8 _part32Glyphs[420];
	uint8 _part32DeclinationDigits[2];
	uint8 _part32AscensionDigits[3];
	int _part32DeclinationIndex;
	int _part32AscensionIndex;
	int _part32LineRow;
	int _part32Blink;
	int _part32Result;
	bool _part32Done;
	uint8 _part68QuestionCounter;
	uint8 _part68LastDialogueCode;
	bool _part68NpcPresent;
	uint8 _parkLadyIdleStep;

	uint8 _currentPalette[768];
	uint8 _paletteBuffer[768];
	uint8 _igorPalette[48];
	uint8 *_igorTempFrames;

	RoomWalkBounds _roomWalkBounds;
	RoomWalkTargetRules _roomWalkTargetRules;
	int _roomWalkTargetBottom; // the last row the click fix scans

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

	// _dialogueFont and _sentenceFont are the same on all versions
	// but the Spanish Floppy
	Font _font;
	FloppyDialogueFont _floppyDialogueFont;
	FloppySentenceFont _floppySentenceFont;
	const IgorFont *_dialogueFont;
	const IgorFont *_sentenceFont;
	uint8 _actionSentenceColors[3];

	ResourceEntry *_resourceEntries;
	Common::Array<StringEntry> _stringEntries;
	Common::Array<Common::String> _optionsMenuTexts;
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
	void playMusic(int num, bool restart = false);
	void updateMusic();
	void playSound(int num, int type);
	void playRoomSound(int sound, int spaFloppySound);
	void stopSound();
	int getTalkMode() const;
	int prepareDialoguePlayback(const DialogueText &text);
	void loadIgorFrames();
	void loadIgorBeachFrames();

	void ADD_DIALOGUE_TEXT(int num, int count, int sound = kNoSpeechSound);
	void SET_DIALOGUE_TEXT(int start, int count);
	void partMainSetExecuteAction(int i, ExecuteActionProc p);


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
	void waitForEndOfIgorDialogue(bool animateHead = true, bool updateRoomBackground = true);
	void fixDialogueTextPosition(int num, int count, int *x, int *y);
	void startCutsceneDialogue(int x, int y, int r, int g, int b);
	void waitForEndOfCutsceneDialogue(int x, int y, int r, int g, int b);

	bool isDialogueSpeechPlaying() const;
	void stopDialogueSpeech();
	void cancelDialogue();

	void partMain();
	void partMainMoveScreenUp(int offset);
	void partMainLookAtPapyrus(bool reveal);
	void partMainExecuteAction(int action);
	void partMainWaitForClick();
	void partMainLookAtNewspaper();
	void partMainLookAtPhoto();

	// map
	void part4ExecuteAction(int action);
	void part4();

	// Igors room
	void part0();
	void part0ExecuteAction(int action);
	void part0ApplyObjectState(int num);
	void part0EntryAnimation();
	void part0WalkInFromCloset();
	void part0EnterFromBelow();
	// Rooftop
	void part1();
	void part1ExecuteAction(int action);
	void part1CloseWindow();
	void part1DrawIgor();
	void part1DrawRoof();
	void part1DrawIgorsEyes(int frame);
	void part1DrawPigeons(int index);
	void part1PigeonsCutscene();
	void part1Explosion();
	// junk room
	void part2();
	void part2ExecuteAction(int action);
	void part2LightUpDynamite();
	void part2ApplyObjectState(int num);
	void part2SearchTrunk();
	void part2DrawFuseSpark(int frame);
	void part2UpdateFuse();
	void part2WalkWhileFuseBurns(int srcX, int srcY, int dstX, int dstY);

	// spring bridge
	void part5ApplyPaperState(int num);
	void part5WalkIgorToScene();
	void part5And6DrawPaperFrame(int frame);
	void part5And6SavePhotographerBackground();
	void part5And6DrawPhotographer();
	void part5And6DrawCamera(int frame);
	void part5And6DrawTripod(bool drawToScreen);
	void part5();
	void part5ExecuteAction(int action);
	void part5TakePaper();
	void part5ScrollRight();
	void part5UpdateBackground();

	// spring rock
	void part6();
	void part6UpdateBackground();
	void part6ExecuteAction(int action);
	void part6TalkToPhotographer();
	void part6HandlePhotographerDialogue();
	void part6UpdatePhotographerDialogue(int action);
	void part6DecodePhotographerTalkingFrame(int frame);
	void part6TakeCamera();
	void part6GiveAnythingToPhotographer();
	void part6GiveRocketToPhotographer();
	void part6ScrollLeft();
	void part6SetPhotographerState(int num);
	void part6AnimatePhotographer(int frame);
	void part6DrawBackgroundOverPhotographer();

	void part7();
	void part7ExecuteAction(int action);
	void part7DrawDoorState(int num);
	void part7OpenCloseDoor(int door, bool open);
	void part7DrawScaledIgor(int scaleStep, int facing, int frame, int dyPos);
	void part7EnterFromOutside();
	void part7ExitToOutside();

	// Dean's office
	void part8();
	void part8ExecuteAction(int action);
	void part8ApplyObjectState(int num);
	void part8DrawDoor(bool open);
	void part8TakeNewspaper();
	void part8DeanCallsSecretary();
	void part8TakeBook();
	void part8TalkToDean();
	void part8GiveBottleToDean();
	void part8DeanDrinksBottle();
	void part8DeanPassesOut();
	void part8HandleDeanDialogue();
	void part8HandleIntercomDialogue();
	void part8DrawDean();
	void part8DrawDeanTalkingFrame(int frame);
	void part8UpdateDeanDialogue(int action);
	void part8DrawSecretaryTalkingFrame(int frame);
	void part8UpdateSecretaryDialogue(int action);
	void part9();
	void part9ExecuteAction(int action);
	void part9ApplyObjectState(int num);
	void part9AnimateDoor(bool open);
	void part9DrawSecretaryFrame(int frame, bool background);
	void part9DrawOpenClosetFrame(int frame);
	void part9OpenFileCabinet();
	void part9OpenCloset(bool search);
	void part9TakeCostumeFromCloset();
	void part9UpdateSecretaryDialogue(int action);
	void part9DrawSecretaryTalkingFrame(int frame);
	void part9SecretarySearchesFile();
	void part9UpdateBackground();

	// Decanato right
	void part10();
	void part10ExecuteAction(int action);
	void part10TakeHamburger();
	void part10ScrollLeft();
	void part10And11DrawObjectState(int num);

	// Decanato left
	void part11();
	void part11ExecuteAction(int action);
	void part11TakeSlug();
	void part11UseButterflyNetWithHole();
	void part11ScrollRight();
	void part11TakeBottle();
	void part11ApplyObjectState(int num);

	// outside church
	void part12ExecuteAction(int action);
	void part12GoToPath();
	void part12TryEnterChurch();
	void part12UseResinWithStone();
	void part12EnterChurch();
	void part12UpdateBackground();
	void part12UpdatePriestDialogue(int action);
	void part12HandlePriestDialogue();
	void part12ApplyObjectState(int num);
	void part12DisablePathBehindChurch();
	void part12PaintOverLizard();
	void part12ExorcismCutscene();
	void part12EnterFromPath();
	void part12EnterFromChurch();
	void part12DrawIdlePriest();
	void part12DrawEntrance();
	void part12DrawPriestTalkingFrame(int frame);
	void part12();

	// inside church
	void part13ExecuteAction(int action);
	void part13GoToPlatform();
	void part13ExitChurch();
	void part13ApplyObjectState(int num);
	void part13EnterFromRight();
	void part13EnterFromBelow();
	void part13();

	// church puzzle
	void part14ExecuteAction(int action);
	void part14UpdateCandleFlicker();
	void part14GoToChurchBell();
	void part14LookAtPuzzle();
	void part14GoBackToChurch();
	void part14EnterMaze();
	void part14UseMatchesOnCandles();
	void part14ApplyObjectState(int num);
	void part14EnterFromChurch();
	void part14EnterFromChurchBell();
	void part14EnterFromMaze();
	void part14DrawEntrance();
	void part14CandleFlicker(int frame);
	void part14BrotherReveal(int start, int end);
	void part14ShowPuzzle();
	void part14LoadChurchMosaicData();
	void part14PushStone(int screenOffset, int w, int h, int animOffset);
	uint8 mosaicSentenceColor(int index) const;
	void part14();
	void part14LoadRoom();
	void part14LoadAnimation();

	// tobias office
	void part15ExecuteAction(int action);
	void part15LeaveRoom();
	void part15TalkToTobias();
	void part15GiveProjectToTobias();
	void part15GiveMoneyToTobias();
	void part15UpdateBackground();
	void part15UpdateTobiasDialogue(int action);
	void part15HandleTobiasDialogue();
	void part15ApplyObjectState(int num);
	void part15WalkIn();
	void part15UpdateCuckooClock();
	void part15WaitForCuckooClock();
	void part15AnimateTobiasIdle();
	void part15DrawCuckooFrame(int frame);
	void part15DrawIgorAndTobiasScene(int frame);
	void part15DrawTobiasTalking(int frame);
	void part15DrawTobiasIdleFrame(int frame);
	void part15();
	void part15Floppy();
	void part15FloppyExecuteAction(int action);
	void part15FloppyLeaveToStreet();
	void part15FloppyGoUpstairs();
	void part15FloppyGiveProjectToTobias();
	void part15FloppyGiveMoneyToTobias();
	void part15FloppyHandOver();
	void part15FloppyUpdateBackground();
	void part15FloppyApplyObjectState(int num);
	void part15FloppyEnterFromStreet();
	void part15FloppyEnterFromStairs();
	void part15FloppyDrawFigure(int figure, int frame);

	// laboratory
	void part16ExecuteAction(int action);
	void part16TakeFlask();
	void part16UpdateMargaretHarrisonDialogue(int action);
	void part16UpdateMargaretDialogue(int action);
	void part16ApplyObjectState(int num);
	void part16EnterScene();
	void part16PhotoCutscene();
	void part16DisplayPhoto();
	void part16DrawTalkingFrame(int frame);
	void part16();
	void part16LoadRoom();
	void part16LoadAnimation();

	// outside college
	void part17ExecuteAction(int action);
	void part17WalkIn(bool lateGame = false);
	void part17TalkToPhilipAndJimmy();
	void part17LeaveThroughPath(bool lateGame = false);
	void part17SwapFolders();
	void part17HandlePhilipDialogue();
	void part17UpdateJimmyDialogue(int action);
	void part17UpdatePhilipDialogue(int action);
	void part17UpdateBackground();
	void part17ApplyObjectState(int num);
	void part17WalkFromMap(bool lateGame = false);
	void part17SetYScaleRamp();
	void part17ClearEntryAreas();
	void part17DisableArea(int lum);
	void part17PaintFirstFrameOfPhilipAndJimmy();
	void part17ChangePathZIndex(int lum);
	void part17WalkFromCollege();
	void part17PhilipToJimmyAnimFrame(int num);
	void part17JimmyTalkingAnimFrame(int num);
	void part17PhilipToIgor(int frame);
	void part17();

	// men toilets
	void part18ExecuteAction(int action);
	void part18UseSlugOnGrating();
	void part18UseBottleOnTap();
	void part18ApplyObjectState(int num);
	void part18WalkIn();
	void part18();
	void part18LoadRoom();
	void part18LoadAnimation();

	// women toilets
	void part19ExecuteAction(int action);
	void part19LookAtSink();
	void part19UsePapyrusOnDryer();
	void part19UpdateWomenDialogue(int action);
	void part19UpdateBackground();
	void part19ApplyObjectState(int num);
	void part19SlugCutscene();
	void part19WalkIn();
	void part19EnterAndLeave();
	void part19DrawTalkingFrame(int frame);
	void part19();

	// college corridor margaret
	void part21ExecuteAction(int action);
	void part21LeaveLeft();
	void part21TalkToMargaret();
	void part21OpenPhysicsDoor();
	void part21ClosePhysicsDoor();
	void part21LeaveRight();
	void part21UseNoteWithSandwiches();
	void part21EnterPhysicsClass();
	void part21UpdateBackground();
	void part21UpdateMargaretBackgroundDialogue(int action);
	void part21UpdateMargaretCompositedDialogue(int action);
	void part21UpdateMargaretNoteDialogue(int action);
	void part21HandleMargaretDialogue();
	void part21ApplyObjectState(int num);
	void part21EnterFromLeft();
	void part21EnterFromRight();
	void part21EnterFromPhysicsClass();
	void part21WaitForIgorWalkWithAnimation();
	void part21DrawMargaretFrameComposited(int frame);
	void part21DrawClosedDoor();
	void part21DrawOpenDoor();
	void part21DrawMargaret();
	void part21MargaretTakesNote();
	void part21SetMargaretBackgroundFrame(int frame);
	void part21();
	void part21Floppy();
	void part21FloppyExecuteAction(int action);
	void part21FloppyLeaveLeft();
	void part21FloppyLeaveRight();
	void part21FloppyTalkToMargaret();
	void part21FloppyOpenPhysicsDoor();
	void part21FloppyClosePhysicsDoor();
	void part21FloppyUseNoteWithSandwiches();
	void part21FloppyEnterPhysicsClass();
	void part21FloppyUpdateBackground();
	void part21FloppyUpdateMargaretDialogue(int action);
	void part21FloppyUpdateMargaretNoteDialogue(int action);
	void part21FloppyHandleMargaretDialogue();
	void part21FloppyApplyObjectState(int num);
	void part21FloppyEnterFromLeft();
	void part21FloppyEnterFromRight();
	void part21FloppyEnterFromPhysicsClass();
	void part21FloppyDrawMargaretFrameComposited(int frame);
	void part21FloppyDrawMargaretHead(int frame);
	void part21FloppyDrawMargaretNoteHead(int frame);
	void part21FloppyMargaretTakesNote();

	// church bell tower
	void part22ExecuteAction(int action);
	void part22LookAtBell();
	void part22Leave();
	void part22ApplyObjectState(int num);
	void part22IgorStep(int facing, int step);
	void part22ScrollStep(int step);
	void part22Enter();
	void part22();

	// college corridor lucas
	void part23ExecuteAction(int action);
	void part23EnterLadiesRoom();
	void part23OpenDoor();
	void part23CloseDoor();
	void part23UpdateBackground();
	void part23ApplyObjectState(int num);
	void part23DrawDoor(int frame);
	void part23DrawLucas();
	void part23WalkFromDoor();
	void part23WalkFromLeft();
	void part23WalkFromRight();
	void part23DrawLucasFrame(int frame);
	void part23();
	void part23Floppy();
	void part23FloppyExecuteAction(int action);
	void part23FloppyEnterLadiesRoom();
	void part23FloppyOpenDoor();
	void part23FloppyCloseDoor();
	void part23FloppyApplyObjectState(int num);
	void part23FloppyEnterFromLeft();
	void part23FloppyEnterFromRight();
	void part23FloppyEnterFromLadiesRoom();

	// college corridor sharon michael
	void part24ExecuteAction(int action);
	void part24EnterChemistryClass();
	void part24CloseDoor();
	void part24OpenDoor();
	void part24GoToLibrary();
	void part24UpdateBackground();
	void part24ApplyObjectState(int num);
	void part24DrawSharonTalkingFrame(int frame);
	void part24DrawMichaelTalkingFrame(int frame);
	void part24DrawClosedDoor();
	void part24DrawOpenDoor();
	void part24EnterFromLeft();
	void part24EnterFromRight();
	void part24EnterFromChemistryClass();
	void part24();
	void part24Floppy();
	void part24FloppyExecuteAction(int action);
	void part24FloppyEnterChemistryClass();
	void part24FloppyOpenDoor();
	void part24FloppyCloseDoor();
	void part24FloppyApplyObjectState(int num);
	void part24FloppyEnterFromLeft();
	void part24FloppyEnterFromRight();
	void part24FloppyEnterFromChemistryClass();


	// college corridor announcement board
	void part25ExecuteAction(int action);
	void part25EnterMensToilet();
	void part25OpenDoor();
	void part25CloseDoor();
	void part25ApplyObjectState(int num);
	void part25EnterFromLeft();
	void part25EnterFromRight();
	void part25EnterFromMensToilet();
	void part25DrawClosedDoor();
	void part25DrawOpenDoor();
	void part25();
	void part25Floppy();
	void part25FloppyExecuteAction(int action);
	void part25FloppyEnterMensToilet();
	void part25FloppyOpenDoor();
	void part25FloppyCloseDoor();
	void part25FloppyApplyObjectState(int num);
	void part25FloppyEnterFromLeft();
	void part25FloppyEnterFromRight();
	void part25FloppyEnterFromMensToilet();

	// college corridor miss barrymore
	void part26ExecuteAction(int action);
	void part26OpenDoor();
	void part26CloseDoor();
	void part26EnterLab();
	void part26UpdateBackground();
	void part26ApplyObjectState(int num);
	void part26DrawClosedDoor();
	void part26DrawOpenDoor();
	void part26EnterFromRight();
	void part26EnterFromLaboratory();
	void part26DrawMissSullivanFrame(int frame);
	void part26();
	void part26Floppy();
	void part26FloppyExecuteAction(int action);
	void part26FloppyOpenDoor();
	void part26FloppyCloseDoor();
	void part26FloppyEnterLab();
	void part26FloppyApplyObjectState(int num);
	void part26FloppyEnterFromRight();
	void part26FloppyEnterFromLaboratory();

	// college lockers
	void part27ExecuteAction(int action);
	void part27OpenPhilipLocker();
	void part27CloseLocker();
	void part27OpenLocker();
	void part27UseFlaskOnLocker();
	void part27ApplyObjectState(int num);
	void part27EnterFromLeft();
	void part27DrawClosedLocker();
	void part27DrawOpenLocker();
	void part27EnterFromLibrary();
	void part27();
	void part27Floppy();
	void part27FloppyExecuteAction(int action);
	void part27FloppyDrawLockerFrame(int frame);
	void part27FloppyOpenPhilipLocker();
	void part27FloppyClosePhilipLocker();
	void part27FloppyTakeBottle();
	void part27FloppyUseFlaskOnLocker();
	void part27FloppyApplyObjectState(int num);
	void part27FloppyEnterFromLeft();

	// college corridor caroline
	void part28ExecuteAction(int action);
	void part28TalkToCaroline();
	void part28TakeFolder();
	void part28UpdateCarolineDialogue(int action);
	void part28UpdateBackground();
	void part28ApplyObjectState(int num);
	void part28DrawCaroline();
	void part28EnterFromRight();
	void part28DrawCarolineTalkingFrame(int frame);
	void part28EnterAndCheckGrades();
	void part28DrawFolder(int frame);
	void part28();
	void part28Floppy();
	void part28FloppyExecuteAction(int action);
	void part28FloppyTalkToCaroline();
	void part28FloppyTakeFolder();
	void part28FloppyUpdateCarolineDialogue(int action);
	void part28FloppyUpdateBackground();
	void part28FloppyApplyObjectState(int num);
	void part28FloppyDrawCaroline();
	void part28FloppyDrawCarolineFrame(int frame);
	void part28FloppyEnterFromRight();
	void part28FloppyEnterAndCheckGrades();

	// college corridor stairs first floor
	void part30ExecuteAction(int action);
	void part30GoUpstairs();
	void part30GoDownstairs();
	void part30UpdateLauraDialogue(int action);
	void part30HandleLauraDialogue();
	void part30ApplyObjectState(int num);
	void part30WalkInFromLeft();
	void part30WalkInFromUpstairs();
	void part30WalkInFromOutside();
	void part30WalkInFromRight();
	void part30LauraCutscene();
	void part30LauraAndIgorBumpIntoEachOther();
	void part30IgorCrossesArms();
	void part30LauraLeaves();
	void part30SetLauraFrame(int frame);
	void part30();
	void part30Floppy();
	void part30FloppyExecuteAction(int action);
	void part30FloppyGoUpstairs();
	void part30FloppyGoDownstairs();
	void part30FloppyUpdateLauraDialogue(int action);
	void part30FloppyHandleLauraDialogue();
	void part30FloppyApplyObjectState(int num);
	void part30FloppyWalkInFromLeft();
	void part30FloppyWalkInFromUpstairs();
	void part30FloppyWalkInFromOutside();
	void part30FloppyWalkInFromRight();
	void part30FloppyLauraCutscene();
	void part30FloppyLauraAndIgorBumpIntoEachOther();
	void part30FloppyIgorCrossesArms();
	void part30FloppyLauraLeaves();
	void part30FloppyDrawCutsceneFrame(int frame, bool keepTextColors);

	// college corridor stairs second floor
	void part31ExecuteAction(int action);
	void part31EnterAstronomyLab();
	void part31OpenDoor();
	void part31GoDownstairs();
	void part31UseKeysWithDoor();
	void part31UpdateBackground();
	void part31ApplyObjectState(int num);
	void part31DrawJohnnyFrame(int frame);
	void part31EnterFromLeft();
	void part31EnterFromAstronomyLab();
	void part31EnterFromDownstairs();
	void part31EnterFromRight();
	void part31DrawOpenDoor();
	void part31();
	void part31Floppy();
	void part20Floppy();
	void part20FloppyExecuteAction(int action);
	void part20FloppyEnterFromLeft();
	void part20FloppyEnterFromRight();
	void part31FloppyExecuteAction(int action);
	void part31FloppyEnterAstronomyLab();
	void part31FloppyOpenDoor();
	void part31FloppyGoDownstairs();
	void part31FloppyUseKeysWithDoor();
	void part31FloppyApplyObjectState(int num);
	void part31FloppyEnterFromLeft();
	void part31FloppyEnterFromAstronomyLab();
	void part31FloppyEnterFromDownstairs();
	void part31FloppyEnterFromRight();
	void part31FloppyDrawDoorFrame(int frame);

	// telescope room
	void part32ExecuteAction(int action);
	void part32WalkIn();
	void part32UseControls();
	void part32WalkToControls();
	void part32Controls();
	void part32CopyControlColor(int dstColor, int srcColor);
	int part32ControlKeyAtCursor() const;
	void part32DrawDigit(int fieldOffset, int index, int key);
	void part32EraseDigit(int fieldOffset, int index);
	void part32ControlsIdle(int fieldColor);
	void part32ExitControls();
	void part32SelectDeclination();
	void part32SelectAscension();
	void part32EnterCoordinates();
	int part32AnimateDeclination();
	int part32AnimateAscension();
	void part32LoadTowerScene();
	void part32LoadGirlScene();
	void part32DrawTowerConversationFrame(int frame);
	void part32UpdateSpeakerADialogue(int action);
	void part32UpdateSpeakerBDialogue(int action);
	void part32SaySpeakerA(const Common::Array<DialogueText> &lines);
	void part32SaySpeakerB(const Common::Array<DialogueText> &lines);
	void part32Scene();
	void part32SceneTowerConversation();
	void part32SceneTowerEmpty();
	void part32SceneNothing();
	void part32SceneGirl();
	void part32();

	// library
	void part33ExecuteAction(int action);
	void part33TakeAstronomyBooks();
	void part33TalkToHarrison();
	void part33WalkOut();
	void part33ShowPhotoToHarrison();
	void part33GiveNoteToHarrison();
	void part33UpdateHarrisonBodyDialogue(int action);
	void part33UpdateHarrisonHeadDialogue(int action);
	void part33UpdateHarrisonHeadDialogueWithStanding(int action);
	void part33SayHarrison(const Common::Array<DialogueText> &lines);
	void part33HandleHarrisonDialogue();
	void part33UpdateBackground();
	void part33ApplyObjectState(int num);
	void part33HarrisonWalksOut();
	void part33DrawHarrisonFrame();
	void part33DrawHarrisonBodyFrame(int frame);
	void part33DrawHarrisonHeadFrame(int frame);
	void part33WalkIn();
	void part33LoadExtraTexts();
	void part33();
	void part33LoadRoom();
	void part33LoadAnimation();

	// park
	void part34And35DrawOldLadyFrame(uint8 *dst, int frame);
	void part34DrawLauraFrame(int frame);
	void part34And35PickUpAnimation(int screenOffset, int framesOffset);
	void part34And35UpdateAmbientSound();
	void part34UpdateLadyDialogue(int action);
	void part34UpdateLauraDialogue(int action);
	void part34QueueReplyText(int reply, int count, int sound, int &textIndex);
	void part34WaitForIgorDialogue();
	void part34WaitForLadyDialogue();
	void part34WaitForLauraDialogue();
	void part34();
	void part34ExecuteAction(int action);
	void part34TakeResin();
	void part34TalkToOldLady();
	void part34GiveLizardToOldLady();
	void part34ScrollRight();
	void part34ApplyObjectState(int num);
	void part34OldLadyIdleAnimationFrame(int step);
	void part34LauraConversation();
	void part34OldLadyConversation();
	void part34UpdateBackground();
	void part35();
	void part35ExecuteAction(int action);
	void part35TakeResin();
	void part35ExitToMap();
	void part35ScrollLeft();
	void part35ApplyObjectState(int num);

	// chemistry classroom
	void part36ExecuteAction(int action);
	void part36TakePen();
	void part36ApplyObjectState(int num);
	void part36WalkIn();
	void part36DrawPen(int frame);
	void part36();

	// physics classroom
	void part37ExecuteAction(int action);
	void part37LookAtPaperBinTakePaper();
	void part37ApplyObjectState(int num);
	void part37WalkIn();
	void part37();

	// outside the maze
	void part50ExecuteAction(int action);
	void part50EnterMaze();
	void part50EnterFromDoor();
	void part50EnterFromHill();
	void part50UpdateBackground();
	void part50();

	// cemetery entrance
	void part68ExecuteAction(int action);
	void part68LoadRoom();
	void part68ApplyObjectState(int num);
	void part68DrawUndertaker(int frame);
	void part68DrawAmbient(int frame);
	void part68DrawVase(int frame);
	void part68DrawPose(int frame);
	void part68UpdateOwlsEyes();
	void part68UpdateBackground();
	void part68PlayAmbientSound();
	void part68UpdateBackgroundTalk();
	void part68SayBorisReplies(int action);
	void part68SayBoris(const Common::Array<DialogueText> &lines);
	void part68SayBorisReplies(const int *replies, const int *sounds, int count);
	void part68EnterFromRight();
	void part68EnterFromLeft();
	void part68EnterFromCemetery();
	void part68TakeVase();
	void part68TalkToBoris();
	void part68EnterCemetery();
	void part68GiveRecordsToBoris();
	void part68Conversation();
	void part68ChooseRandomNames(int code);
	void part68BorisLeaves();
	void part68();

	// swamp
	void part69ExecuteAction(int action);
	void part69ApplyObjectState();
	void part69DrawSwarm(int frame);
	void part69DrawMainFirefly(int frame);
	void part69AnimateFireflies();
	void part69UseVaseWithFireflies();
	void part69TakeVaseBack(int num, int sound);
	void part69EnterFromRight();
	void part69UpdateBackground();
	void part69();

	// map of second area
	void part70ExecuteAction(int action);
	void part70DrawObjectState();
	void part70UpdateBackground();
	void part70();

	// cemetery
	void part71ExecuteAction(int action);
	void part71ApplyObjectState(int what);
	void part71DrawObject(int frame);
	void part71TakeObject();
	void part71Look();
	void part71EnterFromRight();
	void part71Lightning();
	void part71UpdateBackground();
	void part71();

	// forest
	void part72ExecuteAction(int action);
	void part72ApplyObjectState();
	void part72TakeFlower();
	void part72EnterFromMap();
	void part72EnterFromSwamp();
	void part72UpdateBlinkingEyes();
	void part72UpdateBackground();
	void part72();
	void part72PaintBlinkingEyes();

	// police cutscene
	void part73DrawPoliceman(int frame);
	void part73UpdateDialogue(int action);
	void part73Cutscene();

	// ship cutscene
	void part74CycleColors();
	void part74DrawCharacterFrame(int frame);
	void part74UpdateSailorDialogue(int action);
	void part74UpdateCaptainADialogue(int action);
	void part74UpdateCaptainBDialogue(int action);
	void part74Say(int who, const Common::Array<DialogueText> &lines);
	void part74ShipCutscene();

	// philip vodka cutscene
	void part75UpdatePhilipDialogue(int action);
	void part75DrawPhilipFrame(int frame);
	void part75();

	// plane
	void part76ExecuteAction(int action);
	void part76ApplyObjectState();
	void part76DrawParachuteFrame(int frame);
	void part76DrawOpenDoorFrame(int frame);
	void part76OpenDoor();
	void part76TakeParachute();
	void part76UseParachute();
	void part76Entry();
	void part76();

	// college entrance (with cage)
	void part77ExecuteAction(int action);
	void part77ApplyObjectState();
	void part77DrawMonkeyFrame(int frame);
	void part77UpdateDialogue(int action);
	void part77MonkeyCageCutscene();
	void part77EnterFromCollege();
	void part77();

	// Exam cutscene
	void part78DrawProfessorFrame(int frame);
	void part78DrawAlumniFrame(int frame);
	void part78DrawIgorFrame(int frame);
	void part78DrawPhilipFrame(int frame);
	void part78UpdateProfessorDialogueLeft(int action);
	void part78UpdateProfessorDialogueFromDesk(int action);
	void part78UpdateProfessorDialogueToIgor(int action);
	void part78UpdateIgorDialogueLeft(int action);
	void part78UpdateIgorDialogueRight(int action);
	void part78UpdatePhilipDialogue(int action);
	void part78SayStart(int speaker, const Common::Array<DialogueText> &lines);
	void part78WaitForLines(int speaker);
	void part78Say(int speaker, const Common::Array<DialogueText> &lines);
	void part78ProfessorWarnings(int which);
	void part78();

	// Ending
	void part79DrawLauraInWater(int frame);
	void part79DrawReflection(int step);
	void part79ClearReflection();
	void part79UpdateReflection();
	void part79UpdateLauraInWaterDialogue(int action);
	void part79SayLaura(const Common::Array<DialogueText> &lines);
	void part79Scroll();
	void part79LauraInWater();
	void part79DrawGroupFrame(const uint8 *buffer, int table, int base, int frame);
	void part79DrawBoomerangScene(int frame);
	void part79DrawKiss(int frame);
	void part79DrawCircularClosingTransition(int frame);
	void part79DrawPhilipSharkScene(const void *group, int frame);
	void part79AnimateIgorHead(int frame);
	void part79UpdateSpeakerDialogue(int action);
	void part79UpdateLauraDialogue(int action);
	void part79UpdatePhilipDialogue(int action);
	void part79AnimatePhilipShaking();
	void part79SaySpeaker(int startX, int startY, int x, int y, int r, int g, int b, int endFrame, int baseFrame, const Common::Array<DialogueText> &lines);
	void part79SayLaura(int startX, int startY, const Common::Array<DialogueText> &lines);
	void part79SayPhilip(const Common::Array<DialogueText> &lines);
	void part79SayIgor(const Common::Array<DialogueText> &lines);
	void part79SetIgorPos(int x, int y);
	void part79FadeColors(int first, int last, bool fadeInColors);
	void part79FadeInPair(int firstColor);
	void part79BeachScene();
	void part79();

	// Library cutscene
	void part80DrawFrame(int frame);
	void part80UpdateFriendDialogue(int action);
	void part80UpdateLauraDialogue(int action);
	void part80Say(int who, const Common::Array<DialogueText> &lines);
	void part80();

	// Lighthouse
	void part81ExecuteAction(int action);
	void part81LoadRoom();
	void part81LoadAnimation();
	void part81DrawLighthouseKeeperHead(int frame);
	void part81DrawLighthouseKeeper(int frame);
	void part81DrawIgor(int offset);
	void part81DrawLighthouseFrame(int frame);
	void part81UpdateBackground();
	void part81UpdateLighthouseKeeperDialogue(int action);
	void part81SayLighthouseKeeper(const Common::Array<DialogueText> &lines);
	void part81SayIgor(int num, int count, int sound);
	void part81ApplyObjectState(int num);
	void part81ScrollIn();
	void part81TransitionScrollUp();
	void part81TransitionScrollDown();
	void part81LighthouseSendMessage();
	void part81GiveLampToLighthouseKeeper();
	void part81RejectItem();
	void part81Talk();
	void part81DialogueCode(int code);
	void part81HandleDialogue();
	void part81WalkIn();
	void part81();

	// maze
	static const MazeNode MAZE_NODES[108];
	static const MazeRoom *partMazeGetRoom(int part);
	void partMaze();
	void partMazeExecuteAction(int action);
	void partMazeUpdateBackground();
	void partMazeMaybeUpdateFlicker();
	void partMazeFlicker();
	void partMazeDrawFlameFrame(int x, int y, int frame);
	void partMazeWalkIn(const MazeEntry &entry);
	void partMazeEnterFromStairs(const MazeEntry &entry);
	void partMazeWalkToExit(const MazeAction &action);
	void partMazeExitThroughStairs(const MazeAction &action);
	void partMazeGoToNeighbor(int dir);
	int partMazeGetMusicTrack() const;


	void partMainMeanwhile(int img = IMG_Meanwhile);

	// Margaret cutscenes
	void partMargaretDrawBasePose();
	void partMargaretDrawFrame(int frame);
	void partMargaretUpdateDialogue(int action);
	void partMargaret();

	// Intro
	void part85();
	void part85PlayAnimationFrame(int frameOffset2, int frameOffset1, int firstFrame, int lastFrame, int delay);
	void part85ScrollRight();
	void part85DisplayLogo();
	void part85UpdatePhilipLauraDialogue(int action);
	void part85UpdateBackground();
	void part85AnimateIgorHead(int frame);

	// Splash screens
	void part95();
	void part90();
	void part90FadePalette(bool fadeInColors);
	bool part90Hold(uint16 duration, int skipMode, bool optionsMenu);
	void part90Highlight(bool brighten);
	void part90SetUserInterfacePalettes();

	// End credits
	void partCredits();
	void partCreditsDrawText(int x, int y, uint8 color, bool underline, const char *str);

	void setRoomClickFix(int yMax, int xMin, int xMax, bool scanUp, int yBottom = 143);

	void partMainUpdateObjectState(int num);
	void partMainUpdateFiguresOnPaper(int delay);

	void handleRoomInput();

	void executeAction(int action);
	void clearAction(bool redraw = true);
	void formatActionSentence(uint8 color);
	void drawActionSentence(const char *sentence, uint8 color);
	void handleRoomIgorWalk();

	void drawVerbsPanel();
	void redrawVerb(uint8 verb, bool highlight);
	void beginRightButtonVerbPick();
	int getDefaultVerbAt(int x, int y) const;
	void updateHoverVerbHighlight();
	bool isCursorKey(Common::KeyCode key) const;
	bool moveCursorWithKeyboard(Common::KeyCode key);
	void updateRightButtonVerbPick();
	void endRightButtonVerbPick();
	void formatRightButtonSentence();
	int getVerbUnderCursor(int x) const {
		if (_game.version == kIdSpaFloppy)
			return kVerbWalk + x / 40;
		return ((x % 46) < 44) ? (kVerbTalk + x / 46) : 0;
	}
	void handleRoomInventoryScroll();
	void loadOptionsMenuTexts();
	void drawOptionsMenuText(int text, int x, int y, int color);
	void handleOptionsMenu();
	void loadFloppyOptionsMenuTexts();
	void floppyOptionsDrawText(int text, int x, int y, int color);
	void floppyOptionsFillRow(const uint8 *button, int buttonRow, int y);
	void floppyOptionsDrawMainPage(const uint8 *button);
	void floppyOptionsClearCaptionBar(const uint8 *button);
	void floppyOptionsPaintControl(const uint8 *button, int row);
	void floppyOptionsPresent(int selectedButton);
	void floppyOptionsQuitPage(const uint8 *button);
	void floppyOptionsControlPage(const uint8 *button);
	void handleFloppyOptionsMenu();
	void scrollInventory();
	void drawInventory(int start, int mode);
	void pageFloppyInventory(bool forward);
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

	// tickFunc: function that runs on every iteration while Igor walks (rooms with their own wait loop)
	bool waitForIgorMove(IgorMoveTick tickFunc = 0, bool escapeSkips = false, bool forceSkip = false, int fixedPosNum = -1);

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

	bool isCDVersion() const { return !(_game.flags & kFlagFloppy); }
	// The two full floppy releases (the demos also carry the floppy flag)
	bool isFullFloppyVersion() const { return _game.version == kIdEngFloppy || _game.version == kIdSpaFloppy; }
	int getPart() const { return _currentPart / 10; }
	bool compareGameTick(int add, int mod) const { return ((_gameTicks + (add & ~7)) % mod) == 0; }
	bool compareGameTick(int eq) const { return _gameTicks == (eq & ~7); }

	void waitForTimer(int ticks = -1, bool presentScreen = true, uint32 deadline = 0);

	ResourceEntry *findData(int num);
	void unpackExecutable();
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
	int dialogueChoiceOffset(int i) const;
	Common::String dialogueChoiceText(int choice, const char *prefix) const;
	void drawFloppyDialogueChoice(int choice, int color);
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
	bool copyOptionsMenuThumbnail(Graphics::Surface &thumb) const {
		if (!_optionsMenuThumbnail.getPixels())
			return false;
		thumb.copyFrom(_optionsMenuThumbnail);
		return true;
	}

	void handleOptionsMenu_paintQuit();
	bool handleOptionsMenu_handleKeyDownQuit(int key);
	void handleOptionsMenu_paintCtrl();
	bool handleOptionsMenu_handleKeyDownCtrl(int key);

public:
	IgorEngine(OSystem *syst, const IgorGameDescription *gameDesc);
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
		return _canSaveGame;
	}

	void handlePause();

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
	static const RoomDataOffsets PART_15_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_16_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_17_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_18_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_19_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_21_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_21_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_22_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_23_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_24_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_25_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_25_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_26_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_26_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_27_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_27_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_28_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_30_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_30_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_31_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_31_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_20_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_23_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_28_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_24_FLOPPY_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_32_ROOM_DATA_OFFSETS;
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
	static const RoomDataOffsets PART_76_ROOM_DATA_OFFSETS;
	static const RoomDataOffsets PART_77_ROOM_DATA_OFFSETS;
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
	static const uint8 PAL_IGOR_2[];
	static const uint8 PAL_48_1[];
	static const uint8 PAL_96_1[];
};

extern IgorEngine *g_engine;
#define SHOULD_QUIT ::Igor::g_engine->shouldQuit()

} // End of namespace Igor

#endif // IGOR_H
