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

#include "igor/detection.h"
#include "igor/resource_ids.h"

namespace Igor {

struct IgorGameDescription;

enum {
	kDebugEngine   = 400 << 1,
	kDebugResource = 400 << 2,
	kDebugScreen   = 400 << 3,
	kDebugWalk     = 400 << 4,
	kDebugGame     = 400 << 5
};

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
	kQuickSaveSlot = 0,
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
	MAX_DIALOGUE_REPLIES = 70
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
	kInputVarCount
};

struct RoomWalkBounds {
	int x1, y1;
	int x2, y2;
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

enum {
	kIdEngDemo100,
	kIdEngDemo110,
	kIdEngFloppy,
	kIdSpaFloppy,
	kIdEngCD,
	kIdSpaCD
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


class IgorEngine : public Engine {
public:

	typedef void (IgorEngine::*UpdateRoomBackgroundProc)();
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	// MidiPlayer *_midiPlayer;

	Common::File _ovlFile;

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

	int _language;
	DetectedGameVersion _game;
	int _currentCursor;
	bool _roomCursorOn;

	char _globalDialogueTexts[300][MAX_DIALOGUE_TEXT_LENGTH];
	uint8 _walkXScaleRoom[320];
	uint8 _walkYScaleRoom[144 * 3];
	RoomObjectArea _roomObjectAreasTable[MAX_ROOM_OBJECT_AREAS];
	uint8 _roomActionsTable[0x2000];


	Action _currentAction;
	uint8 _actionCode;
	uint8 _actionWalkPoint;

	int16 _inputVars[kInputVarCount];
	WalkData _walkData[100];
	uint8 _walkCurrentPos;
	uint8 _walkDataLastIndex;
	uint8 _walkDataCurrentIndex;
	uint8 _walkCurrentFrame;
	int _walkDataCurrentPosX, _walkDataCurrentPosY;
	int _walkToObjectPosX, _walkToObjectPosY;

	int16 _currentPart;


	char _verbPrepositions[3][7];
	char _roomObjectNames[20][MAX_OBJECT_NAME_LENGTH];
	char _globalObjectNames[35][MAX_OBJECT_NAME_LENGTH];
	char _verbsName[9][MAX_VERB_NAME_LENGTH];

	uint8 _currentPalette[768];
	uint8 _paletteBuffer[768];
	uint8 _igorPalette[48];
	uint8 *_igorTempFrames;

	RoomWalkBounds _roomWalkBounds;
	RoomDataOffsets _roomDataOffsets;
	UpdateRoomBackgroundProc _updateRoomBackground;

	int _gameTicks;
	int _resourceEntriesCount;
	int _soundOffsetsCount;
	uint32 *_soundOffsets;

	ResourceEntry *_resourceEntries;
	Common::Array<StringEntry> _stringEntries;

	static const uint8 _sentenceColorIndex[];
	static const uint8 _fontCharIndex[];
	static const uint8 _fontCharWidth[];
	static const uint32 _fontData[];
	static const uint8 _walkWidthScaleTable[];
	static const uint8 _walkScaleTable[];
	static const float _walkScaleSpeedTable[];
	static const uint8 _mouseCursorMask[];
	static const uint8 _mouseCursorData[];

	void restart();
	void setupDefaultPalette();

	void readTableFile();
	void loadMainTexts();
	const char *getString(int id) const;
	void loadIgorFrames();

	void PART_MAIN();
	void PART_05();

	void PART_05_UPDATE_ROOM_BACKGROUND();

	void handleRoomInput();
	void formatActionSentence(uint8 color);
	int getStringWidth(const char *s) const;
	void drawActionSentence(const char *sentence, uint8 color);
	void drawString(uint8 *dst, const char *s, int x, int y, int color1, int color2, int color3);
	void drawChar(uint8 *dst, int chr, int x, int y, int color1, int color2, int color3);
	void handleRoomIgorWalk();

	void enterPartLoop();
	void leavePartLoop();
	void runPartLoop();

	int lookupScale(int xOffset, int yOffset, int h) const;
	void lookupScale(int curX, int curY, uint8 &scale, uint8 &xScale, uint8 &yScale) const;

	void buildWalkPathArea(int srcX, int srcY, int dstX, int dstY);
	int getVerticalStepsCount(int minX, int minY, int maxX, int maxY);
	int getHorizontalStepsCount(int minX, int minY, int maxX, int maxY);
	void buildWalkPathAreaUpDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaDownDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaRightDirection(int srcX, int srcY, int dstX, int dstY);
	void buildWalkPathAreaLeftDirection(int srcX, int srcY, int dstX, int dstY);
	void waitForIgorMove();

	void moveIgor(int pos, int frame);

	void setCursor(int num);
	void showCursor();
	void hideCursor();

	void scrollPalette(int startColor, int endColor);
	void setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b);
	void setPaletteRange(int startColor, int endColor);
	void updatePalette(int count);

	void fadeInPalette(int count);
	void fadeOutPalette(int count);

	void decodeRoomStrings(const uint8 *p, bool skipObjectNames = false);
	void decodeRoomText(const uint8 *p);
	void decodeRoomAreas(const uint8 *p, int count);
	void decodeRoomMask(const uint8 *p);

	bool compareGameTick(int add, int mod) const { return ((_gameTicks + (add & ~7)) % mod) == 0; } // { return ((_gameTicks + add) % mod) == 0; }
	bool compareGameTick(int eq) const { return _gameTicks == (eq & ~7); } // { return _gameTicks == eq; }

	void waitForTimer(int ticks = -1);

	ResourceEntry *findData(int num);
	uint8 *loadData(int num, uint8 *dst = 0, int *size = 0);
	void loadRoomData(int pal, int img, int box, int msk, int txt);

	void setRoomWalkBounds(int x1, int y1, int x2, int y2);

	void buildWalkPath(int srcX, int srcY, int dstX, int dstY);

public:
	Graphics::Screen *_screen = nullptr;

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
	static const uint8 PAL_48_1[];
	static const uint8 PAL_96_1[];
	static const uint8 PAL_IGOR_1[];
	static const uint8 PAL_IGOR_2[];
};

extern IgorEngine *g_engine;
#define SHOULD_QUIT ::Igor::g_engine->shouldQuit()

} // End of namespace Igor

#endif // IGOR_H
