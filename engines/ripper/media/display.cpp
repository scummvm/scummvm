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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "image/pcx.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const int kScenePresentationTop = 50;
static const uint kPaletteFadeStepDelayMs = 16;

} // End of anonymous namespace

void MediaPlayer::fadePalette(bool fadeIn, uint stepCount) {
	if (stepCount == 0)
		return;

	byte targetPalette[Graphics::PALETTE_SIZE];
	byte fadePalette[Graphics::PALETTE_SIZE];
	PaletteManager *paletteManager = g_system->getPaletteManager();
	paletteManager->grabPalette(targetPalette, 0, Graphics::PALETTE_COUNT);

	for (uint step = 1; step <= stepCount; ++step) {
		const uint scale = fadeIn ? step : stepCount - step;
		for (uint component = 0; component < Graphics::PALETTE_SIZE; ++component)
			fadePalette[component] = (byte)((uint64)targetPalette[component] * scale / stepCount);
		paletteManager->setPalette(fadePalette, 0, Graphics::PALETTE_COUNT);
		presentScreen();
		if (step != stepCount)
			g_system->delayMillis(kPaletteFadeStepDelayMs);
	}
}

bool MediaPlayer::displayScenePcx(const Common::String &path) {
	Common::String source;
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		openSource(path, kSourceConfiguredPath, source));
	Image::PCXDecoder decoder;
	if (!stream || !decoder.loadStream(*stream)) {
		warning("Ripper: could not decode scene PCX '%s'", path.c_str());
		return false;
	}

	const Graphics::Surface *surface = decoder.getSurface();
	const Graphics::Palette &sourcePalette = decoder.getPalette();
	if (!surface || surface->format.bytesPerPixel != 1 ||
			surface->w <= 0 || surface->w > 640 ||
			surface->h <= 0 || surface->h > 300 ||
			sourcePalette.size() < 256) {
		warning("Ripper: invalid scene PCX '%s' size=%dx%d colors=%u",
			path.c_str(), surface ? surface->w : 0, surface ? surface->h : 0,
			sourcePalette.size());
		return false;
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w < 640 || screen->h < 400) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = kScenePresentationTop; y < kScenePresentationTop + 300; ++y)
		memset(screen->getBasePtr(0, y), 0, 640);
	const int x = (640 - surface->w) / 2;
	for (int y = 0; y < surface->h; ++y)
		memcpy(screen->getBasePtr(x, kScenePresentationTop + y),
			surface->getBasePtr(0, y), surface->w);
	g_system->unlockScreen();

	byte palette[256 * 3];
	memcpy(palette, sourcePalette.data(), sizeof(palette));
	_engine->getToolbar()->applySharedPalettePatch(palette, 256);
	_engine->getSettings()->applyVideoPalette(palette, 256, true);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	_engine->getCursor()->refresh();
	presentScreen();
	debugC(1, kDebugVideo,
		"Ripper: displayed scene PCX '%s' source=%ux%u destination=%d,%d interfacePalettePatch=1",
		path.c_str(), surface->w, surface->h, x, kScenePresentationTop);
	return true;
}

} // End of namespace Ripper
