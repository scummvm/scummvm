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

#ifndef FOOL_PROLOGUE_H
#define FOOL_PROLOGUE_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "fool/toolbox.h"
#include "graphics/surface.h"


namespace Fool {

class ZBasic;

enum FoolPrologueFontID : uint16 {
	// We subtract 10 from the original ID so they can coexist with
	// the fonts from the main game.
	kPrologueFontFool = 240,
	kPrologueFontSmall = 241,
};

class FoolPrologue {
public:
	FoolPrologue() {}
	~FoolPrologue() {}

	void run(bool finale);

	void setup(bool finale); // sub_128_004
	void copyScreenToPage(int16 screenPage); // sub_128_1ba
	void setPortBitsToPage(int16 screenPage); // sub_128_1f4
	void delay(int16 numTicks); // sub_128_21e
	void delayFromMarker(int16 numTicks); // sub_128_24a
	void drawTextRight(const Common::U32String &str, int16 x, int16 y); // sub_128_26c
	void drawTextCenter(const Common::U32String &str, int16 x, int16 y); // sub_128_2a6
	void fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID); // sub_128_2f0
	void zoomClose(int16 patternID, PatternMode mode); // sub_128_354
	void drawTreasurePhaseIn(int16 unk1); // sub_128_3ee
	void scanlineBlitPageToScreen(int16 screenPage, int16 left, int16 right, int16 updatePeriod); // sub_128_50a
	void blitPageToScreen(int16 screenPage); // sub_128_610
	void scanlineTransition(int16 patternID); // sub_128_64a
	void zoomTransition(int16 screenPage); // sub_128_6e4
	void zoomFlash(int16 startTop, int16 startLeft, int16 startBottom, int16 startRight, int16 endTop, int16 endLeft, int16 endBottom, int16 endRight, int16 steps); // sub_128_800
	void drawText(const Common::U32String &str, int16 x, int16 y); // sub_128_a6c
	void drawRainRecycle(int16 unk); // sub_128_a8c
	void shuffleScanlines(); // sub_128_ccc
	void drawClickMessage(); // sub_128_de2
	void drawClickMessageRightAlign(); // sub_128_e1c
	void waitForClick(); // sub_128_e58
	void waitForMouseUp(); // sub_128_e80
	void onUpdateEvent(); // sub_128_ed2
	void onDiskEvent(); // sub_128_ee0

	void sub_129_004();
	void sub_129_764();

	void prologueRun(); // sub_130_004
	void prologueBufferNextPicture(); // sub_130_cea
	void prologueDrawLoadingMsg(); // sub_130_d28
	void drawRain(); // sub_130_db0
	void drawRainDrop(); // sub_130_e82
	void prologueRenderNextText(); // sub_130_f48

	// fool_finale.cpp
	void finaleRun(); // sub_131_004
	void finaleDrawLoadingMsg(); // sub_131_4dc0
	void finaleDrawFoolUhOh(); // sub_131_4e48
	void finaleStartText(); // sub_131_4e98
	void finaleDrawWind(int16 offset); // sub_131_4f96
	void finaleCardRotate(); // sub_131_5038

private:
	Toolbox *_toolbox;
	ZBasic *_zbasic;
	bool _quit = false;

	// last tick count
	uint32 _tickMarker; // var_i32_2
	int16 var_i16_6;
	GrafPtr _grafPtrWindow; // var_i32_8
	GrafPtr _grafPtrMenu; // var_i32_c

	int16 _windowWidth; // var_i16_10
	int16 _windowHeight; // var_i16_12
	int16 _windowLeft; // var_i16_14
	int16 _windowTop; // var_i16_16

	EventRecord _event; // var_ev_22

	WindowRecord var_window_24;

	BitMap var_i32_32;

	int16 var_i16_36;
	Common::Rect var_i16_38;
	BitMap var_i32_40;
	int16 var_i16_44;
	Common::Rect var_i16_46;
	BitMap var_i32_4e;

	int16 var_i16_52;
	Common::Rect var_i16_54;

	Common::Rect var_i16_5c;
	Common::Rect var_i16_64;
	Common::Rect var_i16_6c;

	int16 var_i16_74;

	Common::U32String var_str_76;

	int16 var_i16_176;
	int16 var_i16_180;
	int16 var_i16_18c;
	int16 var_i16_18e;

	int16 var_i16_192;

	int16 var_i16_1a4;
	uint32 var_i32_1a6;

	// 1 for prologue, 2 for finale
	int16 var_i16_1aa;
	byte *var_i32_1ac;
	byte *var_i32_1b0;

	int16 var_i16_1b4;
	int16 var_i16_1b6;
	int16 var_i16_1b8;
	int16 var_i16_1ba;

	uint32 var_i32_1c0;

	Common::U32String var_i16_1c4;

	uint32 var_i32_2c4;
	int16 var_i16_2c8;
	int16 var_i16_2ca;
	int16 var_i16_2cc;
	Common::U32String var_i16_2ce;

	int16 _prologueLoading; // var_i16_3ce
	int16 _prologuePicIndex; // var_i16_3d4
	int16 var_i16_3e6;

	int16 var_i16_3ec;
	int16 var_i16_3ee;

	int16 var_i16_3fc;

	// picture resource handles
	PicHandle _pics[92] = { nullptr }; // arr_i32_0

	Pattern _patterns[5]; // arr_pat_194


	Common::Rect arr_i16_1bc;
	Common::Rect arr_i32_1c4;

	int16 arr_i16_1e8[1004] = { 0 };

	GrafPort _grafPortWindow; // arr_i32_9c0
	GrafPort _grafPortMenu; // arr_i32_a8a

	BitMap arr_i32_1e3fc[16];

	BitMap arr_i32_3bca4;

	BitMap arr_i32_41296[12];

	int16 _randScanline[SCREEN_HEIGHT] = { 0 }; // arr_i16_412ea

	int16 arr_i16_41af4[32];
	Common::Rect arr_rect_41af4;
	Common::Rect arr_i16_41afc;
	int16 arr_i16_41b04[3];
	Common::Rect arr_i16_41b0a;

	double arr_f64_41bbe[16] = { 0 };

};

} // End of namespace Fool

#endif // FOOL_PROLOGUE_H
