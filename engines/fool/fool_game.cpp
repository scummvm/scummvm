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

#include "common/endian.h"
#include "common/memstream.h"
#include "common/ustr.h"
#include "common/util.h"
#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/fool_prologue.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {



// Based on m68k disassembly of the Fool's Errand v2.0, (c) 1988 Cliff Johnson.

// v1.1 - original release, single-density disks, different about menu
// v2.0 - fixes full-screen rendering on higher-resolution displays, new about menu, can disable sounds
// v3.0 - newer ZBasic, changed a few graphics assets, removed custom menu font and sounds for compatibility


// Offset adjustments for the ZBasic string table to align
// across game versions
static const int fool11ZStrOffset[] = {
	128, 59, 81, 170, 186, 188, 192, 194, 202, 239, 258, 280, 315, 324, 13, 21
};

static const int fool20ZStrOffset[] = {
	102, 78, 103, 191, 207, 209, 213, 215, 223, 260, 279, 301, 336, 345, 18, 22
};

static const int fool30ZStrOffset[] = {
	97, 73, 98, 161, 177, 179, 183, 185, 193, 230, 251, 273, 308, 317, 8, 12
};

// Fool's Errand v1.1 is missing FOND data, below is taken from 2.0
static const byte fondChicago[] = {
	96, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 12, 0, 0, 0, 12
};
static const byte fondFool[] = {
	96, 0, 0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 1, 0, 12, 0, 0, 125, 12, 0, 24, 0, 0,
	125, 24
};
static const byte fondLarge[] = {
	96, 0, 0, 254, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 127, 24
};
static const byte fondPuzzle[] = {
	96, 0, 0, 251, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 2, 0, 12, 0, 0, 125, 140, 0, 24, 0, 0,
	125, 152, 0, 48, 0, 0, 125, 176
};
static const byte fondSmall[] = {
	96, 0, 0, 252, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 126, 9
};

void FoolGame::run() {
	_toolbox = new Toolbox();
	_zbasic = new ZBasic(_toolbox);

	Common::MacFinderInfo finfo;
	if (_toolbox->GetFInfo(Common::U32String("The Fool's Errand"), 0, finfo) == kNoErr) {
		_zbasic->loadProgram(Common::Path("The Fool's Errand", ':'));
	// v1.0 filename ends with "tm"
	} else if (_toolbox->GetFInfo(Common::U32String("xn--The Fool's Errand-306j"), 0, finfo) == kNoErr) {
		_zbasic->loadProgram(Common::Path("xn--The Fool's Errand-306j", ':'));
	} else {
		error("FoolGame::run: Fool's Errand program not found");
		return;
	}

	int16 fontChicago = 0;
	switch (_version) {
	case kFool11:
		_zstrOffset = fool11ZStrOffset;
		break;
	case kFool30:
		_zstrOffset = fool30ZStrOffset;
		// v3.0 calls the font "Foolish Chicago" and changes the index
		fontChicago = 255;
		break;
	case kFool20:
	default:
		_zstrOffset = fool20ZStrOffset;
		break;
	}

	if (_version == kFool11) {
		// v1.1 doesn't include any FOND chunks, so we have to provide them here.
		_zbasic->injectFOND(fondFool, sizeof(fondFool), Common::String("Fool"));
		_zbasic->injectFOND(fondChicago, sizeof(fondChicago), Common::String("Foolish Chicago"));
		_zbasic->injectFOND(fondLarge, sizeof(fondLarge), Common::String("Large"));
		_zbasic->injectFOND(fondPuzzle, sizeof(fondPuzzle), Common::String("Puzzle"));
		_zbasic->injectFOND(fondSmall, sizeof(fondSmall), Common::String("Small"));
	} else {
		// Load in the FOND chunks from the executable
		int16 exec = _zbasic->getFileId();
		Handle foolFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), kFontFool);
		_toolbox->_injectFOND(exec, foolFOND->data(), foolFOND->size(), Common::String("Fool"));
		Handle chicagoFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), fontChicago);
		// v2.0 sets the ID to 0 in order to override Chicago, patch it.
		chicagoFOND->data()[3] = kFontChicago;
		_toolbox->_injectFOND(exec, chicagoFOND->data(), chicagoFOND->size(), Common::String("Foolish Chicago"));
		Handle largeFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), kFontLarge);
		_toolbox->_injectFOND(exec, largeFOND->data(), largeFOND->size(), Common::String("Large"));
		Handle puzzleFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), kFontPuzzle);
		_toolbox->_injectFOND(exec, puzzleFOND->data(), puzzleFOND->size(), Common::String("Puzzle"));
		Handle smallFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), kFontSmall);
		_toolbox->_injectFOND(exec, smallFOND->data(), smallFOND->size(), Common::String("Small"));
	}


	// Fool's Errand has an embedded version of Chicago with custom characters
	_zbasic->setMenuFont(kFontChicago, 12);

	// Start the game
	foolRun();
	delete _zbasic;
	_zbasic = nullptr;
	delete _toolbox;
	_toolbox = nullptr;
}

void FoolGame::foolRun() {
	// 128:0004
	// zero out variables
	_storyCurrentChapter = 0;
	_sunMapRestored = 0;
	_menuDisabled = false;
	_menuHidesPlayfield = false;
	_isAutoSaving = false;
	// zero out pattern buffer; menuLoadingMessage uses entry
	// 0 for filling the top bar
	for (int i = 0; i <= 0x50; i++) {
		Common::fill(_patterns[i].data, _patterns[i].data + 8, 0);
	}

	// Define the bitmap surfaces (normally pointers to raw memory)
	this->arr_bmp_5dfc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_b3ec = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_bbbc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_c38c = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_fa3c = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_109dc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_138bc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));

	// 128:0004
	_zbasic->unk_331(0xdac0, 0);
	_zbasic->unk_331(0x1b58, 1);
	_zbasic->unk_331(0x1b58, 2);

	_toolbox->SetRect(_screenClipRect, 0, MENU_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);

	// copyright + zbasic notice
	//this->var_str_384 = _zbasic->str(0);
	//this->var_str_384 = _zbasic->str(1);
	//this->var_str_384 = _zbasic->str(2);

	// 128:0086
	// 128:0086: MOVEQ - 0x0,D0
	// 128:0088: SNE - -0x98c(A5)
	_zbasic->unk_44(0);

	this->var_i16_484 = 0;

	// 128:0096: LEA - [0x3818],A0
	// 128:009a: MOVE.L - A0,-0x8ee(A5)
	// 128:009e: JMP - [0x118]
	// 128:0118: JMP - [0x1e0]
	// 128:01e0: JMP - [0x2ba]
	// 128:02ba: JMP - [0x33e]
	// 128:033e: JMP - [0x3d6]
	// 128:03d6: JMP - [0x402]
	// 128:0402: JMP - [0x424]
	// 128:0424: JMP - [0x442]
	// 128:0442: JMP - [0x45e]
	// 128:045e: JMP - [0x478]
	// 128:0478
	//this->var_str_69a.clear(); // was: str(6);
	//this->var_i32_79a = &this->var_str_69a;
	// 128:0496: JMP - [0x4d6]
	// 128:04d6: JMP - [0x50a]
	// 128:050a: JMP - [0x558]
	// 128:0558: JMP - [0x5fa]
	// 128:05fa: JMP - [0x648]
	// 128:0648: JMP - [0x698]
	// 128:0698: JMP - [0x70e]
	// 128:070e: JMP - [0x8b0]
	// 128:08b0: JMP - [0x914]
	// 128:0914: JMP - [0x95e]
	// 128:095e: JMP - [0xbda]
	// 128:0bda: JMP - [0xc66]
	// 128:0c66: JMP - [0xd30]
	// 128:0d30: JMP - [0xdfa]
	// 128:0dfa: JMP - [0x1786]
	// 128:1786: JMP - [0x1c28]
	// 128:1c28: JMP - [0x1c46]
	// 128:1c46: JMP - [0x1c7a]

	// 128:1c7a
	this->sub_129_068();
	this->var_i32_7c8 = _zbasic->mem(-1);
	// 128:1c88: JMP - [0x1ee2]
	while (_stateFlags != kStateQuit) {
		do {
			this->getNextEvent(-1);
			if (_quit)
				return;

			if ((_event.where.y >= 0x8c) && (_event.where.y <= 0x11d)) {
				this->var_i16_7cc = -1;
			}
			// 128:1cbe
			if (_event.where.y > 0x11d) {
				this->var_i16_7cc = 1;
			}
			// 128:1cce
			if ((_event.where.x <= 0x8c) || (_event.where.y < 0xdc)) {
				this->var_i16_7cc = 0;
			}
			// 128:1cf6
			if (this->var_i16_7cc != this->var_i16_7b2) {
				this->var_i16_7b2 = this->var_i16_7cc;
				if (this->var_i16_7b2 == 0) {
					if ((this->var_i16_7ce & 2) == 0) {
						_toolbox->InitCursor();
					} else {
						_toolbox->SetCursor(_cursors[0x10]);
					}
					// 128:1d42
				} else {
					// 128:1d46
					if (this->var_i16_7b2 == -1) {
						// arrow pointing up
						_toolbox->SetCursor(_cursors[0x2]);
					}
					// 128:1d5e
					if (this->var_i16_7b2 == 1) {
						// arrow pointing down
						_toolbox->SetCursor(_cursors[0x1]);
					}
				}
			}
			// 128:1d76
			if (_event.what == kMouseDown) {
				this->sub_128_1f76();
			}
			if ((_storyCurrentChapter == 0x10) &&
					(_puzzleCompletionStatus[0x10] < 0x64) &&
					(_event.where.y > 0x113) &&
					(_event.where.x < 0x8c)) {
				setStateBits(kStatePuzzleSelect);
			}
			// 128:1de0
			if (_sunMapRestored && (_keyLastPressed == 0x20)) {
				setStateBits(0x100);
			}
		// 128:1e06
		} while (_stateFlags == 0);

		if (_stateFlags & kStateReturn) {
			this->clearStateBits(kStateReturn);
		}
		if (_stateFlags & kStateSaveGame) {
			this->saveGame();
		}
		if (_stateFlags & kStateNewGame) {
			this->newGame();
		}
		if (_stateFlags & kStateOpenGame) {
			this->openGame();
		}
		if (_stateFlags & kStateChapterSelect) {
			this->menuChapterSelect();
		}
		if (_stateFlags & 0x200) {
			this->sub_128_1f44();
		}
		if (_stateFlags & kStatePuzzleSelect) {
			this->puzzleRun();
		}
		if (_stateFlags & kStateMetapuzzleSelect) {
			this->metapuzzleRun();
		}
		if (_stateFlags & kStatePrintStory) {
			this->printStory();
		}
		if (_stateFlags & kStateMetapuzzleComplete) {
			this->sub_144_004();
		}
	// 128:1ee2
	}
	// 128:1eec
	this->sub_128_1f1e();
	_zbasic->unk_158();
	_zbasic->unk_4();
	_toolbox->SetPort(this->var_i32_8);
	fillRect(0, 0, MENU_HEIGHT, _windowWidth, 2);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::copyScreen(int16 put, BitMap &bmp) {
	// 128:00a2
	// the original code would use the memory at 5dfc + 2*arg2.
	// to make this less bad, our version passes a BitMap pointer
	warning("copyScreen: put %d, bmp %p", put, (void *)&bmp);
	if (put == 0) {
		_zbasic->get(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bmp);
	}

	// 128:00e6
	if (put == 1) {
		_zbasic->put(0, 0, bmp, kSrcCopy);
	}
}

void FoolGame::openSaveFileDialog(const Common::U32String &title, const Common::U32String &filename) {
	// 128:011c
	this->var_str_384 = _zbasic->str(3);

	this->var_i16_586 = 0;

	this->copyScreen(0, this->arr_bmp_138bc);

	// 128:015a
	_event.where.y = this->var_i16_58 + 0x3d;
	_event.where.x = this->var_i16_56 + 0x68;

	// 128:017e
	_toolbox->SFPutFile(_event.where, title, filename, 0, this->var_sfr_5e);
	this->sub_128_6244();

	this->copyScreen(1, this->arr_bmp_138bc);

	// 128:01b0
	if (this->var_sfr_5e.good == 1) {
		this->var_str_486 = this->var_sfr_5e.fName;
		this->var_i16_586 = this->var_sfr_5e.vRefNum;
	}

}

void FoolGame::sub_128_1e4(const Common::String &osType) {
	// 128:01e4
	this->var_str_588 = _zbasic->str(4);
	this->var_i16_688 = 0;

	this->copyScreen(0, this->arr_bmp_138bc);
	// 128:0218
	_event.where.y = this->var_i16_58 + 0x3d;
	_event.where.x = this->var_i16_56 + 0x53;
	// 128:023c
	// get offset of bytes in string and pretend it's an OSType
	// this->var_i32_168 = *(this->var_str_172 + 1);
	SFTypeList typeList = { { 0 } };
	typeList.types[0] = osType.at(0) << 24;
	typeList.types[0] += osType.at(1) << 16;
	typeList.types[0] += osType.at(2) << 8;
	typeList.types[0] += osType.at(3) << 0;

	_toolbox->SFGetFile(_event.where, _zbasic->str(5), 0, 1, typeList, 0, this->var_sfr_5e);
	this->sub_128_6244();
	this->copyScreen(1, this->arr_bmp_138bc);

	if (this->var_sfr_5e.good == 1) {
		// 128:02a2
		this->var_str_588 += this->var_sfr_5e.fName;
		this->var_i16_688 = this->var_sfr_5e.vRefNum;
	}
}

void FoolGame::getGridFromMouse(int16 &gridX, int16 &gridY) {
	// 128:02be
	// convert mouse coordinates to grid coordinates
	gridX = (_event.where.x - this->arr_i16_1eb8[4]) / (this->arr_i16_1eb8[6]);

	gridY = (_event.where.y - this->arr_i16_1eb8[5]) / (this->arr_i16_1eb8[7]);
}

void FoolGame::sub_128_342(int16 unk2, int16 unk1) {
	// 128:0342
	this->var_i16_68c = unk1;
	this->var_i16_68a = unk2;
	if (this->var_i16_68a < 1) {
		this->var_i16_68a = 1;
	}
	// 128:035c
	if (this->var_i16_68a > this->arr_i16_1eb8[0]) {
		this->var_i16_68a = this->arr_i16_1eb8[0];
	}
	if (this->var_i16_68c < 1) {
		this->var_i16_68c = 1;
	}
	if (this->var_i16_68c > this->arr_i16_1eb8[1]) {
		this->var_i16_68c = this->arr_i16_1eb8[1];
	}
}

void FoolGame::delay(int16 numTicks) {
	// 128:03da
	uint32 prev = _toolbox->TickCount();

	do {
		_toolbox->Delay(0);
	} while (_toolbox->TickCount() < (prev + numTicks));
}

void FoolGame::delayFromMarker(int16 numTicks) {
	// 128:0402
	do {
		_toolbox->Delay(0);
	} while (_toolbox->TickCount() < (this->var_i32_692 + numTicks));
}

int16 FoolGame::puzzlesReadByte() {
	// 128:0428
	// read a byte
	int16 data = *(byte *)(&_puzzleDataBuffer->data()[this->_puzzleDataPtr]);
	debugC(8, kDebugLoading, "Read[%04x]: %02x", this->_puzzleDataPtr, data);
	this->_puzzleDataPtr += 1;
	return data;
}

int16 FoolGame::puzzlesReadShort() {
	// 128:0446
	// read a short
	int16 data = READ_BE_INT16(&_puzzleDataBuffer->data()[this->_puzzleDataPtr]);
	debugC(8, kDebugLoading, "Read[%04x]: %04x", this->_puzzleDataPtr, data);
	this->_puzzleDataPtr += 2;
	return data;
}

