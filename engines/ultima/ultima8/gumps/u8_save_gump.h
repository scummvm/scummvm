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

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class EditWidget;

/**
 * U8-style load/save gump (a single number + editable widget entry in the paged save/load gump)
 */
class U8SaveGump : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	U8SaveGump(bool save, int page);
	~U8SaveGump() override;

	void InitGump(Gump *newparent, bool take_focus = true) override;
	void Close(bool no_del = false) override;

	Gump *onMouseDown(int button, int32 mx, int32 my) override;
	void onMouseClick(int button, int32 mx, int32 my) override;
	bool OnKeyDown(int key, int mod) override;
	void ChildNotify(Gump *child, uint32 message) override;
	void OnFocus(bool gain) override;

	static Gump *showLoadSaveGump(Gump *parent, bool save);

protected:
	bool _save;
	int _page;

	Std::vector<EditWidget *> _editWidgets;
	Std::vector<Std::string> _descriptions;

	void loadDescriptions();

	bool loadgame(int saveIndex);
	bool savegame(int saveIndex, const Std::string &name);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
