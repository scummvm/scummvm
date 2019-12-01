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

#ifndef ULTIMA8_GUMPS_U8SAVEGUMP_H
#define ULTIMA8_GUMPS_U8SAVEGUMP_H

#include "ultima8/gumps/gump.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

// U8-style load/save gump.

class EditWidget;

class U8SaveGump : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	U8SaveGump(bool save, int page);
	virtual ~U8SaveGump();

	virtual void InitGump(Gump *newparent, bool take_focus = true);
	virtual void Close(bool no_del = false);

	virtual Gump *OnMouseDown(int button, int mx, int my);
	virtual void OnMouseClick(int button, int mx, int my);
	virtual bool OnKeyDown(int key, int mod);
	virtual void ChildNotify(Gump *child, uint32 message);
	virtual void OnFocus(bool gain);

	static std::string getFilename(int index);

	static Gump *showLoadSaveGump(Gump *parent, bool save);

protected:
	bool save;
	int page;

	std::vector<EditWidget *> editwidgets;
	std::vector<std::string> descriptions;

	void loadDescriptions();

	bool loadgame(int index);
	bool savegame(int index, const std::string &name);
};

} // End of namespace Ultima8

#endif
