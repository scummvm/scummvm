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

/* These are return codes for widget event functions */

#ifndef NUVIE_GUI_GUI_STATUS_H
#define NUVIE_GUI_GUI_STATUS_H

namespace Ultima {
namespace Nuvie {

typedef enum {
	GUI_QUIT,               /* Close the GUI */
	GUI_REDRAW,           /* The GUI needs to be redrawn */
	GUI_YUM,                /* The event was eaten by the widget */
	GUI_PASS,               /* The event should be passed on */
	GUI_DRAG_AND_DROP /* Drag and Drop sequence initiated by GUI */
} GUI_status;

typedef enum {
	WIDGET_VISIBLE,
	WIDGET_HIDDEN,
	WIDGET_DELETED
} WIDGET_status;

/* GUI idle function -- run when no events pending */
typedef GUI_status(*GUI_IdleProc)(void);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
