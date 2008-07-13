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
#include "gui/ThemeRenderer.h"

namespace GUI {

bool ThemeRenderer::loadDefaultXML() {
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
	
	"<drawdata id = 'popup' cache = false>"
		"<drawstep func = 'square' stroke = 0 fg_color = '0, 0, 0' fill = 'gradient' gradient_start = '214, 113, 8' gradient_end = '240, 200, 25' shadow = 3 />"
		"<drawstep func = 'triangle' fg_color = '0, 0, 0' fill = 'foreground' width = '12' height = '12' xpos = '-16' ypos = 'center' orientation = 'bottom' />"
	"</drawdata>"
	
	"<drawdata id = 'default_bg' cache = false>"
		"<drawstep func = 'roundedsq' radius = 12 stroke = 4 fg_color = '206, 121, 99' fill = 'gradient' gradient_start = '255, 231, 140' gradient_end = '255, 243, 206' shadow = 3 />"
	"</drawdata>"

	"<drawdata id = 'button_idle' cache = false>"
		"<text vertical_align = 'center' horizontal_align = 'center' color = '255, 255, 255' />"
		"<drawstep func = 'roundedsq' radius = '8' stroke = 0 fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 3 />"
	"</drawdata>"

	"<drawdata id = 'button_hover' cache = false>"
		"<text vertical_align = 'center' horizontal_align = 'center' color = '255, 255, 255' />"
		"<drawstep func = 'roundedsq' radius = '8' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 3 />"
	"</drawdata>"

	"<drawdata id = 'checkbox_disabled' cache = false>"
		"<text vertical_align = 'top' horizontal_align = 'left' color = '255, 255, 255' />"
		"<drawstep func = 'square' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 0 />"
	"</drawdata>"

	"<drawdata id = 'checkbox_enabled' cache = false>"
		"<text vertical_align = 'top' horizontal_align = 'left' color = '255, 255, 255' />"
		"<drawstep func = 'square' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 0 />"
		"<drawstep func = 'circle' radius = 'auto' fill = 'foreground' />"
	"</drawdata>"
	
	"<drawdata id = 'widget_default' cache = false>"
		"<drawstep func = 'roundedsq' gradient_factor = 6 radius = '4' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 0 />"
	"</drawdata>"
"</render_info>"

"<layout_info>"
"</layout_info>";

	if (!parser()->loadBuffer((const byte*)defaultXML, strlen(defaultXML), false))
		return false;

	return parser()->parse();
}

}
