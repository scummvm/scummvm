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

#include "scummhelp.h"
#include "common/translation.h"

#define ADD_BIND(k,d) do { key[i] = k; dsc[i] = d; i++; } while (0)
#define ADD_TEXT(d) ADD_BIND("",d)
#define ADD_LINE ADD_BIND("","")

#define HELP_NUM_LINES 15

namespace DS {

void updateStrings(byte gameId, byte version, Common::Platform platform,
			int page, Common::U32String &title, Common::U32String *&key, Common::U32String *&dsc) {
	key = new Common::U32String[HELP_NUM_LINES];
	dsc = new Common::U32String[HELP_NUM_LINES];
	int i = 0;
	switch (page) {
	case 1: {
		title = _("DS Controls (right handed):");
		ADD_BIND(_("Pad Left"), _("Left mouse button"));
		ADD_BIND(_("Pad Right"), _("Right mouse button"));
		ADD_BIND(_("Pad Up"), _("Mouse hover mode (no click)"));
		ADD_BIND(_("Pad Down"), _("Skip dialog line (some games)"));
		ADD_BIND(_("Start"), _("Pause/Game menu"));
		ADD_BIND(_("Select"), _("DS Options menu"));
		ADD_BIND("B", _("Skip cutscenes"));
		ADD_BIND("A", _("Switch screens"));
		ADD_BIND("Y", _("Show/hide debug console"));
		ADD_BIND("X", _("Show/hide keyboard"));
		ADD_BIND("L+Pad/Pen", _("Scroll current touch screen view"));
		ADD_BIND("L+B/A", _("Zoom in/out"));
		break;
	}

	case 2: {
		title = _("DS Controls (left handed):");
		ADD_BIND("Y", _("Left mouse button"));
		ADD_BIND("A", _("Right mouse button"));
		ADD_BIND("X", _("Mouse hover mode (no click)"));
		ADD_BIND("B", _("Skip dialog line (some games)"));
		ADD_BIND(_("Start"), _("Pause/Game menu"));
		ADD_BIND(_("Select"), _("DS Options menu"));
		ADD_BIND(_("Pad Down"), _("Skip cutscenes"));
		ADD_BIND(_("Pad Up"), _("Show/hide keyboard"));
		ADD_BIND(_("Pad Left"), _("Show/hide debug console"));
		ADD_BIND(_("Pad Right"), _("Swap screens"));
		ADD_BIND("R+Pad/Pen", _("Scroll current touch screen view"));
		ADD_BIND("R+dwn/rgt", _("Zoom in/out"));
		break;
	}

	case 3: {
		title = _("Indiana Jones Fight controls:");
		ADD_BIND(_("Pad Left"), _("Move left"));
		ADD_BIND(_("Pad Right"), _("Move right"));
		ADD_BIND(_("Pad Up"), _("High guard"));
		ADD_BIND(_("Pad Down"), _("Guard down"));
		ADD_BIND("Y", _("Guard middle"));
		ADD_BIND("X", _("Punch high"));
		ADD_BIND("A", _("Punch middle"));
		ADD_BIND("B", _("Punch low"));
		break;
	}
	}


	while (i < HELP_NUM_LINES) {
		ADD_LINE;
	}

}

} // End of namespace DS



#undef ADD_BIND
#undef ADD_TEXT
#undef ADD_LINE