int32 FoolGame::puzzlesReadLong() {
	// 128:0462
	// read a long
	int32 data = READ_BE_INT32(&_puzzleDataBuffer->data()[this->_puzzleDataPtr]);
	debugC(8, kDebugLoading, "Read[%04x]: %08x", this->_puzzleDataPtr, data);
	this->_puzzleDataPtr += 4;
	return data;
}

Common::String FoolGame::puzzlesReadString() {
	// 128:049a
	// read a pascal string
	this->var_i16_79e = *(byte *)(&_puzzleDataBuffer->data()[this->_puzzleDataPtr]);
	Common::String result((const char *)&_puzzleDataBuffer->data()[this->_puzzleDataPtr+1], this->var_i16_79e);
	debugC(8, kDebugLoading, "Read[%04x]: %s", this->_puzzleDataPtr, result.c_str());
	this->_puzzleDataPtr += this->var_i16_79e + 1;
	return result;
}

void FoolGame::toggleMouseCursor(bool visible) {
	// toggle the mouse cursor off or on
	// 128:04da
	if (!visible) {
		if (this->_mouseVisible) {
			_toolbox->HideCursor();
		}
	} else {
		if (!this->_mouseVisible) {
			_toolbox->ShowCursor();
		}
	}
	this->_mouseVisible = visible;
}

void FoolGame::playTone(int16 freq, int16 duration, bool wait) {
	// 128:050e
	// audio tone
	if (_soundEnabled) {
		// the volume was originally 150, this is crazy loud.
		_zbasic->sound(freq, duration, 20, 0);
		if (wait) {
			while (_zbasic->soundBusy()) {
				_toolbox->Delay(0);
			}
		}
	}
}

void FoolGame::drawPuzzleButton(const Common::U32String &symbol) {
	// 128:055c
	_toolbox->PenNormal();
	_toolbox->SetRect(this->arr_rect_1910c, 0x6c, 0x127, 0x84, 0x137);
	_toolbox->EraseRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	_toolbox->FrameRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
	int16 width = _toolbox->StringWidth(symbol);
	_toolbox->MoveTo(0x78 - (width / 2), 0x133);
	_toolbox->DrawString(symbol);
}

int16 FoolGame::getVolRefNum() {
	// 128:05fe
	ParamBlockRec pb;
	_toolbox->PBGetVol(pb);
	return pb.ioVRefNum;
}

OSErr FoolGame::setVolRefNum(int16 volRefNum) {
	// 128:064c
	ParamBlockRec pb;
	pb.ioVRefNum = volRefNum;
	return _toolbox->PBSetVol(pb);
}

void FoolGame::fillRect(int16 patternID, PatternMode mode, int16 top, int16 left, int16 bottom, int16 right) {
	// 128:069c
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[patternID]);
	_toolbox->PenMode(mode);
	Common::Rect bounds; // arr_rect_5b7c
	_toolbox->SetRect(bounds, left, top, right, bottom);
	_toolbox->PaintRect(bounds);
	_toolbox->PenNormal();
}

void FoolGame::drawTarotCard(int16 rectID, int16 deckPos, int16 type) {
	// 128:0712
	_toolbox->PenNormal();
	if (type == 0) {
		_toolbox->PenSize(0x3, 0x3);
		_toolbox->PenPat(_patterns[0]);
		_toolbox->FrameRoundRect(_screenGrid[rectID], 0xf, 0xf);
		_toolbox->PenSize(1, 1);
		_toolbox->PenPat(_patterns[2]);
		_toolbox->FrameRoundRect(_screenGrid[rectID], 0xf, 0xf);
		// 128:079e
		_zbasic->picture(_screenGrid[rectID].left + 3, _screenGrid[rectID].top + 3, this->arr_i32_192c0[this->arr_i16_5cbc[deckPos]]);
	}
	// 128:0806
	if (type == 1) {
		_toolbox->PenPat(_patterns[1]);
		_toolbox->PaintRoundRect(_screenGrid[rectID], 0xc, 0xc);
	}
	if (type == 2) {
		_toolbox->PenMode(kPatOr);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->PaintRoundRect(_screenGrid[rectID], 0xc, 0xc);
	}
	if (type == 3) {
		_toolbox->InvertRoundRect(_screenGrid[rectID], 0xc, 0xc);
	}
	_toolbox->PenNormal();
}

void FoolGame::fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID) {
	// 128:08b4
	Common::Rect bounds; // arr_rect_5b7c
	_toolbox->SetRect(bounds, left, top, right, bottom);
	_toolbox->FillRect(bounds, _patterns[patternID]);
}

void FoolGame::drawTextCenter(const Common::U32String &str, int16 yPos) {
	// 128:0918
	int16 width = _toolbox->StringWidth(str);
	_toolbox->MoveTo((SCREEN_WIDTH / 2) - (width / 2), yPos);
	_toolbox->DrawString(str);
}

void FoolGame::zoomRect(int16 startTop, int16 startLeft, int16 startBottom, int16 startRight, int16 endTop, int16 endLeft, int16 endBottom, int16 endRight, int16 patternID, PatternMode mode, int16 steps) {
	// Zoom-fill the screen with checkerboard pattern
	// after e.g. clicking a story button.
	// 128:0962
	Common::Rect start;
	Common::Rect end;
	start.top = startTop;
	start.left = startLeft;
	start.bottom = startBottom;
	start.right = startRight;
	end.top = endTop;
	end.left = endLeft;
	end.bottom = endBottom;
	end.right = endRight;
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[patternID]);
	_toolbox->PenMode(mode);
	// 128:0a42
	// unrolled loop
	this->arr_bcd_5dbc[0] = (float)(start.top);
	this->arr_bcd_5dbc[1] = (float)(start.left);
	this->arr_bcd_5dbc[2] = (float)(start.bottom);
	this->arr_bcd_5dbc[3] = (float)(start.right);
	this->arr_bcd_5dbc[4] = (float)((end.top) - (start.top))/(float)(steps);
	this->arr_bcd_5dbc[5] = (float)((end.left) - (start.left))/(float)(steps);
	this->arr_bcd_5dbc[6] = (float)((end.bottom) - (start.bottom))/(float)(steps);
	this->arr_bcd_5dbc[7] = (float)((end.right) - (start.right))/(float)(steps);
	// 128:0af0
	_toolbox->PaintRect(start);
	for (int i = 1; i < steps-1; i++) {
		for (int j = 0; j <= 3; j++) {
			this->arr_bcd_5dbc[j] = (float)this->arr_bcd_5dbc[j] + (float)this->arr_bcd_5dbc[j+4];
		}
		Common::Rect temp; // arr_rect_5b92
		temp.top = (int)this->arr_bcd_5dbc[0];
		temp.left = (int)this->arr_bcd_5dbc[1];
		temp.bottom = (int)this->arr_bcd_5dbc[2];
		temp.right = (int)this->arr_bcd_5dbc[3];
		// 128:0ba6
		_toolbox->PaintRect(temp);
		// new: force a redraw delay
		_toolbox->Delay(0);
	}
	// 128:0bc8
	_toolbox->PaintRect(end);
	_toolbox->PenNormal();
}

void FoolGame::sub_128_bde(int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:0bde
	this->arr_i16_4758[0] = unk6;
	this->arr_i16_4758[1] = unk5;
	this->arr_i16_4758[2] = unk4;
	this->arr_i16_4758[3] = unk3;
	this->arr_i16_4758[4] = unk2;
	this->arr_i16_4758[5] = unk1;
	this->storyDrawZoom();
}

void FoolGame::getNextEvent(uint32 unk1) {
	// 128:0c6a

	// This function is usually called at the start of an event processing loop,
	// so yield to the event pump/display update when necessary.
	if (_event.what == kNullEvent)
		_toolbox->Delay(0);

	this->var_i16_78a = _toolbox->GetNextEvent(unk1, _event);
	if ((_event.what == kMouseDown) && (_event.where.y < MENU_HEIGHT)) {
		this->onClickMenu();
	}
	// 128:0caa
	_toolbox->GlobalToLocal(_event.where);
	if (_event.what == kKeyDown) {
		// the original just checked the command key,
		// non-mac PCs expect the control key to work
		if ((_event.modifiers & (kModCommandKeyDown | kModLControlKeyDown)) == 0) {
			this->sub_128_5f9e();
		} else {
			this->onMenuKey();
		}
	}
	// 128:0ce0
	if ((_event.what == kAutoKey) && ((_event.modifiers & (kModCommandKeyDown | kModLControlKeyDown)) == 0)) {
		this->sub_128_5f9e();
	}
	if (_event.what == kUpdateEvt) {
		this->onUpdateEvent();
	}
	if (_event.what == kDiskEvt) {
		this->onDiskEvent();
	}
	if ((_event.what == kScummVMQuitEvt) || (_event.what == kScummVMReturnToLauncherEvt)) {
		this->menuQuit();
	}
}

void FoolGame::flashRect(int16 top, int16 left, int16 bottom, int16 right, int16 millis) {
	// flash a rectangle on the screen until we get a click
	// 128:0d34
	Common::Rect bounds; // arr_rect_5b7c
	bounds.top = top;
	bounds.left = left;
	bounds.bottom = bottom;
	bounds.right = right;
	while (_event.modifiers & kModMouseButtonUp) {
		// 128:0d94
		do {
			// FIXME: Flashing far too intense
			_toolbox->InvertRect(bounds);
			int ticks = 0;
			do {
				// originally this used getNextEvent, but we avoid that here
				// so that menu events don't get intercepted.
				// originally this mask was 0, but we change it here to
				// intercept all events, and fall back to NullEvent +
				// wait for vsync if no events were received.
				this->var_i16_78a = _toolbox->GetNextEvent(-1, _event);
				if (_event.what == kNullEvent) {
					_toolbox->Delay(0);
					ticks += 1;
				}
			} while (!((ticks >= (millis*60/1000)) || ((_event.modifiers & kModMouseButtonUp) == 0)));
		} while ((_event.modifiers & kModMouseButtonUp) != 0);
	}
}

void FoolGame::showChoiceModal(uint16 font, int16 lineCount, int16 buttonCount, bool beep) {
	// 128:0dfe
	_toolbox->SetPort(this->var_i32_4);
	this->var_i16_7b2 = 0xa;
	_toolbox->InitCursor();
	this->toggleMouseCursor(true);
	if (beep) {
		this->playTone(0x19, 0x64, false);
	}
	// 128:0e46
	this->copyScreen(0, this->arr_bmp_138bc);
	_zbasic->text(font, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_i16_7b4 = buttonCount*0x46;
	this->var_i16_7b6 = 0;
	for (int i = 0; i <= lineCount; i++) {
	// 128:0e86
		this->var_i16_7ba = _toolbox->StringWidth(_modalText[i]);
		if (this->var_i16_7ba > this->var_i16_7b4) {
			this->var_i16_7b4 = this->var_i16_7ba;
		}
		this->var_i16_7b6 += 0x11;
	}
	// 128:0ed6
	if (buttonCount >= 0) {
		this->var_i16_7bc = 0xa4 + (this->var_i16_7b6 / 2);
		this->var_i16_7b6 += 0x28;
	} else {
		this->var_i16_7b6 += 0xd;
	}
	// 128:0f08
	this->var_i16_7b4 = (this->var_i16_7b4 / 2) + 0xf;
	this->var_i16_7b6 = (this->var_i16_7b6 / 2);
	_toolbox->PenNormal();
	Common::Rect bounds; // arr_rect_5b7c
	_toolbox->SetRect(bounds, 0xf5-this->var_i16_7b4, 0xa0-this->var_i16_7b6, 0x10b+this->var_i16_7b4, 0xb6+this->var_i16_7b6);
	_toolbox->PenPat(_patterns[0]);
	_toolbox->FrameRect(bounds);
	_toolbox->InsetRect(bounds, 1, 1);
	_toolbox->PenSize(5, 5);
	_toolbox->PenPat(_patterns[2]);
	_toolbox->FrameRect(bounds);
	_toolbox->InsetRect(bounds, 5, 5);
	_toolbox->PenSize(5, 5);
	// 128:0ff8
	_toolbox->PenPat(_patterns[1]);
	_toolbox->FrameRect(bounds);
	_toolbox->InsetRect(bounds, 5, 5);
	_toolbox->FillRect(bounds, _patterns[2]);
	_toolbox->PenNormal();
	int16 strY = 0xbe - this->var_i16_7b6;
	// 128:1056
	for (int i = 0; i <= lineCount; i++) {
		this->drawTextCenter(_modalText[i], strY);
		// 128:1086
		// 128:1086: CLR.W - -0x772(A5)
		strY += 0x11;
	}
	// 128:10a0
	if (buttonCount != 0) {
		_toolbox->PenNormal();
		_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);

		// 128:10c0
		// loop to zero out three button rects
		Common::Rect button1 = Common::Rect(0, 0, 0, 0); // arr_rect_5b7c
		Common::Rect button2 = Common::Rect(0, 0, 0, 0); // arr_rect_5b84
		Common::Rect button3 = Common::Rect(0, 0, 0, 0); // arr_rect_5b8c
		// 128:10e2
		if (buttonCount == 1) {
			_toolbox->SetRect(button1, 0xe2, this->var_i16_7bc, 0x11e, this->var_i16_7bc+0x14);
			// 128:1122
		} else if (buttonCount == 2) {
			_toolbox->SetRect(button1, 0xbf, this->var_i16_7bc, 0xfb, this->var_i16_7bc+0x14);
			_toolbox->SetRect(button2, 0x105, this->var_i16_7bc, 0x141, this->var_i16_7bc+0x14);
			// 128:1182
		} else if (buttonCount == 3) {
			_toolbox->SetRect(button1, 0x9c, this->var_i16_7bc, 0xd8, this->var_i16_7bc+0x14);
			_toolbox->SetRect(button2, 0xe2, this->var_i16_7bc, 0x11e, this->var_i16_7bc+0x14);
			_toolbox->SetRect(button3, 0x128, this->var_i16_7bc, 0x164, this->var_i16_7bc+0x14);
		}
		// 128:1208
		if (buttonCount >= 1) {
			_toolbox->EraseRoundRect(button1, 0xa, 0xa);
			_toolbox->FrameRoundRect(button1, 0xa, 0xa);
			this->var_str_172 = _modalText[(lineCount + 1)];
			this->var_i16_30 = _toolbox->StringWidth(this->var_str_172);
			// 128:1274
			this->var_i16_30 = button1.left + ((button1.right - button1.left) / 2) - (this->var_i16_30 / 2);
			// 128:12d4
			_toolbox->MoveTo(this->var_i16_30, this->var_i16_7bc + 0xe);
			_toolbox->DrawString(this->var_str_172);
			if (buttonCount > 1) {
				_toolbox->InsetRect(button1, -2, -2);
				_toolbox->PenPat(_patterns[0]);
				_toolbox->FrameRoundRect(button1, 0xa, 0xa);
				_toolbox->PenNormal();
				_toolbox->InsetRect(button1, 2, 2);
			}
		}
		// 128:134c
		if (buttonCount >= 2) {
			_toolbox->EraseRoundRect(button2, 0xa, 0xa);
			_toolbox->FrameRoundRect(button2, 0xa, 0xa);
			this->var_str_172 = _modalText[(lineCount+2)];
			this->var_i16_30 = _toolbox->StringWidth(this->var_str_172);
			this->var_i16_30 = button2.left + ((button2.right - button2.left)/2) - (this->var_i16_30 / 2);
			// 128:1418
			_toolbox->MoveTo(this->var_i16_30, this->var_i16_7bc + 0xe);
			_toolbox->DrawString(this->var_str_172);
		}
		// 128:1432
		if (buttonCount == 3) {
			_toolbox->EraseRoundRect(button3, 0xa, 0xa);
			_toolbox->FrameRoundRect(button3, 0xa, 0xa);
			this->var_str_172 = _modalText[lineCount+3];
			this->var_i16_30 = _toolbox->StringWidth(this->var_str_172);
			this->var_i16_30 = (button3.left + ((button3.right - button3.left)/2)) - (this->var_i16_30/2);

			_toolbox->MoveTo(this->var_i16_30, this->var_i16_7bc + 0xe);
			_toolbox->DrawString(this->var_str_172);
		}
		// 128:1518
		this->sub_128_61ec();
		_savePromptChoice = 0;
		// 128:1522
		do {
			this->var_i16_7a8 = _toolbox->GetNextEvent(0xa, _event);
			_toolbox->GlobalToLocal(_event.where);
			if (_event.what == kMouseDown) {
				// 128:154a
				_savePromptChoice = 0;
				Common::Rect target;
				if (_toolbox->PtInRect(_event.where, button1)) {
					_savePromptChoice = 1;
					target = button1;
				}
				if (_toolbox->PtInRect(_event.where, button2)) {
					_savePromptChoice = 2;
					target = button2;
				}
				if (_toolbox->PtInRect(_event.where, button3)) {
					_savePromptChoice = 3;
					target = button3;
				}

				if (_savePromptChoice > 0) {
					// 128:15d2
					this->var_i16_30 = (_savePromptChoice - 1)*4;
					do {
						_toolbox->InvertRoundRect(target, 0xa, 0xa);

						// 128:1624
						while ((_event.what != kMouseUp) && (_toolbox->PtInRect(_event.where, target))) {

							this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
							_toolbox->GlobalToLocal(_event.where);
							if (_event.what == kNullEvent) {
								_toolbox->Delay(0);
							}
						}

						_toolbox->InvertRoundRect(target, 0xa, 0xa);
						// 128:1686
						while ((_event.what != kMouseUp) && (!_toolbox->PtInRect(_event.where, target))) {
							this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
							_toolbox->GlobalToLocal(_event.where);
							if (_event.what == kNullEvent) {
								_toolbox->Delay(0);
							}
						}
						// 128:16ea
					} while (_event.what != kMouseUp);

					if (!_toolbox->PtInRect(_event.where, target)) {
						_savePromptChoice = 0;
					}
					this->sub_128_61ec();
				}
			}
			// 128:172c
			if (_event.what == kKeyDown) {
				_keyLastPressed = _event.message & 0xff;
				if (_keyLastPressed == 0xd) {
					_savePromptChoice = 1;
				}
			}
			if (_event.what == kNullEvent)
				_toolbox->Delay(0);
		// 128:175c
		} while (_savePromptChoice == 0);

		this->sub_128_61ec();
		this->copyScreen(1, this->arr_bmp_138bc);
		_toolbox->SetPort(this->var_i32_0);
	}
	// 128:1784

}

