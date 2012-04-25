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

#include "common/file.h"
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
	kDebugSound     = 1 << 2
};

struct LilliputGameDescription;

struct struct18560 {
	byte _field0;
	int16 _field1;
	int16 _field3;
	int16 _field5[8];
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

	struct18560 _arr18560[4];
	byte _byte1714E;
	byte _byte184F4;
	byte _nextDisplayCharacterX;
	byte _nextDisplayCharacterY;
	byte _sound_byte16F06;
	byte _byte16F09;
	byte _keyboard_nextIndex;
	byte _keyboard_oldIndex;
	byte _byte12A04;
	byte _byte12A05;
	byte _byte12A06;
	byte _byte12A07;
	byte _byte12A08;
	byte _byte12A09;
	byte _byte1881D;
	byte _byte16552;
	byte _byte12FE4;
	byte _byte12FE3;

	byte _array147D1[3];
	byte _buffer1[45056];
	byte _buffer2[45056];
	byte _buffer3[45056];
	byte *_bufferIsoMap;
	byte *_bufferCubegfx;
	byte *_bufferMen;
	byte *_bufferMen2;
	byte *_bufferIsoChars;
	byte *_bufferIdeogram;
	byte _buffer10[4032];
	byte _curPalette[768];
	byte _displayStringBuf[160];

	bool _saveFlag;
	bool _int8installed;

	int _word10800_ERULES;
	int _word10807_ERULES;
	int _word16EFA;
	int _word17081_nextIndex;
	int _word16EFE;
	int _word1817B;
	int _word15BC8;
	int _word15BCA;
	int _word15AC2;
	int _currentDisplayCharacter;
	int _displayStringIndex;
	int _word1289D;
	short _word15E5D[40];
	byte _byte15E35[40];

	int _array11D49[40];
	int _array1289F[40];
	int _array12861[30];

	byte *_rulesChunk1;
	int _rulesBuffer2PrevIndx;
	int _characterPositionX[40];
	int _characterPositionY[40];
	byte _rulesBuffer2_3[40];
	int _rulesBuffer2_4[40];
	byte _rulesBuffer2_5[40];
	byte _rulesBuffer2_6[40];
	byte _rulesBuffer2_7[40];
	byte _rulesBuffer2_8[40];
	byte _rulesBuffer2_9[40];
	byte _rulesBuffer2_10[40];
	byte _rulesBuffer2_11[40];
	byte _rulesBuffer2_12[40];
	byte _rulesBuffer2_13[40];
	byte _rulesBuffer2_14[40];
	byte _rulesBuffer2_15[40 * 32];
	byte *_ptr_rulesBuffer2_15;
	byte _rulesBuffer2_16[40 * 32];
	int *_rulesChunk3;
	int _rulesChunk3_size;
	byte *_rulesChunk4;
	byte *_initScript;
	int _initScript_size;
	byte *_menuScript;
	int _menuScript_size;
	int *_arrayGameScriptIndex;
	int _gameScriptIndexSize;
	byte *_arrayGameScripts;
	byte _rulesChunk9[60];
	int _rulesChunk10_size;
	int *_rulesChunk10;
	byte *_rulesChunk11;
	int _rulesChunk12_size;
	int _rulesBuffer12_1[40];
	int _rulesBuffer12_2[40];
	int _rulesBuffer12_3[40];
	int _rulesBuffer12_4[40];
	int _word12F68_ERULES;
	byte _rulesBuffer13_1[20];
	int _rulesBuffer13_2[20];
	int _rulesBuffer13_3[20];
	byte _rulesBuffer13_4[20];
	byte _array10999[40];
	byte _array109C1[40];
	byte _array15AC8[256];
	byte _charactersToDisplay[40];
	byte _characterRelativePositionX[40];
	byte _characterRelativePositionY[40];
	byte _characterDisplayX[40];
	byte _characterDisplayY[40];
	byte _array12299[40];
	byte _array109E9[40];
	byte _array10A11[40];
	byte _array16E94[40];

	byte _buffer1_45k[45056];
	byte _buffer2_45k[45056];
	byte _buffer3_45k[45056];
	byte _buffer10_4032[4032];

	const LilliputGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void newInt8();
	void update();

	void displayFunction1(byte *buf, int var1, int var2, int var4);
	void displayFunction1a(byte *buf, int var2, int var4);
	void displayFunction2(byte *buf, int var2, int var4);
	void displayFunction3(int var1, int var2, int var4);
	void displayFunction4();
	void displayFunction5();
	void displayFunction6();
	void displayFunction7();
	void displayFunction8();
	void displayFunction9();
	void displayFunction10();
	void displayFunction12();
	void displayFunction13(byte *buf, int var1, int var2, int var3);
	void displayFunction14();
	void displayFunction15();
	void displayFunction16();
	void displayFunction17();
	void displayFunction18(int index, int x, int y, int flags);
	void displayString(byte *buf, int var2, int var4);
	void displayChar(int index, int var1);

	void sortCharacters();
	void scrollToViewportCharacterTarget();
	void viewportScrollTo(int var1, int var3);
	void sub189DE();
	void sub16626();

	void renderCharacters(byte *buf, int x, int y);
	int sub16799(int param1, int index);
	int sub16B0C(int param1, int param2);
	void sub18A3E(byte param1);
	void prepareGoldAmount(int param1);
	void sub12F37();
	int sub16675(int idx, int var1);
	int sub16685(int idx, int var1);
	void sub16EBC();
	void sub16CA0();
	int sub166DD(int index, int var1);
	void sub171CF();
	void sub12FE5();
	int sub16DD5(int x1, int y1, int x2, int y2);
	void displayCharactersOnMap();
	void restoreMapPoints();
	void displayHeroismIndicator();
	void sub130EE();
	void sub1305C(byte index, byte var2);
	void sub13184(bool &forceReturnFl);
	int sub13240(int posX, int posY, int var3, int var4);
	void sub131B2(int var2, int var4, bool &forceReturnFl);
	void sub131FC(int var2, int var4);
	void sub1546F(byte displayX, byte displayY);
	void sub15498(byte x, byte y, int var2);

	void initGame(const LilliputGameDescription *gd);
	byte *loadVGA(Common::String filename, bool loadPal);
	byte *loadRaw(Common::String filename);
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

	int _mouseX;
	int _mouseY;
	int _oldMouseX;
	int _oldMouseY;
	int _mouseDisplayX;
	int _mouseDisplayY;
	int _mouseButton;
	int _savedMousePosDivided;
	int _skipDisplayFlag1;
	int _skipDisplayFlag2;

	byte _byte16F07_menuId;
	byte _byte12FCE;
	byte _byte129A0;
	byte _numCharactersToDisplay;
	byte _byte16C9F; 
	int _word10804;
	int _word16213;
	int _word16215;

	void pollEvent();
	void sub170EE(int index);
	void sub130DD();
	void sub16217();
	void setNextDisplayCharacter(int var1);
	void handleGameScripts();

	// Temporary stubs
	byte _keyboard_getch();

protected:
	Common::EventManager *_eventMan;
	bool _shouldQuit;
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
