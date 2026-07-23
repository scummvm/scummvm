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
 */

#include "common/events.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "engines/dialogs.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/shared.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"

namespace Scumm {

static Rebel2MenuCommand getPSXMenuCommand(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat)
		return getRebel2MenuCommand(event.kbd);
	if (event.type != Common::EVENT_CUSTOM_ENGINE_ACTION_START)
		return kRebel2MenuCommandNone;

	switch (event.customType) {
	case kScummActionInsaneUp:
		return kRebel2MenuCommandUp;
	case kScummActionInsaneDown:
		return kRebel2MenuCommandDown;
	case kScummActionInsaneAttack:
		return kRebel2MenuCommandAccept;
	case kScummActionInsaneBack:
	case kScummActionInsaneSkip:
		return kRebel2MenuCommandCancel;
	default:
		return kRebel2MenuCommandNone;
	}
}

Rebel2PSX::MenuResult Rebel2PSX::runMainMenu(const RA2PSXMainMenuUI &ui) {
	Graphics::Surface surface;
	surface.create(_vm->_screenWidth, _vm->_screenHeight, g_system->getScreenFormat());

	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(true);
	int selection = 0;
	MenuResult result = kMenuQuit;
	bool redraw = true;

	while (!_vm->shouldQuit()) {
		bool openGlobalMenu = false;
		bool openOptions = false;
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT ||
					event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_vm->quitGame();
				break;
			}

			if (event.type == Common::EVENT_MAINMENU ||
					(event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat &&
					 event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
				openGlobalMenu = true;
				continue;
			}

			if (event.type == Common::EVENT_MOUSEMOVE ||
					event.type == Common::EVENT_LBUTTONDOWN) {
				const int xOffset = (surface.w - 320) / 2;
				const int yOffset = (surface.h - 240) / 2;
				for (int i = 0; i < 2; ++i) {
					Common::Rect rect = ui.itemRect(i);
					rect.translate(xOffset, yOffset);
					if (!rect.contains(event.mouse.x, event.mouse.y))
						continue;
					if (selection != i) {
						selection = i;
						redraw = true;
					}
					if (event.type == Common::EVENT_LBUTTONDOWN) {
						if (i == 0)
							result = kMenuStart;
						else
							openOptions = true;
					}
					break;
				}
			}

			const Rebel2MenuCommand command = getPSXMenuCommand(event);
			const int oldSelection = selection;
			const int commandResult = applyRebel2MenuCommand(command, 2, selection);
			if (selection != oldSelection)
				redraw = true;
			if (commandResult == kRebel2MenuResultCancel)
				openGlobalMenu = true;
			else if (commandResult == 0)
				result = kMenuStart;
			else if (commandResult == 1)
				openOptions = true;
		}

		if (_vm->shouldQuit())
			break;
		if (result == kMenuStart)
			break;
		if (openGlobalMenu) {
			_vm->openMainMenuDialog();
			redraw = true;
			continue;
		}
		if (openOptions) {
			GUI::ConfigDialog dialog;
			dialog.runModal();
			g_system->applyBackendSettings();
			_vm->syncSoundSettings();
			redraw = true;
			continue;
		}

		if (redraw) {
			surface.fillRect(Common::Rect(surface.w, surface.h), 0);
			ui.draw(surface, selection);
			g_system->copyRectToScreen(surface.getPixels(), surface.pitch,
					0, 0, surface.w, surface.h);
			g_system->updateScreen();
			redraw = false;
		}
		g_system->delayMillis(10);
	}

	surface.free();
	CursorMan.showMouse(cursorWasVisible);
	return _vm->shouldQuit() ? kMenuQuit : result;
}

} // End of namespace Scumm