void FoolGame::showBehold(int16 unk2, int16 unk1, const Common::U32String &message) {
	// behold the Nth key of thoth animation
	this->var_i16_32 = unk1;
	this->var_i16_30 = unk2;
	if (this->var_i16_30 > 0) {
		if ((_puzzleFlags[this->var_i16_30] & kFlagMenuDiamond)) {
			_puzzleFlags[this->var_i16_30] ^= kFlagMenuDiamond;
			_puzzleFlags[this->var_i16_30] |= kFlagMenuEnabled;
		}
		// 128:1818
		int menuNo = 3 + ((this->var_i16_30 - 1) / 0x10);
		int itemNo = 1 + ((this->var_i16_30 - 1) % 0x10);
		// add the wadjet eye next to the chapter name
		_zbasic->menu(menuNo, itemNo, 1, _puzzleName[this->var_i16_30] + Common::U32String(" ~ ")); // was: str(9)
	}
	// 128:188a
	this->toggleMouseCursor(false);
	_zbasic->picture(0, MENU_HEIGHT, this->var_pic_7c2);
	_zbasic->text(kFontFool, 0x18, Graphics::kMacFontShadow | Graphics::kMacFontOutline, kSrcBic);
	Common::Rect rect;
	for (int j = 0; j <= 1; j++) {
		// 128:18c4
		for (int i = 0; i <= 0x64; i += 4) {
			this->var_i32_692 = _toolbox->TickCount();
			_toolbox->SetRect(
				rect,
				(0x105 - (int)(i*2.2f)),
				i + 0xa5,
				(0x105 + (int)(i*2.2f)),
				(0xa6 + (int)(i*1.6f))
			);
			// 128:1982
			_toolbox->InvertRect(rect);
			this->delayFromMarker(0);
		}
		// 128:19a4
	}
	// 128:19b2
	_toolbox->PenNormal();
	_toolbox->PaintRect(rect);
	_toolbox->PenMode(kPatXor);
	_toolbox->FrameRect(rect);
	_toolbox->PenMode(kPatCopy);
	this->var_i16_484 = _toolbox->StringWidth(message);
	// 128:19ec
	_toolbox->MoveTo(0x105 - (this->var_i16_484/2), 0x12e);
	_toolbox->DrawString(message);
	// loop count was 0x4d
	for (int i = 0; i <= 0x24; i++) {
		this->var_i32_692 = _toolbox->TickCount();
		_toolbox->InvertRect(rect);
		// this was 1, however the flashing was far too intense
		this->delayFromMarker(8);
	}
	for (int i = 1; i <= 0x24; i++) {
		// 128:1a4c
		this->var_i32_692 = _toolbox->TickCount();
		rect.top -= (int)(i*0.4f);
		rect.left -= (int)(i*0.1f);
		rect.bottom += (int)(i*0.05f);
		rect.right += (int)(i*0.1f);
		// 128:1b82
		if (rect.top < MENU_HEIGHT) {
			rect.top = MENU_HEIGHT;
		}
		// 128:1bac
		if (this->var_i16_32 == 0) {
			_toolbox->InvertRect(rect);
		}
		// 128:1bc2
		if (this->var_i16_32 == 1) {
			_toolbox->FillRect(rect, _patterns[3]);
		}
		// 128:1be6
		if (this->var_i16_32 == 2) {
			_toolbox->FillRect(rect, _patterns[0x47]);
		}
		// 128:1c0a
		// was 1
		this->delayFromMarker(2);
	}
	this->toggleMouseCursor(true);
	_toolbox->PenNormal();
}

void FoolGame::setStateBits(uint16 bits) {
	// 128:1c2c
	_stateFlags |= bits;
}

void FoolGame::clearStateBits(uint16 bits) {
	// 128:1c4a
	if (_stateFlags & bits) {
		_stateFlags ^= bits;
	}
	// 128:1c78
}

