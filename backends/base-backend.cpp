/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/base-backend.h"
#include "backends/events/default/default-events.h"
#include "gui/message.h"

void BaseBackend::displayMessageOnOSD(const char *msg) {
	// Display the message for 1.5 seconds
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
}


static Common::EventManager *s_eventManager = 0;

Common::EventManager *BaseBackend::getEventManager() {
	// FIXME/TODO: Eventually this method should be turned into an abstract one,
	// to force backends to implement this conciously (even if they
	// end up returning the default event manager anyway).
	if (!s_eventManager)
		s_eventManager = new DefaultEventManager(this);
	return s_eventManager;
}

#if defined(UNIX)
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/Residual Preferences"
#else
#define DEFAULT_CONFIG_FILE ".residualrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "residual.ini"
#endif

Common::SeekableReadStream *BaseBackend::createConfigReadStream() {
	Common::FSNode file(DEFAULT_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *BaseBackend::createConfigWriteStream() {
#ifdef __DC__
	return 0;
#else
	Common::FSNode file(DEFAULT_CONFIG_FILE);
	return file.createWriteStream();
#endif
}
