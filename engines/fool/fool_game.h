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

#ifndef FOOL_GAME_H
#define FOOL_GAME_H

#include "common/rect.h"
#include "common/str.h"

#include "common/ustr.h"
#include "fool/toolbox.h"

namespace Fool {

class ZBasic;

enum FoolStateBits : uint16 {
	kStateNull = 0x00,
	kStateReturn = 0x01,
	kStateUndo = 0x02,
	kStateSaveGame = 0x04,
	kStateNewGame = 0x08,
	kStateOpenGame = 0x10,
	kStateQuit = 0x20,
	kStateChapterSelect = 0x40,
	kStatePuzzleSelect = 0x80,
	kStateMetapuzzleSelect = 0x100,
	kStatePrintStory = 0x400,
	kStateMetapuzzleComplete = 0x800,
};

enum FoolPuzzleFlags : uint16 {
	kFlagMenuDisabled = 0x00,
	kFlagMenuEnabled = 0x01,
	kFlagMenuDiamond = 0x02,
	kFlagMapTile = 0x08,
};

enum FoolVersion {
	kFool11 = 1,
	kFool20 = 2,
	kFool30 = 3,
};

enum FoolZStrOffset : uint16 {
	kOffsetVersion = 0,
	kOffsetAbout,
	kOffsetThreeShips,
	kOffsetWordSearch,
	kOffsetJigsaw,
	kOffsetPolyomino,
	kOffsetReveal,
	kOffsetSentence,
	kOffsetMaze,
	kOffsetMetapuzzle,
	kOffsetCards,
	kOffsetThoth,
	kOffsetHumbug,
	kOffsetJustice,
};

enum FoolFontID : uint16 {
	kFontChicago = 0,
	kFontFool = 250,
	kFontPuzzle = 251,
	kFontSmall = 252,
	kFontLarge = 254,
};

class FoolGame {
public:
	FoolGame(FoolVersion version): _version(version) {}
	~FoolGame() {}

	void run();

