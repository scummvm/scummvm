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

TopMenu::TopMenu(HugoEngine *vm) : Dialog(0, 0, kMenuWidth, kMenuHeight), arrayBmp(0),
	_vm(vm) {
	init();
}

void TopMenu::init() {
	int x = kMenuX;
	int y = kMenuY;

	_whatButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "What is it?", kCmdWhat);
	x += kButtonWidth + kButtonPad;

	_musicButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Music", kCmdMusic);
	x += kButtonWidth + kButtonPad;

	_soundFXButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Sound FX", kCmdSoundFX);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_loadButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Load game", kCmdLoad);
	x += kButtonWidth + kButtonPad;

	_saveButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Save game", kCmdSave);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_recallButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Recall last command", kCmdRecall);
	x += kButtonWidth + kButtonPad;
	
	_turboButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Turbo", kCmdTurbo);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;
	
	_lookButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Description of the scene", kCmdLook);
	x += kButtonWidth + kButtonPad;
	
	_inventButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Inventory", kCmdInvent);
	x += kButtonWidth + kButtonPad;

}

void TopMenu::loadBmpArr(Common::File &in) {
	uint16 arraySize = in.readUint16BE();

	arrayBmp = (Graphics::Surface **)malloc(sizeof(Graphics::Surface *) * (arraySize));
	for (int i = 0; i < arraySize; i++) {
		uint16 bmpSize = in.readUint16BE();
		uint32 filPos = in.pos();
		Common::SeekableSubReadStream stream(&in, filPos, filPos + bmpSize); 
		arrayBmp[i] = Graphics::ImageDecoder::loadFile(stream, g_system->getOverlayFormat());
		in.skip(bmpSize);
	}

	// Set the graphics to the 'on' buttons 
	_whatButton->setGfx(arrayBmp[2*kMenuWhat]);
	_musicButton->setGfx(arrayBmp[2*kMenuMusic]);
	_soundFXButton->setGfx(arrayBmp[2*kMenuSoundFX]);
	_loadButton->setGfx(arrayBmp[2*kMenuLoad]);
	_saveButton->setGfx(arrayBmp[2*kMenuSave]);
	_recallButton->setGfx(arrayBmp[2*kMenuRecall]);
	_turboButton->setGfx(arrayBmp[2*kMenuTurbo]);
	_lookButton->setGfx(arrayBmp[2*kMenuLook]);
	_inventButton->setGfx(arrayBmp[2*kMenuInventory]);
}

} // End of namespace Hugo
