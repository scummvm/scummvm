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

#include "harvester/room_presentation.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/blit.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/flow.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
#include "harvester/room_support.h"

namespace Harvester {

static const byte kTransparentPaletteIndex = 0;
static const float kNormalPaletteBrightness = 1.0f;
static const float kDimPaletteBrightness = 0.6f;

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

static void setScaledRoomPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

void RoomPresentation::blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	int destX = x;
	int destY = y;
	int srcX = 0;
	int srcY = 0;
	int width = (int)bitmap.width;
	int height = (int)bitmap.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = bitmap.pixels.data() + srcY * bitmap.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, bitmap.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

RoomPresentation::RoomPresentation(HarvesterEngine &engine, SceneState &scene,
		const Art *art, const Graphics::Font *inspectFont, bool useNativeInspectFont)
	: _engine(engine), _scene(scene), _art(art), _inspectFont(inspectFont),
	  _useNativeInspectFont(useNativeInspectFont) {
}

bool RoomPresentation::captureDialogueBackdrop(IndexedBitmap &dialogueBackdrop) {
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return false;

	drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
	return captureScreenBackdrop(*activeScreen, dialogueBackdrop);
}

Common::Error RoomPresentation::applyLightingCommand(StartupLightingCommand lightingCommand) {
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return Common::kNoError;

	switch (lightingCommand) {
	case kStartupLightingCommandNone:
		return Common::kNoError;
	case kStartupLightingCommandDim:
		_scene.targetPaletteBrightness = kDimPaletteBrightness;
		drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
		setScaledRoomPalette(*activeScreen, _scene.palette, _scene.targetPaletteBrightness);
		activeScreen->makeAllDirty();
		activeScreen->update();
		return Common::kNoError;
	case kStartupLightingCommandNormal:
		if (ResourceManager *resources = _engine.getResources()) {
			if (!_scene.state.palettePath.empty() &&
					!loadPaletteResource(*resources, _scene.state.palettePath, _scene.palette)) {
				warning("Harvester: unable to reload room palette '%s' for CHANGE_LIGHTING NORMAL",
					_scene.state.palettePath.c_str());
			}
		}
		_scene.targetPaletteBrightness = kNormalPaletteBrightness;
		drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
		setScaledRoomPalette(*activeScreen, _scene.palette, _scene.targetPaletteBrightness);
		activeScreen->makeAllDirty();
		activeScreen->update();
		return Common::kNoError;
	case kStartupLightingCommandBlack: {
		byte blackPalette[256 * 3] = { 0 };
		activeScreen->fillRect(activeScreen->getBounds(), 0);
		activeScreen->setPalette(blackPalette);
		activeScreen->makeAllDirty();
		activeScreen->update();
		return Common::kNoError;
	}
	case kStartupLightingCommandFadeIn:
		debugC(1, kDebugRoom,
			"Harvester: CHANGE_LIGHTING FADE_IN has no direct room-side equivalent yet; preserving control flow");
		return Common::kNoError;
	}

	return Common::kNoError;
}

Common::Error RoomPresentation::runModalShowText(const ResolvedText &modalText, Flow &flow) {
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen || !_art || !_inspectFont)
		return Common::kNoError;
	if (!resolveInspectTextboxBitmap(*_art, modalText)) {
		debug(1, "Harvester: unsupported SHOW_TEXT textbox '%s' text='%s'",
			modalText.boxName.c_str(), modalText.value.c_str());
		return Common::kNoError;
	}

	IndexedBitmap backdrop;
	if (!captureDialogueBackdrop(backdrop))
		return Common::kReadingFailed;

	Graphics::FrameLimiter limiter(g_system, 60);
	bool initialInputReleased = false;
	bool dismissPressed = false;
	uint frameCount = 0;

	while (!_engine.shouldQuit()) {
		Common::Event event;
		Common::Error result = Common::kNoError;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (flow.handleSystemEvent(event, result))
				return result;

			if (!initialInputReleased) {
				if (event.type == Common::EVENT_LBUTTONUP ||
						event.type == Common::EVENT_RBUTTONUP ||
						event.type == Common::EVENT_KEYUP) {
					initialInputReleased = true;
				}
				continue;
			}

			if (!dismissPressed) {
				if (event.type == Common::EVENT_LBUTTONDOWN ||
						event.type == Common::EVENT_RBUTTONDOWN) {
					dismissPressed = true;
				} else if (event.type == Common::EVENT_KEYDOWN) {
					return Common::kNoError;
				}
				continue;
			}

			if (event.type == Common::EVENT_LBUTTONUP ||
					event.type == Common::EVENT_RBUTTONUP) {
				return Common::kNoError;
			}
		}

		if (!initialInputReleased && frameCount++ > 0)
			initialInputReleased = true;

		blitBitmap(*activeScreen, backdrop, 0, 0);
		setScaledRoomPalette(*activeScreen, _scene.palette, _scene.targetPaletteBrightness);
		drawRoomInspectText(*activeScreen, *_art, *_inspectFont, modalText, _useNativeInspectFont);
		activeScreen->makeAllDirty();
		activeScreen->update();

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

} // End of namespace Harvester
