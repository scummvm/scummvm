/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "common/rect.h"

#include "engines/stark/ui/world/fmvscreen.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/settings.h"

namespace Stark {

FMVScreen::FMVScreen(Gfx::Driver *gfx, Cursor *cursor) :
		SingleWindowScreen(Screen::kScreenFMV, gfx, cursor) {
	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kOriginalHeight);
	_visible = true;

	_decoder = new Video::BinkDecoder();
	_decoder->setDefaultHighColorFormat(Gfx::Driver::getRGBAPixelFormat());
	_decoder->setSoundType(Audio::Mixer::kSFXSoundType);

	_texture = _gfx->createTexture();
	_surfaceRenderer = _gfx->createSurfaceRenderer();
}

FMVScreen::~FMVScreen() {
	delete _decoder;
	delete _texture;
	delete _surfaceRenderer;
}

void FMVScreen::play(Common::String name) {
	if (!StarkSettings->getBoolSetting(Settings::kHighFMV) && StarkSettings->hasLowResFMV()) {
		name.erase(name.size() - 4);
		name += "_lo_res.bbb";
	}

	Common::SeekableReadStream *stream = StarkArchiveLoader->getExternalFile(name, "Global/");
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

void FMVScreen::onRender() {
	// TODO: Refactor this into an update method
	if (isPlaying()) {
		if (_decoder->needsUpdate()) {
			const Graphics::Surface *decodedSurface = _decoder->decodeNextFrame();
			_texture->update(decodedSurface);
		}
	} else {
		stop();
	}

	_surfaceRenderer->render(_texture, Common::Point(0, Gfx::Driver::kTopBorderHeight));
}

bool FMVScreen::isPlaying() {
	return _decoder->isPlaying() && !_decoder->endOfVideo();
}

void FMVScreen::stop() {
	_decoder->stop();
	StarkUserInterface->onFMVStopped();
}


} // End of namespace Stark
