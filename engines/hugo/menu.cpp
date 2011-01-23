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
 * $URL$
 * $Id$
 *
 */

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/sound.h"
#include "hugo/util.h"
#include "graphics/imagedec.h"
#include "common/substream.h"

namespace Hugo {

enum {
	kMenuWidth = 320,
	kMenuHeight = 24,
	kMenuX = 5,
	kMenuY = 1,
	kButtonWidth = 20,
	kButtonHeight = 20,
	kButtonPad = 1,
	kButtonSpace = 5
};

enum {
	kCmdWhat = 'WHAT',
	kCmdMusic = 'MUZK',
	kCmdSoundFX = 'SOUN',
	kCmdLoad = 'LOAD',
	kCmdSave = 'SAVE',
	kCmdRecall = 'RECL',
	kCmdTurbo = 'TURB',
	kCmdLook = 'LOOK',
	kCmdInvent = 'INVT'
};

TopMenu::TopMenu(HugoEngine *vm) : Dialog(0, 0, kMenuWidth, kMenuHeight), arrayBmp(0), arraySize(0),
	_vm(vm) {
	init();
}

TopMenu::~TopMenu() {
	for (int i = 0; i < arraySize; i++) {
		arrayBmp[i * 2]->free();
		delete arrayBmp[i * 2];
		arrayBmp[i * 2 + 1]->free();
		delete arrayBmp[i * 2 + 1];
	}
	delete[] arrayBmp;
}

void TopMenu::init() {
	int x = kMenuX;
	int y = kMenuY;

	_whatButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "What is it?", kCmdWhat);
	_musicButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Music", kCmdMusic);
	_soundFXButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Sound FX", kCmdSoundFX);
	_loadButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Load game", kCmdLoad);
	_saveButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Save game", kCmdSave);
	_recallButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Recall last command", kCmdRecall);
	_turboButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Turbo", kCmdTurbo);
	_lookButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Description of the scene", kCmdLook);
	_inventButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Inventory", kCmdInvent);
}

void TopMenu::reflowLayout() {
	_w = g_system->getOverlayWidth();

	int scale = (_w > 320 ? 2 : 1);

	_h = kMenuHeight * scale;

	int x = kMenuX * scale;
	int y = kMenuY * scale;

	_whatButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_musicButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_soundFXButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_loadButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_saveButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_recallButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;
	
	_turboButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;
	
	_lookButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;
	
	_inventButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	// Set the graphics to the 'on' buttons, except for the variable ones 
	_whatButton->setGfx(arrayBmp[4 * kMenuWhat + scale - 1]);
	_musicButton->setGfx(arrayBmp[4 * kMenuMusic + scale - 1 + ((_vm->_config.musicFl) ? 0 : 2)]);
	_soundFXButton->setGfx(arrayBmp[4 * kMenuSoundFX + scale - 1 + ((_vm->_config.soundFl) ? 0 : 2)]);
	_loadButton->setGfx(arrayBmp[4 * kMenuLoad + scale - 1]);
	_saveButton->setGfx(arrayBmp[4 * kMenuSave + scale - 1]);
	_recallButton->setGfx(arrayBmp[4 * kMenuRecall + scale - 1]);
	_turboButton->setGfx(arrayBmp[4 * kMenuTurbo + scale - 1 + ((_vm->_config.turboFl) ? 0 : 2)]);
	_lookButton->setGfx(arrayBmp[4 * kMenuLook + scale - 1]);
	_inventButton->setGfx(arrayBmp[4 * kMenuInventory + scale - 1]);
}

void TopMenu::loadBmpArr(Common::File &in) {
	arraySize = in.readUint16BE();

	delete arrayBmp;
	arrayBmp = new Graphics::Surface *[arraySize * 2];
	for (int i = 0; i < arraySize; i++) {
		uint16 bmpSize = in.readUint16BE();
		uint32 filPos = in.pos();
		Common::SeekableSubReadStream stream(&in, filPos, filPos + bmpSize); 
		arrayBmp[i * 2] = Graphics::ImageDecoder::loadFile(stream, g_system->getOverlayFormat());
		arrayBmp[i * 2 + 1] = new Graphics::Surface();
		arrayBmp[i * 2 + 1]->create(arrayBmp[i * 2]->w * 2, arrayBmp[i * 2]->h * 2, arrayBmp[i * 2]->bytesPerPixel);
		byte *src = (byte *)arrayBmp[i * 2]->pixels;
		byte *dst = (byte *)arrayBmp[i * 2 + 1]->pixels;
		
		for (int j = 0; j < arrayBmp[i * 2]->h; j++) {
			src = (byte *)arrayBmp[i * 2]->getBasePtr(0, j);
			dst = (byte *)arrayBmp[i * 2 + 1]->getBasePtr(0, j * 2);
			for (int k = arrayBmp[i * 2]->w; k > 0; k--) {
				for (int m = arrayBmp[i * 2]->bytesPerPixel; m > 0; m--) {
					*dst++ = *src++;
				}
				src -= arrayBmp[i * 2]->bytesPerPixel;

				for (int m = arrayBmp[i * 2]->bytesPerPixel; m > 0; m--) {
					*dst++ = *src++;
				}
			}
			src = (byte *)arrayBmp[i * 2 + 1]->getBasePtr(0, j * 2);
			dst = (byte *)arrayBmp[i * 2 + 1]->getBasePtr(0, j * 2 + 1);
			for (int k = arrayBmp[i * 2 + 1]->pitch; k > 0; k--) {
				*dst++ = *src++;
			}
		}
		
		in.skip(bmpSize);
	}
}

void TopMenu::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	switch (command) {
	case kCmdWhat:
		close();
		_vm->_file->instructions();
		break;
	case kCmdMusic:
		close();
		_vm->_sound->toggleMusic();
		break;
	case kCmdSoundFX:
		close();
		_vm->_sound->toggleSound();
		break;
	case kCmdLoad:
		close();
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		_vm->getGameStatus().viewState = kViewPlay;
		break;
	case kCmdSave:
		close();
		if (_vm->getGameStatus().viewState == kViewPlay) {
			if (_vm->getGameStatus().gameOverFl)
				Utils::gameOverMsg();
			else
				_vm->_file->saveGame(-1, Common::String());
		}
		break;
	case kCmdRecall:
		close();
		_vm->getGameStatus().recallFl = true;
		break;
	case kCmdTurbo:
		close();
		_vm->_parser->switchTurbo();
		break;
	case kCmdLook:
		close();
		_vm->_parser->command("look around");
		break;
	case kCmdInvent:
		close();
		_vm->_parser->showInventory();
		break;
	default:
		Dialog::handleCommand(sender, command, data);
	}
}

void TopMenu::handleMouseUp(int x, int y, int button, int clickCount) {
	if (y > _h)
		close();
	else
		Dialog::handleMouseUp(x, y, button, clickCount);
}

} // End of namespace Hugo
