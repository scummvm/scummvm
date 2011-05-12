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
#include "video/qt_decoder.h"

#include "pegasus/pegasus.h"

namespace Pegasus {

enum {
	kCreditsCore = 0,
	kCreditsSupport,
	kCreditsOriginal,
	kCreditsTalent,
	kCreditsOther,
	kCreditsMainMenu
};

static const int s_startCreditsSegment[] = { 0, 16, 25, 37, 39 };

static int findButtonForFrame(int frame) {
	int button = kCreditsCore;
	for (int i = kCreditsCore; i < kCreditsMainMenu; i++)
		if (frame >= s_startCreditsSegment[i])
			button = i;

	return button;
}

void PegasusEngine::runCredits() {
	Video::QuickTimeDecoder *creditsVideo = new Video::QuickTimeDecoder();
	if (!creditsVideo->loadFile("Images/Credits/Credits.movie"))
		error("Could not open credits movie");

	// We're not playing, just retrieving frames
	creditsVideo->pauseVideo(true);

	int curButton = kCreditsCore;
	int frame = 0;

	drawCredits(curButton, false, frame, creditsVideo);
	_system->updateScreen();

	bool continueLooping = true;
	while (!shouldQuit() && continueLooping) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			bool needsUpdate = false;

			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_UP:
					if (curButton != kCreditsCore)
						curButton--;
					frame = s_startCreditsSegment[curButton];
					needsUpdate = true;
					break;
				case Common::KEYCODE_DOWN:
					if (curButton != kCreditsMainMenu) {
						curButton++;
						if (curButton == kCreditsMainMenu)
							frame = 43;
						else
							frame = s_startCreditsSegment[curButton];
						needsUpdate = true;
					}
					break;
				case Common::KEYCODE_LEFT:
					if (frame > 0) {
						frame--;
						curButton = findButtonForFrame(frame);
						needsUpdate = true;
					}
					break;
				case Common::KEYCODE_RIGHT:
					if (frame < 43) {
						frame++;
						curButton = findButtonForFrame(frame);
						needsUpdate = true;
					}
					break;
				case Common::KEYCODE_RETURN:
					drawCredits(curButton, true, frame, creditsVideo);
					_system->updateScreen();
					continueLooping = (curButton != kCreditsMainMenu);
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}

			if (needsUpdate) {
				drawCredits(curButton, false, frame, creditsVideo);
				_system->updateScreen();
			}
		}

		_system->delayMillis(10);
	}

	delete creditsVideo;
}

void PegasusEngine::drawCredits(int button, bool highlight, int frame, Video::QuickTimeDecoder *video) {
	static const int s_creditsButtonY[] = { 224, 260, 296, 332, 366, 407 };

	_gfx->drawPict("Images/Credits/CredScrn.pict", 0, 0, false);

	if (highlight)
		_gfx->drawPict("Images/Credits/MainMenu.pict", 32, 412, false);

	if (button == kCreditsMainMenu)
		_gfx->drawPictTransparent("Images/Credits/SelectL.pict", 30, s_creditsButtonY[button], _gfx->getColor(0xf8, 0xf8, 0xf8));
	else
		_gfx->drawPictTransparent("Images/Credits/SelectS.pict", 40, s_creditsButtonY[button], _gfx->getColor(0xf8, 0xf8, 0xf8));

	video->seekToTime(frame * 200);
	_video->copyFrameToScreen(video->decodeNextFrame(), video->getWidth(), video->getHeight(), 288, 0);
}

void PegasusEngine::runDemoCredits() {
	_gfx->drawPict("Images/Demo/DemoCredits.pict", 0, 0, true);

	bool continueLooping = true;
	while (!shouldQuit() && continueLooping) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_system->updateScreen();
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
		}

		_system->delayMillis(10);
	}
}

} // End of namespace Pegasus
