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

enum FoolStateBits : uint16 {
	kStateNull = 0x00,
	kStateReturn = 0x01,
	kStateUndo = 0x02,
	kStateSaveGame = 0x04,
	kStateNewGame = 0x08,
	kStateOpenGame = 0x10,
	kStateQuit = 0x20,
	kStateChapterSelect = 0x40,
};

class FoolGame {
public:
	FoolGame() {}
	~FoolGame() {}

	void run();

	void sub_128_004();
	void copyScreen(int16 put, BitMap &bmp); // sub_128_0a2
	void sub_128_11c(const Common::U32String &unk2, const Common::U32String &unk1);
	void sub_128_1e4(const Common::U32String &unk1);
	void sub_128_2be(int16 unk2, int16 unk1);
	void sub_128_342(int16 unk2, int16 unk1);
	void sub_128_3da(int16 unk1);
	void sub_128_406(int16 unk1);
	int16 puzzlesReadByte(); // sub_128_428
	int16 puzzlesReadShort(); // sub_128_446
	int32 puzzlesReadLong(); // sub_128_462
	Common::U32String puzzlesReadString(); // sub_128_49a
	void sub_128_4da(int16 unk1);
	void sub_128_50e(int16 freq, int16 duration, int16 wait);
	void sub_128_55c(const Common::U32String &unk1);
	int16 sub_128_5fe();
	OSErr sub_128_64c(int16 unk1);
	void sub_128_69c(int16 unk6, PatternMode unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void drawTarotCard(int16 rectID, int16 deckPos, int16 type); // sub_128_712
	void fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID); // sub_128_8b4
	void sub_128_918(const Common::U32String &unk1);
	void sub_128_962(int16 unk11, int16 unk10, int16 unk9, int16 unk8, int16 unk7, int16 unk6, int16 unk5, int16 unk4, int16 unk3, PatternMode unk2, int16 unk1);
	void sub_128_bde(int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void getNextEvent(uint32 mask); // sub_128_c6a
	void sub_128_d34(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_dfe(int16 unk4, int16 unk3, int16 unk2, int16 unk1);

	void sub_128_178a(int16 unk2, int16 unk1);
	void setStateBits(int16 unk1); // sub_128_1c2c
	void clearStateBits(int16 unk1); // sub_128_1c4a
	void sub_128_1f1e();
	void sub_128_1f44();
	void sub_128_1f76();
	void sub_128_2078();
	void sub_128_20d0();
	void menuChapterSelect(); // sub_128_2126
	void sub_128_21c8();
	void storyRenderPage(); // sub_128_2202
	void sub_128_2664();
	void sub_128_26f6();
	void sub_128_271a();
	void menuNewGame(); // sub_128_27d6
	void sub_128_2808();
	void menuOpenGame(); // sub_128_2988
	void menuSaveGame(); // sub_128_2a06
	void sub_128_2a0e();
	void menuSaveGameAs(); // sub_128_2a92
	void menuQuit(); // sub_128_2ab6
	void menuPrintStory(); // sub_128_2ae8
	void sub_128_2b0a();
	void newGame(); // sub_128_2bc6
	void openGame(); // sub_128_2e3e

	void sub_128_3032();
	void sub_128_32c8();
	void sub_128_32fa();
	void saveGame(); // sub_128_3536
	void sub_128_3744();
	void sub_128_3774();
	void sub_128_378a();
	void sub_128_37ce();
	void sub_128_388a();
	void sub_128_39a0();
	void sub_128_3de6();
	void sub_128_3fb6();

	void sub_128_4168();
	void sub_128_41aa();
	void sub_128_41d8();
	void sub_128_4472();
	void sub_128_4a92();
	void sub_128_5140();
	void sub_128_5b30();
	void sub_128_5baa();
	void sub_128_5c20();
	void sub_128_5ef0();
	void sub_128_5f16();
	void sub_128_5f9e();
	void sub_128_5fb4();
	void sub_128_5fea();
	void sub_128_6154();
	void sub_128_6186();
	void sub_128_61c2();
	void sub_128_61ec();
	void sub_128_6244();

	void sub_129_004();
	void sub_129_068();
	void sub_129_123a();

	// fool_jumble.cpp:
	void jumbleRun();
	void sub_130_c56();
	void sub_130_c66();
	void sub_130_d2e();
	void sub_130_d90();
	void sub_130_1004();
	void sub_130_10a6();
	void sub_130_1282();
	void sub_130_1426();
	void sub_130_1476();
	void sub_130_15da();
	void sub_130_172c();
	void sub_130_19ac();
	void sub_130_19da();
	void sub_130_1a16();
	void sub_130_1c1a();
	void sub_130_1c52();
	void sub_130_1c6c();
	void sub_130_1dcc();
	void sub_130_1e5c();
	void sub_130_1e76();
	void sub_130_201a();
	void sub_130_2078();
	void sub_130_2094();
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
	void polyominoMove(); // sub_133_f04
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
	void sub_135_5b6();
	void sentenceAddLeft(); // sub_135_7d6
	void sentenceAddRight(); // sub_135_7fa
	void sub_135_81e();
	void sub_135_86a();
	void sub_135_94c();
	void sub_135_9ba();
	void sub_135_a34();
	void sub_135_b16();
	void sub_135_c1c();
	void sub_135_cee();

	void sub_136_004();

	// fool_sun.cpp
	void sunMapRun(); // sub_137_004
	void sunMapOnClick(); // sub_137_3e2
	void sunMapDragSelect(); // sub_137_598
	void sunMapMoveSelected(); // sub_137_880
	void sunMapUndoMove(); // sub_137_d60
	void sunMapDropSelected(); // sub_137_dde
	void sunMapCheckIfSolved(); // sub_137_124e
	void sunMapRevealPiece(); // sub_137_131a

	void sub_138_004();

	// fool_cards.cpp
	void cardsRun(); // sub_139_004
	void sub_139_4de();
	void sub_139_50e();
	void cardsShuffleDeck(); // sub_139_51e
	void sub_139_582();
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

	void sub_140_004();

	// fool_death.cpp
	void deathRun(); // sub_141_004
	void deathMoveBlackEye(); // sub_141_3ca
	void deathDrawWhiteEye(); // sub_141_882
	void deathCaught(); // sub_141_8b8
	void deathDrawZoom(); // sub_141_934

	void sub_142_004();
	void sub_142_852();
	void sub_142_12ac();
	void sub_144_004();

private:
	bool _quit = false;

	GrafPtr var_i32_0;
	GrafPtr var_i32_4;
	GrafPtr var_i32_8;
	Handle var_i32_c;
	int16 var_i16_30;
	int16 var_i16_32;
	int16 var_i16_34;
	int16 var_i16_36;
	int16 var_i16_38;
	int16 var_i16_3a;
	int16 var_i16_44;
	EventRecord var_ev_46;
	int16 var_i16_56;
	int16 var_i16_58;
	int16 var_i16_5a;
	int16 var_i16_5c;
	SFReply var_sfr_5e; // -> aa
	int16 var_i16_16c;
	ProcPtr var_i32_16e;
	Common::U32String var_str_172;
	Common::U32String var_str_272;
	RGBColor var_i16_372;
	int16 var_i16_378;
	int16 var_i16_37a;
	uint32 var_i32_37c;
	Common::U32String var_str_384;
	int16 var_i16_484;
	Common::U32String var_str_486;
	int16 var_i16_586;
	Common::U32String var_str_588;
	int16 var_i16_688;
	int16 var_i16_68a;
	int16 var_i16_68c;
	uint32 var_i32_68e;
	uint32 var_i32_692;
	// this is normally a raw pointer to some memory,
	// instead we track it as a handle + offset
	uint32 var_ptr_696;
	Handle var_bytes_696;
	Common::U32String var_str_69a;
	int16 var_i16_78a;
	int16 var_i16_79e;
	int16 var_i16_7a0;
	int16 var_i16_7a2;
	int16 var_i16_7a8;
	int16 var_i16_7aa;
	int16 var_i16_7ac;
	int16 var_i16_7ae;
	int16 var_i16_7b0;
	int16 var_i16_7b2;
	int16 var_i16_7b4;
	int16 var_i16_7b6;
	int16 var_i16_7b8;
	int16 var_i16_7ba;
	int16 var_i16_7bc;
	int16 var_i16_7be;
	int16 keyLastPressed; // var_i16_7c0
	PicHandle var_pic_7c2;
	uint16 stateFlags; // var_i16_7c6
	uint32 var_i32_7c8;
	int16 var_i16_7cc;
	int16 var_i16_7ce;
	int16 var_i16_7d0;
	int16 var_i16_7d2;
	int16 storyCurrentPage; // var_i16_7d4
	int16 var_i16_7d6;
	int16 storyNextPage; // var_i16_7d8
	int16 storyPageCount; // var_i16_7da
	int16 selectedMenuChapter; // var_i16_7dc
	int16 var_i16_7de;
	int16 var_i16_7e0;
	int16 activePuzzle; // var_i16_7e2
	int16 var_i16_7e4;
	int16 var_i16_7e6;
	Common::U32String var_str_7e8;
	int16 var_i16_8e8;
	Common::U32String var_str_8ec;
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
	int16 var_i16_c04;
	Common::String activePuzzleBuffer; // var_str_c06

	int16 var_i16_d06;
	int16 var_i16_d0c;
	Common::U32String var_str_d12;
	int16 var_i16_e12;
	int16 var_i16_e14;
	int16 selectedMenuID; // var_i16_e16
	int16 selectedMenuItem; // var_i16_e18
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
	int32 var_i32_1036;
	int16 var_i16_103a;
	int16 var_i16_103c;
	int16 var_i16_103e;
	int16 var_i16_1040;
	int16 jumbleSubPuzzleCount; // var_i16_1056
	int16 jumbleCurrentSubPuzzle; // var_i16_1058
	int16 var_i16_105a;
	int16 var_i16_105c;
	int16 var_i16_105e;
	int16 var_i16_1060;
	int16 var_i16_1062;
	int16 var_i16_1064;
	int16 var_i16_1066;
	int16 jumbleGameType; // var_i16_1068
	int16 var_i16_106a;
	SourceMode var_i16_106c;
	int16 var_i16_106e;
	Common::U32String var_str_1070;
	Common::U32String var_str_1170;
	int16 var_i16_1270;
	Common::U32String var_str_1272;
	int16 var_i16_1372;

	Common::U32String var_str_1374;
	Common::U32String var_str_1474;
	int16 var_i16_1574;
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
	int16 var_i16_1a9e;
	int16 var_i16_1aa0;
	int16 var_i16_1aa4;
	int16 var_i16_1aa6;
	int16 var_i16_1aa8;
	int16 var_i16_1aaa;
	PolyHandle var_poly_1aac;
	int16 var_i16_1ab0;
	int16 var_i16_1ab2;
	int16 var_i16_1ab4;
	int16 var_i16_1ab6;
	int16 var_i16_1ab8;
	int16 var_i16_1aba;
	int16 var_i16_1abc;
	int16 var_i16_1abe;

	int16 var_i16_1e00;

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

	int16 var_i16_233a;

	// FIXME: bounds on the following are guessed! need to trace index range
	int16 pageLineRanges[202]; // arr_i16_0
	int16 pageLineFace[1301]; // arr_i16_194
	int16 pageLineBreak[1301]; // arr_i16_bbe
	int16 arr_i16_15e8[101];
	int16 arr_i16_16b2[0x100];
	int16 arr_i16_18b2[101];
	int16 arr_i16_197c[101];
	int16 pageToChapter[101]; // arr_i16_1a46
	int16 arr_i16_1b10[6969];
	int16 pageVisible[101]; // arr_i16_1b90
	int16 puzzleCompletionStatus[101]; // arr_i16_1c5a
	int16 arr_i16_1d24[101];
	int16 arr_i16_1dee[202];
	int16 arr_i16_1eb8[0x29];
	// used just for death?
	Common::Rect arr_rect_1eb8;
	Common::Rect arr_rect_1ec0;
	Common::Rect arr_rect_1f38[0x200];
	int16 arr_i16_2f38[0xc00]; // 96x32?
	int16 arr_i16_3738[0x400];
	int16 arr_i16_3b38[0xc00]; // 96x32?
	Common::Rect arr_rect_4338;
	// general-purpose puzzle buffer
	int16 arr_i16_4338[0x200];
	uint16 bitLUT[16]; // arr_i16_4738
	int16 arr_i16_4758[15];
	Common::Rect arr_rect_4776;
	int16 arr_i16_47d8[0x100];
	PicHandle arr_pic_49d8[0x80];
	int16 sunMapTileID[0x52]; // arr_i16_4bd8
	int16 arr_i16_4c7c[0x52];
	int16 arr_i16_4d20[48];
	Common::SharedPtr<Cursor> arr_curs_4d88[0x2b]; // 0x44
	Pattern arr_pat_58f4[0x51];
	Common::Rect arr_rect_5b7c;
	Common::Rect arr_rect_5b84;
	Common::Rect arr_rect_5b8c;
	Common::Rect arr_rect_5b92;
	int16 arr_i16_5bbc[0x1b];
	int16 arr_i16_5cbc[26];
	float arr_bcd_5dbc[8];
	Handle arr_bytes_5dfc; // used for data buffering
	BitMap arr_bmp_5dfc; // 22000, close to a screen page
	BitMap arr_jigsaw_5dfc[128];
	BitMap arr_bmp_b3ec;
	BitMap arr_bmp_bbbc; // used by death
	BitMap arr_bmp_c38c; // used by death
	Handle arr_bytes_109dc; // used for data buffering
	BitMap arr_bmp_109dc;
	BitMap arr_bmp_138bc;
	GrafPort arr_grafport_18eae;
	GrafPort arr_grafport_18f78;
	GrafPort arr_grafport_19042;
							// Also 109dc and 15fcc
	Common::Rect arr_rect_1910c;
	PicHandle arr_i32_1912c[0x54];
	PicHandle arr_i32_192c0[101];
	PolyHandle arr_poly_192c0[101];
	int32 arr_i32_19454[0x65];
	Common::U32String arr_str_195e8[101]; // 0x20 each

	Common::U32String arr_str_1a288[101]; // 0x10 each

	Common::U32String arr_str_1a8d8[7]; // 0x66 each
	Common::Rect arr_rect_1ae06;
};

}

#endif // FOOL_GAME_H
