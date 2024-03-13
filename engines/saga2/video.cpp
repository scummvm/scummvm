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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "graphics/paletteman.h"
#include "video/smk_decoder.h"

#include "saga2/saga2.h"

namespace Saga2 {

#define VIDEO_EXT ".SMK"

void Saga2Engine::startVideo(const char *fileName, int x, int y) {
	Common::String fname = fileName;

	if (!fname.hasSuffix(VIDEO_EXT))
		fname += VIDEO_EXT;

	if (!_smkDecoder)
		_smkDecoder = new Video::SmackerDecoder();

	if (!_smkDecoder->loadFile(Common::Path(fname))) {
		warning("startVideo: Cannot open file %s", fname.c_str());

		return;
	}

	_videoX = x;
	_videoY = y;

	_smkDecoder->start();
}

bool Saga2Engine::checkVideo() {
	if (!_smkDecoder)
		return false;

	if (_smkDecoder->endOfVideo())
		return false;

	if (_smkDecoder->needsUpdate()) {
		const Graphics::Surface *frame = _smkDecoder->decodeNextFrame();
		if (frame) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, _videoX, _videoY, frame->w, frame->h);

			if (_smkDecoder->hasDirtyPalette())
				g_system->getPaletteManager()->setPalette(_smkDecoder->getPalette(), 0, 256);

			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	return true;
}

void Saga2Engine::abortVideo() {
	endVideo();
}

void Saga2Engine::endVideo() {
	if (_smkDecoder)
		_smkDecoder->stop();

	delete _smkDecoder;
	_smkDecoder = nullptr;
}

} // end of namespace Saga2
