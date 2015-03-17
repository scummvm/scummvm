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

#include "engines/stark/services/fmvplayer.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"

namespace Stark {

FMVPlayer::FMVPlayer() {
	_decoder = new Video::BinkDecoder();
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	_texture = gfx->createTexture();
	_decoder->setDefaultHighColorFormat(gfx->getScreenFormat());
}

FMVPlayer::~FMVPlayer() {
	delete _decoder;
	delete _texture;
}

void FMVPlayer::play(const Common::String &name) {
	// TODO: Clear existing
	ArchiveLoader *archiveLoader = StarkServices::instance().archiveLoader;

	Common::SeekableReadStream *stream = archiveLoader->getExternalFile(name, "Global/");
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

void FMVPlayer::render() {
	// TODO: Refactor this.
	if (_decoder->isPlaying()) {
		if (_decoder->needsUpdate()) {
			const Graphics::Surface *decodedSurface = _decoder->decodeNextFrame();
			_texture->update(decodedSurface);
		}
	}
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	gfx->drawSurface(_texture, Common::Point(0, Gfx::Driver::kTopBorderHeight));
}

bool FMVPlayer::isPlaying() {
	if (_decoder->isPlaying() && !_decoder->endOfVideo()) {
		return true;
	}
	return false;
}

void FMVPlayer::stop() {
	_decoder->stop();
}


} // End of namespace Stark
