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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/video/coktel_decoder.h"

#ifdef GRAPHICS_VIDEO_COKTELDECODER_H

namespace Graphics {

CoktelDecoder::State::State() : left(0), top(0), right(0), bottom(0), flags(0), speechId(0) {
}


CoktelDecoder::CoktelDecoder(Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) :
	_mixer(&mixer), _soundType(soundType), _width(0), _height(0), _frameCount(0), _paletteDirty(false) {

	memset(_palette, 0, 768);
}

CoktelDecoder::~CoktelDecoder() {
}

uint16 CoktelDecoder::getWidth() const {
	return _width;
}

uint16 CoktelDecoder::getHeight() const {
	return _height;
}

uint32 CoktelDecoder::getFrameCount() const {
	return _frameCount;
}

byte *CoktelDecoder::getPalette() {
	return _palette;
}

bool CoktelDecoder::hasDirtyPalette() const {
	return _paletteDirty;
}

Common::Rational CoktelDecoder::getFrameRate() const {
	return _frameRate;
}


PreIMDDecoder::PreIMDDecoder(uint16 width, uint16 height,
	Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType) {

	_width  = width;
	_height = height;
}

PreIMDDecoder::~PreIMDDecoder() {
}

bool PreIMDDecoder::seek(uint32 frame, int whence, bool restart) {
	return false;
}

bool PreIMDDecoder::load(Common::SeekableReadStream &stream) {
	return false;
}

void PreIMDDecoder::close() {
}

bool PreIMDDecoder::isVideoLoaded() const {
	return false;
}

Surface *PreIMDDecoder::decodeNextFrame() {
	return 0;
}

PixelFormat PreIMDDecoder::getPixelFormat() const {
	return PixelFormat::createFormatCLUT8();
}

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELDECODER_H
