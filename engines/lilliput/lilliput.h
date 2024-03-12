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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LILLIPUT_LILLIPUT_H
#define LILLIPUT_LILLIPUT_H

#include "lilliput/console.h"
#include "lilliput/script.h"
#include "lilliput/sound.h"
#include "lilliput/stream.h"
#include "lilliput/detection.h"

#include "common/file.h"
#include "common/rect.h"
#include "common/events.h"

#include "engines/engine.h"
#include "graphics/surface.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Lilliput engine.
 *
 * Status of this engine:
 * - Adventures of Robin Hood is mostly working without sound
 *
 * Games using this engine:
 * - Adventures of Robin Hood
 * - Rome: Pathway to Rome
 */
namespace Lilliput {

static const int kSavegameVersion = 1;

enum LilliputDebugChannels {
	kDebugEngine    = 1 << 0,
	kDebugScript    = 1 << 1,
	kDebugSound     = 1 << 2,
	kDebugEngineTBC = 1 << 3,
	kDebugScriptTBC = 1 << 4
};

enum InterfaceHotspotStatus {
	kHotspotOff      = 0,
	kHotspotDisabled = 1,
	kHotspotEnabled  = 2,
	kHotspotSelected = 3
};

#define kSeqNone	0
#define kSeqNoInc	1 << 0
#define kSeqRepeat	1 << 1

struct LilliputGameDescription;

struct SmallAnim {
	bool _active;
	Common::Point _pos;
	int16 _frameIndex[8];
};

class LilliputEngine : public Engine {
public:
	LilliputEngine(OSystem *syst, const LilliputGameDescription *gd);
	~LilliputEngine() override;

	OSystem *_system;

	Common::RandomSource *_rnd;
	LilliputScript *_scriptHandler;
	LilliputSound *_soundHandler;
	Graphics::Surface *_mainSurface;

	SmallAnim _smallAnims[4];
	int _smallAnimsFrameIndex;

	byte _handleOpcodeReturnCode;
	byte _keyDelay;
	byte _lastAnimationTick;
	byte _animationTick;
	Common::Point _nextDisplayCharacterPos;
	byte _int8Timer;
	Common::Event _lastKeyPressed;
	Common::EventType _lastEventType;
	byte _keyboard_nextIndex;
	byte _keyboard_oldIndex;
	Common::Event _keyboard_buffer[8];
	byte _byte12A05;
	bool _refreshScreenFlag;
	byte _byte16552;
	int8 _lastInterfaceHotspotIndex;
	byte _lastInterfaceHotspotButton; // Unused: set by 2 functions, but never used elsewhere
	byte _debugFlag; // Mostly useless, as the associated functions are empty
	byte _debugFlag2; // Unused byte, set by an opcode

	byte _codeEntered[3];
	char _homeInDirLikelyhood[4];
	byte *_bufferIsoMap;
	byte *_bufferCubegfx;
	byte *_bufferMen;
	byte *_bufferMen2;
	byte *_bufferIsoChars;
	byte *_bufferIdeogram;
	byte *_normalCursor;
	byte *_greenCursor;
	byte _curPalette[768];
	byte _displayStringBuf[160];

	bool _saveFlag;
	bool _displayMap;

	int _word10800_ERULES;
	byte _numCharacters;
	Common::Point _currentScriptCharacterPos;
	int _nextCharacterIndex;
	int8 _waitingSignal;
	int8 _waitingSignalCharacterId;
	uint16 _newModesEvaluatedNumber;
	Common::Point _savedSurfaceUnderMousePos;
	bool _displayGreenHand;
	bool _isCursorGreenHand;
	int _currentDisplayCharacter;
	int _displayStringIndex;
	int _signalTimer;
	Common::Point _curCharacterTilePos;

	int16 _mapSavedPixelIndex[40];
	byte _mapSavedPixel[40];
	int16 _characterSignals[40];
	int16 _signalArr[40];
	int16 _signalArray[30];

