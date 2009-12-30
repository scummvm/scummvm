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

#include "common/archive.h"
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
}

VMDDecoder::~VMDDecoder() {
	closeFile();
}

uint32 VMDDecoder::getFrameWaitTime() {
	return _vmdDecoder->getFrameWaitTime();
}

bool VMDDecoder::loadFile(const char *fileName) {
	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;

	if (!_vmdDecoder->load(*_fileStream))
		return false;

	if (_vmdDecoder->getFeatures() & Graphics::CoktelVideo::kFeaturesPalette) {
		getPalette();
		setPalette(_palette);
	}

	if (_vmdDecoder->getFeatures() & Graphics::CoktelVideo::kFeaturesSound)
		_vmdDecoder->enableSound(*_mixer);

	_videoInfo.width = _vmdDecoder->getWidth();
	_videoInfo.height = _vmdDecoder->getHeight();
	_videoInfo.frameCount = _vmdDecoder->getFramesCount();
	_videoInfo.frameRate = _vmdDecoder->getFrameRate();
	_videoInfo.frameDelay = _videoInfo.frameRate * 100;
	_videoInfo.currentFrame = 0;
	_videoInfo.firstframeOffset = 0;	// not really necessary for VMDs

	if (_vmdDecoder->hasExtraData())
		warning("This VMD video has extra embedded data, which is currently not handled");

	_videoFrameBuffer = new byte[_videoInfo.width * _videoInfo.height];
	memset(_videoFrameBuffer, 0, _videoInfo.width * _videoInfo.height);

	_vmdDecoder->setVideoMemory(_videoFrameBuffer, _videoInfo.width, _videoInfo.height);

	return true;
}

void VMDDecoder::closeFile() {
	if (!_fileStream)
		return;

	_vmdDecoder->unload();

	delete _fileStream;
	_fileStream = 0;

	delete[] _videoFrameBuffer;
	_videoFrameBuffer = 0;
}

bool VMDDecoder::decodeNextFrame() {
	if (_videoInfo.currentFrame == 0)
		_videoInfo.startTime = g_system->getMillis();

	Graphics::CoktelVideo::State state = _vmdDecoder->nextFrame();

	if (state.flags & Graphics::CoktelVideo::kStatePalette) {
		getPalette();
		setPalette(_palette);
	}

	return ++_videoInfo.currentFrame < _videoInfo.frameCount;
}

void VMDDecoder::getPalette() {
	const byte *pal = _vmdDecoder->getPalette();

	for (int i = 0; i < 256; i++) {
		_palette[i * 3 + 0] = pal[i * 3 + 0] << 2;
		_palette[i * 3 + 1] = pal[i * 3 + 1] << 2;
		_palette[i * 3 + 2] = pal[i * 3 + 2] << 2;
	}
}

} // End of namespace Graphics

#endif
