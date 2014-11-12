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

#ifndef ZVISION_MENU_H
#define ZVISION_MENU_H

#include "graphics/surface.h"
#include "common/rect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"

namespace ZVision {

enum menuBar {
	menuBar_Exit = 0x1,
	menuBar_Settings = 0x2,
	menuBar_Restore = 0x4,
	menuBar_Save = 0x8,
	menuBar_Items = 0x100,
	menuBar_Magic = 0x200
};

class menuHandler {
public:
	menuHandler(ZVision *engine);
	virtual ~menuHandler() {};
	virtual void onMouseMove(const Common::Point &Pos) {};
	virtual void onMouseDown(const Common::Point &Pos) {};
	virtual void onMouseUp(const Common::Point &Pos) {};
	virtual void process(uint32 deltaTimeInMillis) {};

	void setEnable(uint16 flags) {
		menu_bar_flag = flags;
	}
	uint16 getEnable() {
		return menu_bar_flag;
	}
protected:
	uint16 menu_bar_flag;
	ZVision *_engine;
};

class menuZgi: public menuHandler {
public:
	menuZgi(ZVision *engine);
	~menuZgi();
	void onMouseMove(const Common::Point &Pos);
	void onMouseUp(const Common::Point &Pos);
	void process(uint32 deltaTimeInMillis);
private:
	Graphics::Surface menuback[3][2];
	Graphics::Surface menubar[4][2];


	Graphics::Surface *items[50][2];
	uint item_id[50];

	Graphics::Surface *magic[12][2];
	uint magic_id[12];

	int menu_mousefocus;
	bool inmenu;

	int mouse_on_item;

	bool   scrolled[3];
	float scrollPos[3];

	enum {
		menu_ITEM = 0,
		menu_MAGIC = 1,
		menu_MAIN = 2
	};

	bool clean;
	bool redraw;

};

class menuNem: public menuHandler {
public:
	menuNem(ZVision *engine);
	~menuNem();
	void onMouseMove(const Common::Point &Pos);
	void onMouseUp(const Common::Point &Pos);
	void process(uint32 deltaTimeInMillis);
private:
	Graphics::Surface but[4][6];
	Graphics::Surface menubar;

	bool inmenu;

	int mouse_on_item;

	bool   scrolled;
	float scrollPos;

	bool redraw;

	int frm;
	int16 delay;

};

}

#endif