	void foolRun(); // sub_128_004
	void copyScreen(int16 put, BitMap &bmp); // sub_128_0a2
	void openSaveFileDialog(const Common::U32String &title, const Common::U32String &filename); // sub_128_11c
	void sub_128_1e4(const Common::U32String &unk1);
	void sub_128_2be(int16 unk2, int16 unk1);
	void sub_128_342(int16 unk2, int16 unk1);
	void delay(int16 numTicks); // sub_128_3da
	void delayFromMarker(int16 numTicks); // sub_128_406
	int16 puzzlesReadByte(); // sub_128_428
	int16 puzzlesReadShort(); // sub_128_446
	int32 puzzlesReadLong(); // sub_128_462
	Common::String puzzlesReadString(); // sub_128_49a
	void toggleMouseCursor(bool visible); // sub_128_4da
	void playTone(int16 freq, int16 duration, bool wait);
	void drawPuzzleButton(const Common::U32String &symbol); // sub_128_55c
	int16 sub_128_5fe();
	OSErr sub_128_64c(int16 unk1);
	void fillRect(int16 patternID, PatternMode mode, int16 top, int16 left, int16 bottom, int16 right); // sub_128_69c
	void drawTarotCard(int16 rectID, int16 deckPos, int16 type); // sub_128_712
	void fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID); // sub_128_8b4
	void drawStringCenter(const Common::U32String &str, int16 yPos); // sub_128_918
	void zoomRect(int16 startTop, int16 startLeft, int16 startBottom, int16 startRight, int16 endTop, int16 endLeft, int16 endBottom, int16 endRight, int16 patternID, PatternMode mode, int16 steps); // sub_128_962
	void sub_128_bde(int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void getNextEvent(uint32 mask); // sub_128_c6a
	void flashRect(int16 top, int16 left, int16 bottom, int16 right, int16 millis); // sub_128_d34
	void showChoiceModal(uint16 font, int16 lineCount, int16 buttonCount, bool beep); // sub_128_dfe

	void showBehold(int16 unk2, int16 unk1, const Common::U32String &message); // sub_128_178a
	void setStateBits(uint16 bits); // sub_128_1c2c
	void clearStateBits(uint16 bits); // sub_128_1c4a
	void sub_128_1ef8();
	void sub_128_1f1e();
	void sub_128_1f44();
	void sub_128_1f76();
	void sub_128_2078();
	void sub_128_20d0();
	void menuChapterSelect(); // sub_128_2126
	void sub_128_21c8();
	void storyRenderPage(); // sub_128_2202
	void menuClickMessage(); // sub_128_2664
	void sub_128_26f6();
	void fetchPuzzleData(); // sub_128_271a
	void menuNewGame(); // sub_128_27d6
	void sub_128_2808();
	void menuOpenGame(); // sub_128_2988
	void menuSaveGame(); // sub_128_2a06
	void saveGameAs(); // sub_128_2a0e
	void menuSaveGameAs(); // sub_128_2a92
	void menuQuit(); // sub_128_2ab6
	void menuPrintStory(); // sub_128_2ae8
	void printStory(); // sub_128_2b0a
	void newGame(); // sub_128_2bc6
	void openGame(); // sub_128_2e3e

	void sub_128_3032();
	void sub_128_32c8();
	void savePrompt(); // sub_128_32fa
	void autoSaveGame();
	void saveGame(); // sub_128_3536
	void sub_128_3744();
	void cursorWatch(); // sub_128_3774
	void cursorExplodingWatch(); // sub_128_378a
	void cursorExplodingWatchShort(); // sub_128_37ce
	void sub_128_388a();
	void puzzleRun(); // sub_128_39a0
	void storyUnlockChapter(); // sub_128_3de6
	void puzzleSetupMenu(); // sub_128_3fb6

	void puzzleLoadContext(); // sub_128_4168
	void puzzleSaveContext(); // sub_128_41aa
	void sub_128_41d8();
	void sub_128_4472();
	void menuAbout(); // sub_128_4a92
	void menuPrologue();
	void menuFinale();

	// fool_threeships.cpp
	void shipsRun(); // sub_128_5140
	void sub_128_55ac();
	void sub_128_57a2();
	void sub_128_5a6c();

	void onClickMenu(); // sub_128_5b30
	void sub_128_5baa();
	void sub_128_5c20();
	void thothHidePlayfield(); // sub_128_5ef0
	void thothShowPlayfield(); // sub_128_5f16
	void sub_128_5f9e();
	void sub_128_5fb4();
	void sub_128_5fea();
	void sub_128_6154();
	void waitForMouseUp(); // sub_128_6186
	void waitForClick(); // sub_128_61c2
	void sub_128_61ec(); // sub_128_61ec
	void sub_128_6244();

	void sub_129_004();
	void sub_129_068();
	void menuLoadingMessage(int16 percent); // sub_129_123a

	// fool_jumble.cpp:
	void jumbleRun();
	void sub_130_c56();
	void jumbleStoreState(); // sub_130_c66
	void sub_130_d2e();
	void jumbleRunSubstitution(); // sub_130_d90
	void sub_130_1004();
	void jumbleRunWordSquare(); // sub_130_10a6
	void jumbleRunHiddenMessage(); // sub_130_1282
	void sub_130_1426();
	void jumbleOnClick(); // sub_130_1476
	void jumbleOnKey(); // sub_130_15da
	void sub_130_172c();
	void sub_130_19ac();
	void jumbleClickFixedSquare(); // sub_130_19da
	void sub_130_1a16();
	void sub_130_1c1a();
	void jumbleSelectSquare(); // sub_130_1c52
	void sub_130_1c6c();
	void jumbleDrawLetter(); // sub_130_1dcc
	void sub_130_1e5c();
	void sub_130_1e76();
	void sub_130_201a();
	void jumbleNextPosition(); // sub_130_2078
	void jumblePreviousPosition(); // sub_130_2094
	void sub_130_20aa();
	void sub_130_20d2();
	void sub_130_20fe();
	void sub_130_2178();
	void sub_130_2226();
	void sub_130_22ee();
	void sub_130_23cc();
	void sub_130_24aa();
	void sub_130_2548();
	void sub_130_25d8();
	void sub_130_2790();

	// fool_wordsearch.cpp
	void wordSearchRun(); // sub_131_004
	void wordSearchOnClick(); // sub_131_8aa
	void wordSearchBadSelect(); // sub_131_cbe
	void wordSearchDrawFooter(); // sub_131_d22
	void wordSearchStoreState(); // sub_131_d66
	void wordSearchSuccess(); // sub_131_f42

	// fool_jigsaw.cpp
	void jigsawRun(); // sub_132_004
	void jigsawOnClick(); // sub_132_518
	void jigsawDragSelect(); // sub_132_67a
	void jigsawMoveSelected(); // sub_132_962
	void jigsawCancelSelect(); // sub_132_e5a
	void jigsawDropSelected(); // sub_132_ed8
	void jigsawStoreState(); // sub_132_1384
	void jigsawCheckIfSolved(); // sub_132_13ea
	void jigsawSuccess(); // sub_132_1444

	// fool_polyomino.cpp
	void polyominoRun(); // sub_133_004
	void polyominoOnClick(); // sub_133_87c
	void polyominoCancelMove(); // sub_133_eb2
	void polyominoMove(int16 x, int16 y); // sub_133_f04
	void polyominoOnClickFixed(); // sub_133_10a0
	void polyominoReset(); // sub_133_11cc
	void polyominoDrawFrame(); // sub_133_12d4
	void polyominoCheckIfSolved(); // sub_133_12f2
	void polyominoSuccess(); // sub_133_13e2
	void polyominoStoreState(); // sub_133_1452

	// fool_reveal.cpp
	void revealRun(); // sub_134_004
	void revealOnClick(); // sub_134_67c
	void revealSelectButton(); // sub_134_74a
	void revealReset(); // sub_134_7bc
	void revealSuccess(); // sub_134_872

	// fool_sentence.cpp
	void sentenceRun(); // sub_135_004
	void sentenceOnClick(); // sub_135_5b6
	void sentenceAddLeft(int16 index); // sub_135_7d6
	void sentenceAddRight(int16 index); // sub_135_7fa
	void sentenceAddLeftRight(int16 index); // sub_135_81e
	void sentenceReplace(int16 index); // sub_135_86a
	void sentenceReverse(); // sub_135_94c
	void sentenceUndo(); // sub_135_9ba
	void sentenceDrawButton(int16 gridIndex); // sub_135_a34
	void sentenceDrawBuffer(); // sub_135_b16
	void sentenceStoreState(); // sub_135_c1c
	void sentenceSuccess(); // sub_135_cee

	// fool_maze.cpp
	void mazeRun(); // sub_136_004
	void mazeOnClick(); // sub_136_79e
	void mazeFlashWall(); // sub_136_994
	void mazeMovePlayer(); // sub_136_a22
	void sub_136_ade();
	void mazeHotspot(); // sub_136_b00
	void mazePrintMessage(); // sub_136_d64
	void mazePickUpItem(); // sub_136_e4c
	void mazeUseItem(); // sub_136_ed8
	void mazeWanderingWinds(); // sub_136_f74
	void mazeDrawLetter(); // sub_136_115a
	void sub_136_137c();
	void sub_136_163c();
	void sub_136_1650();
	void mazeHiddenDoorOpen(); // sub_136_1756
	void mazeHiddenDoorShut(); // sub_136_17a4
	void sub_136_1806();
	void mazeSetTrigger(); // sub_136_185a
	void mazeClearTrigger(); // sub_136_1898
	void mazePickUpTone(); // sub_136_18f4
	void mazeFireDemon(); // sub_136_1932
	void sub_136_19d2();
	void mazeNoisySprite(); // sub_136_1cf4
	void mazeDelay(); // sub_136_1ddc
	void mazeYeetObject(); // sub_136_1df4
	void mazeThornsGetScroll(); // sub_136_1e4c
	void sub_136_21fa();
	void sub_136_2200();
	void mazeMovementTrail(); // sub_136_2208

	void mazeLoadTone(int16 offset); // sub_136_24ae
	void mazePlayTone(); // sub_136_2538

	void sub_136_2582(); // sub_136_2582
	void mazeDrawPlayer(); // sub_136_2664
	void mazeSetupMenu(); // sub_136_274e
	void mazeAddWallLeft(); // sub_136_2a7c
	void mazeAddWallTop(); // sub_136_2b30
	void mazeAddWallRight(); // sub_136_2be2
	void mazeAddWallBottom(); // sub_136_2c96
	void mazeDrawWallLeft(); // sub_136_2d4c
	void mazeDrawWallTop(); // sub_136_2dd0
	void mazeDrawWallRight(); // sub_136_2e52
	void mazeDrawWallBottom(); // sub_136_2ed6
	void mazeStoreState(); // sub_136_2f5c
	void sub_136_3408();
	void sub_136_342a();
	void mazeLoadState(); // sub_136_3466
	void sub_136_3a30();
	void sub_136_3a70();

	// fool_sun.cpp
	void sunMapRun(); // sub_137_004
	void sunMapOnClick(); // sub_137_3e2
	void sunMapDragSelect(); // sub_137_598
	void sunMapMoveSelected(); // sub_137_880
	void sunMapUndoMove(); // sub_137_d60
	void sunMapDropSelected(); // sub_137_dde
	void sunMapCheckIfSolved(); // sub_137_124e
	void sunMapRevealPiece(); // sub_137_131a

	// fool_metapuzzle.cpp
	void metapuzzleRun(); // sub_138_004
	void metapuzzleOnClick(); // sub_138_1b4
	void sub_138_21e();
	void metapuzzleSetupMenu(); // sub_138_3e0
	void metapuzzleWheel(); // sub_138_49e
	void metapuzzleSecretCode(); // sub_138_550
	void metapuzzleSecretCodeDrawText(); // sub_138_864
	void sub_138_9c4();
	void sub_138_a06();
	void metapuzzleSecretCodeReset(); // sub_138_a22
	void sub_138_a90();
	void metapuzzleOnOption(); // sub_138_b06
	void metapuzzleOnShift(); // sub_138_b6a

	// fool_cards.cpp
	void cardsRun(); // sub_139_004
	void cardsReset(); // sub_139_4de
	void sub_139_50e();
	void cardsShuffleDeck(); // sub_139_51e
	void cardsDrawTable(); // sub_139_582
	void cardsOnClick(); // sub_139_6b0
	void sub_139_a22();
	void cardsOpponentYields(); // sub_139_da8
	void cardsRevealHands(); // sub_139_dee
	void sub_139_17fc();
	void sub_139_191c();
	void sub_139_19da();
	void cardsDrawScores(); // sub_139_1b12
	void cardsStoreState(); // sub_139_1cba
	void sub_139_1d3e();

	// fool_thoth.cpp
	void highPriestessRun(); // sub_140_004
	void thoth99Enchantments(); // sub_140_458
	void thothMoveEnchantment(); // sub_140_9f0
	void thothDrawEnchantment(); // sub_140_b2c
	void sub_140_c4c();
	void thothScrambleScreen(); // sub_140_dd4
	void sub_140_e3c();
	void sub_140_f84();
	void thothKey1st(); // sub_140_12fc
	void sub_140_15fc();
	void thothKey2nd(); // sub_140_1486
	void thothKey3rd(); // sub_140_18fa
	void thothKey3rdOnClick(); // sub_140_1bc4
	void sub_140_1f68();
	void sub_140_205e();
	void thothKeyLast(); // sub_140_206a
	void thothKeyLastOnClick(); // sub_140_22b8
	void thothBadSelect(); // sub_140_24ae
	void hermitNextStage(); // sub_140_25e4
	void hermitScreenFlash(); // sub_140_2662
	void hermitScreenZoom(); // sub_140_26ca
	void hermitScreenBehold(); // sub_140_28bc
	void sub_140_2968();
	void sub_140_2978();
	void thothKeyLastSetup(); // sub_140_2998
	void sub_140_2f92();
	Common::Rect thothRandomSquare(); // sub_140_3050
	Common::Rect thothRandomHRect(); // sub_140_30da
	Common::Rect thothRandomVRect(); // sub_140_3148
	void sub_140_3296();
	void sub_140_32ac();
	void thothAdjustPuzzleData(); // sub_140_3372
	void sub_140_3412();

	// fool_death.cpp
	void deathRun(); // sub_141_004
	void deathMoveBlackEye(); // sub_141_3ca
	void deathDrawWhiteEye(); // sub_141_882
	void deathCaught(); // sub_141_8b8
	void deathDrawZoom(); // sub_141_934

	// fool_humbug.cpp
	void humbugRun(); // sub_142_004
	void humbugPlayTrail(); // sub_142_0e6
	void sub_142_370();
	void humbugSuccess(); // sub_142_5f2
	void sub_142_630();

	// fool_justice.cpp
	void justiceRun(); // sub_142_852
	void sub_142_9be();
	void justiceOnClick(); // sub_142_cb2
	void justiceZoom(); // sub_142_f58
	void justiceDrawBlock(); // sub_142_f96
	void justiceRemoveBlock(); // sub_142_10bc
	void justiceStoreState(); // sub_142_111e
	void justiceResetGrid(); // sub_142_11fe

	void hermitRun(); // sub_142_12ac

	// fool_straightpath.cpp
	void straightPathRun(); // sub_143_004
	void straightPathOnClick(); // sub_143_5c0
	void straightPathReset(); // sub_143_784
	void straightPathDrawText(); // sub_143_864
	void straightPathClearText(); // sub_143_890
	void straightPathSuccess(); // sub_143_8b0

	void sub_144_004();


private:
	FoolVersion _version;
	const int *_zstrOffset;
	int _fontChicago;
	Toolbox *_toolbox;
	ZBasic *_zbasic;
	bool _quit = false;

	GrafPtr var_i32_0;
	GrafPtr var_i32_4;
	GrafPtr var_i32_8;
	GrafPtr var_i32_8_thoth;
	Handle var_i32_c;
	int16 var_i16_30;
	int16 var_i16_32;
	int16 var_i16_34;
	int16 var_i16_36;
	int16 var_i16_38;
	int16 var_i16_42;
	int16 var_i16_44;
	EventRecord _event; // var_ev_46
	int16 var_i16_56; // var_i16_56
	int16 var_i16_58; // var_i16_58
	int16 _windowWidth; // var_i16_5a
	int16 _windowHeight; // var_i16_5c
	SFReply var_sfr_5e; // -> aa
	int16 var_i16_16c;
	ProcPtr var_i32_16e;
	Common::U32String var_str_172;
	Common::U32String var_str_272;
	RGBColor var_i16_372;
	bool _soundEnabled; // var_i16_378
	bool _screenOversized; // var_i16_37a
	uint32 var_i32_37c;
	Common::U32String var_str_384;
	int16 var_i16_484;
	Common::U32String var_str_486;
	int16 var_i16_586;
	Common::U32String var_str_588;
	int16 var_i16_688;
	int16 var_i16_68a;
	int16 var_i16_68c;
	uint32 var_i32_692;
	// this is normally a raw pointer to some memory,
	// instead we track it as a handle + offset
	uint32 _puzzleDataPtr; // var_ptr_696
	Handle _puzzleDataBuffer; // var_bytes_696
	int16 var_i16_78a;
	int16 var_i16_79e;
	bool _mouseVisible; // var_i16_7a0
	int16 var_i16_7a8;
	int16 var_i16_7aa;
	int16 var_i16_7b0;
	int16 var_i16_7b2;
	int16 var_i16_7b4;
	int16 var_i16_7b6;
	int16 var_i16_7b8;
	int16 var_i16_7ba;
	int16 var_i16_7bc;
	// this was reused, I don't know why
	int16 _savePromptChoice; // var_i16_7be
	int16 var_i16_7be;
	int16 _keyLastPressed; // var_i16_7c0
	PicHandle var_pic_7c2;
	uint16 _stateFlags; // var_i16_7c6
	uint32 var_i32_7c8;
	int16 var_i16_7cc;
	int16 var_i16_7ce;
	int16 _storyCurrentChapter; // var_i16_7d0
	int16 _sunMapRestored; // var_i16_7d2
	int16 _storyCurrentPage; // var_i16_7d4
	int16 var_i16_7d6;
	int16 _storyNextPage; // var_i16_7d8
	int16 _storyPageCount; // var_i16_7da
	int16 _selectedMenuChapter; // var_i16_7dc
	int16 var_i16_7de;
	int16 var_i16_7e0;
	int16 _activePuzzle; // var_i16_7e2
	int16 var_i16_7e4;
	int16 var_i16_7e6;
	Common::U32String var_str_7e8;
	int16 var_i16_8e8;
	Common::U32String _saveFileName; // var_str_8ec
	bool _isAutoSaving;
	int16 var_i16_9ec;
	int16 var_i16_9f2;
	Common::U32String var_str_9f4;

	Common::U32String var_str_af4;

	int16 var_i16_bfc;
	// Used as both a pointer and a bitstore
	uint32 var_i32_bf8;
	MenuHandle var_menu_bf8;
	int16 var_i16_c00;
	int16 var_i16_c02;
	int16 _activePuzzleStatus; // var_i16_c04
	Common::String _activePuzzleBuffer; // var_str_c06

	int16 var_i16_d08;
	int16 var_i16_d0a;
	bool _activePuzzleSolved; // var_i16_d0c
	int16 var_i16_d0e;
	int16 var_i16_d10;
	Common::U32String var_str_d12;
	bool _menuDisabled; // var_i16_e12
	bool _menuHidesPlayfield; // var_i16_e14
	int16 _selectedMenuID; // var_i16_e16
	int16 _selectedMenuItem; // var_i16_e18
	int16 var_i16_e1a;
	int16 var_i16_e1c;
	int16 var_i16_e1e;
	int16 var_i16_e20;
	Common::U32String var_str_e22;
	int16 var_i16_f22;
	GrafPtr var_i32_f24;
	OSType var_i32_f28;
	int16 var_i16_f2c;
	int16 var_i16_f2e;
	Common::U32String var_str_f30;
	int16 var_i16_1030;
	PicHandle _metapuzzleWheelPic; // var_pic_1032
	int32 var_i32_1036;
	int16 var_i16_103a;
	int16 var_i16_103c;
	int16 var_i16_103e;
	int16 var_i16_1040;
	int16 _jumbleSubPuzzleCount; // var_i16_1056
	int16 _jumbleCurrentSubPuzzle; // var_i16_1058
	int16 var_i16_105a;
	int16 var_i16_105c;
	int16 var_i16_105e;
	int16 var_i16_1060;
	int16 _polyominoPosX; // var_i16_1062
	int16 _polyominoPosY; // var_i16_1064
	int16 _jumbleGameType; // var_i16_1068
	int16 var_i16_106a;
	SourceMode var_i16_106c;
	int16 var_i16_106e;
	int16 _jumblePosition; // var_i16_106e
	Common::U32String var_str_1070;
	Common::U32String _sentenceBuffer; // var_str_1070
	Common::U32String var_str_1170;
	int16 var_i16_1270;
	Common::U32String var_str_1272;
	Common::U32String _sentenceGoal; // var_str_1272
	int16 var_i16_1372;

	Common::U32String var_str_1374;
	Common::U32String var_str_1474;
	int16 var_i16_1574;
	int16 _jumbleSelected; // var_i16_1574
	int16 var_i16_1576;

	Common::U32String var_str_1578;

	int16 var_i16_1678;
	int16 var_i16_167a;
	Common::U32String var_str_167c;
	Common::U32String var_str_177c;

	int16 var_i16_187c;
	int16 var_i16_187e;
	int16 var_i16_1880;
	int16 var_i16_1882;
	int16 var_i16_1884;
	int16 var_i16_1886;
	int16 var_i16_1888;
	int16 var_i16_188a;
	int16 var_i16_188c;

	Common::U32String var_str_188e;

	int16 var_i16_198e;
	int16 var_i16_1990;
	int16 var_i16_1992;
	Common::U32String var_str_1994;

	int16 var_i16_19ae;

	int16 var_i16_1a94;
	int16 var_i16_1a96;
	int16 var_i16_1a98;
	int16 var_i16_1a9a;
	uint16 var_i16_1a9c;
	int16 _jigsawPieceCount; // var_i16_1a9e
	int16 var_i16_1aa0;
	int16 _metapuzzleSecretCodeSpaceOffset; // var_i16_1aa2
	int16 var_i16_1aa4;
	int16 var_i16_1aa6;
	int16 var_i16_1aa8;
	int16 var_i16_1aaa;

	int16 var_i16_1ab0; // var_i16_1ab0
	int16 var_i16_1ab2;
	int16 var_i16_1ab4;
	int16 var_i16_1ab6;
	int16 var_i16_1ab8;
	int16 var_i16_1aba;
	int16 var_i16_1abc;
	int16 var_i16_1abe;
	PicHandle var_i32_1ac0;
	int16 var_i16_1ac4;
	Common::String var_str_1ac8;
	int16 var_i16_1bc8;

	int16 _mazePlayerX; // var_i16_1bcc
	int16 _mazePlayerY; // var_i16_1bce
	int16 var_i16_1bd0;

	int16 _mazePlayerDirection; // var_i16_1bd2
	int16 var_i16_1bd4;
	int16 var_i16_1bd6;
	int16 var_i16_1bd8;
	int16 var_i16_1bda;
	int16 var_i16_1bdc;
	Common::String var_str_1bde;

	int16 var_i16_1cde;
	int16 var_i16_1ce0;
	Common::String var_str_1ce2;

	int16 var_i16_1de2;
	int16 var_i16_1de4;

	int16 var_i16_1de6;

	int16 var_i16_1de8;
	int16 var_i16_1dea;
	int16 var_i16_1dec;

	int16 var_i16_1df0;

	PatternMode var_i16_1dee;
	SourceMode var_src_1dee;

	int16 var_i16_1df2;
	int16 mazeToneFreq; // var_i16_1df4
	int16 var_i16_1df6;
	int16 var_i16_1df8;
	int16 var_i16_1dfa;
	int16 var_i16_1dfc;
	int16 var_i16_1dfe;

	int16 var_i16_1e00;
	int16 mazeToneFreqStep; // var_i16_1e02
	int16 var_i16_1e04;

	int16 var_i16_1e06;

	uint16 var_i16_1f08;
	int16 var_i16_1f0a;
	Common::U32String _metapuzzleSecretCodeCipher; // var_str_1f0c

	int16 _metapuzzleSecretCodeCount; // var_i16_200c
	int16 var_i16_200e;
	int16 var_i16_2010;
	int16 var_i16_2012;
	int16 var_i16_2014;
	int16 var_i16_2016;
	int16 var_i16_2018;
	Common::U32String var_str_201a;
	int16 var_i16_211a;
	Common::U32String var_str_211c;


	int16 var_i16_221c;
	int16 var_i16_221e;
	Common::U32String var_str_2222;
	int16 var_i16_2322;
	int16 _hermitPathStage; // var_i16_2324
	int16 var_i16_2326;
	int16 var_i16_2328;
	uint32 var_ptr_232a;
	Handle var_bytes_232a;
	int16 var_i16_232e;

	int16 var_i16_2330;
	int16 var_i16_2332;

	int16 var_i16_2338;
	bool _deathWhiteEyeNeedsDraw; // var_i16_233a
	int16 var_i16_233e;
	int16 _humbugTrailIndex; // var_i16_233c

	int16 var_i16_2340;
	int16 var_i16_2342;

	// FIXME: bounds on the following are guessed! need to trace index range
	int16 _pageLineRanges[202]; // arr_i16_0
	int16 _pageLineFace[1301]; // arr_i16_194
	int16 _pageLineBreak[1301]; // arr_i16_bbe
	int16 arr_i16_15e8[101];
	int16 arr_i16_16b2[0x100];
	int16 _puzzleType[101]; // arr_i16_18b2
	int16 _puzzleUnlockChapter[101]; // arr_i16_197c
	int16 _pageToChapter[101]; // arr_i16_1a46
	int16 _puzzleMenuInstructions[0x40]; // arr_i16_1b10
	int16 _pageVisible[101]; // arr_i16_1b90
	int16 _puzzleCompletionStatus[101]; // arr_i16_1c5a
	uint16 _puzzleFlags[101]; // arr_i16_1d24
	int16 _pageLines[202]; // arr_i16_1dee
	int16 arr_i16_1eb8[0x29];
	// used just for death?
	Common::Rect _deathBlackEye; // arr_rect_1eb8
	Common::Rect _screenGrid[0x200]; // arr_rect_1f38
	int16 arr_i16_2f38[0x400]; // arr_i16_2f38, 96x32?
	int16 arr_i16_3738[0x200];
	int16 arr_i16_3b38[0x400]; // 96x32?
	Common::Rect arr_rect_4338;
	// general-purpose puzzle buffer
	int16 arr_i16_4338[0x200];
	uint16 _bitLUT[16]; // arr_i16_4738
	int16 arr_i16_4758[15];
	Common::Rect arr_rect_4776;
	int16 arr_i16_47d8[0x100];
	PicHandle _polyominoPics[0x80]; // arr_pic_49d8
	int16 _sunMapTileID[0x52]; // arr_i16_4bd8
	int16 arr_i16_4c7c[0x52];
	int16 arr_i16_4d20[52];
	Common::SharedPtr<Cursor> _cursors[0x2b]; // arr_curs_4d88, 0x44
	Pattern _patterns[0x51]; // arr_pat_58f4
	int16 arr_i16_5bbc[128];
	int16 arr_i16_5cbc[128];
	int16 _mazeInvItemCount[128]; // arr_i16_5cbc
	float arr_bcd_5dbc[8];
	BitMap arr_bmp_5dfc; // 22000, close to a screen page
	BitMap _jigsawPieces[128];
	Common::Point _humbugTrail[0xa43];
	BitMap arr_bmp_b3ec;
	BitMap arr_bmp_bbbc; // used by death
	BitMap arr_bmp_c38c; // used by death
	BitMap arr_bmp_fa3c;
	Handle arr_bytes_109dc; // used for data buffering
	BitMap arr_bmp_109dc;
	BitMap arr_bmp_138bc;
	GrafPort arr_grafport_18eae;
	GrafPort arr_grafport_18f78;
	GrafPort arr_grafport_19042;
	GrafPort arr_grafport_19042_thoth;
							// Also 109dc and 15fcc
	Common::Rect arr_rect_1910c;
	Common::Rect arr_rect_19114;
	PicHandle _sunMapTilePic[0x54]; // arr_i32_1912c
	PicHandle arr_i32_192c0[101];
	PolyHandle _revealPoly[101]; // arr_poly_192c0
	int32 _puzzleDataOffsets[101]; // arr_i32_19454
	Common::U32String _puzzleName[101]; // arr_str_195e8, 0x20 each

	Common::U32String _pageNumberText[101]; // arr_str_1a288, 0x10 each

	Common::U32String _modalText[13]; // arr_str_1a8d8, 0x66 each
	Common::Rect _screenClipRect; // arr_rect_1ae06
};

}

#endif // FOOL_GAME_H
