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
		"<color name = 'blue' rgb = '0, 255, 255' />"
	"</palette>"
	
	"<fonts>"
		"<font id = 'text_default' type = 'default' color = '0, 0, 0' />"
		"<font id = 'text_hover' type = 'default' color = '255, 255, 255' />"
		"<font id = 'text_disabled' type = 'default' color = '128, 128, 128' />"
		"<font id = 'text_inverted' type = 'default' color = '173, 40, 8' />"
	"</fonts>"

	"<defaults fill = 'gradient' fg_color = '255, 255, 255' />"
	
	"<drawdata id = 'text_selection' cache = false>"
		"<drawstep func = 'roundedsq' radius = 4 fill = 'foreground' fg_color = '255, 255, 255' />"
	"</drawdata>"

	"<drawdata id = 'mainmenu_bg' cache = false>"
		"<drawstep func = 'fill' fill = 'gradient' gradient_start = '214, 113, 8' gradient_end = '240, 200, 25' />"
	"</drawdata>"
	
	"<drawdata id = 'separator' cache = false>"
		"<drawstep func = 'square' fill = 'foreground' height = '3' ypos = 'center' fg_color = '0, 0, 0' />"
	"</drawdata>"
	
	"<drawdata id = 'scrollbar_base' cache = false>"
		"<drawstep func = 'roundedsq' stroke = 1 radius = 6 fill = 'background' fg_color = '120, 120, 120' bg_color = '255, 243, 206' />"
	"</drawdata>"
	
	"<drawdata id = 'scrollbar_handle_hover' cache = false>"
		"<drawstep func = 'roundedsq' stroke = 1 radius = 6 fill = 'gradient' fg_color = '255, 255, 255' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' />"
	"</drawdata>"
	
	"<drawdata id = 'scrollbar_handle_idle' cache = false>"
		"<drawstep func = 'roundedsq' stroke = 1 radius = 6 fill = 'background' fg_color = '120, 120, 120' bg_color = '255, 255, 255' />"
	"</drawdata>"
	
	"<drawdata id = 'scrollbar_button_idle' cache = false>"
		"<drawstep func = 'roundedsq' radius = '4' fill = 'none' fg_color = '120, 120, 120' stroke = 1 />"
		"<drawstep func = 'triangle' fg_color = '0, 0, 0' fill = 'foreground' width = '9' height = '7' xpos = 'center' ypos = 'center' orientation = 'top' />"
	"</drawdata>"
	
	"<drawdata id = 'scrollbar_button_hover' cache = false>"
	"<drawstep func = 'roundedsq' radius = '4' fill = 'background' fg_color = '120, 120, 120' bg_color = '206, 121, 99' stroke = 1 />"
		"<drawstep func = 'triangle' fg_color = '0, 0, 0' fill = 'foreground' width = '9' height = '7' xpos = 'center' ypos = 'center' orientation = 'top' />"
	"</drawdata>"
	
	"<drawdata id = 'tab_active' cache = false>"
		"<text font = 'text_default' vertical_align = 'center' horizontal_align = 'center' />"
		"<drawstep func = 'tab' radius = '4' stroke = '2' fill = 'gradient' gradient_end = '255, 231, 140' gradient_start = '255, 243, 206' shadow = 3 />"
	"</drawdata>"
	
	"<drawdata id = 'tab_inactive' cache = false>"
		"<text font = 'text_disabled' vertical_align = 'center' horizontal_align = 'center' />"
		"<drawstep func = 'tab' radius = '4' stroke = '0' fill = 'foreground' fg_color = '206, 121, 99' shadow = 3 />"
	"</drawdata>"
	
	"<drawdata id = 'widget_slider' cache = false>"
		"<drawstep func = 'roundedsq' stroke = 1 radius = 8 fill = 'none' fg_color = '0, 0, 0' />"
	"</drawdata>"
	
	"<drawdata id = 'slider_full' cache = false>"
		"<drawstep func = 'roundedsq' stroke = 1 radius = 8 fill = 'gradient' fg_color = '0, 0, 0' gradient_start = '214, 113, 8' gradient_end = '240, 200, 25' />"
	"</drawdata>"
	
	"<drawdata id = 'popup_idle' cache = false>"
		"<drawstep func = 'square' stroke = 0 fg_color = '0, 0, 0' fill = 'gradient' gradient_start = '214, 113, 8' gradient_end = '240, 200, 25' shadow = 3 />"
		"<drawstep func = 'triangle' fg_color = '0, 0, 0' fill = 'foreground' width = '12' height = '12' xpos = '-16' ypos = 'center' orientation = 'bottom' />"
		"<text font = 'text_default' vertical_align = 'center' horizontal_align = 'right'/>"
	"</drawdata>"
	
	
	"<drawdata id = 'popup_hover' cache = false>"
		"<drawstep func = 'square' stroke = 0 fg_color = '0, 0, 0' fill = 'gradient' gradient_start = '214, 113, 8' gradient_end = '240, 200, 25' shadow = 0 />"
		"<drawstep func = 'triangle' fg_color = '0, 0, 0' fill = 'foreground' width = '12' height = '12' xpos = '-16' ypos = 'center' orientation = 'bottom' />"
		"<text font = 'text_hover' vertical_align = 'center' horizontal_align = 'right' />"
	"</drawdata>"
	
	"<drawdata id = 'default_bg' cache = false>"
		"<drawstep func = 'roundedsq' radius = 12 stroke = 2 fg_color = '255, 255, 255' fill = 'gradient' gradient_start = '255, 231, 140' gradient_end = '255, 243, 206' shadow = 3 />"
	"</drawdata>"

	"<drawdata id = 'button_idle' cache = false>"
		"<text font = 'text_default' vertical_align = 'center' horizontal_align = 'center' />"
		"<drawstep func = 'roundedsq' radius = '8' stroke = 0 fill = 'foreground' shadow = 3 />"
	"</drawdata>"

	"<drawdata id = 'button_hover' cache = false>"
		"<text font = 'text_hover' vertical_align = 'center' horizontal_align = 'center' />"
		"<drawstep func = 'roundedsq' radius = '8' stroke = '1' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 0 />"
	"</drawdata>"
	
	"<drawdata id = 'button_disabled' cache = false>"
		"<text font = 'text_disabled' vertical_align = 'center' horizontal_align = 'center' />"
		"<drawstep func = 'roundedsq' radius = '8' stroke = 0 fill = 'foreground' fg_color = '200, 200, 200' shadow = 3 />"
	"</drawdata>"

	"<drawdata id = 'checkbox_disabled' cache = false>"
		"<text font = 'text_disabled' vertical_align = 'top' horizontal_align = 'left' />"
		"<drawstep func = 'square' fill = 'gradient' gradient_start = '206, 121, 99' gradient_end = '173, 40, 8' shadow = 0 />"
	"</drawdata>"

	"<drawdata id = 'checkbox_enabled' cache = false>"
		"<text font = 'text_default' vertical_align = 'top' horizontal_align = 'left' />"
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
