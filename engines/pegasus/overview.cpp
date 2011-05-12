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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "video/qt_decoder.h"

#include "pegasus/pegasus.h"

namespace Pegasus {

void PegasusEngine::runInterfaceOverview() {
	CursorMan.showMouse(true);
	_gfx->setCursor(kPointingCursor);

	Video::QuickTimeDecoder *overviewVideo = new Video::QuickTimeDecoder();
	if (!overviewVideo->loadFile("Images/Interface/Overview Mac.movie"))
		error("Could not open overview video");

	// Pause the video, we're only getting frames from it
	overviewVideo->pauseVideo(true);

	// Draw the main image
	overviewVideo->seekToTime(1000);
	_video->copyFrameToScreen(overviewVideo->decodeNextFrame(), overviewVideo->getWidth(), overviewVideo->getHeight(), kViewScreenOffset, kViewScreenOffset);

	// Draw the rest of the interface
	drawInterfaceOverview();

	bool continueLooping = true;
	while (!shouldQuit() && continueLooping) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				// TODO: Highlighted images and changing the viewscreen image
				_system->updateScreen();
				break;
			case Common::EVENT_KEYDOWN:
				// Break on any keypress, but ignore the meta keys
				// Except for num lock! num lock on OS9 is 'clear' and we need that for the inventory panel (along with the tilde)
				continueLooping = (event.kbd.keycode == Common::KEYCODE_INVALID || (event.kbd.keycode >= Common::KEYCODE_CAPSLOCK && event.kbd.keycode <= Common::KEYCODE_COMPOSE));
				break;
			default:
				break;
			}
		}

		_system->delayMillis(10);
	}

	CursorMan.showMouse(false);
	delete overviewVideo;
}

void PegasusEngine::drawInterfaceOverview() {
	_gfx->drawPict("Images/Interface/OVTop.mac", 0, 0, false);
	_gfx->drawPict("Images/Interface/OVLeft.mac", 0, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/OVRight.mac", 640 - kViewScreenOffset, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/OVBottom.mac", 0, kViewScreenOffset + 256, false);
	_system->updateScreen();
}

} // End of namespace Pegasus
