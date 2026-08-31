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

#include "scooby/scooby.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "offsets.h"
#include "scooby/console.h"
#include "scooby/detection.h"
#include "scooby/file.h"
#include "scooby/gfx.h"
#include "scooby/md/vdp.h"

namespace Scooby {

ScoobyEngine *g_engine;

ScoobyEngine::ScoobyEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Scooby") {
	g_engine = this;
}

ScoobyEngine::~ScoobyEngine() {
	delete _screen;
}

uint32 ScoobyEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String ScoobyEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error ScoobyEngine::run() {
	_file = new File(this);
	_vdp = new VDP();
	_gfx = new Gfx(_vdp);
	_limiter = new Graphics::FrameLimiter(g_system, 60);

	setupInitialVdpRegisters();
	// // Initialize 320x200 paletted graphics mode
	// initGraphics(320, 200);
	// _screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	introSequence();
	mainMenu();

	// // Draw a series of boxes on screen as a sample
	// for (int i = 0; i < 100; ++i)
	// 	_screen->frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);
	// _screen->update();

	// Simple event handling loop
	byte pal[256 * 3] = { 0 };
	Common::Event e;
	int offset = 0;


	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		// Cycle through a simple palette
		++offset;
		// for (int i = 0; i < 256; ++i)
		// 	pal[i * 3 + 1] = (i + offset) % 256;
		// g_system->getPaletteManager()->setPalette(pal, 0, 256);
		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_limiter->delayBeforeSwap();
		// _screen->update();
		_gfx->drawFrame();
		_limiter->startFrame();
	}

	delete _limiter;
	delete _gfx;
	delete _vdp;
	delete _file;

	return Common::kNoError;
}

Common::Error ScoobyEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

void ScoobyEngine::setupInitialVdpRegisters() {
	_file->offsetFromStart(data_initialVdpRegisters);

	for (uint16 i = 0; i <= 0x12; i++) {
		uint16 command = 0x8000u | (uint16)(i << 8u) | _file->readByte();
		debug("VDP init %d %04X", i, command);
		_vdp->control_port_w(command);
	}
}

void ScoobyEngine::loadPalette(uint32 offset, uint16 *palette) {
	_file->offsetFromStart(offset);
	for (int i = 0; i < 64; i++) {
		palette[i] = _file->readUint16();
	}
}

void ScoobyEngine::waitForFrames(uint16 numFrames) {
	for (uint16 i = 0; i < numFrames && !Engine::shouldQuit(); i++) {
		_limiter->delayBeforeSwap();
		_gfx->drawFrame();
		_limiter->startFrame();
		// TODO poll for updates here.
	}
}

