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
 */

#ifndef LILLIPUT_LILLIPUT_H
#define LILLIPUT_LILLIPUT_H

#include "lilliput/console.h"
#include "lilliput/script.h"
#include "lilliput/sound.h"
#include "lilliput/stream.h"

#include "common/file.h"
#include "common/rect.h"

#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Robin engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Adventures of Robin Hood
 * - Rome: Pathway to Rome
 */
namespace Lilliput {

static const int kSavegameVersion = 1;

enum GameType {
	kGameTypeNone  = 0,
	kGameTypeRobin,
	kGameTypeRome
};

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

struct LilliputGameDescription;

struct SmallAnim {
	bool _active;
	Common::Point _pos;
	int16 _frameIndex[8];
};

class LilliputEngine : public Engine {
public:
	LilliputEngine(OSystem *syst, const LilliputGameDescription *gd);
	~LilliputEngine();

	OSystem *_system;

	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;
	LilliputScript *_scriptHandler;
	LilliputSound *_soundHandler;
	Graphics::Surface *_mainSurface;

	SmallAnim _smallAnims[4];
	int _smallAnimsFrameIndex;

	byte _byte1714E;
	byte _byte184F4;
	byte _lastAnimationTick;
	byte _animationTick;
	Common::Point _nextDisplayCharacterPos;
	byte _sound_byte16F06;
	byte _lastKeyPressed;
	byte _keyboard_nextIndex;
	byte _keyboard_oldIndex;
	byte _byte12A05;
	byte _byte12A06;
	byte _byte12A07;
	byte _byte12A08;
	bool _refreshScreenFlag;
	byte _byte16552;
	int8 _lastInterfaceHotspotIndex;
	byte _lastInterfaceHotspotButton; // Unused: set by 2 functions, but never used elsewhere
	byte _byte16F08;
	byte _byte15EAD;
	byte _debugFlag; // Mostly useless, as the associated functions are empty
	byte _byte14837; // Unused byte, set by an opcode

	byte _array147D1[3];
	char _array1692B[4];
	byte *_bufferIsoMap;
	byte *_bufferCubegfx;
	byte *_bufferMen;
	byte *_bufferMen2;
	byte *_bufferIsoChars;
	byte *_bufferIdeogram;
	byte _curPalette[768];
	byte _displayStringBuf[160];

	bool _saveFlag;
	bool _int8installed;
	byte _displayMap;

	int _word10800_ERULES;
	byte _numCharacters;
	Common::Point _currentScriptCharacterPos;
	int _nextCharacterIndex;
	int _word16EFE;
	uint16 _word1817B;
	Common::Point _savedSurfaceUnderMousePos;
	int _word15AC2;
	int _currentDisplayCharacter;
	int _displayStringIndex;
	int _word1289D;
	Common::Point _word16937Pos;

	short _word15E5D[40];
	byte _byte15E35[40];
	int16 _array11D49[40];
	int16 _array1289F[40];
	int16 _array12861[30];

	byte *_rulesChunk1;
	int16 _currentScriptCharacter;
	int16 _characterPositionX[40];
	int16 _characterPositionY[40];
	int8 _characterPositionAltitude[40];
	int16 _characterFrameArray[40];
	int8 _rulesBuffer2_5[40];
	int8 _rulesBuffer2_6[40];
	byte _rulesBuffer2_7[40];
	byte _spriteSizeArray[40];
	byte _characterDirectionArray[40];
	byte _rulesBuffer2_10[40];
	byte _rulesBuffer2_11[40];
	byte _rulesBuffer2_12[40];
	byte _rulesBuffer2_13_posX[40];
	byte _rulesBuffer2_14_posY[40];
	byte _characterVariables[1400 + 3120];
	byte *_currentCharacterVariables;
	byte _rulesBuffer2_16[40 * 32];
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
	byte _rulesChunk9[60];
	byte _rulesChunk10_size;
	int16 *_rulesChunk10;
	byte *_rulesChunk11;
	int16 _rectNumb;
	int16 _rectXMinMax[40];
	int16 _rectYMinMax[40];
	Common::Point _rulesBuffer12Pos3[40];
	Common::Point _rulesBuffer12Pos4[40];
	int _interfaceHotspotNumb;
	byte _rulesBuffer13_1[20];
	int16 _interfaceHotspotsX[20];
	int16 _interfaceHotspotsY[20];
	byte _rulesBuffer13_4[20];
	int16 _array10999PosX[40];
	int16 _array109C1PosY[40];
	byte _savedSurfaceUnderMouse[16 * 16];
	byte _charactersToDisplay[40];
	int16 _characterRelativePositionX[40];
	int16 _characterRelativePositionY[40];
	int16 _characterDisplayX[40];
	int16 _characterDisplayY[40];
	byte _array12299[40];
	int16 _array109E9PosX[40];
	int16 _array10A11PosY[40];
	byte _array16E94[40];
	byte _array16C54[4];
	byte _array16C58[4];
	byte _savedSurfaceGameArea1[176 * 256]; // 45056
	byte _savedSurfaceGameArea2[176 * 256]; // 45056
	byte _savedSurfaceGameArea3[176 * 256]; // 45056
	byte _savedSurfaceSpeech[16 * 252];

