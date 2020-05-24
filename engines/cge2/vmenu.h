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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_VMENU_H
#define CGE2_VMENU_H

#define kMenuBarVerticalMargin   1
#define kMenuBarHorizontalMargin 3
#define kLt                      3
#define kRb                      1

#include "cge2/cge2.h"
#include "cge2/talk.h"

namespace CGE2 {

class Choice {
protected:
	CGE2Engine *_vm;
public:
	char *_text;

	virtual void proc() = 0;

	Choice(CGE2Engine *vm);
	virtual ~Choice() {};
};

class ExitGameChoice : public Choice {
public:
	ExitGameChoice(CGE2Engine *vm);
	void proc() override;
};

class ReturnToGameChoice : public Choice {
public:
	ReturnToGameChoice(CGE2Engine *vm);
	void proc() override;
};

class MenuBar : public Talk {
public:
	MenuBar(CGE2Engine *vm, uint16 w, byte *c);
};

class VMenu : public Talk {
	CGE2Engine *_vm;

	uint16 _items;
	Common::Array<Choice *> _menu;
public:
	char *_vmgt;
	static VMenu *_addr;
	int _recent;
	MenuBar *_bar;

	VMenu(CGE2Engine *vm, Common::Array<Choice *> list, V2D pos, ColorBank col);
	~VMenu() override;
	void touch(uint16 mask, V2D pos, Common::KeyCode keyCode) override;
	char *vmGather(Common::Array<Choice *> list);
};

} // End of namespace CGE2

#endif // CGE2_VMENU_H