void ScoobyEngine::introSequence() {
	//SEGA Logo
	byte buf[0x8000];
	uint16 palette[64];

	_file->offsetFromStart(data_SegaLogoTilemap);
	_file->read(buf, 48 * 2);

	_gfx->loadTilemap(VDP::PlaneA, buf, 10, 11, 0xc, 4);

	uint16 size = _file->decompressBytes(data_SegaLogoTiles_packed, buf, sizeof(buf));
	debug("Size: %d", size);
	_vdp->writeVRAM(0, buf, size);

	loadPalette(data_SegaLogoPalette, palette);

	fadeFromBlack(palette);

	waitForFrames(0x32);

	/* shine anim on sega logo */
	for (int i = 0; i < 9; i++) {
		uint16 temp = palette[10];
		for (int j = 0; j < 8; j++) {
			palette[10 - j] = palette[10 - j - 1];
			_vdp->writePalette(10 - j, palette[10 - j]);
		}
		palette[10 - 8] = temp;
		_vdp->writePalette(10 - 8, temp);
		waitForFrames(3);
	}

	waitForFrames(100);

	fadeToBlack();
	_vdp->zeroVRAM(0, 0x8000);

	/* VDP: Mode Register 4
   320 pixel (40 cell) wide mode.
   Progressive mode. */
	_vdp->control_port_w(0x8c81);

	// Scooby title
	_file->offsetFromStart(data_ScoobyLogoTilemap);
	_file->read(buf, 0x28 * 0x1c * 2);
	_gfx->loadTilemap(VDP::PlaneA, buf, 0, 0, 0x28, 0x1c);

	_file->offsetFromStart(data_ScoobyLogoTiles);
	_file->read(buf, 0x20e0 * 2);
	_vdp->writeVRAM(0, buf, 0x20e0 * 2);

	loadPalette(data_ScoobyLogoPalette, palette);

	fadeFromBlack(palette);

	waitForFrames(300);

	fadeToBlack();
	_vdp->zeroVRAM(0, 0x8000);

	/* VDP: Mode Register 4
   256 pixel (32 cell) wide mode.
   Progressive mode. */
	_vdp->control_port_w(0x8c00);

	_DAT_00ff09eb_flags |= 2u;
	_DAT_00ff07f8 = 0;

	// Acclaim logo
	_file->offsetFromStart(data_AcclaimLogoBgTilemap);
	_file->read(buf, 0x20 * 0x6 * 2);
	_gfx->loadTilemap(VDP::PlaneA, buf, 0, 9, 0x20, 0x6);

	_file->offsetFromStart(data_AcclaimLogoFgTilemap);
	_file->read(buf, 0x180 * 2);
	_vdp->writeVRAM(0xe480, buf, 0x180 * 2);

	size = _file->decompressBytes(data_AcclaimLogoTiles, buf, sizeof(buf));
	_vdp->writeVRAM(0, buf, size);

	loadPalette(data_AcclaimLogoPalette, palette);

	_SHORT_00ff07fa = -0x100;
	_vdp->writeVRAMWord(0xdc00, 0);
	_vdp->writeVRAMWord(0xdc02, _SHORT_00ff07fa);

	fadeFromBlack(palette);

	//TODO this was done in vblank handler in original game
	do {
		waitForFrames(1);
		_SHORT_00ff07fa += 4;
		_vdp->writeVRAMWord(0xdc00, 0);
		_vdp->writeVRAMWord(0xdc02, _SHORT_00ff07fa);
	} while (_SHORT_00ff07fa < 0x30);

	waitForFrames(200);

	fadeToBlack();
	_vdp->zeroVRAM(0, 0x8000);

	// Sunsoft logo
	_file->offsetFromStart(data_SunSoftLogoTilemap);
	_file->read(buf, 0x1a * 0x9 * 2);
	_gfx->loadTilemap(VDP::PlaneA, buf, 3, 8, 0x1a, 0x9);

	size = _file->decompressBytes(data_SunSoftLogoTiles, buf, sizeof(buf));
	_vdp->writeVRAM(0, buf, size);
	loadPalette(data_SunSoftLogoPalette, palette);

	fadeFromBlack(palette);
	waitForFrames(300);
	fadeToBlack();
	_vdp->zeroVRAM(0, 0x8000);

	//Illusions logo animation
	size = _file->decompressBytes(data_IllusionsLogoTiles, buf, sizeof(buf));
	_vdp->writeVRAM(0, buf, size);

	_file->decompressBytes(data_IllusionsLogoAnimTilemap, buf, sizeof(buf));
	_gfx->loadTilemap(VDP::PlaneA, buf, 8, 8, 0x14, 0x5);
	loadPalette(data_IllusionsLogoPalette, palette);

	fadeFromBlack(palette);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 15; j++) {
			waitForFrames(2);
			_gfx->loadTilemap(VDP::PlaneA, buf + j * 200, 8, 8, 0x14, 0x5);
		}
	}

	_file->decompressBytes(data_IllusionsLogoFullTilemap, buf, sizeof(buf));
	_gfx->loadTilemap(VDP::PlaneA, buf, 4, 5, 0x19, 0xf);

	waitForFrames(200);
	fadeToBlack();
	_vdp->zeroVRAM(0, 0x8000);
}

void ScoobyEngine::fadeFromBlack(const uint16 *palette) {
	uint16 tempPalette[64];
	memset(tempPalette, 0, sizeof(tempPalette));
	_vdp->writeCRAM(0, (byte *)tempPalette, 0x80);

	for (int fadeCounter = 0; fadeCounter < 8; fadeCounter++) {
		for (int i = 0; i < 64; i++) {
			if ((palette[i] & 0xe00u) != (tempPalette[i] & 0xe00u)) {
				tempPalette[i] += 0x200;
			}
			if ((palette[i] & 0xe0u) != (tempPalette[i] & 0xe0u)) {
				tempPalette[i] += 0x20;
			}
			if ((palette[i] & 0xeu) != (tempPalette[i] & 0xeu)) {
				tempPalette[i] += 0x2;
			}
			_vdp->writePalette(i, tempPalette[i]);
		}
		waitForFrames(1);
	}
}