void FoolGame::sub_128_1ef8() {
	// 128:1ef8
	_toolbox->SetPort(this->var_i32_8);
	fillRect(0, 0, MENU_HEIGHT, _windowWidth, 2);
	_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_128_1f1e() {
	// 128:1f1e
	_toolbox->InitCursor();
	this->toggleMouseCursor(false);
	this->sub_128_bde(1, 1, kSrcCopy, 1, 1, 1);
}

void FoolGame::sub_128_1f44() {
	// 128:1f44
	this->sub_128_61ec();
	_storyCurrentPage = 0;
	this->clearStateBits(0x200);
	this->sub_128_5fea();
	this->copyScreen(1, this->arr_bmp_5dfc);
	this->storyRenderPage();
	_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_128_1f76() {
	// 128:1f76
	// hold down mouse on scroll
	if (this->var_i16_7cc != 0) {
		do {
			this->var_i32_692 = _toolbox->TickCount();
			this->sub_128_20d0();
			// original was 5 ticks
			this->delayFromMarker(10);
			// new: empty the event queue, so feedback is instant
			_toolbox->FlushEvents(-1, 0);
			this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
		} while ((_event.modifiers & kModMouseButtonUp) == 0);
	} else {
		// 128:1fee
		if ((_storyCurrentChapter == 0x10) && (_puzzleCompletionStatus[0x10] < 0x64) && (_event.where.y > 0x113) && (_event.where.x < 0x8c)) {
			return;
		}
		// 128:2042
		if (this->var_i16_7d6 != 0) {
			if (_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
				this->sub_128_2078();
			}
		}
		// 128:2076
	}
}


void FoolGame::sub_128_2078() {
	// clicking the story button
	// 128:2078
	_toolbox->InvertRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	this->waitForMouseUp();
	_toolbox->InvertRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	if (_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
		this->sub_128_21c8();
	}
}

void FoolGame::sub_128_20d0() {
	// 128:20d0
	// change page on scroll
	do {
		_storyNextPage += this->var_i16_7cc;
		if (_storyNextPage < 1) {
			_storyNextPage = 1;
			return;
		}
		if (_storyNextPage > _storyPageCount) {
			_storyNextPage = _storyPageCount;
			return;
		}
	} while (_pageVisible[_storyNextPage] == 0);
	this->storyRenderPage();
}

void FoolGame::menuChapterSelect() {
	this->clearStateBits(kStateChapterSelect);
	// 128:2132
	for (int i = _selectedMenuChapter; i <= _storyPageCount; i++) {
		if (_selectedMenuChapter == _pageToChapter[i]) {
			_storyNextPage = i;
			i = _storyPageCount;
		}
		// 128:2160
	}
	// 128:2170
	_storyCurrentChapter = _pageToChapter[_storyNextPage];
	if (this->var_i16_7ce & 4) {
		this->var_i16_7ce ^= 4;
		if (_puzzleType[_storyCurrentChapter] > 0) {
			// 128:21bc
			this->sub_128_21c8();
			return;
		}
	}
	// 128:21c2
	this->storyRenderPage();
}

void FoolGame::sub_128_21c8() {
	// 128:21c8
	if ((_storyCurrentChapter == 1) && (_sunMapRestored > 0)) {
		setStateBits(0x100);
	} else {
		setStateBits(kStatePuzzleSelect);
	}
}

void FoolGame::storyRenderPage() {
	// 128:2202
	if (_storyNextPage != _storyCurrentPage) {
		if ((_pageLines[_storyNextPage] < 0xe) && (this->var_i16_7de > 0)) {
			this->fillRect(1, kPatBic, this->var_i16_7de, 0x37, 0x11d, 0x1db);
			// 128:226e
			// grab the last N-1 lines of text from the screen
			_zbasic->get(0x3c, 0x122 - (0xf - _pageLines[_storyNextPage]) * 0xf, 0x1d6, 0x11d, this->arr_bmp_b3ec);
			this->var_i16_7e0 = 1;
		} else {
			// 128:22d4
			this->var_i16_7e0 = 0;
		}
	}
	// 128:22da
	// erase lower controls on scroll
	fillRect(0x127, 0x69, 0x138, 0x190, 0);
	_storyCurrentChapter = _pageToChapter[_storyNextPage];
	// if we've solved a puzzle, fill the scroll with crazy patterns
	if ((this->var_i16_7ce & 0x8) != 0) {
		this->var_i16_7ce ^= 0x8;
		this->sub_128_bde(1, 0, kSrcCopy, 0, 1, 0);
		this->sub_128_bde(1, 0, kSrcCopy, 0, 0, 0);
	}
	// 128:235e
	if (_puzzleType[_storyCurrentChapter] > 0) {
		// set story button icon
		Common::U32String iconStr = Common::U32String("?"); // was: str(10)
		if (_puzzleCompletionStatus[_storyCurrentChapter] == 0x63) {
			iconStr = Common::U32String("~"); // wadjet eye, was: str(11)
		}
		// 128:23b8
		if (_puzzleCompletionStatus[_storyCurrentChapter] >= 0x64) {
			iconStr = Common::U32String("*"); // was: str(12)
		}
		// 128:23e8
		this->drawPuzzleButton(iconStr);
		// 128:23f0: CLR.W - -0x772(A5)
		this->var_i16_7d6 = 1;
		_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
		_toolbox->MoveTo(0x8a, 0x133);
		_toolbox->DrawString(_puzzleName[_storyCurrentChapter]);
	} else {
		// 128:2430
		this->var_i16_7d6 = 0;
		_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
		_toolbox->MoveTo(0x70, 0x133);
		_toolbox->DrawString(_puzzleName[_storyCurrentChapter]);
	}
	// 128:2468
	if (!_pageNumberText[_storyNextPage].empty()) { // was: str(13)
		_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
		_toolbox->DrawString(_pageNumberText[_storyNextPage]);
	}
	// 128:24be
	if (_storyNextPage != _storyCurrentPage) {
		_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
		fillRect(0x2f, 0x37, 0x11f, 0x1db, 0);
		if (this->var_i16_7e0 != 0) {
			// Paste the previous lines of text, if we copied them
			_zbasic->put(0x3c, 0x32, this->arr_bmp_b3ec, kSrcCopy);
		}
		// 128:252a
		// y-position where the story text should start
		int16 strY = 0x11d - (_pageLines[_storyNextPage] * 0xf);
		debugC(5, kDebugLoading, "FoolGame::storyRenderPage: storyNextPage %d, lines %d", _storyNextPage, _pageLines[_storyNextPage]);
		if (_pageLines[_storyNextPage] < 0xf) {
			_toolbox->MoveTo(0xfa, strY-0xf);
			_toolbox->DrawString(Common::U32String("∞")); // was: str(14)
			this->var_i16_7de = strY - 0x1e;
		} else {
		// 128:25a6
			this->var_i16_7de = strY - 0xf;
		}
		// 128:25b6
		// printing the story to the screen
		_toolbox->MoveTo(0x41, strY);
		for (int i = _pageLineRanges[_storyNextPage*2]; i <= _pageLineRanges[_storyNextPage*2+1]; i++) {
			_toolbox->TextFace(_pageLineFace[i]);
			_toolbox->DrawString(_zbasic->index(0, i));
			if (_pageLineBreak[i] != 0) {
				// 128:262a
				strY += 0xf;
				_toolbox->MoveTo(0x41, strY);
			}
			// 128:263a
		}
		// 128:265c
		_storyCurrentPage = _storyNextPage;
	}
	// 128:2662
}

void FoolGame::menuClickMessage() {
	// 128:2664
	// hide menu and replace with "click mouse to continue" message
	_toolbox->SetPort(this->var_i32_8);
	fillRect(0, 7, 0x13, _windowWidth-7, 0);
	_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
	Common::U32String message = Common::U32String("Click Mouse to Continue"); // was: str(15)
	int16 width = _toolbox->StringWidth(message);
	_toolbox->MoveTo((_windowWidth / 2) - (width / 2), 0xf);
	_toolbox->DrawString(message);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_128_26f6() {
	// 128:26f6
	for (int i = 0; i <= 0x28; i++) {
		this->arr_i16_1eb8[i] = 0;
	}
}

void FoolGame::fetchPuzzleData() {
	int position = _puzzleDataOffsets[_activePuzzle] - 1;
	// record index
	this->var_i16_484 = (int16)(position / 1000);
	// offset at record
	this->var_i16_7e4 = (int16)(position % 1000);
	_zbasic->record(1, this->var_i16_484, this->var_i16_7e4);
	this->_puzzleDataPtr = 0;
	int length = _puzzleDataOffsets[_activePuzzle + 1] - _puzzleDataOffsets[_activePuzzle];
	debugC(8, kDebugLoading, "fetchPuzzleData: seek puzzles to puzzle %d at position %x (rec: %d, offset: %d), length %x",
			_activePuzzle,
			position,
			this->var_i16_484,
			this->var_i16_7e4,
			length
	);
	this->arr_bytes_109dc = _zbasic->readFile(1, length);
	this->_puzzleDataBuffer = this->arr_bytes_109dc;

}

// new game
void FoolGame::menuNewGame() {
	// 128:27d6
	this->savePrompt();
	if (_savePromptChoice == 3)
		return;
	if (this->var_i16_7ce & 1) {
		setStateBits(kStateNewGame | kStateReturn);
	} else {
		setStateBits(kStateNewGame);
	}
}


void FoolGame::sub_128_2808() {
	// 128:2808
	this->var_i16_7e6 = 0;
	// 128:280e: LEA - [0x3808],A0
	// 128:2812: MOVE.L - A0,-0x8ee(A5)
	_zbasic->openR(2, this->var_str_588, 0x400, this->var_i16_688);
	this->var_str_7e8 = _zbasic->readFileStr(2, 0x11);
	if (this->var_str_7e8 == Common::U32String("The Fool's Errand")) { // str(16)
		this->var_i16_8e8 = _zbasic->readFileInt(2);
		this->var_i16_8e8 = _zbasic->readFileInt(2);
		this->var_i16_8e8 = _zbasic->readFileInt(2);
		this->var_i16_8e8 = _zbasic->readFileInt(2);
		for (int i = 1; i <= 0x64; i++) {
			this->var_i16_8e8 = _zbasic->readFileInt(2);
			this->var_i16_8e8 = _zbasic->readFileInt(2);
			this->var_i16_8e8 = _zbasic->readFileInt(2);
			this->var_i16_8e8 = _zbasic->readFileInt(2);
			this->var_str_7e8 = _zbasic->readFileStr(2, this->var_i16_8e8);
		}
		// 128:28c8
		for (int i = 1; i <= 0x51; i++) {
			this->var_i16_8e8 = _zbasic->readFileInt(2);
		}

	} else {
		// 128:28ec
		this->var_i16_7e6 = 1;
	}
	_zbasic->close(2);
	// 128:28f8: LEA - [0x3818],A0
	// 128:28fc: MOVE.L - A0,-0x8ee(A5)
	if (this->var_i16_7e6 != 0) {
		_modalText[0] = Common::U32String::format("The file '%s' cannot be opened.", this->var_str_588.encode().c_str()); // was: str(17), str(18)
		_modalText[1] = Common::U32String("Okay"); // was: str(19)
		this->showChoiceModal(kFontChicago, 0, 1, 1);
		this->var_str_588.clear(); // was: str(20)
		this->var_i16_7e6 = 0;
	}
}

void FoolGame::menuOpenGame() {
	// 128:2988
	// File -> Open
	this->sub_128_1e4(Common::String("FOOL")); // was: str(21)
	if (this->var_str_588.empty()) { // was: str(22)
		return;
	}
	this->sub_128_2808();
	if (this->var_str_588.empty()) { // was: str(23)
		return;
	}
	this->savePrompt();
	// 128:29d4
	if (_savePromptChoice != 3) {
		if ((this->var_i16_7ce & 1) != 0) {
			setStateBits(kStateOpenGame | kStateReturn);
		} else {
			// 128:29fa
			setStateBits(kStateOpenGame);
		}
		this->cursorWatch();
	}
}

void FoolGame::menuSaveGame() {
	// 128:2a06
	// File -> Save
	setStateBits(kStateSaveGame);
}

void FoolGame::saveGameAs() {
	// 128:2a0e
	if (_saveFileName.empty()) { // was: str(24)
		// 128:2a28
		this->openSaveFileDialog(Common::U32String("Name of game?"), Common::U32String()); // was: str(25), str(26)
	} else {
		// 128:2a48
		this->openSaveFileDialog(Common::U32String("New Name?"), _saveFileName); // was: str(27)
	}
	// 128:2a60
	if (!this->var_str_486.empty()) { // was: str(28)
		_saveFileName = this->var_str_486;
		this->var_i16_9ec = this->var_i16_586;
		// 128:2a90
	}
}

void FoolGame::menuSaveGameAs() {
	// 128:2a92
	// File -> Save As
	this->saveGameAs();
	if (!this->var_str_486.empty()) { // was: str(29)
		setStateBits(kStateSaveGame);
	}

}

// quit
void FoolGame::menuQuit() {
	// 128:2ab6
	this->savePrompt();
	if (_savePromptChoice == 3) {
		return;
	}
	if ((this->var_i16_7ce & 1) != 0) {
		setStateBits(kStateQuit | kStateReturn);
	} else {
		setStateBits(kStateQuit);
	}
}

// print story
void FoolGame::menuPrintStory() {
	// 128:2ae8
	if (this->var_i16_7ce & 1) {
		_stateFlags = kStatePrintStory | kStateReturn;
	} else {
		_stateFlags = kStatePrintStory;
	}
}



void FoolGame::printStory() {
	// 128:2b0a
	_stateFlags = kStateNull;
	// printing the story
	warning("STUB: %s", __func__);
}

void FoolGame::newGame() {
	// 128:2bc6
	this->clearStateBits(kStateNewGame);
	this->cursorWatch();
	this->sub_128_3744();
	_saveFileName.clear(); // was: str(33);

	this->var_i16_9ec = 0;
	// set up the initially visible story chapters
	this->arr_i16_4758[0] = 0x103;
	this->arr_i16_4758[1] = 0;
	this->arr_i16_4758[2] = 0x1e;
	this->arr_i16_4758[3] = 0x1fc;
	this->arr_i16_4758[4] = (int16)0xa050;
	this->arr_i16_4758[5] = 0x3f7;
	this->var_i16_484 = 0;
	for (int j = 0; j <= 0x5; j++) {
		// 128:2c5e
		for (int i = 0; i <= 0xf; i++) {
			this->var_i16_484++;
			if (this->arr_i16_4758[j] & _bitLUT[i]) {
				_pageVisible[this->var_i16_484] = 1;
			} else {
				// 128:2ca6
				_pageVisible[this->var_i16_484] = 0;
			}
			// 128:2cb6
		}
	}
	// 128:2cce
	for (int16 i = 1; i <= 0x51; i++) {
		_sunMapTileID[i] = i;
	}
	_zbasic->unk_20();
	// 128:2cf6
	// shuffle around the pieces of the sun's map
	for (int i = 1; i <= 0x64; i++) {
		this->var_i16_484 = _zbasic->rndInt(0x51);
		this->var_i16_7e4 = _zbasic->rndInt(0x51);
		_zbasic->swapInt(_sunMapTileID[this->var_i16_484], _sunMapTileID[this->var_i16_7e4]);
		_puzzleCompletionStatus[i] = 0;
		_puzzleFlags[i] = kFlagMenuDisabled;
	}
	// 128:2d6a
	this->var_i16_484 = 0;
	for (int i = 1; i <= _storyPageCount; i++) {
		if (_pageToChapter[i] == this->var_i16_484+1) {
			// 128:2d9c
			this->var_i16_484++;
			if (_pageVisible[i] == 1) {
				if (_puzzleUnlockChapter[this->var_i16_484] != 0) {
					this->var_i16_9f2 = kFlagMenuDiamond;
				} else {
					this->var_i16_9f2 = kFlagMenuEnabled;
				}
				// 128:2de2
				_puzzleFlags[this->var_i16_484] = this->var_i16_9f2 + 4;
			}
			// 128:2e02
		}
		// 128:2e02
	}
	// 128:2e14
	_zbasic->indexClear(0x2);
	_storyNextPage = 1;
	this->var_i16_7ce = 0;
	_activePuzzle = 0;
	_sunMapRestored = 0;
	this->var_i16_7b2 = 0;
	this->sub_128_3032();
	// 128:2e3c
}

void FoolGame::openGame() {
	// 128:2e3e
	// save game loading code?
	this->clearStateBits(kStateOpenGame);
	this->cursorWatch();
	this->sub_128_3744();
	_saveFileName = this->var_str_588;
	this->var_i16_9ec = this->var_i16_688;
	this->var_i16_7e6 = 0;
	// FIXME: disk error handler??
	// 128:2e68: LEA - [0x3808],A0
	// 128:2e6c: MOVE.L - A0,-0x8ee(A5)
	// 128:2e70: SF - 0x8,D0
	_zbasic->openR(2, _saveFileName, 0x400, this->var_i16_9ec);

	Common::String magic = _zbasic->readFileStr(2, 0x11);
	if (!magic.equals("The Fool's Errand")) {
		error("FoolGame::openGame: invalid save file");
		return;
	}
	_storyNextPage = _zbasic->readFileInt(2);
	_activePuzzle = _zbasic->readFileInt(2);
	this->var_i16_7ce = _zbasic->readFileInt(2);
	_sunMapRestored = _zbasic->readFileInt(2);
	for (int i = 1; i <= 0x64; i++) {
		_puzzleFlags[i] = _zbasic->readFileInt(2);
		_puzzleCompletionStatus[i] = _zbasic->readFileInt(2);
		_pageVisible[i] = _zbasic->readFileInt(2);
		int16 payloadSize = _zbasic->readFileInt(2);
		debugC(5, kDebugLoading, "openGame: puzzle %d:, puzzleFlags: %d, puzzleCompletionStatus: %d, pageVisible: %d, payload size: %d", i, _puzzleFlags[i], _puzzleCompletionStatus[i], _pageVisible[i], payloadSize);
		Common::String state = _zbasic->readFileStr(2, payloadSize);
		if (debugChannelSet(5, kDebugLoading)) {
			Common::hexdump((const byte *)state.c_str(), payloadSize);
		}
		_zbasic->indexRawSet(state, 2, i);
	}
	// 128:2f54
	debugCN(5, kDebugLoading, "openGame: sun map tile IDs: ");
	for (int i = 1; i <= 0x51; i++) {
		_sunMapTileID[i] = _zbasic->readFileInt(2);
		debugCN(5, kDebugLoading, "%d, ", _sunMapTileID[i]);
	}
	debugCN(5, kDebugLoading, "\n");
	// 128:2f84
	_zbasic->close(2);
	if (this->var_i16_7e6 != 0) {
		_modalText[0] = Common::U32String::format("The file '%s' cannot be opened.", _saveFileName.encode().c_str()); // was: str(34), str(35)
		_modalText[1] = Common::U32String("Okay"); // was: str(36)
		this->var_i16_7e6 = 0;
		if ((this->var_i16_7ce & 1) == 0) {
			setStateBits(kStateNewGame | kStateReturn);
		} else {
			setStateBits(kStateNewGame);
		}
	} else {
		// 128:302c
		this->sub_128_3032();
	}

}

void FoolGame::sub_128_3032() {
	_storyCurrentChapter = 0;
	_stateFlags = kStateNull;
	this->var_i16_484 = 0;
	this->var_i16_68c = 3;
	// render chapter menu headings
	for (int j = 3; j <= 7; j++) {
		this->var_str_384 = Common::U32String("-") + _zbasic->chr(0x7d+j) + Common::U32String("-"); // was: str(37)
		_zbasic->menu(j, 0, 1, this->var_str_384);
		for (int i = 1; i <= 0x10; i++) {
			this->var_i16_484++;
			this->var_i16_9f2 = _puzzleFlags[this->var_i16_484] & (kFlagMenuDiamond | kFlagMenuEnabled);
			if (_puzzleCompletionStatus[this->var_i16_484] == 0x63) {
				this->var_str_384 = Common::U32String(" ~ "); // wadjet eye for puzzles with enchantment
			} else {
			// 128:30e4
				this->var_str_384 = Common::U32String(" "); // was: str(40)
			}
			// 128:30f8
			this->var_str_9f4 = _puzzleName[this->var_i16_484] + this->var_str_384;
			if ((j == 3) && (i == 1)) { // add shortcut modifier for the sun's map
				this->var_str_9f4 = Common::U32String("/M") + this->var_str_9f4;
			}
			// 128:315e
			_zbasic->menu(j, i, this->var_i16_9f2, this->var_str_9f4);
		}
		// 128:3180
	}
	// 128:318e
	fillRect(0x1e, 0x37, 0x32, 0xc8, 0);
	fillRect(0x2f, 0x37, 0x11f, 0x1db, 0x0);
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
	int16 strY = 0x3c;
	this->var_i16_7de = 0x2d;
	// draw the first page of text
	for (int i = 1; i <= 0xf; i++) {
		// 128:31ec
		_toolbox->MoveTo(0x41, strY);
		_toolbox->DrawString(_zbasic->index(0, i));
		strY += 0xf;
	}
	// 128:3228
	this->copyScreen(0, this->arr_bmp_5dfc);
	_storyCurrentPage = 1;
	this->storyRenderPage();
	if ((this->var_i16_7ce & 1) == 0) {
		this->cursorExplodingWatch();
	} else {
		if ((_activePuzzle > 0) && (_activePuzzle <= 0x50)) {
			setStateBits(kStatePuzzleSelect);
		} else {
			setStateBits(0x100);
		}
	}
	// 128:3294
	this->menuToggleSound();
	if (_sunMapRestored == 0) {
		_zbasic->menu(2, 7, 0, Common::U32String());
	} else {
		_zbasic->menu(2, 7, 1, Common::U32String());
	}
}

void FoolGame::menuToggleSound() {
	if (!_soundEnabled) {
		_zbasic->menu(2, 6, 1, Common::U32String());
	} else {
		_zbasic->menu(2, 6, 0x86, Common::U32String());
	}
}

void FoolGame::savePrompt() {
	// 128:32fa
	if (this->var_i16_7ce == 666) {
		return;
	}
	_savePromptChoice = 0;
	// The original code would only show the save prompt if there was
	// no game in progress.
	// Instead, we always show the prompt.

	Common::U32String previous = _saveFileName;
	_saveFileName.clear();
	// v1.1 uses a full-screen modal without a cancel button, so we hardcode the strings from v2.0 here.
	// was: str(43)
	_modalText[0] = Common::U32String("Do you wish to save this game?");
	_modalText[1] = Common::U32String("Yes");
	_modalText[2] = Common::U32String("No");
	_modalText[3] = Common::U32String("Cancel");
	this->showChoiceModal(kFontChicago, 0, 3, 0);
	if (_savePromptChoice > 1) {
		_saveFileName = previous;
		return;
	}
	this->saveGameAs();
	if (_saveFileName.empty()) { // was: str(47)
		// 128:33da
		_savePromptChoice = 3;
		_saveFileName = previous;
		return;
	}
	// 128:33e2
	setStateBits(kStateSaveGame);

}

void FoolGame::autoSaveGame() {
	// Fool's Errand had an autosave feature; once you saved or opened
	// a game, this became the "game in progress", and on quit/load
	// the game would do the equivalent of hitting "Save".
	// In general, ScummVM treats user-initated save games as a snapshot,
	// and writes any automatic progress to a dedicated "Autosave" slot.
	// This wrapper changes the autosave behaviour to match the rest of ScummVM.
	_isAutoSaving = true;
	Common::U32String previous = _saveFileName;
	_saveFileName = Common::U32String::format("%s-Autosave", g_engine->getGameId().c_str());
	saveGame();
	_saveFileName = previous;
	_isAutoSaving = false;
}

void FoolGame::saveGame() {
	// write save file
	this->clearStateBits(kStateSaveGame);
	if (this->var_i16_7ce == 666)
		return;
	// 128:3548
	if (_saveFileName.empty()) { // was: str(57)
		this->saveGameAs();
		if (_saveFileName.empty()) { // was: str(58)
			return;
		}
	}
	// 128:3580
	if ((this->var_i16_7ce & 1) != 0) {
		this->puzzleSaveContext();
	}
	// 128:3594
	do {
		if (!_isAutoSaving) {
			this->cursorWatch();
		}
		// 128:3598
		this->var_i16_7e6 = 0;
		// 128:359e: LEA - [0x3808],A0
		// 128:35a2: MOVE.L - A0,-0x8ee(A5)
		_zbasic->defOpen(Common::U32String("FOOLgf87")); // str(59)
		_zbasic->openW(2, _saveFileName, 0x400, this->var_i16_9ec);

		if (this->var_i16_7e6 != 0xa) {
			this->var_str_af4 = Common::U32String("The Fool's Errand"); // str(60)
			_zbasic->writeFileStr(2, this->var_str_af4.encode(Common::kMacRoman));
			_zbasic->writeFileInt(2, _storyNextPage);
			_zbasic->writeFileInt(2, _activePuzzle);
			_zbasic->writeFileInt(2, this->var_i16_7ce);
			_zbasic->writeFileInt(2, _sunMapRestored);
			for (int i = 1; i <= 0x64; i++) {
				_zbasic->writeFileInt(2, _puzzleFlags[i]);
				_zbasic->writeFileInt(2, _puzzleCompletionStatus[i]);
				_zbasic->writeFileInt(2, _pageVisible[i]);
				Common::String state = _zbasic->indexRaw(2, i);
				_zbasic->writeFileInt(2, (int16)state.size());
				_zbasic->writeFileStr(2, state);
			}
			// 128:36ba
			for (int i = 1; i <= 0x51; i++) {
				_zbasic->writeFileInt(2, _sunMapTileID[i]);
			}
			_zbasic->close(2);
		}
		// 128:36fe
		if (this->var_i16_7e6 != 0) {
			// problem with saving
			this->sub_128_388a();
			do {
				this->saveGameAs();
			} while (_saveFileName.empty());  // was: str(61)
			// 128:3726: BRA - [0x3594]

		} else {
			break;
		}
	} while (true);
	// 128:372a
	// 128:372a: LEA - [0x3818],A0
	// 128:372e: MOVE.L - A0,-0x8ee(A5)
	if (_stateFlags == kStateNull) {
		if (!_isAutoSaving) {
			this->cursorExplodingWatchShort();
		}
		_toolbox->InitCursor();
	}
}

void FoolGame::sub_128_3744() {
	// 128:3744
	for (int i = 3; i <= 7; i++) {
		MenuHandle menu = _toolbox->GetMHandle(i);
		_toolbox->DeleteMenu(i);
		_toolbox->DisposeMenu(menu);
	}
	_toolbox->DrawMenuBar();
}

// watch cursor
void FoolGame::cursorWatch() {
	// 128:3774
	_toolbox->SetCursor(_cursors[3]);
	this->var_i16_7b2 = 0xa;
}

void FoolGame::cursorExplodingWatch() {
	// 128:378a
	for (int j = 0; j <= 1; j++) {
		for (int i = 3; i <= 6; i++) {
			_toolbox->SetCursor(_cursors[i]);
			this->delay(3);
		}
	// 128:37c2
	}
	for (int i = 7; i <= 0xf; i++) {
		_toolbox->SetCursor(_cursors[i]);
		this->delay(4);
	}

	this->var_i16_7b2 = 0xa;
}

void FoolGame::cursorExplodingWatchShort() {
	// 128:37ce
	for (int i = 7; i <= 0xf; i++) {
		_toolbox->SetCursor(_cursors[i]);
		this->delay(4);
	}
	this->var_i16_7b2 = 0xa;
}

void FoolGame::sub_128_388a() {
	// 128:388a
	if ((this->var_i16_7e6 & 0xff) == 2) {
		// problem with saving
	}
	warning("STUB: %s", __func__);
}

void FoolGame::puzzleRun() {
	// 128:39a0
	fillRect(0x127, 0x69, 0x138, 0x190, 0);
	_stateFlags = kStateNull;
	this->var_i16_c00 = 0;
	this->var_i16_7d6 = 0;
	this->var_i16_7b2 = 0xa;
	_activePuzzle = _storyCurrentChapter;
	this->var_i16_7ce |= 1;
	_toolbox->InitCursor();
	this->copyScreen(0, this->arr_bmp_5dfc);
	// 128:39fa
	if (this->arr_i16_15e8[_storyCurrentChapter] > 0) {
		this->sub_128_41d8();
	}
	// 128:3a12
	this->puzzleSetupMenu();
	this->sub_128_26f6();
	this->sub_128_61ec();
	this->puzzleLoadContext();
	// 128:3a22
	// 128:3a38: JSR - "ZBASIC_115"
	debugC(5, kDebugLoading, "puzzleRun: chapter 0x%x, puzzle module %d", _storyCurrentChapter, _puzzleType[_storyCurrentChapter]-1);
	switch (_puzzleType[_storyCurrentChapter]-1) {
	case 0:
	case 1:
	case 2:
	case 3:
		this->jumbleRun();
		break;
	case 4:
		this->wordSearchRun();
		break;
	case 5:
		this->jigsawRun();
		break;
	case 6:
		this->polyominoRun();
		break;
	case 7:
		this->revealRun();
		break;
	case 8:
		this->sentenceRun();
		break;
	case 9:
		this->mazeRun();
		break;
	case 10:
		this->jumbleRun();
		break;
	case 11:
		this->sunMapRun();
		break;
	case 12:
		this->cardsRun();
		break;
	case 13:
		this->highPriestessRun();
		break;
	case 14:
		this->deathRun();
		break;
	case 15:
		this->shipsRun();
		break;
	case 16:
		this->humbugRun();
		break;
	case 17:
		this->justiceRun();
		break;
	case 18:
		this->hermitRun();
		break;
	default:
		warning("%s: broke out of switch", __func__);
		break;
	}
	// 128:3a8a
	this->sub_128_5fea();
	if (_sunMapRestored == 1) {
		setStateBits(0x100);
		return;
	}
	// 128:3aa4
	this->puzzleSaveContext();
	if ((_stateFlags & kStateQuit) == 0) {
		_toolbox->PenNormal();
		this->copyScreen(1, this->arr_bmp_5dfc);
		MenuHandle menu = _toolbox->GetMHandle(8);
		_toolbox->DeleteMenu(8);
		_toolbox->DisposeMenu(menu);
		_toolbox->DrawMenuBar();
	}
	// 128:3aea
	_zbasic->indexClear(1);
	if (_puzzleCompletionStatus[_storyCurrentChapter] == 0x63) {
		this->storyUnlockChapter();
		autoSaveGame(); // autosave on progress
	}
	if (_puzzleCompletionStatus[_storyCurrentChapter] == 0x64) {
		// bodge for completing The Chariot
		if (_storyCurrentChapter == 0x41) {
			if ((_puzzleFlags[0x41] & kFlagMenuDiamond) != 0) {
				_puzzleFlags[0x41] ^= kFlagMenuDiamond;
				_puzzleFlags[0x41] |= kFlagMenuEnabled;
			}
			// 128:3b9e
			for (int i = 1; i <= 5; i++) {
				_zbasic->menu(7, i, 1, Common::U32String());
				if (i > 1) {
					_puzzleFlags[0x40 + i] |= 0x5;
				}
			}
			// 128:3c04
			_storyNextPage = 0;
			// enable the four King Of ... chapters
			for (int i = 0x42; i <= 0x45; i++) {
				for (int j = 1; j <= _storyPageCount; j++) {
					if (_pageToChapter[j] == i) {
						_pageVisible[j] = 1;
						if ((i == 0x42) && (_storyNextPage == 0)) {
							_storyNextPage = j;
						}
					}
					// 128:3c6c
					if (_pageToChapter[j] > i) {
						j = _storyPageCount;
					}
					// 128:3c8e
				}
			}
			// 128:3cae
			this->var_i16_7ce |= 8;
		} else {
			// 128:3cc0
			if ((_puzzleFlags[_storyCurrentChapter] & 2) != 0) {
				_puzzleFlags[_storyCurrentChapter] ^= 2;
				_puzzleFlags[_storyCurrentChapter] |= 1;
			}
			// 128:3d3a
			this->var_i16_484 = 3 + ((_storyCurrentChapter - 1) / 0x10);
			this->var_i16_7e4 = 1 + ((_storyCurrentChapter - 1) % 0x10);
			_zbasic->menu(this->var_i16_484, this->var_i16_7e4, 1, Common::U32String());
			this->storyUnlockChapter();
		}
		autoSaveGame();
	}
	// 128:3d82
	this->var_i16_c02 = 0;
	this->var_i16_c00 = 0;
	_toolbox->SetPort(this->var_i32_0);
	if ((_stateFlags & (kStateQuit | kStateOpenGame | kStateNewGame)) == 0) {
		if ((this->var_i16_7ce & 1) == 0) {
			this->var_i16_7ce ^= 1;
		}
		if ((_stateFlags & kStateChapterSelect) == 0) {
			this->storyRenderPage();
		}
	}
	// 128:3de0
	this->sub_128_61ec();
	// 128:3de4
}

void FoolGame::storyUnlockChapter() {
	// 128:3de6
	if ((_puzzleUnlockChapter[_storyCurrentChapter] > 0) &&
		((_puzzleFlags[_puzzleUnlockChapter[_storyCurrentChapter]] & 3) == 0)) {
		_storyNextPage = 0;
		this->var_i16_7ce |= 8;
		for (int i = 1; i <= _storyPageCount; i++ ) {
			// 128:3e56
			if (_pageToChapter[i] == _puzzleUnlockChapter[_storyCurrentChapter]) {
				_pageVisible[i] = 1;
				if (_storyNextPage == 0) {
					_storyNextPage = i;
				}
			}
			// 128:3ea6
			if (_pageToChapter[i] > _puzzleUnlockChapter[_storyCurrentChapter]) {
				i = _storyPageCount;
			}
			// 128:3eda
		}
		// 128:3eec
		_storyCurrentChapter = _pageToChapter[_storyNextPage];
		int16 menuNo = 3 + ((_storyCurrentChapter - 1) / 0x10);
		int16 itemNo = 1 + ((_storyCurrentChapter - 1) % 0x10);
		if (_puzzleUnlockChapter[_storyCurrentChapter] == 0) {
			this->var_i16_9f2 = 1;
			_puzzleCompletionStatus[_storyCurrentChapter] = 0x64;
		} else {
		// 128:3f66
			this->var_i16_9f2 = 2;
		}
		_puzzleFlags[_storyCurrentChapter] |= (this->var_i16_9f2 + 4);
		_zbasic->menu(menuNo, itemNo, this->var_i16_9f2, Common::U32String());
	}
	// 128:3fb4
}

void FoolGame::puzzleSetupMenu() {
	// 128:3fb6
	// new: delete existing menu 8
	MenuHandle menu = _toolbox->GetMHandle(8);
	_toolbox->DeleteMenu(8);
	_toolbox->DisposeMenu(menu);

	_zbasic->menu(8, 0, 1, _puzzleName[_activePuzzle]);
	Common::U32String returnMsg = Common::U32String("/RReturn to Scroll  "); // was: str(72)
	if ((_activePuzzle == 0x34) || (_activePuzzle == 0x35)) {
		returnMsg = Common::U32String("/RRun for your life@  "); // was: str(73)
	}
	// 128:4024
	if (_activePuzzle > 0x50) {
		returnMsg = Common::U32String("/RReturn to Map  "); // was: str(74)
	}
	_zbasic->menu(8, 1, 1, returnMsg);
	// 128:4056
	if (_puzzleMenuInstructions[_puzzleType[_activePuzzle]*2] <= _puzzleMenuInstructions[_puzzleType[_activePuzzle]*2 + 1]) {
		_zbasic->menu(8, 2, 0, Common::U32String("-")); // was: str(75)
		// 128:40c6
		int16 itemNo = 2;
		for (int i = _puzzleMenuInstructions[_puzzleType[_activePuzzle]*2]; i <= _puzzleMenuInstructions[_puzzleType[_activePuzzle]*2 + 1]; i++) {
			// 128:40f6
			itemNo++;
			Common::U32String instructMsg = _zbasic->index(0, i) + Common::U32String("  "); // was: str(76)

			// New: Override instructions for card game.
			// v3 changed the score from 700 points to 666 points, but didn't change the puzzle data
			// file (where the menu entries come from). V3 applies a bodge with a program string.
			if ((_activePuzzle == 7) &&(i == _puzzleMenuInstructions[_puzzleType[_activePuzzle]*2]+7)) {
				instructMsg = Common::U32String("• The first player to earn over 666 points wins the game.  ");
			}

			_zbasic->menu(8, itemNo, 1, instructMsg);
		}
	}
	// 128:4166
}

void FoolGame::puzzleLoadContext() {
	// 128:4168
	_activePuzzleStatus = _puzzleCompletionStatus[_activePuzzle];
	if (_activePuzzleStatus == 0x64) {
		_activePuzzleStatus = 0x65;
	}
	_activePuzzleBuffer = _zbasic->indexRaw(2, _activePuzzle);
	if (debugChannelSet(5, kDebugLoading)) {
		Common::String inter = _activePuzzleBuffer;
		debugC(5, kDebugLoading, "puzzleLoadContext: loading puzzle %d context, state %d, size %d", _activePuzzle, _activePuzzleStatus, inter.size());
		Common::hexdump((const byte *)inter.c_str(), inter.size());
	}
}

void FoolGame::puzzleSaveContext() {
	// 128:41aa
	_puzzleCompletionStatus[_activePuzzle] = _activePuzzleStatus;
	_zbasic->indexRawSet(_activePuzzleBuffer, 2, _activePuzzle);
	if (debugChannelSet(5, kDebugLoading)) {
		Common::String inter = _activePuzzleBuffer;
		debugC(5, kDebugLoading, "puzzleSaveContext: saving puzzle %d context, state %d, size %d", _activePuzzle, _activePuzzleStatus, inter.size());
		Common::hexdump((const byte *)inter.c_str(), inter.size());
	}
}

void FoolGame::sub_128_41d8() {
	// 128:41d8
	for (int i = 0; i <= 5; i++) {
		this->arr_i16_4758[i] = this->arr_i16_16b2[i + (this->arr_i16_15e8[_activePuzzle]*8)];
	}
	// 128:422e
	if ((_activePuzzle == 0x34) && (_puzzleCompletionStatus[0x34] > 1)) {
		// 128:4262
		if (_puzzleCompletionStatus[0x34] == 2) {
			this->arr_i16_4758[0] = 0;
			this->arr_i16_4758[1] = 2;
			this->arr_i16_4758[2] = 0xa;
		}
		// 128:42aa
		if (_puzzleCompletionStatus[0x34] == 3) {
			this->arr_i16_4758[0] = 0;
			this->arr_i16_4758[1] = 3;
			this->arr_i16_4758[2] = 0;
		}
		// 128:42f0
		if (_puzzleCompletionStatus[0x34] >= 4) {
			if (_sunMapRestored == 0) {
				this->arr_i16_4758[0] = 0;
				this->arr_i16_4758[1] = 0x47;
				this->arr_i16_4758[2] = 0;
			} else {
				// 128:4346
				return;
			}
		}
		// 128:4348
	}
	// 128:4348
	if ((_activePuzzle == 0x17) && (_puzzleCompletionStatus[0x17] == 0x63)) {
		this->arr_i16_4758[0] = 0;
		this->arr_i16_4758[1] = 2;
		this->arr_i16_4758[1] = 0xa;
	}
	// 128:43aa
	if ((_activePuzzle == 0x3f) && (_puzzleCompletionStatus[0x3f] == 0x63)) {
		this->arr_i16_4758[0] = 0;
		this->arr_i16_4758[1] = 3;
		this->arr_i16_4758[2] = 0;
	}
	// 128:440a
	if (_activePuzzle == 0x48) {
		if (_puzzleCompletionStatus[0x48] == 0x63) {
			this->arr_i16_4758[0] = 0;
			this->arr_i16_4758[1] = 0x47;
			this->arr_i16_4758[2] = 0;
		} else {
			// 128:4462
			this->arr_i16_4758[1] = 0x31;
		}
	}
	storyDrawZoom();
}

void FoolGame::storyDrawZoom() {
	// 128:4472
	if (this->arr_i16_4758[1] == 0) {
		this->arr_i16_4758[1] = _activePuzzle;
	}
	if (this->arr_i16_4758[2] == 0) {
		this->arr_i16_4758[2] = 0x8;
	}
	if (this->arr_i16_4758[0] == 0) {
		// user clicked the puzzle button, zoom-fill with pattern
		zoomRect(0x130, 0x76, 0x130, 0x76, MENU_HEIGHT, 0, SCREEN_HEIGHT, SCREEN_WIDTH, this->arr_i16_4758[1], (PatternMode)this->arr_i16_4758[2], 0x19);
		return;
	}
	// 128:4520
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[this->arr_i16_4758[1]]);
	_toolbox->PenMode((PatternMode)this->arr_i16_4758[2]);
	if (this->arr_i16_4758[5] == 0) {
		// 128:456a
		this->arr_i16_4758[12] = 0x1d;
		this->arr_i16_4758[6] = 0x4;
		this->arr_i16_4758[7] = 0x7;
	}
	// 128:459a
	if (this->arr_i16_4758[5] == 1) {
		this->arr_i16_4758[12] = 0x2a;
		this->arr_i16_4758[6] = 0x4;
		this->arr_i16_4758[7] = 0x6;
	}
	// 128:45e4
	_toolbox->PenSize(this->arr_i16_4758[7], this->arr_i16_4758[6]);
	if ((this->arr_i16_4758[4] == 0) && (this->arr_i16_4758[5] == 0)) {
		this->arr_i16_4758[8] = 0xa7;
		this->arr_i16_4758[9] = 0x109;
		this->arr_i16_4758[10] = 0xa7;
		this->arr_i16_4758[11] = 0x109;
	}
	// 128:467c
	if ((this->arr_i16_4758[4] == 0) && (this->arr_i16_4758[5] == 1)) {
		this->arr_i16_4758[8] = 0xb5;
		this->arr_i16_4758[9] = 0x100;
		this->arr_i16_4758[10] = 0xb5;
		this->arr_i16_4758[11] = 0x100;
	}
	// 128:46f6
	if (this->arr_i16_4758[4] == 1) {
		this->arr_i16_4758[6] *= -1;
		this->arr_i16_4758[7] *= -1;
	}
	// 128:4754
	if ((this->arr_i16_4758[4] == 1) && (this->arr_i16_4758[5] == 0)) {
		this->arr_i16_4758[8] = 0x2b;
		this->arr_i16_4758[9] = 0x31;
		this->arr_i16_4758[10] = 0x123;
		this->arr_i16_4758[11] = 0x1e1;
	}
	// 128:47ce
	if ((this->arr_i16_4758[4] == 1) && (this->arr_i16_4758[5] == 1)) {
		this->arr_i16_4758[8] = 0x10;
		this->arr_i16_4758[9] = -0x7;
		this->arr_i16_4758[10] = 0x15a;
		this->arr_i16_4758[11] = 0x207;
	}
	// 128:4850
	if ((this->arr_i16_15e8[_activePuzzle] == 0xd) || (this->arr_i16_15e8[_activePuzzle] == 0xe)) {
		fillRect(0x14f, 0, SCREEN_HEIGHT, 0x7, 1);
		fillRect(0x14f, 0x1f9, SCREEN_HEIGHT, SCREEN_WIDTH, 1);
	}
	// 128:48d6
	for (int i = 0; i <= this->arr_i16_4758[12]; i++) {
		this->arr_i16_4758[8] -= this->arr_i16_4758[6];
		this->arr_i16_4758[9] -= this->arr_i16_4758[7];
		this->arr_i16_4758[10] += this->arr_i16_4758[6];
		this->arr_i16_4758[11] += this->arr_i16_4758[7];
		if (this->arr_i16_4758[8] < 0x14) {
			this->arr_i16_4758[8] = 0x14;
		}
		// 128:49e2
		if (this->arr_i16_4758[3] == 0) {
			this->arr_i16_4758[1] += 1;
			if (this->arr_i16_4758[1] > 0x50) {
				this->arr_i16_4758[1] = 1;
			}
			// 128:4a42
			_toolbox->PenPat(_patterns[this->arr_i16_4758[1]]);
		}
		// 128:4a64
		// FIXME: These can sometimes be invalid.
		// Create manually to avoid the assert crash in the constructor.
		Common::Rect temp;
		temp.left = this->arr_i16_4758[9];
		temp.top = this->arr_i16_4758[8];
		temp.right = this->arr_i16_4758[11];
		temp.bottom = this->arr_i16_4758[10];
		_toolbox->FrameRect(temp);
		// add fake delay for drawing visibility
		if ((i % 4) == 0) {
			_toolbox->Delay(0);
		}
	}
	// 128:4a8e
	_toolbox->PenNormal();
}

// about screen
void FoolGame::menuAbout() {
	int aboutOff = _zstrOffset[kOffsetAbout];

	// 128:4a92
	if (_saveFileName.empty()) { // was: str(77)
		// untitled game in progress
		_modalText[0] = _zbasic->chr(0x22) + _zbasic->str(aboutOff+0) + _zbasic->chr(0x22) + Common::U32String(" in progress");
	} else {
		// 128:4aee
		_modalText[0] = _zbasic->chr(0x22) + _saveFileName + _zbasic->chr(0x22) + Common::U32String(" in progress");
	}
	// 128:4b28
	int16 modalLines = -1;
	_modalText[1] = _zbasic->str(aboutOff+3);
	if (_sunMapRestored == 0) {

		int16 mapPiecesRemaining = 0x51;
		for (int i = 1; i <= 0x51; i++) {
			if ((_puzzleFlags[i] & (kFlagMenuEnabled | kFlagMenuDiamond)) != 0) {
				mapPiecesRemaining--;
			}
		}
		// 128:4b88
		if (mapPiecesRemaining >= 0) {
			// sun's map is incomplete
			_modalText[2] = _zbasic->str(aboutOff+4);
			if (mapPiecesRemaining == 1) {
				// there is 1 missing piece
				_modalText[3] = _zbasic->str(aboutOff+5);
			} else {
				// 128:4bde
				// there are N missing pieces
				_modalText[3] = Common::U32String::format("%s %d%s", _zbasic->str(aboutOff+6).encode().c_str(), mapPiecesRemaining, _zbasic->str(aboutOff+7).encode().c_str());
			}
			// 128:4c1a
			int16 unsolvedPuzzles = 0;
			for (int i = 2; i <= 0x50; i++) {
				if ((_puzzleCompletionStatus[i] < 0x63) && (_puzzleType[i] > 0)) {
					unsolvedPuzzles++;
				}
			}
			// 128:4c74
			_modalText[4] = _zbasic->str(aboutOff+8); // ' '
			if (unsolvedPuzzles == 1) {
				// There is 1 unsolved puzzle.
				_modalText[5] = _zbasic->str(aboutOff+9);
			} else {
				// 128:4cc0
				// There are X unsolved puzzles.
				_modalText[5] = Common::U32String::format("%s %d%s", _zbasic->str(aboutOff+10).encode().c_str(), unsolvedPuzzles, _zbasic->str(aboutOff+11).encode().c_str());
			}
			// 128:4cfc
			modalLines = 5;

		} else {
			// 128:4d06
			// You have all the map pieces, but they are not arranged in the correct order.
			_modalText[2] = _zbasic->str(aboutOff+12);
			_modalText[3] = _zbasic->str(aboutOff+13);
			_modalText[4] = _zbasic->str(aboutOff+14);
			_modalText[5] = _zbasic->str(aboutOff+15);
			// Only the Book of Thoth remains unsolved.
			_modalText[6] = _zbasic->str(aboutOff+16);
			_modalText[7] = _zbasic->str(aboutOff+17);
			modalLines = 7;
		}
		// 128:4dc0
	} else {
		// 128:4dc4
		// The sun's map is restored.
		_modalText[2] = _zbasic->str(aboutOff+18);
		_modalText[3] = _zbasic->str(aboutOff+19);
		// Only the Book of Thoth remains unsolved.
		_modalText[4] = _zbasic->str(aboutOff+20);
		_modalText[5] = _zbasic->str(aboutOff+21);
		modalLines = 5;
	}
	// 128:4e42
	_toolbox->SetPort(this->var_i32_4);
	this->var_i16_7b2 = 0xa;
	_toolbox->InitCursor();
	this->toggleMouseCursor(true);
	this->copyScreen(0, this->arr_bmp_138bc);
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_i16_7b4 = 0;
	this->var_i16_7b6 = 0;
	this->var_i16_7b8 = 0;
	for (int i = 0; i <= modalLines; i++) {
		this->var_i16_7ba = _toolbox->StringWidth(_modalText[i]);
		if (this->var_i16_7ba > this->var_i16_7b4) {
			this->var_i16_7b4 = this->var_i16_7ba;
		}
		// 128:4ec4
		this->var_i16_7b6 += 0x11;
	}
	// 128:4eda
	this->var_i16_7b6 += 0xd;
	this->var_i16_7b4 = (this->var_i16_7b4/2) + 0xf;
	this->var_i16_7b6 = (this->var_i16_7b6/2);
	Common::Rect bounds; // arr_rect_5b7c
	_toolbox->SetRect(
		bounds,
		0xf5 - this->var_i16_7b4,
		0x6e - this->var_i16_7b6,
		0x10b + this->var_i16_7b4,
		0xcf + this->var_i16_7b6
	);
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[0]);
	_toolbox->FrameRect(bounds);
	// 128:4f6a
	_toolbox->InsetRect(bounds, 1, 1);
	_toolbox->PenSize(5, 5);
	_toolbox->PenPat(_patterns[2]);
	_toolbox->FrameRect(bounds);
	_toolbox->InsetRect(bounds, 5, 5);
	_toolbox->PenSize(5, 5);
	_toolbox->PenPat(_patterns[1]);
	_toolbox->FrameRect(bounds);
	// 128:4fe2
	_toolbox->InsetRect(bounds, 5, 5);
	_toolbox->FillRect(bounds, _patterns[0]);
	_toolbox->PenNormal();
	// 128:5014
	_toolbox->SetRect(
		bounds,
		0xb5,
		0x82 - this->var_i16_7b6,
		0xd5,
		0x82 - this->var_i16_7b6 + 0x20
	);
	_toolbox->PlotIcon(bounds, this->var_i32_c);
	// These strings don't appear in the v1.1 about box
	_toolbox->MoveTo(0xde, 0x8c - this->var_i16_7b6);
	_toolbox->DrawString(Common::U32String("the fool's errand"));  // was: str(100)
	_toolbox->MoveTo(0xee, 0x9b - this->var_i16_7b6);
	_toolbox->DrawString(Common::U32String("by Cliff Johnson")); // was: str(101)

	// 128:509a
	int16 strY = 0xbe - this->var_i16_7b6;
	for (int i = 0; i <= modalLines; i++) {
		this->drawTextCenter(_modalText[i], strY);
		strY += 0x11;
	}
	// 128:50f4
	_zbasic->text(kFontSmall, 0x9, Graphics::kMacFontRegular, kSrcOr);
	strY += 0xe;
	this->drawTextCenter(_zbasic->str(_zstrOffset[kOffsetVersion]), strY); // version string
	this->menuClickMessage();
	this->waitForClick();
	_toolbox->DrawMenuBar();
	this->copyScreen(0x1, this->arr_bmp_138bc);
	_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::menuPrologue() {
	// this is brand new
	this->copyScreen(0, this->arr_bmp_138bc);
	FoolPrologue fp(_version, _zstrOffset);
	fp.run(false, this->arr_bmp_138bc);
	this->copyScreen(1, this->arr_bmp_138bc);
	_toolbox->DrawMenuBar();
}

void FoolGame::menuFinale() {
	// this is brand new
	this->copyScreen(0, this->arr_bmp_138bc);
	FoolPrologue fp(_version, _zstrOffset);
	fp.run(true, this->arr_bmp_138bc);
	this->copyScreen(1, this->arr_bmp_138bc);
	_toolbox->DrawMenuBar();
}

void FoolGame::onClickMenu() {
	// 128:5b30
	if (_menuDisabled)
		return;
	_event.what = kNullEvent;
	if (_menuHidesPlayfield) {
		this->thothHidePlayfield();
	}
	uint32 menuData = _toolbox->MenuSelect(_event.where);
	_selectedMenuID = (uint16)(menuData >> 16);
	_selectedMenuItem = (uint16)(menuData & 0xffff);
	if (_selectedMenuID > 0) {
		// 128:5b8c
		this->onMenuSelect();
		_toolbox->HiliteMenu(0);
	}
	// 128:5b94
	if (_menuHidesPlayfield) {
		this->thothShowPlayfield();
	}
	this->sub_128_61ec();
}

void FoolGame::onMenuKey() {
	if (_menuDisabled)
		return;
	uint32 menuData = _toolbox->MenuKey((char)(_event.message & 0xff));
	_toolbox->Delay(0);
	_selectedMenuID = (uint16)(menuData >> 16);
	_selectedMenuItem = (uint16)(menuData & 0xffff);
	if (_selectedMenuID > 0) {
		if (_menuHidesPlayfield) {
			this->thothHidePlayfield();
		}
		this->onMenuSelect();
		if (_menuHidesPlayfield) {
			this->thothShowPlayfield();
		}
		_toolbox->HiliteMenu(0);
	}
	// 128:5c1a
	this->sub_128_61ec();
}

void FoolGame::onMenuSelect() {
	_stateFlags = kStateNull;
	if (_selectedMenuID == 1) { // Eye menu
		if (_selectedMenuItem == 2) {
			this->menuAbout();
		} else if (_selectedMenuItem == 3) {
			this->menuPrologue();
		} else if (_selectedMenuItem == 4) {
			this->menuFinale();
		}
	}
	if (_selectedMenuID == 2) { // File menu
		if (_selectedMenuItem == 1) {
			this->menuNewGame();
		} else if (_selectedMenuItem == 2) {
		// 128:5c5c
			this->menuOpenGame();
		} else if (_selectedMenuItem == 3) {
		// 128:5c6c
			this->menuSaveGame();
		} else if (_selectedMenuItem == 4) {
			this->menuSaveGameAs();
		} else if (_selectedMenuItem == 6) {
			_soundEnabled = !_soundEnabled;
			// 128:5cbc
			this->menuToggleSound();
		// 128:5cc4
		} else if (_selectedMenuItem == 7) {
			this->menuPrintStory();
		} else if (_selectedMenuItem == 9) {
			this->menuQuit();
		}
	}
	// 128:5cea
	if ((_selectedMenuID >= 3) && (_selectedMenuID <= 7)) { // Chapter menus
		this->var_i16_e1a = 0;
		_selectedMenuChapter = _selectedMenuItem + (_selectedMenuID - 3)*0x10;
		if ((_selectedMenuChapter == 1) && (_event.what == kKeyDown)) {
			this->var_i16_e1a = 1;
		}
		// 128:5d56
		if ((_event.modifiers & kModLOptionKeyDown) != 0) {
			this->var_i16_e1a = 1;
		} else {
			// 128:5d74
			do {
				this->var_i16_7a8 = _toolbox->GetNextEvent(1 << kKeyDown, _event);
				_toolbox->GlobalToLocal(_event.where);
				if ((_event.modifiers & kModLOptionKeyDown) != 0) {
					this->var_i16_e1a = 1;
				}
			} while (_event.what != kNullEvent);
		}
		// 128:5dae
		if (((this->var_i16_7ce & 1) == 0) && (this->var_i16_e1a == 0) && (_storyCurrentChapter != _selectedMenuChapter)) {
			setStateBits(kStateChapterSelect);
		}
		// 128:5df6
		if (((this->var_i16_7ce & 1) == 0) && (this->var_i16_e1a == 1)) {
			setStateBits(kStateChapterSelect);

		}
		// 128:5e38
		if (((this->var_i16_7ce & 1) != 0) && (this->var_i16_e1a == 0)) {
			setStateBits(kStateChapterSelect | kStateReturn);
		}
		// 128:5e5e
		if (((this->var_i16_7ce & 1) == 0) && (this->var_i16_e1a == 1) && (_storyCurrentChapter != _selectedMenuChapter)) {
			setStateBits(kStateChapterSelect | kStateReturn);
			this->var_i16_7ce |= 0x4;
		}
	}
	// 128:5eaa
	if (_selectedMenuID == 8) { // Puzzle menu
		if (_selectedMenuItem == 1) {
			// return to scroll / run for your life
			setStateBits(kStateReturn);
		}
		// 128:5ec6
		if ((_selectedMenuItem == 3) && (this->var_i16_c00 == 1)) {
			setStateBits(kStateUndo);
		}
	}
	// 128:5eee
}

void FoolGame::thothHidePlayfield() {
	// 128:5ef0
	this->copyScreen(0, this->arr_bmp_b3ec);
	_zbasic->picture(0, MENU_HEIGHT, this->var_pic_7c2);
}

void FoolGame::thothShowPlayfield() {
	// 128:5f16
	if (_stateFlags == kStateSaveGame) {
		if (this->arr_i16_1eb8[0] < 1) {
			this->arr_i16_1eb8[0] = 1;
		}
		// 128:5f4c
		_activePuzzleBuffer = _zbasic->encodeInt(this->arr_i16_1eb8[0]);
		this->saveGame();
	}
	// 128:5f70
	if ((_stateFlags & kStateReturn) == 0) {
		this->waitForMouseUp();
		this->copyScreen(1, this->arr_bmp_b3ec);
	}
}

void FoolGame::sub_128_5f9e() {
	// 128:5f9e
	_keyLastPressed = _event.message & 0xff;
}

void FoolGame::onUpdateEvent() {
	// 128:5fb4
	_toolbox->BeginUpdate(*_event.windowPtr);
	_toolbox->EndUpdate(*_event.windowPtr);
	if ((this->var_i16_7ce & 1) != 0) {
		setStateBits(0x200 | kStateReturn);
	} else {
		setStateBits(0x200);
	}
}

void FoolGame::sub_128_5fea() {
	if (_screenOversized) {
		_toolbox->SetPort(this->var_i32_8);
		_toolbox->PenNormal();
		fillRect(MENU_HEIGHT, 0, _windowHeight, this->var_i16_56-3, 2);
		fillRect(MENU_HEIGHT, 0, this->var_i16_58+0x11, _windowWidth, 2);
		fillRect(MENU_HEIGHT, this->var_i16_56+0x203, _windowHeight, _windowWidth, 2);
		fillRect(this->var_i16_58 + 0x159, 0, _windowHeight, _windowWidth, 2);
		Common::Rect bounds; // arr_rect_5b7c
		_toolbox->SetRect(bounds, this->var_i16_56-2, this->var_i16_58+0x12, this->var_i16_56+0x202, this->var_i16_58+0x158);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->FrameRect(bounds);
		_toolbox->SetRect(bounds, this->var_i16_56-1, this->var_i16_58+0x13, this->var_i16_56+0x201, this->var_i16_58+0x157);
		_toolbox->PenPat(_patterns[2]);
		_toolbox->FrameRect(bounds);
		_toolbox->_defaultMenu->setOverlayDirty(true);
		_toolbox->SetPort(this->var_i32_0);
	}
	// 128:6152
}

void FoolGame::onDiskEvent() {
	// 128:6154
	this->var_i16_e1c = _event.message >> 16;
	this->var_i16_e1e = _event.message & 0xffff;
	if (this->var_i16_e1c != 0) {
		_zbasic->unk_11(this->var_i16_e1e);
	}
}


void FoolGame::waitForMouseUp() {
	// 128:6186
	// wait until mouse button is up
	do {
		// was originally a mask of 6
		this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
		_toolbox->GlobalToLocal(_event.where);
		if (_event.what == kNullEvent)
			_toolbox->Delay(0);
	} while ((_event.modifiers & kModMouseButtonUp) == 0);
}

void FoolGame::waitForClick() {
	// 128:61c2
	this->waitForMouseUp();
	do {
		// was originally a mask of 2
		this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
		_toolbox->GlobalToLocal(_event.where);
		if (_event.what == kNullEvent)
			_toolbox->Delay(0);
	} while ((_event.what != kMouseDown));
	this->waitForMouseUp();
}


void FoolGame::sub_128_61ec() {
	// 128:61ec
	do {
		this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
		if (_event.what == kUpdateEvt) {
			this->onUpdateEvent();
		}
		if (_event.what == kDiskEvt) {
			this->onDiskEvent();
		}
		if (_event.what == kNullEvent)
			_toolbox->Delay(0);
	} while (!((_event.what == kNullEvent) && (_event.modifiers & kModMouseButtonUp)));
	_keyLastPressed = 0;
}

void FoolGame::sub_128_6244() {
	do {
		this->var_i16_7a8 = _toolbox->GetNextEvent(-1, _event);
		if (_event.what == kUpdateEvt) {
			_toolbox->BeginUpdate(*_event.windowPtr);
			_toolbox->EndUpdate(*_event.windowPtr);
		}
		// 128:6272
		if (_event.what == kDiskEvt) {
			this->onDiskEvent();
		}
		_toolbox->Delay(0);
	} while ((_event.what == kNullEvent) && ((_event.modifiers & kModMouseButtonUp) == 0));
	// SEGMENT_RETURN
}

// end
// 128:62ae

void FoolGame::sub_129_004() {
	this->var_i16_7e6 = 0;
	// 129:000a: LEA - 0x3ea(A5),A0
	// 129:000e: MOVE.L - A0,-0x8ee(A5)
	// 129:0012: SF - 0x8,D0
	_zbasic->openR(1, Common::U32String("Fool's Puzzles"), 0x3e8, this->var_i16_e20); // was: str(110)
	if (this->var_i16_7e6 == 0) {
		this->var_str_e22 = Common::U32String("Fool's Puzzles"); // was: str(111)
		this->var_i16_f22 = this->var_i16_e20;
		_zbasic->close(1);
		this->var_i16_7e6 = 0;
	}

	// 129:005e: LEA - 0x3f2(A5),A0
	// 129:0062: MOVE.L - A0,-0x8ee(A5)
}

void FoolGame::sub_129_068() {
	this->var_i16_7ce = 666;
	this->var_i16_372 = { 0, 0, 0 };
	_soundEnabled = true;
	this->_mouseVisible = true;
	this->toggleMouseCursor(false);
	for (size_t i = 0; i <= 3; i++) {
		_patterns[1].data[2*i] = 0xaa;
		_patterns[1].data[2*i+1] = 0x55;
		_patterns[2].data[2*i] = 0xff;
		_patterns[2].data[2*i+1] = 0xff;
	}
	// 129:00d8
	// get current A5 - 0x72
	// 129:00d8: MOVE.L - 0x904,D0
	// 129:00de: JSR - "PEEKLONG"
	// 129:00e2: SUBI.L - 0x72,D0
	// 129:00e8: JSR - "PEEKWORD"
	_windowWidth = g_engine->_wm._screenDims.width();  // window width?
	// 129:00f0: MOVE.L - 0x904,D0
	// 129:00f6: JSR - "PEEKLONG"
	// 129:00fa: SUBI.L - 0x74,D0
	// 129:0100: JSR - "PEEKWORD"
	_windowHeight = g_engine->_wm._screenDims.height(); // window height?
	this->var_i16_56 = (_windowWidth - SCREEN_WIDTH)/2;
	this->var_i16_58 = (_windowHeight - SCREEN_HEIGHT)/2;
	// 129:0138
	if ((this->var_i16_56 == 0) && (this->var_i16_58 == 0)) {
		_screenOversized = false;
	} else {
		_screenOversized = true;
	}
	// 129:0166
	_zbasic->window(1, Common::String(), 0, 0, _windowWidth, _windowHeight, kWindowDialogOneLine);
	_zbasic->coordinateWindow();
	_toolbox->ClearMenuBar();
	_toolbox->DrawMenuBar();

	// very cursed check for the ROM85 flag
	// 129:0196: MOVE.L - 0x28e,D0
	// 129:019c: JSR - "BCD_FROM_INT"
	// 129:01a0: JSR - "CAST_INT"
	// 129:01a4: JSR - "PEEKWORD"
	// 129:01a8: CMPI.L - 0x3fff,D0
	if (true) {
		this->var_i16_372 = { 0x4e20, 0x4e20, 0x4e20 };
		_toolbox->SetCPixel(0x64, 0x64, this->var_i16_372);
		_toolbox->GetCPixel(0x64, 0x64, this->var_i16_372);

	}

	if ((_windowWidth != SCREEN_WIDTH) || (_windowHeight != SCREEN_HEIGHT)) {
		fillRect(0, 0, _windowHeight, _windowWidth, 2);
	} else {
		// 129:0224
		fillRect(0, 0, _windowHeight, _windowWidth, 1);
	}
	// 129:023e
	_toolbox->GetPort(this->var_i32_f24);

	this->var_i32_8 = &this->arr_grafport_19042;
	_toolbox->OpenPort(this->var_i32_8);
	// this grafport is used for drawing on the menu bar area;
	// on normal hardware this isn't a problem, as the changes are
	// made directly to the screen framebuffer.
	// in ScummVM, the window and the menu bar are widgets, with surfaces
	// that are composed to the screen framebuffer.
	// as such, to avoid a clash, we need to draw onto the menu bar widget.
	this->var_i32_8->portBits = _toolbox->_defaultMenuBits;
	this->var_i32_8->portRect = _toolbox->_defaultMenuBits->getBounds();

	// Thoth, on the other hand, expects to be able to draw to the screen.
	this->var_i32_8_thoth = &this->arr_grafport_19042_thoth;
	_toolbox->OpenPort(this->var_i32_8_thoth);
	_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->MovePortTo(this->var_i16_56, this->var_i16_58);
	_toolbox->ClipRect(_screenClipRect);

	this->var_i32_4 = &this->arr_grafport_18f78;
	_toolbox->OpenPort(this->var_i32_4);
	_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->MovePortTo(this->var_i16_56, this->var_i16_58);
	_toolbox->ClipRect(_screenClipRect);

	this->var_i32_0 = &this->arr_grafport_18eae;
	_toolbox->OpenPort(this->var_i32_0);
	_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->MovePortTo(this->var_i16_56, this->var_i16_58);
	_toolbox->ClipRect(_screenClipRect);

	this->sub_128_5fea();
	// 129:02c8
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1);

	if (false) {
	//if (this->var_i16_372.red + this->var_i16_372.blue + this->var_i16_372.green != 0) {
		// we're in colour mode, chide the user.
		// removed in v3.0
		// was: str(113) to str(116)
		_modalText[0] =Common::U32String("Set your monitor to");
		_modalText[1] = Common::U32String("2 color black and white");
		_modalText[2] = Common::U32String("and start the game again.");
		_modalText[3] = Common::U32String("Okay");
		this->showChoiceModal(kFontChicago, 2, 1, 0);
		_zbasic->unk_4();
	}
	// 129:0390
	this->sub_128_61ec();
	this->var_i32_37c = _zbasic->mem(-1);
	if (this->var_i32_37c < 0x1d4c0) {
		// not enough memory, chide the user
		// was: str(117) to str(123)
		_modalText[0] = Common::U32String("There is not enough available memory");
		_modalText[1] = Common::U32String("to run the program at this time.");
		_modalText[2] = Common::U32String();
		_modalText[3] = Common::U32String("Check your instruction manual for");
		_modalText[4] = Common::U32String("for possible solutions.");
		_modalText[5] = Common::U32String();
		_modalText[6] = Common::U32String("Quit");
		this->showChoiceModal(kFontChicago, 5, 1, 1);
		_zbasic->unk_4();
	}
	// 129:0496
	this->var_i16_7e4 = 1;
	this->var_str_e22.clear(); // was: str(124);
	_saveFileName.clear(); // was: str(125);

	this->var_i16_484 = _zbasic->finderInfo(this->var_i16_7e4, this->var_str_588, this->var_i32_f28, this->var_i16_688);

	if (!this->var_str_588.empty()) { // was: str(126)
		if (this->var_str_588 == Common::U32String("Fool's Puzzles")) { // was: str(127)
			this->var_str_e22 = Common::U32String("Fool's Puzzles"); // was: str(128)
			this->var_i16_f22 = this->var_i16_688;
		} else {
			// 129:052e
			this->sub_128_2808();
			if (!this->var_str_588.empty()) { // was: str(129)
				// 129:054c
				_saveFileName = this->var_str_588;
				this->var_i16_9ec = this->var_i16_688;
			}
		}
	}
	// 129:0562
	this->var_i16_f2c = this->getVolRefNum(); // volume ref num
	this->var_i16_e20 = this->var_i16_f2c;
	if (this->var_str_e22.empty()) { // was: str(130)
		this->sub_129_004();
	}
	// 129:058c
	if (this->var_str_e22.empty()) { // was: str(131)
		for (this->var_i16_f2e = 1; this->var_i16_f2e < 0x3e8; this->var_i16_f2e++) {
			this->var_str_f30 = _zbasic->files(-this->var_i16_f2e, Common::U32String(), Common::U32String(), this->var_i16_e20); // was: str(132)
			if ((this->var_i16_e20 == 0) || (this->var_str_f30.empty())) { // was: str(133)
				this->var_i16_f2e = 0x3e8;
			}
			// 129:0606
			this->sub_129_004();
			if (!this->var_str_e22.empty()) { // was: str(134)
				this->var_i16_f2e = 0x3e8;
			}
		}
	}
	// 129:0636
	if (this->var_str_e22.empty()) { // was: str(135)
		do {
			_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcBic);
			fillRect(0x17, 0x92, 0x31, 0x16e, 2);
			//this->var_i16_7a2 = 0x28;
			// prompt to locate the fool's puzzles file
			// 129:0690: CLR.W - -0x772(A5)
			this->toggleMouseCursor(true);
			this->sub_128_1e4(Common::String("PUZZ")); // was: str(137)
			// 129:06a6: CLR.W - -0x772(A5)
			fillRect(0x17, 0x92, 0x31, 0x16e, 1);
			this->toggleMouseCursor(false);
			if (this->var_str_588.empty()) { // was: str(138)
				_zbasic->unk_4();
			}
			this->var_str_e22 = this->var_str_588;
			this->var_i16_f22 = this->var_i16_688;
		} while (this->var_str_e22 != Common::U32String("Fool's Puzzles")); // was: str(139)
	}
	// 129:0718
	this->setVolRefNum(this->var_i16_f22);
	this->var_i16_1030 = _toolbox->OpenResFile(Common::Path(this->var_str_e22, ':'));
	_toolbox->UseResFile(this->var_i16_1030);
	this->var_i16_484 = this->setVolRefNum(this->var_i16_f2c);
	for (int i = 0; i <= 1; i++) {
		// scroll background
		this->arr_i32_192c0[i] = _toolbox->GetPicture(i + 0x54);
		PicHandle h = this->arr_i32_192c0[i];
		_toolbox->DetachResource(h);
	}
	// 129:07a2
	_zbasic->picture(0, MENU_HEIGHT, this->arr_i32_192c0[0]);
	_zbasic->get(0x0, 0x59, SCREEN_WIDTH, 0xb4, this->arr_bmp_5dfc);
	_zbasic->get(0x0, 0x14f, 0x7, SCREEN_HEIGHT, this->arr_bmp_b3ec);
	_zbasic->get(0x1f9, 0x14f, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_bmp_109dc);
	// 129:0846
	// unfurl the scroll by blitting the lower half a bunch of times
	for (int i = 0; i <= 0x15; i++) {
		this->var_i16_484 = _zbasic->readDataInt();
		_zbasic->put(0, this->var_i16_484, this->arr_bmp_5dfc, kSrcCopy);
		this->delay(2);
	}
	// 129:0888
	_zbasic->put(0, 0x14f, this->arr_bmp_b3ec, kSrcCopy);
	_zbasic->put(0x1f9, 0x14f, this->arr_bmp_109dc, kSrcCopy);
	_zbasic->picture(0x39, 0x29, this->arr_i32_192c0[1]);

	// 129:08ee
	for (int i = 0; i <= 1; i++) {
		_toolbox->ReleaseResource(this->arr_i32_192c0[i]);
	}

	// 129:091a
	this->var_i32_692 = _toolbox->TickCount();
	this->menuLoadingMessage(40);

	// load sun's map tiles
	//byte fakePal[768];
	//Common::fill(fakePal, fakePal+3, 0xff);
	//Common::fill(fakePal+3, fakePal+768, 0x00);

	for (int i = 1; i <= 0x53; i++) {
		_sunMapTilePic[i] = _toolbox->GetPicture(i);
		//_sunMapTilePic[i]->getSurface()->debugPrint(0, 0, 0, 0, 0, -1, 160, fakePal);

		_toolbox->DetachResource(_sunMapTilePic[i]);
	}
	// 129:097c
	this->menuLoadingMessage(60);

	// wadjet eye
	this->var_pic_7c2 = _toolbox->GetPicture(0x56);
	_toolbox->DetachResource(this->var_pic_7c2);
	_metapuzzleWheelPic = _toolbox->GetPicture(0xac);
	_toolbox->DetachResource(_metapuzzleWheelPic);
	// icon of a scroll
	this->var_i32_c = _toolbox->GetIcon(0x101);
	_toolbox->DetachResource(this->var_i32_c);
	// 129:09c2: SF - 0x8,D0
	_zbasic->openR(1, Common::U32String("Fool's Puzzles"), 1000, this->var_i16_f22); // was: str(140)
	this->var_i32_1036 = _zbasic->readFileDblInt(1);

	this->_puzzleDataPtr = 0;

	// read into pointer
	this->_puzzleDataBuffer = _zbasic->readFile(1, this->var_i32_1036);

	// 129:0a0a
	this->menuLoadingMessage(80);
	for (int i = 1; i <= 0x64; i++) {
		_puzzleDataOffsets[i] = puzzlesReadLong();
	}
	// 129:0a4e
	// quickdraw patterns
	// reworked slightly to fill the pattern buffer directly
	for (int i = 0; i <= 0x50; i++) {
		for (int j = 0; j <= 0x7; j++) {
			_patterns[i].data[j] = (byte)puzzlesReadByte();
		}
	}
	// 129:0a8a
	for (int i = 1; i <= 0x2a; i++) {
		_cursors[i] = Common::SharedPtr<Cursor>(new Cursor());
		for (int j = 0; j < 16; j++) {
			_cursors[i]->data[j] = (uint16)puzzlesReadShort();
		}
		for (int j = 0; j < 16; j++) {
			_cursors[i]->mask[j] = (uint16)puzzlesReadShort();
		}
		_cursors[i]->mouse.y = puzzlesReadShort();
		_cursors[i]->mouse.x = puzzlesReadShort();
		_cursors[i]->render();
	}
	// 129:0ad4
	for (int i = 0; i <= 0xf; i++) {
		// _bitLUT is 1 << i
		_bitLUT[i] = puzzlesReadShort();
	}
	// 129:0b02
	for (int i = 1; i <= 0x51; i++) {
		this->arr_i16_4c7c[i] = puzzlesReadByte();
	}
	// 129:0b30
	for (int i = 1; i <= 0xc; i++) {
		for (int j = 0; j <= 3; j++) {
			this->arr_i16_4d20[i*4 + j] = puzzlesReadShort();
		}
	}
	// 129:0b78
	this->var_i16_103a = puzzlesReadShort();
	// story menu entries + endgame puzzles
	for (int i = 1; i <= this->var_i16_103a; i++) {
		_puzzleType[i] = puzzlesReadShort();
		this->arr_i16_15e8[i] = puzzlesReadShort();
		_puzzleUnlockChapter[i] = puzzlesReadShort();
		_puzzleName[i] = puzzlesReadString().decode(Common::kMacRoman);
	}
	// 129:0c0a
	this->var_i16_103a = puzzlesReadShort();
	for (int i = 1; i <= this->var_i16_103a; i++) {
		for (int j = 0; j <= 5; j++) {
			this->arr_i16_16b2[i*8+j] = puzzlesReadShort();
		}
	}
	// 129:0c5e
	_storyPageCount = puzzlesReadShort();
	for (int i = 1; i <= _storyPageCount; i++) {
		_pageToChapter[i] = puzzlesReadShort();
	}
	// 129:0c98
	for (int i = 1; i <= _storyPageCount; i++) {
		_pageLineRanges[i*2] = puzzlesReadShort();
		_pageLineRanges[i*2 + 1] = puzzlesReadShort();
	}
	// 129:0ce0
	this->var_i16_103a = puzzlesReadShort();
	for (int i = 1; i <= this->var_i16_103a; i++) {
		_puzzleMenuInstructions[i*2] = puzzlesReadShort();
		_puzzleMenuInstructions[i*2 + 1] = puzzlesReadShort();
	}
	// 129:0d3c
	// story text
	this->var_i16_103c = puzzlesReadShort();
	this->var_i16_68a = 1;
	Common::fill(_pageLineFace, _pageLineFace + 1301, 0);
	Common::fill(_pageLineBreak, _pageLineBreak + 1301, 0);
	for (int i = 1; i <= this->var_i16_103c; i++) {
		this->var_i16_103e = puzzlesReadByte();
		// indent at start of story paragraph
		if (this->var_i16_103e & 0x8) {
			this->var_i16_103e ^= 0x8;
			// 129:0d7e
			_zbasic->indexRawSet(_zbasic->space(4).encode(Common::kMacRoman) + puzzlesReadString(), 0, i);
		} else {
		// 129:0da4
			_zbasic->indexRawSet(puzzlesReadString(), 0, i);
		}
		// 129:0db8
		// 0x4 == newline
		// 0x1 == normal formatting
		if ((this->var_i16_103e == 1) || (this->var_i16_103e == 4)) {
			_pageLineFace[i] = Graphics::kMacFontRegular;
		}

		// 129:0dea
		// 0x2 = bold formatting
		if ((this->var_i16_103e == 2) || (this->var_i16_103e == 5)) {
			_pageLineFace[i] = Graphics::kMacFontBold;
		}
		// 129:0e1e
		// 0x3 = outlined
		if ((this->var_i16_103e == 3) || (this->var_i16_103e == 6)) {
			_pageLineFace[i] = Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontShadow;
		}
		// 129:0e52
		if (this->var_i16_103e > 3) {
			_pageLineBreak[i] = 1;
		}
		// 129:0e6e
	}
	// 129:0e80
	this->menuLoadingMessage(100);
	this->var_i16_103a = puzzlesReadShort();
	for (int i = this->var_i16_103c + 1; i <= this->var_i16_103a; i++) {
		_zbasic->indexRawSet(puzzlesReadString(), 0, i);
	}
	// 129:0ec2
	for (int j = 1; j <= 0x50; j++) {
		this->var_i16_1040 = 0;
		for (int i = 1; i <= _storyPageCount; i++) {
			if (_pageToChapter[i] == j) {
				this->var_i16_1040++;
			}
			// 129:0ef4
			if (_pageToChapter[i] > j) {
				i = _storyPageCount;
			}
			// 129:0f16
		}
		if (this->var_i16_1040 > 1) {
			this->var_i16_484 = 0;
			for (int i = 1; i <= _storyPageCount; i++) {
				if (_pageToChapter[i] == j) {
					this->var_i16_484++;
					// - X of N
					_pageNumberText[i] = Common::U32String::format(" - %d of %d", this->var_i16_484, this->var_i16_1040); // was: str(141), str(142)
				}
				// 129:0fb6
				if (_pageToChapter[i] > j) {
					i = _storyPageCount;
				}
				// 129:0fd8
			}
		}
		// 129:0fea
	}
	// 129:0ff8
	Common::fill(_pageLines, _pageLines + 202, 0);
	for (int j = 1; j <= _storyPageCount; j++) {
		for (int i = _pageLineRanges[j*2]; i <= _pageLineRanges[j*2 + 1]; i++) {
			if (_pageLineBreak[i] != 0) {
				// 129:102e
				_pageLines[j]++;
			}
			// 129:105a
		}
	// 129:107a
	}
	// 129:108c
	_toolbox->UseResFile(this->var_i16_1030);
	this->delayFromMarker(0xc8);
	this->var_i16_7ce = 0;
	this->var_i16_7b2 = 0xa;
	_toolbox->InitCursor();
	this->toggleMouseCursor(true);
	// The menus are so different between versions, it's easiest to hardcode the v2.0 ones.
	// Apple menu
	_zbasic->menu(1, 0, 1, Common::U32String("~")); // wadjet eye, was: str(143)
	_zbasic->menu(1, 1, 0, Common::U32String("-"));
	_zbasic->menu(1, 2, 1, Common::U32String("/FAbout Fool's Errand"));
	// this is brand new
	_zbasic->menu(1, 3, 1, Common::U32String("Show Prologue"));
	_zbasic->menu(1, 4, 1, Common::U32String("Show Finale"));
	_zbasic->menu(1, 5, 0, Common::U32String("-"));

	// File menu
	_zbasic->menu(2, 0, 1, Common::U32String("File"));
	_zbasic->menu(2, 1, 1, Common::U32String("/NNew"));
	_zbasic->menu(2, 2, 1, Common::U32String("/OOpen"));
	_zbasic->menu(2, 3, 1, Common::U32String("/SSave"));
	_zbasic->menu(2, 4, 1, Common::U32String("/ASave As"));
	_zbasic->menu(2, 5, 0, Common::U32String("-"));
	_zbasic->menu(2, 6, 1, Common::U32String("Sound"));
	_zbasic->menu(2, 7, 1, Common::U32String("Print Story"));
	_zbasic->menu(2, 8, 0, Common::U32String("-"));
	_zbasic->menu(2, 9, 1, Common::U32String("/QQuit"));

	this->sub_128_6244();
	// 129:11f6
	if (_saveFileName.empty()) { // was: str(157)
		// cold start
		_stateFlags = kStateNewGame;
	} else {
		// loading a save game
		_stateFlags = kStateOpenGame;
		this->var_str_588 = _saveFileName;
		this->var_i16_688 = this->var_i16_9ec;
	}
	// 129:1236
	// JMP - [0x19f6]
	// SEGMENT_RETURN
}

