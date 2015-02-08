/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/stark/visual/smacker.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

#include "common/str.h"
#include "common/archive.h"

#include "video/smk_decoder.h"

namespace Stark {

VisualSmacker::VisualSmacker(Common::SeekableReadStream *stream) :
		Visual(TYPE) {
	_surface = new Graphics::Surface();

	_smacker = new Video::SmackerDecoder();
	_smacker->loadStream(stream);
	_smacker->start();
	update(0);
}

VisualSmacker::~VisualSmacker() {
	_smacker->close();
	delete _smacker;

	_surface->free();
	delete _surface;
}

VisualSmacker *VisualSmacker::load(Common::SeekableReadStream *stream) {
	return new VisualSmacker(stream);
}

void VisualSmacker::render(GfxDriver *gfx, const Common::Point &position) {
	gfx->drawSurface(_surface, position);
}

void VisualSmacker::update(uint32 delta) {
	if (_smacker->endOfVideo()) {
		_smacker->rewind();
		_smacker->start();
	}
	if (_smacker->needsUpdate()) {
		const Graphics::Surface *temp = _smacker->decodeNextFrame();
		if (temp->format.bytesPerPixel == 1) {
			Graphics::Surface *converted = temp->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), _smacker->getPalette());
			_surface->free();
			_surface->copyFrom(*converted);
			converted->free();
			delete converted;
		} else {
			_surface->free();
			_surface->copyFrom(*temp);
		}
	}
}

bool VisualSmacker::isDone() {
	return _smacker->endOfVideo();
}

} // End of namespace Stark