void ScoobyEngine::fadeToBlack() {
	for (int fadeCounter = 0; fadeCounter < 8; fadeCounter++) {
		for (int i = 0; i < 64; i++) {
			uint16 colour = _vdp->readPaletteRecord(i);
			if ((colour & 0xe00u) != 0) {
				colour -= 0x200;
			}
			if ((colour & 0xe0u) != 0) {
				colour -= 0x20;
			}
			if ((colour & 0xeu) != 0) {
				colour -= 0x2;
			}
			_vdp->writePalette(i, colour);
		}
		waitForFrames(1);
	}
}

void ScoobyEngine::mainMenu() {
	byte buf[0x10000];
	uint16 palette[0x40];

	// fadeToBlack();
	// DAT_00ff09ed = DAT_00ff09ed & 235;
	// DAT_00ff09eb_flags = DAT_00ff09eb_flags & 127;

	/* VDP: Window Plane Horizontal Position
	   Draw window from HP to left edge of screen.
	   Position: 0x0 (in units of 8 pixels). */
	_vdp->control_port_w(0x9100);
	/* VDP: Window Plane Vertical Position
	   Draw window from VP to top edge of screen.
	   Position: 0x0 (in units of 8 pixels). */
	_vdp->control_port_w(0x9200);

	// DAT_00ff0016 = 0;
	// DAT_00ff001c = 0;
	// DAT_00ff001a = 0;
	// DAT_00ff000a = currentRoomId;
	// _DAT_00ff0010 = 224;
	// DAT_00ff0012 = 224;
	_vdp->writeVRAMWord(0xdc00,0);
	_vdp->writeVRAMWord(0xdc02,0);

	// _vdp->control_port_w(0x4000);
	// _vdp->control_port_w(0x0010);
	// _vdp->data_port_w16(0);
	// _vdp->data_port_w16(0);
	uint32 b = 0;
	_vdp->writeVSRAM(0, reinterpret_cast<byte *>(&b), 4);

	// DAT_00ff000c = 0;
	// DAT_00ff000e = 3;
	// DAT_00ff09eb_flags = DAT_00ff09eb_flags & 247;
	// DAT_00ff09eb_flags = DAT_00ff09eb_flags | 16;

	_vdp->zeroVRAM(0xdc00, 0x1c0);
	_vdp->zeroVRAM(0, 0x8000);
	// DAT_00ff09eb_flags = DAT_00ff09eb_flags | 2;
	_vdp->control_port_w(0x7c00);
	_vdp->control_port_w(0x0003);
	/* VDP: Sprite Table
   Location - 0xfc00 */
	_vdp->control_port_w(0x857e);
	_vdp->data_port_w16(0);

	_vdp->control_port_w(0x857e);
//	write_volatile_4(0xc00000,0);
//	vBlankFunctionPtr = FUN_0000a38c_mainmenu_vblank;
//	hBlankFunctionPtr = noOp_hblank;
	/* VDP: Window Plane Horizontal Position
	   Draw window from HP to left edge of screen.
	   Position: 0x0 (in units of 8 pixels). */
	_vdp->control_port_w(0x9100);
	/* VDP: Window Plane Vertical Position
	   Draw window from VP to top edge of screen.
	   Position: 0x0 (in units of 8 pixels). */
	_vdp->control_port_w(0x9200);
	/* VDP: Plane Size
	   Width - 256 pixels (32 cells)
	   Height - 256 pixels (32 cells) */
	_vdp->control_port_w(0x9000);

	uint32 size = _file->decompressBytes(data_MainMenuTiles, buf, sizeof(buf));
	_vdp->writeVRAM(0, buf, size);

	byte src[0x132];
	byte dest[0x200];
	_file->offsetFromStart(0x16d38);
	_file->read(src, 0x132);

	unpackRLE(src, dest);
	// FUN_00009c64(src, dest);

	// printBuf(dest, 512);

	_vdp->writeVRAM(size, dest, 0x200);

	_file->offsetFromStart(data_MainMenuBackgroundTilemap);
	_file->read(buf, 0x380 * 2);
	_vdp->writeVRAM(0xe000, buf, 0x380 * 2);

	_file->offsetFromStart(0x85f8);
	_file->read(buf, 0x34 * 2);
	_vdp->writeVRAM(0xfc00, buf, 0x34 * 2);

	loadPalette(0x16cb8, palette);
	fadeFromBlack(palette);
}

