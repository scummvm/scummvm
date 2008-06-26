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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "gui/InterfaceManager.h"

namespace GUI {

bool InterfaceManager::loadDefaultXML() {
	const char *defaultXML =
/**
 * Default theme description file. Work in progress.
 * Newlines are not necessary, parser ignores them.
 * You may use single quotes (') instead of scaped double quotes.
 * Remember to indent properly the XML so it's easier to read and
 * to maintain!
 */
"<render_info>"
	"<palette>"
		"<color name = 'red' rgb = '255, 0, 0' />"
		"<color name = 'green' rgb = '0, 255, 0' />"
		"<color name = 'blue' rgb = '0, 0, 255' />"
	"</palette>"
	"<default fill = 'gradient' fg_color = '255, 255, 255' />"
	"<drawdata id = 'mainmenu_bg' cache = false>"
		"<drawstep func = 'fill' fill = 'gradient' gradient_start = '214, 113, 8' gradient_end = '240, 200, 25' />"
	"</drawdata>"
	"<drawdata id = 'button_idle' cache = false>"
		"<drawstep func = 'roundedsq' radius = '8' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' size = 'auto' />"
	"</drawdata>"
"</render_info>";

	if (!parser()->loadBuffer(defaultXML, true))
		return false;

	return parser()->parse();
}

}