	byte *_sequencesArr;
	int16 _currentScriptCharacter;
	Common::Point _characterPos[40];
	int8 _characterPosAltitude[40];
	int16 _characterFrameArray[40];
	int8 _characterCarried[40];
	int8 _characterBehindDist[40];
	byte _characterAboveDist[40];
	byte _spriteSizeArray[40];
	byte _characterDirectionArray[40];
	byte _characterMobility[40];
	byte _characterTypes[40];
	byte _characterBehaviour[40];
	Common::Point _characterHomePos[40];
	byte _characterVariables[1400 + 3120];
	byte *_currentCharacterAttributes;
	byte _poseArray[40 * 32];
	int *_packedStringIndex;
	int _packedStringNumb;
	char *_packedStrings;
	byte *_initScript;
	int _initScriptSize;
	byte *_menuScript;
	int _menuScriptSize;
	int *_arrayGameScriptIndex;
	int _gameScriptIndexSize;
	byte *_arrayGameScripts;
	byte _cubeFlags[60];
	byte _listNumb;
	int16 *_listIndex;
	byte *_listArr;
	int16 _rectNumb;
	Common::Rect _enclosureRect[40];
	Common::Point _keyPos[40];
	Common::Point _portalPos[40];
	int _interfaceHotspotNumb;
	byte _interfaceTwoStepAction[20];
	Common::Point _interfaceHotspots[20];
	Common::KeyCode _keyboardMapping[20];
	Common::Point _characterTargetPos[40];
	byte _savedSurfaceUnderMouse[16 * 16];
	byte _charactersToDisplay[40];
	Common::Point _characterRelativePos[40];
	Common::Point _characterDisplay[40];
	int8 _characterMagicPuffFrame[40];
	Common::Point _characterSubTargetPos[40];
	byte _specialCubes[40];
	byte _doorEntranceMask[4];
	byte _doorExitMask[4];
	byte _savedSurfaceGameArea1[176 * 256]; // 45056
	byte _savedSurfaceGameArea2[176 * 256]; // 45056
	byte _savedSurfaceGameArea3[176 * 256]; // 45056
	byte _savedSurfaceSpeech[16 * 252];

	const LilliputGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void newInt8();
	void update();

	void display16x16IndexedBuf(byte *buf, int index, Common::Point pos, bool transparent = true, bool updateScreen = true);
	void display16x16Buf(byte *buf, Common::Point pos, bool transparent = true, bool updateScreen = true);
	void fill16x16Rect(byte col, Common::Point pos);
	void saveSurfaceGameArea();
	void saveSurfaceSpeech();
	void displayInterfaceHotspots();
	void displayLandscape();
	void displaySpeechBubble();
	void displaySpeech(byte *buf);
	void initGameAreaDisplay();
	void displayIsometricBlock(byte *buf, int var1, int posX, int posY, int var3);
	void displayGameArea();
	void prepareGameArea();
	void displayRefreshScreen();
	void restoreSurfaceSpeech();
	void displayCharacterStatBar(int8 type, int16 averagePosX, int8 score, int16 posY);
	void displayCharacter(int index, Common::Point pos, int flags);
	void displayString(byte *buf, Common::Point pos);
	void displayChar(int index, int var1);
	void displaySmallAnims();
	void displaySmallIndexedAnim(byte index, byte subIndex);

	void unselectInterfaceHotspots();
	void startNavigateFromMap();
	void resetSmallAnims();
	void paletteFadeOut();
	void paletteFadeIn();

	void sortCharacters();
	void scrollToViewportCharacterTarget();
	void viewportScrollTo(Common::Point goalPos);
	void checkSpeechClosing();
	void updateCharPosSequence();
	void evaluateDirections(int index);
	byte homeInAvoidDeadEnds(int indexb, int indexs);
	void signalDispatcher(byte type, byte index, int var4);
	void sendMessageToCharacter(byte index, int var4);
	int16 checkEnclosure(Common::Point pos);
	int16 checkOuterEnclosure(Common::Point pos);
	byte sequenceSetMobility(int index, Common::Point var1);
	byte sequenceEnd(int index);
	void homeInPathFinding(int index);

	void renderCharacters(byte *buf, Common::Point pos);

