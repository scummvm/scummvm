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
	
	static const OverviewHotspot overviewHotspots[] = {
		{ Common::Rect(0, 0, 640, 480), 1000 },     // Main
		{ Common::Rect(131, 39, 212, 63), 660 },    // Date
		{ Common::Rect(210, 33, 326, 63), 330 },    // Compass
		{ Common::Rect(324, 39, 448, 63), 800 },    // Energy Bar
		{ Common::Rect(531, 35, 601, 57), 2330 },   // Energy Alert
		{ Common::Rect(63, 63, 577, 321), 1730 },   // View Window
		{ Common::Rect(69, 317, 355, 331), 1360 },  // Inventory Panel
		{ Common::Rect(353, 317, 559, 331), 130 },  // BioChip Panel
		{ Common::Rect(75, 333, 173, 431), 1460 },  // Inventory Box
		{ Common::Rect(171, 333, 365, 431), 2060 }, // Inventory/BioChip Display
		{ Common::Rect(363, 333, 461, 431), 1860 }, // BioChip Box
		{ Common::Rect(540, 348, 640, 468), 530 },  // Keyboard
	};

	// Draw the rest of the interface
	int curHotspot;
	for (int i = 0; i < ARRAYSIZE(overviewHotspots); i++)
		if (overviewHotspots[i].rect.contains(_eventMan->getMousePos()))
			curHotspot = i;
	drawInterfaceOverview(overviewHotspots[curHotspot], overviewVideo);
	_system->updateScreen();

	bool continueLooping = true;
	while (!shouldQuit() && continueLooping) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			bool updateScreen = false;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				updateScreen = true;
				break;
			case Common::EVENT_KEYDOWN:
				// Break on any keypress, but ignore the meta keys
				// Except for num lock! num lock on OS9 is 'clear' and we need that for the inventory panel (along with the tilde)
				continueLooping = (event.kbd.keycode == Common::KEYCODE_INVALID || (event.kbd.keycode >= Common::KEYCODE_CAPSLOCK && event.kbd.keycode <= Common::KEYCODE_COMPOSE));
				break;
			case Common::EVENT_LBUTTONDOWN:
				continueLooping = false;
				break;
			default:
				break;
			}

			int oldHotspot = curHotspot;

			for (int i = 0; i < ARRAYSIZE(overviewHotspots); i++)
				if (overviewHotspots[i].rect.contains(_eventMan->getMousePos()))
					curHotspot = i;

			if (oldHotspot != curHotspot) {
				drawInterfaceOverview(overviewHotspots[curHotspot], overviewVideo);
				updateScreen = true;
			}

			if (updateScreen)
				_system->updateScreen();
		}

		_system->delayMillis(10);
	}

	CursorMan.showMouse(false);
	delete overviewVideo;
}

void PegasusEngine::drawInterfaceOverview(const OverviewHotspot &hotspot, Video::QuickTimeDecoder *video) {
	_gfx->drawPict("Images/Interface/OVTop.mac", 0, 0, false);
	_gfx->drawPict("Images/Interface/OVLeft.mac", 0, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/OVRight.mac", 640 - kViewScreenOffset, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/OVBottom.mac", 0, kViewScreenOffset + 256, false);

	video->seekToTime(hotspot.time);
	_video->copyFrameToScreen(video->decodeNextFrame(), video->getWidth(), video->getHeight(), kViewScreenOffset, kViewScreenOffset);

	if (hotspot.time == 530) {
		// The keyboard is special
		// Interesting how the file is "controller" and not keyboard. The PlayStation/Pippin versions probably
		// had similar names...
		_gfx->drawPict("Images/Interface/OVcontrollerHilite.mac", hotspot.rect.left, hotspot.rect.top, false);
	} else if (hotspot.time != 1000) {
		// TODO: Thicker line (=4px) with rounded edges
		uint32 color = _system->getScreenFormat().RGBToColor(232, 232, 0); // Yellow
		Graphics::Surface *screen = _system->lockScreen();
		screen->frameRect(hotspot.rect, color);
		_system->unlockScreen();
	}
}

} // End of namespace Pegasus