void FoolGame::menuLoadingMessage(int16 percent) {
	// 129:123a
	// draw the loading text on the menu bar
	_toolbox->SetPort(this->var_i32_8);
	fillRect(0, 7, 0x13, _windowWidth - 7, 0);
	_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// Loading Game text during initial puzzle load
	Common::U32String message = Common::U32String::format("Loading Game - %d%%", percent); // was: str(158)
	int16 width = _toolbox->StringWidth(message);
	_toolbox->MoveTo((_windowWidth / 2) - (width / 2), 0xe);
	_toolbox->DrawString(message);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(this->var_i32_0);
	_toolbox->Delay(0);
}


void FoolGame::sub_144_004() {
	// 144:0004
	_toolbox->ReleaseResource(var_pic_7c2);
	var_i32_7c8 = _zbasic->mem(-1);
	_stateFlags = kStateQuit;
	if (!_screenOversized) {
		_toolbox->SetPort(var_i32_8);
		fillRect(0, 0, MENU_HEIGHT, SCREEN_WIDTH, 0x47);
		_toolbox->SetPort(var_i32_0);
	} else {
		// 144:0046
		sub_128_1ef8();
	}
	// 144:004a
	autoSaveGame(); // was: saveGame
	_toolbox->InitCursor();
	toggleMouseCursor(false);
	if (!_screenOversized) {
		var_i16_42 = 0;
		var_i16_44 = 0xab;
	} else {
		// 144:0070
		var_i16_42 = 0x14;
		var_i16_44 = 0xb5;
		_toolbox->SetPort(var_i32_0);
	}
	// 144:0082
	_zbasic->get(1, var_i16_42+1, SCREEN_WIDTH, SCREEN_HEIGHT-1, arr_bmp_5dfc);
	for (int16 i = 0xa; i <= 0xf0; i += 0xa) {
		var_i32_692 = _toolbox->TickCount();
		_zbasic->put(
			i,
			var_i16_42 + (int16)(i*0.6f),
			SCREEN_WIDTH - i,
			SCREEN_HEIGHT - (int16)(i*0.6f),
			arr_bmp_5dfc, kSrcCopy);
		// 144:015a
		delayFromMarker(0xf);
	}
	// 144:0170
	delay(0x3c);
	Common::Rect temp;
	for (int16 i = 1; i <= 0x100; i++) {
		var_i32_692 = _toolbox->TickCount();

		_toolbox->SetRect(
			temp,
			0x100 - i,
			var_i16_44 - (int16)(i*0.7f),
			0x100 + i,
			var_i16_44 + (int16)(i*0.7f)
		);
		_toolbox->InvertRect(temp);
		delayFromMarker(1);
	}
	// 144:0238
	for (int16 i = 0; i <= 0xe; i++) {
		var_i32_692 = _toolbox->TickCount();
		_toolbox->InvertRect(temp);
		delayFromMarker(1);
	}
	// 144:0268
	// Every version has a slightly different end message, and all
	// contain instructions for how to open up the "Show Finale" file.
	// This also contains a typo which was fixed in the DOS edition.
	// Easy option: just hardcode the message here
	// was: str(368)
	_modalText[0] = Common::U32String("What's this?");
	_modalText[1] = Common::U32String("More evil treachery afoot?");
	_modalText[2] = Common::U32String("Has not the fool gained the gift of wisdom?");
	_modalText[3] = Common::U32String();
	_modalText[4] = Common::U32String("Congratulations!");
	_modalText[5] = Common::U32String("You may now view the finale.");
	_modalText[7] = Common::U32String("Okay");
	showChoiceModal(kFontFool, 6, 1, 0);
	if (!_screenOversized) {
		_toolbox->ClearMenuBar();
		_toolbox->DrawMenuBar();
	}
	sub_128_1f1e();
	toggleMouseCursor(true);
	// 144:03e2
	// write the finale file to the disk. we don't need to do this
	//if (_saveFileName.empty()) {
	//	sub_144_5ca();
	//}
	//sub_144_406();
}

};
