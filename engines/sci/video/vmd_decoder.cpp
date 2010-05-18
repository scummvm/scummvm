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

#ifdef ENABLE_SCI32

#include "sci/video/vmd_decoder.h"

#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/system.h"

#include "graphics/dither.h"

#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Sci {

VMDDecoder::VMDDecoder(Audio::Mixer *mixer) : _mixer(mixer) {
	_vmdDecoder = new Graphics::Vmd(new Graphics::PaletteLUT(5, Graphics::PaletteLUT::kPaletteYUV));
	_surface = 0;
	_dirtyPalette = false;
	_fileStream = 0;
}

VMDDecoder::~VMDDecoder() {
	close();
}

bool VMDDecoder::load(Common::SeekableReadStream &stream) {
	close();

	if (!_vmdDecoder->load(stream))
		return false;

	_fileStream = &stream;

	if (_vmdDecoder->getFeatures() & Graphics::CoktelVideo::kFeaturesPalette)
		loadPaletteFromVMD();

	if (_vmdDecoder->getFeatures() & Graphics::CoktelVideo::kFeaturesSound)
		_vmdDecoder->enableSound(*_mixer);

	if (_vmdDecoder->hasExtraData())
		warning("This VMD video has extra embedded data, which is currently not handled");

	_surface = new Graphics::Surface();
	_surface->create(_vmdDecoder->getWidth(), _vmdDecoder->getHeight(), 1);
	_vmdDecoder->setVideoMemory((byte *)_surface->pixels, _surface->w, _surface->h);
	return true;
}

void VMDDecoder::close() {
	if (!_fileStream)
		return;

	_vmdDecoder->unload();

	delete _fileStream;
	_fileStream = 0;

	_surface->free();
	delete _surface;
	_surface = 0;

	reset();
}

Graphics::Surface *VMDDecoder::decodeNextFrame() {
	Graphics::CoktelVideo::State state = _vmdDecoder->nextFrame();

	if (state.flags & Graphics::CoktelVideo::kStatePalette)
		loadPaletteFromVMD();

	if (_curFrame == -1)
		_startTime = g_system->getMillis();

	_curFrame++;
	return _surface;
}

void VMDDecoder::loadPaletteFromVMD() {
	const byte *pal = _vmdDecoder->getPalette();

	for (int i = 0; i < 256; i++) {
		_palette[i * 3 + 0] = pal[i * 3 + 0] << 2;
		_palette[i * 3 + 1] = pal[i * 3 + 1] << 2;
		_palette[i * 3 + 2] = pal[i * 3 + 2] << 2;
	}

	_dirtyPalette = true;
}

} // End of namespace Graphics

#endif
