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

#include "agds/animation.h"
#include "agds/agds.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "graphics/transparent_surface.h"
#include "video/flic_decoder.h"

namespace AGDS {

Animation::Animation(): _flic() {
}

Animation::~Animation() {
	delete _flic;
}

bool Animation::load(Common::SeekableReadStream* stream) {
	Video::FlicDecoder * flic = new Video::FlicDecoder;
	if (flic->loadStream(stream)) {
		delete _flic;
		_flic = flic;
		return true;
	} else {
		delete flic;
		return false;
	}
}


void Animation::paint(AGDSEngine *engine, Graphics::Surface & backbuffer, Common::Point dst) {
	const Graphics::Surface * frame = _flic->decodeNextFrame();
	if (!frame) {
		_flic->rewind();
		frame = _flic->decodeNextFrame();
	}
	Graphics::TransparentSurface * c = engine->convertToTransparent(frame->convertTo(engine->pixelFormat(), _flic->getPalette()));
	Common::Rect srcRect = c->getRect();
	if (Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect()))
		c->blit(backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect);
	delete c;
}

int Animation::width() const {
	return _flic? _flic->getWidth(): 0;
}
int Animation::height() const {
	return _flic? _flic->getHeight(): 0;
}



}