	const LilliputGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void newInt8();
	void update();

	void display16x16IndexedBuf(byte *buf, int index, Common::Point pos);
	void display16x16Buf(byte *buf, Common::Point pos);
	void SaveSurfaceUnderMouseCursor(byte *buf, Common::Point pos);
	void fill16x16Rect(byte col, Common::Point pos);
	void displayMousePointer();
	void restoreSurfaceUnderMousePointer();
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
	void sub15F75();
	void resetSmallAnims();
	void paletteFadeOut();
	void paletteFadeIn();

	void sortCharacters();
	void scrollToViewportCharacterTarget();
	void viewportScrollTo(Common::Point goalPos);
	void checkSpeechClosing();
	void sub16626();
	void sub16A08(int index);
	byte sub16A76(int indexb, int indexs);
	void sub17224(byte type, byte index, int var4);
	void sub17264(byte index, int var4);
	int16 findHotspot(Common::Point pos);
	int16 reverseFindHotspot(Common::Point pos);
	byte sub16722(int index, Common::Point var1);
	byte sub166EA(int index);
	void sub167EF(int index);

	void renderCharacters(byte *buf, Common::Point pos);

	byte sub16799(int index, Common::Point param1);
	byte getDirection(Common::Point param1, Common::Point param2);
	void addCharToBuf(byte character);
	void numberToString(int param1);
	void sub12F37();
	byte sub16675_moveCharacter(int idx, Common::Point var1);
	void sub16685(int idx, Common::Point var1);
	void sub16EBC();
	void sub16CA0();
	byte sub166DD(int index, Common::Point var1);
	void sub171CF();
	void sub12FE5();
	int16 sub16DD5(int x1, int y1, int x2, int y2);
	void displayCharactersOnMap();
	void restoreMapPoints();
	void displayHeroismIndicator();
	void sub130EE();
	void sub1305C(byte index, byte var2);
	void checkInterfaceHotspots(bool &forceReturnFl);
	bool isMouseOverHotspot(Common::Point mousePos, Common::Point hotspotPos);
	void sub131B2(Common::Point pos, bool &forceReturnFl);
	void sub131FC(Common::Point pos);
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
	void sub16B31_moveCharacter(int index, int16 speed);
	void sub16B8F_moveCharacter(int index, Common::Point pos, int direction);
	byte sub1675D(int index, Common::Point var1);
	byte sub16729(int index, Common::Point var1);
	byte sub166F7(int index, Common::Point var1, int tmpVal);
	void sub1693A_chooseDirections(int index);

	void initGame(const LilliputGameDescription *gd);
	byte *loadVGA(Common::String filename, int fileSize, bool loadPal);
	byte *loadRaw(Common::String filename, int filesize);
	void loadRules();

	void displayVGAFile(Common::String fileName);
	void initPalette();
	void fixPaletteEntries(uint8 *palette, int num);

	GameType getGameType() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const;
	const char *getCopyrightString() const;

	Common::String getSavegameFilename(int slot);
	void syncSoundSettings();

	Common::Point _mousePos;
	Common::Point _oldMousePos;
	Common::Point _mouseDisplayPos;
	int _mouseButton;
	Common::Point _savedMousePosDivided;
	int _skipDisplayFlag1;
	int _skipDisplayFlag2;

	byte _byte16F07_menuId;
	byte _byte12FCE;
	byte _byte129A0;
	byte _numCharactersToDisplay;
	byte _byte16C9F;
	int16 _word10804;
	bool _shouldQuit;

	void pollEvent();
	void setCurrentCharacter(int index);
	void unselectInterfaceButton();
	void moveCharacters();
	void setNextDisplayCharacter(int var1);
	void handleGameScripts();

	// Added by Strangerke
	byte *getCharacterVariablesPtr(int16 index);

	// Temporary stubs
	byte _keyboard_getch();

protected:
	Common::EventManager *_eventMan;
	int _lastTime;

	// Engine APIs
	Common::Error run();
	void handleMenu();

private:
	static LilliputEngine *s_Engine;

	LilliputConsole *_console;
	GameType _gameType;
	Common::Platform _platform;

	void initialize();
};

} // End of namespace Lilliput

#endif
