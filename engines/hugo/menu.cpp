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

namespace Hugo {

enum {
	kMenuX = 5,
	kMenuY = 1,
	kButtonWidth = 18,
	kButtonHeight = 18,
	kButtonPad = 1,
	kButtonSpace = 5
};

enum {
	kCmdWhat = 'WHAT',
	kCmdMusic = 'MUZK',
	kCmdVolume = 'VOLM',
	kCmdLoad = 'LOAD',
	kCmdSave = 'SAVE',
	kCmdUndo = 'UNDO',
	kCmdText = 'TEXT',
	kCmdLook = 'LOOK',
	kCmdBomb = 'BOMB'
};

TopMenu::TopMenu(HugoEngine *vm) : Dialog(0, 0, 320, 20),
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

	_volumeButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Volume", kCmdVolume);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_loadButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Load game", kCmdLoad);
	x += kButtonWidth + kButtonPad;

	_saveButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Save game", kCmdSave);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_undoButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Undo", kCmdUndo);
	x += kButtonWidth + kButtonPad;
	
	_textButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Text", kCmdText);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;
	
	_lookButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Look", kCmdLook);
	x += kButtonWidth + kButtonPad;
	
	_bombButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Bomb", kCmdBomb);
	x += kButtonWidth + kButtonPad;

}

} // End of namespace Hugo