	void checkNumericCode();
	void keyboard_handleInterfaceShortcuts(bool &forceReturnFl);
	byte sequenceCharacterHomeIn(int index, Common::Point param1);
	byte getDirection(Common::Point param1, Common::Point param2);
	void addCharToBuf(byte character);
	void numberToString(int param1);
	void handleCharacterTimers();
	byte sequenceMoveCharacter(int idx, int moveType, int poseType);
	void setCharacterPose(int idx, int poseIdx);
	void checkSpecialCubes();
	void checkInteractions();
	byte sequenceSetCharacterDirection(int index, int direction, int poseType);
	void handleSignals();
	void checkInterfaceActivationDelay();
	int16 checkObstacle(int x1, int y1, int x2, int y2);
	void displayCharactersOnMap();
	void restoreMapPoints();
	void displayHeroismIndicator();
	void handleGameMouseClick();
	void handleInterfaceHotspot(byte index, byte button);
	void checkInterfaceHotspots(bool &forceReturnFl);
	bool isMouseOverHotspot(Common::Point mousePos, Common::Point hotspotPos);
	void checkClickOnCharacter(Common::Point pos, bool &forceReturnFl);
	void checkClickOnGameArea(Common::Point pos);
	void displaySpeechBubbleTail(Common::Point displayPos);
	void displaySpeechBubbleTailLine(Common::Point pos, int var2);
	void displaySpeechLine(int vgaIndex, byte *srcBuf, int &bufIndex);
	void checkMapClosing(bool &forceReturnFl);
	void turnCharacter1(int index);
	void turnCharacter2(int index);
	void moveCharacterUp1(int index);
	void moveCharacterUp2(int index);
	void moveCharacterDown1(int index);
	void moveCharacterDown2(int index);
	void moveCharacterSpeed2(int index);
	void moveCharacterSpeed4(int index);
	void moveCharacterBack2(int index);
	void moveCharacterSpeed3(int index);
	void moveCharacterForward(int index, int16 speed);
	void checkCollision(int index, Common::Point pos, int direction);
	byte sequenceSeekMovingCharacter(int index, Common::Point var1);
	byte sequenceSound(int index, Common::Point var1);
	byte sequenceRepeat(int index, Common::Point var1, int tmpVal);
	void homeInChooseDirection(int index);

	void initGame(const LilliputGameDescription *gd);
	byte *loadVGA(const Common::Path &filename, int fileSize, bool loadPal);
	byte *loadRaw(const Common::Path &filename, int filesize);
	void loadRules();

	void displayVGAFile(const Common::Path &fileName);
	void initPalette();
	void fixPaletteEntries(uint8 *palette, int num);

	GameType getGameType() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const override;
	const char *getCopyrightString() const;

	Common::String getSavegameFilename(int slot);
	void syncSoundSettings() override;

	Common::Point _mousePos;
	Common::Point _oldMousePos;
	Common::Point _mouseDisplayPos;
	int _mouseButton;
	bool _mouseClicked;
	Common::EventType _mousePreviousEventType;
	Common::Point _savedMousePosDivided;
	int _skipDisplayFlag1;
	int _skipDisplayFlag2;

	byte _actionType;
	bool _delayedReactivationAction;
	int8 _selectedCharacterId;
	byte _numCharactersToDisplay;
	int16 _host;
	bool _shouldQuit;

	void pollEvent();
	void setCurrentCharacter(int index);
	void unselectInterfaceButton();
	void moveCharacters();
	void setNextDisplayCharacter(int var1);
	void handleGameScripts();

	// Added by Strangerke
	byte *getCharacterAttributesPtr(int16 index);

	// Temporary stubs
	Common::Event _keyboard_getch();
	bool _keyboard_checkKeyboard();
	void _keyboard_resetKeyboardBuffer();

protected:
	Common::EventManager *_eventMan;
	int _lastTime;

	// Engine APIs
	Common::Error run() override;
	void handleMenu();

private:
	static LilliputEngine *s_Engine;

	GameType _gameType;
	Common::Platform _platform;

	void initialize();
};

} // End of namespace Lilliput

#endif
