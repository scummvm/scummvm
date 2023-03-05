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

#include "common/rect.h"

#include "engines/stark/ui/world/fmvscreen.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/bitmap.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/settings.h"

namespace Stark {

FMVScreen::FMVScreen(Gfx::Driver *gfx, Cursor *cursor) :
		SingleWindowScreen(Screen::kScreenFMV, gfx, cursor) {
	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kOriginalHeight);
	_visible = true;

	_bitmap = _gfx->createBitmap();
	_bitmap->setSamplingFilter(StarkSettings->getImageSamplingFilter());

	_decoder = new Video::BinkDecoder();
	_decoder->setDefaultHighColorFormat(_bitmap->getBestPixelFormat());
	_decoder->setSoundType(Audio::Mixer::kSFXSoundType);

	_surfaceRenderer = _gfx->createSurfaceRenderer();
}

FMVScreen::~FMVScreen() {
	delete _decoder;
	delete _bitmap;
	delete _surfaceRenderer;
}

void FMVScreen::play(const Common::String &name) {
	Common::SeekableReadStream *stream = nullptr;

	// Play the low-resolution video, if possible
	if (!StarkSettings->getBoolSetting(Settings::kHighFMV) && StarkSettings->hasLowResFMV()) {
		Common::String lowResName = name;
		lowResName.erase(lowResName.size() - 4);
		lowResName += "_lo_res.bbb";

		stream = StarkArchiveLoader->getExternalFile(lowResName, "Global/");
		if (!stream) {
			debug("Could not open %s", lowResName.c_str());
		}
	}

	// Play the original video
	if (!stream) {
		stream = StarkArchiveLoader->getExternalFile(name, "Global/");
	}

	if (!stream) {
		warning("Could not open %s", name.c_str());
		return;
	}

	_decoder->loadStream(stream);
	if (!_decoder->isVideoLoaded()) {
		error("Could not open %s", name.c_str());
	}
	_decoder->start();
}

void FMVScreen::onGameLoop() {
	if (isPlaying()) {
		if (_decoder->needsUpdate()) {
			const Graphics::Surface *decodedSurface = _decoder->decodeNextFrame();
			_bitmap->update(decodedSurface);
		}
	} else {
		stop();
	}
}

void FMVScreen::onRender() {
	_surfaceRenderer->render(_bitmap, Common::Point(0, Gfx::Driver::kTopBorderHeight),
			Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
}

bool FMVScreen::isPlaying() {
	return _decoder->isPlaying() && !_decoder->endOfVideo();
}

void FMVScreen::stop() {
	_decoder->stop();
	StarkUserInterface->onFMVStopped();
}


} // End of namespace Stark
