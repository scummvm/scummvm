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

#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const uint32 kPaletteFadeIntervalTicks = 4;
static const float kPaletteBrightnessBlack = 0.0f;
// Keep WAIT_BM visible while room construction runs
static const uint32 kTransitionWaitFrameMinMs = 120;

static void setScaledDisplayPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float clampedBrightness = CLIP<float>(brightness, 0.0f, 1.0f);
	for (uint i = 0; i < ARRAYSIZE(scaledPalette); ++i)
		scaledPalette[i] = (byte)CLIP<int>((int)(palette[i] * clampedBrightness + 0.5f), 0, 255);
	screen.setPalette(scaledPalette);
}

} // End of anonymous namespace

void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

Common::Error Flow::showCdChangePrompt(int discNumber) {
	if (discNumber <= 0)
		return Common::kNoError;

	ResourceManager *resources = _engine.getResources();
	if (!resources)
		return Common::kReadingFailed;
	const int previousDisc = resources->getCurrentDisc();

	if (_engine.shouldShowCdChangePrompts()) {
		bool promptCompleted = false;
		Common::Error promptError = _menu.showCdChangePrompt(discNumber, *resources, *this, promptCompleted);
		// System-event exits can return kNoError without completing the prompt.
		if (promptError.getCode() != Common::kNoError || !promptCompleted)
			return promptError;
	}

	if (!_engine.activateDisc(discNumber)) {
		warning("Harvester: unable to activate disc %d resources", discNumber);
		return Common::kReadingFailed;
	}
	if ((discNumber == 1 || discNumber == 3) &&
			previousDisc > 0 && previousDisc != resources->getCurrentDisc()) {
		Script *script = _engine.getScript();
		if (!script || !script->reloadTownWorld(*resources)) {
			warning("Harvester: unable to reload town script after disc prompt %d -> %d",
				previousDisc, discNumber);
			return Common::kReadingFailed;
		}
	}

	return Common::kNoError;
}

Common::Error Flow::beginRoomSetupTransition() {
	Graphics::Screen *screen = _engine.getScreen();
	if (screen) {
		byte displayPalette[256 * 3];
		screen->getPalette(displayPalette);
		bool hasVisiblePalette = false;
		for (byte value : displayPalette) {
			if (value != 0) {
				hasVisiblePalette = true;
				break;
			}
		}

		if (hasVisiblePalette) {
			for (double brightness = 1.0;
					brightness > kPaletteBrightnessBlack; brightness -= 0.1) {
				const uint32 startTick = getRuntimeClockTicks();
				setScaledDisplayPalette(*screen, displayPalette, brightness);
				screen->makeAllDirty();
				screen->update();

				Common::Error fadeError = Common::kNoError;
				if (waitForPaletteFadeTick(startTick, fadeError))
					return fadeError;
			}

			setScaledDisplayPalette(*screen, displayPalette, kPaletteBrightnessBlack);
			screen->makeAllDirty();
			screen->update();
		}
	}

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->hideCursor();

	const Art *art = _engine.getArt();
	if (art && screen)
		art->drawWaitFrame(*screen);
	_roomSetupTransitionShownTick = g_system ? g_system->getMillis() : 0;

	Common::Error result = Common::kNoError;
	if (pumpTransitionEvents(result))
		return result;

	return Common::kNoError;
}

Common::Error Flow::waitForRoomSetupTransitionHold() {
	if (_roomSetupTransitionShownTick == 0 || !g_system)
		return Common::kNoError;

	const uint32 deadline = _roomSetupTransitionShownTick + kTransitionWaitFrameMinMs;
	while ((int32)(deadline - g_system->getMillis()) > 0) {
		Common::Error result = Common::kNoError;
		if (pumpTransitionEvents(result))
			return result;
		g_system->delayMillis(1);
	}

	_roomSetupTransitionShownTick = 0;
	return Common::kNoError;
}

Common::Error Flow::fadeInRoomScene(const byte *palette, float targetBrightness) {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen || !palette)
		return Common::kNoError;

	for (double brightness = 0.0; brightness < targetBrightness; brightness += 0.1) {
		const uint32 startTick = getRuntimeClockTicks();
		setScaledPalette(*screen, palette, brightness);
		screen->makeAllDirty();
		screen->update();

		Common::Error fadeError = Common::kNoError;
		if (waitForPaletteFadeTick(startTick, fadeError))
			return fadeError;
	}

	setScaledPalette(*screen, palette, targetBrightness);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

Common::Error Flow::fadePalette(const byte *palette, float fromBrightness,
		float toBrightness) {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen || !palette)
		return Common::kNoError;

	const double step = fromBrightness < toBrightness ? 0.1 : -0.1;
	for (double brightness = fromBrightness;
			step > 0.0f ? brightness < toBrightness : brightness > toBrightness;
			brightness += step) {
		const uint32 startTick = getRuntimeClockTicks();
		setScaledPalette(*screen, palette, brightness);
		screen->makeAllDirty();
		screen->update();

		Common::Error fadeError = Common::kNoError;
		if (waitForPaletteFadeTick(startTick, fadeError))
			return fadeError;
	}

	setScaledPalette(*screen, palette, toBrightness);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

bool Flow::waitForPaletteFadeTick(uint32 startTick, Common::Error &result) {
	const uint32 deadline = startTick + kPaletteFadeIntervalTicks;
	while ((int32)(deadline - getRuntimeClockTicks()) > 0) {
		if (pumpTransitionEvents(result))
			return true;
		g_system->delayMillis(1);
	}
	debugC(4, kDebugScene,
		"Harvester: palette fade step tick=%u->%u interval=%u",
		startTick, getRuntimeClockTicks(), kPaletteFadeIntervalTicks);
	return false;
}

bool Flow::pumpTransitionEvents(Common::Error &result) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (handleSystemEvent(event, result))
			return true;
	}

	return false;
}

} // End of namespace Harvester
