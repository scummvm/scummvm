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

#include "harvester/menu/menu.h"
#include "harvester/menu/menu_render.h"

#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/framelimiter.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/entity.h"
#include "harvester/art.h"
#include "harvester/flow/flow.h"

namespace Harvester {

namespace {

static const char *const kCdChangePromptPalettePath = "1:/GRAPHIC/PAL/CD1.PAL";

} // End of anonymous namespace

bool MenuSystem::loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

bool MenuSystem::loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	return decodeBitmap(data, bitmap);
}

int MenuSystem::getNativeRoomMenuSelectionFromMouse(const Graphics::Font &selectedFont, uint itemCount,
		const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;

	int selection = mousePos.y - MenuRendering::kMenuStartY;
	if (selection < 1)
		selection = 1;

	selection /= MAX<int>(1, MenuRendering::getNativeRoomMenuLineHeight(selectedFont));
	if (selection >= static_cast<int>(itemCount))
		selection = static_cast<int>(itemCount) - 1;

	return selection;
}

MenuSystem::MenuSystem(HarvesterEngine &engine, Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems)
	: _engine(engine), _mousePos(mousePos), _menuItems(menuItems) {
}

Common::Error MenuSystem::showCdChangePrompt(int discNumber, ResourceManager &resources,
		Flow &flow, bool &promptCompleted) {
	promptCompleted = false;
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen)
		return Common::kReadingFailed;

	const Common::String bitmapPath = Common::String::format("1:/GRAPHIC/OTHER/CD%d.BM", discNumber);
	IndexedBitmap promptBitmap;
	byte promptPalette[256 * 3];
	if (!loadBitmapResource(resources, bitmapPath, promptBitmap) ||
			!loadPaletteResource(resources, kCdChangePromptPalettePath, promptPalette)) {
		warning("Harvester: unable to load CD change prompt assets for disc %d", discNumber);
		return Common::kReadingFailed;
	}

	debugC(1, kDebugScene, "Harvester: showing CD change prompt for disc %d", discNumber);
	_engine.stopMusic();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		Common::Error result = Common::kNoError;
		if (flow.handleSystemEvent(event, result))
			return result;
		if (event.type == Common::EVENT_MOUSEMOVE)
			_mousePos = event.mouse;
	}

	bool waitingForRelease = false;
	bool needsRedraw = true;
	bool activateRequestedDisc = false;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit() && !activateRequestedDisc) {
		if (needsRedraw) {
			MenuRendering::renderCdChangePromptScreen(_engine, promptBitmap, promptPalette);
			needsRedraw = false;
		}

		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_mousePos = event.mouse;
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				waitingForRelease = true;
				break;
			case Common::EVENT_LBUTTONUP:
				if (waitingForRelease)
					activateRequestedDisc = true;
				break;
			default:
				break;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
	promptCompleted = true;
	return Common::kNoError;
}

Common::Error MenuSystem::runHelpScreen(const byte *palette, float paletteBrightness, Flow &flow) {
	ResourceManager *resources = _engine.getResources();
	if (!resources)
		return Common::kReadingFailed;

	IndexedBitmap mouseHelp;
	IndexedBitmap keysHelp;
	byte mouseHelpPalette[256 * 3];
	byte keysHelpPalette[256 * 3];
	if (!loadBitmapResource(*resources, "1:/GRAPHIC/OTHER/MOUSHELP.BM", mouseHelp) ||
			!loadBitmapResource(*resources, "1:/GRAPHIC/OTHER/KEYSHELP.BM", keysHelp) ||
			!loadPaletteResource(*resources, "1:/GRAPHIC/PAL/MOUSHELP.PAL", mouseHelpPalette) ||
			!loadPaletteResource(*resources, "1:/GRAPHIC/PAL/KEYSHELP.PAL", keysHelpPalette)) {
		return Common::kReadingFailed;
	}

	int page = 0;
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			MenuRendering::renderHelpScreen(_engine, page == 0 ? mouseHelp : keysHelp,
				page == 0 ? mouseHelpPalette : keysHelpPalette);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_RBUTTONDOWN:
				if (Graphics::Screen *screen = _engine.getScreen()) {
					MenuRendering::applyMenuPalette(*screen, _engine, palette, paletteBrightness);
					screen->fillRect(screen->getBounds(), 0);
				}
				return Common::kNoError;
			case Common::EVENT_LBUTTONDOWN:
				page ^= 1;
				needsRedraw = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					if (Graphics::Screen *screen = _engine.getScreen()) {
						MenuRendering::applyMenuPalette(*screen, _engine, palette, paletteBrightness);
						screen->fillRect(screen->getBounds(), 0);
					}
					return Common::kNoError;
				}
				page ^= 1;
				needsRedraw = true;
				break;
			default:
				break;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

} // End of namespace Harvester
