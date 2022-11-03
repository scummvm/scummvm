/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/jacl/jacl.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"
#include "glk/jacl/language.h"

namespace Glk {
namespace JACL {

extern char                     user_id[];
extern char                     prefix[];

void log_error(const char *message, int console) {
	/* LOG A MESSAGE TO THE CONSOLE */

	char            consoleMessage[256];
	event_t         event;

	// BUILD A STRING SUITABLE FOR DISPLAY ON THE CONSOLE.
	Common::sprintf_s(consoleMessage, "ERROR: %s^", message);

	g_vm->glk_set_style(style_Alert);
	write_text(consoleMessage);
	g_vm->glk_set_style(style_Normal);

	// FLUSH THE GLK WINDOW SO THE ERROR GETS DISPLAYED IMMEDIATELY.
	g_vm->glk_select_poll(&event);
}

} // End of namespace JACL
} // End of namespace Glk
