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

#ifndef HUGO_TOPMENU_H
#define HUGO_TOPMENU_H

#include "gui/dialog.h"

namespace Hugo {

enum MenuOption {
	kMenuWhat = 0,
	kMenuMusic,
	kMenuSoundFX,
	kMenuLoad,
	kMenuSave,
	kMenuRecall,
	kMenuTurbo,
	kMenuLook,
	kMenuInventory
};

class TopMenu : public GUI::Dialog {
public:
	TopMenu(HugoEngine *vm);
	~TopMenu();

	void reflowLayout();
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data);
	void handleMouseUp(int x, int y, int button, int clickCount);

	void loadBmpArr(Common::File &in);

protected:
	void init();

	HugoEngine *_vm;

	GUI::PicButtonWidget *_whatButton;
	GUI::PicButtonWidget *_musicButton;
	GUI::PicButtonWidget *_soundFXButton;
	GUI::PicButtonWidget *_loadButton;
	GUI::PicButtonWidget *_saveButton;
	GUI::PicButtonWidget *_recallButton;
	GUI::PicButtonWidget *_turboButton;
	GUI::PicButtonWidget *_lookButton;
	GUI::PicButtonWidget *_inventButton;

	Graphics::Surface **arrayBmp;
	uint16 arraySize;
};

}

#endif // HUGO_TOPMENU_H