// void ScoobyEngine::FUN_00009c64(uint8 *src,uint8 *dest)
//
// {
// 	byte bVar1;
// 	byte bVar2;
// //	undefined4 in_D0;
// 	int16 runLength;
// 	uint16 uVar4;
// 	byte *pbVar5;
// 	byte *pbVar6;
// 	byte *puVar7;
// 	uint32 *endAddress;
// 	uint32 expandedSize;
// 	uint32 curPos = 0;
//
// 	if ((DAT_00ff09ed_flags & 8) != 0) {
// 		/* VDP: Mode Register 4
// 		   256 pixel (32 cell) wide mode.
// 		   Enable shadow/highlight mode.
// 		   Progressive mode. */
// 		_vdp->control_port_w(0x8c08);
// 	}
// 	pbVar5 = src + 4;
// //	endAddress = (undefined4 *)(dest + (short)*(undefined4 *)src);
// 	expandedSize = READ_BE_UINT32(src);
// 	for (uint32 curPos = 0; curPos < expandedSize; ) {
// 		while( true ) {
// 			pbVar6 = pbVar5 + 1;
// 			bVar1 = *pbVar5;
// 			runLength = (uint)bVar1;
// 			puVar7 = dest;
// 			if (-1 < (char)*pbVar5) break;
// 			pbVar5 = pbVar5 + 2;
// 			bVar2 = *pbVar6;
// 			runLength = (uint16)(byte)-bVar1;
// 			if (curPos % 2 == 1) { //((short)((short)dest << 0xf) < 0) {
// 				puVar7 = dest + 1;
// 				*dest = bVar2;
// 				curPos++;
// 				runLength--;
// 			}
// 			for (int i = 0; i <= runLength / 4; i++) {
// 				memset(puVar7, bVar2, 4);
// 				puVar7 += 4;
// 				curPos += 4;
// 			}
// //			uVar4 = runLength / 4; //._2_2_ >> 2;
// //			do {
// //				puVar7 = puVar7 + 4;
// //				memset(puVar7, bVar2, 4);
// ////				*puVar7 = CONCAT22(CONCAT11(bVar2,bVar2),CONCAT11(bVar2,bVar2));
// //				if (false) break;
// //				uVar4 = uVar4 - 1;
// //				puVar7 = puVar7;
// //			} while (uVar4 != 0xffff);
// 			dest = (byte *)(puVar7 - (int)(short)((runLength & 3u) ^ 3));
// 			if (curPos == expandedSize) {
// 				/* VDP: Mode Register 4
// 				   256 pixel (32 cell) wide mode.
// 				   Progressive mode. */
// 				_vdp->control_port_w(0x8c00);
// 				return;
// 			}
// 		}
// 		do {
// 			pbVar5 = pbVar6 + 1;
// 			dest = puVar7 + 1;
// 			curPos++;
// 			*(byte *)puVar7 = *pbVar6;
// 			if (false) break;
// 			runLength--;
// 			pbVar6 = pbVar5;
// 			puVar7 = dest;
// 		} while (runLength != -1);
// 	}
// 	/* VDP: Mode Register 4
//    256 pixel (32 cell) wide mode.
//    Progressive mode. */
// 	_vdp->control_port_w(0x8c00);
// }

void ScoobyEngine::unpackRLE(uint8 *src,uint8 *dest) {
	int len = 0;
	int cur = 0;
	if ((DAT_00ff09ed_flags & 8) != 0) {
		/* VDP: Mode Register 4
		   256 pixel (32 cell) wide mode.
		   Enable shadow/highlight mode.
		   Progressive mode. */
		_vdp->control_port_w(0x8c08);
	}

	len = READ_BE_UINT32(src);
	src += 4;

	do {
		if (*src & 0x80) {
			byte runLength = *src;
			runLength = -runLength;
			src++;
			byte b = *src;
			memset(dest, b, runLength);
			dest += runLength;
			cur += runLength;
		} else {
			int runLength = *src;
			src++;
			len--;
			for ( int i = 0; i < runLength; i++ ) {
				*dest = *src;
				dest++;
				src++;
				cur++;
			}
			len -= runLength;
		}
	} while (cur < len);
	/* VDP: Mode Register 4
256 pixel (32 cell) wide mode.
Progressive mode. */
	_vdp->control_port_w(0x8c00);
}

} // End of namespace Scooby
