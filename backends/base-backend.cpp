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

#include "backends/base-backend.h"

#include "graphics/pixelbuffer.h"
#include "graphics/scalerplugin.h"

#ifndef DISABLE_DEFAULT_EVENT_MANAGER
#include "backends/events/default/default-events.h"
#endif

#ifndef DISABLE_DEFAULT_AUDIOCD_MANAGER
#include "backends/audiocd/default/default-audiocd.h"
#endif


#include "gui/message.h"

bool BaseBackend::setScaler(const char *name, int factor) {
	if (!name)
		return false;

	if (!scumm_stricmp(name, "default"))
		return setScaler(getDefaultScaler(), factor);

	const PluginList &scalerPlugins = ScalerMan.getPlugins();

	for (uint scalerIndex = 0; scalerIndex < scalerPlugins.size(); scalerIndex++) {
		if (!scumm_stricmp(scalerPlugins[scalerIndex]->get<ScalerPluginObject>().getName(), name)) {
			return setScaler(scalerIndex, factor);
		}
	}

	return false;
}

void BaseBackend::displayMessageOnOSD(const Common::U32String &msg) {
	// Display the message for 1.5 seconds
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
}

void BaseBackend::initBackend() {
	// Init audio CD manager
#ifndef DISABLE_DEFAULT_AUDIOCD_MANAGER
	if (!_audiocdManager)
		_audiocdManager = new DefaultAudioCDManager();
#endif

	OSystem::initBackend();
}

void BaseBackend::fillScreen(uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(Common::Rect(screen->w, screen->h), col);
	unlockScreen();
}

void EventsBaseBackend::initBackend() {
	// Init Event manager
#ifndef DISABLE_DEFAULT_EVENT_MANAGER
	if (!_eventManager)
		_eventManager = new DefaultEventManager(this);
#endif

	BaseBackend::initBackend();
}
