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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/fmv/movieplayer.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/panel.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"
#include "sword25/sfx/soundengine.h"

namespace Sword25 {

#define BS_LOG_PREFIX "MOVIEPLAYER"

#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */

Service *OggTheora_CreateObject(Kernel *pKernel) {
#ifdef ENABLE_THEORADEC
	return new MoviePlayer(pKernel);
#else
	return NULL;
#endif
}

#ifdef ENABLE_THEORADEC
MoviePlayer::MoviePlayer(Kernel *pKernel) : Service(pKernel), _decoder(g_system->getMixer()) {
	if (!registerScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");
}

MoviePlayer::~MoviePlayer() {
	_decoder.close();
}

bool MoviePlayer::loadMovie(const Common::String &filename, uint z) {
	// Get the file and load it into the decoder
	Common::SeekableReadStream *in = Kernel::GetInstance()->GetPackage()->getStream(filename);
	_decoder.load(in);

	// Ausgabebitmap erstellen
	GraphicEngine *pGfx = Kernel::GetInstance()->GetGfx();
	_outputBitmap = pGfx->GetMainPanel()->addDynamicBitmap(_decoder.getWidth(), _decoder.getHeight());
	if (!_outputBitmap.isValid()) {
		BS_LOG_ERRORLN("Output bitmap for movie playback could not be created.");
		return false;
	}

	// Skalierung des Ausgabebitmaps berechnen, so dass es möglichst viel Bildschirmfläche einnimmt.
	float screenToVideoWidth = (float)pGfx->GetDisplayWidth() / (float)_outputBitmap->getWidth();
	float screenToVideoHeight = (float)pGfx->GetDisplayHeight() / (float)_outputBitmap->getHeight();
	float scaleFactor = MIN(screenToVideoWidth, screenToVideoHeight);

	if (abs((int)(scaleFactor - 1.0f)) < FLT_EPSILON)
		scaleFactor = 1.0f;

	_outputBitmap->setScaleFactor(scaleFactor);

	// Z-Wert setzen
	_outputBitmap->setZ(z);

	// Ausgabebitmap auf dem Bildschirm zentrieren
	_outputBitmap->setX((pGfx->GetDisplayWidth() - _outputBitmap->getWidth()) / 2);
	_outputBitmap->setY((pGfx->GetDisplayHeight() - _outputBitmap->getHeight()) / 2);

	return true;
}

bool MoviePlayer::unloadMovie() {
	_decoder.close();
	_outputBitmap.erase();

	return true;
}

bool MoviePlayer::play() {
	_decoder.pauseVideo(false);
	return true;
}

bool MoviePlayer::pause() {
	_decoder.pauseVideo(true);
	return true;
}

void MoviePlayer::update() {
	if (_decoder.isVideoLoaded()) {
		Graphics::Surface *s = _decoder.decodeNextFrame();

		// Transfer the next frame
		assert(s->bytesPerPixel == 4);
		byte *frameData = (byte *)s->getBasePtr(0, 0);
		_outputBitmap->setContent(frameData, s->pitch * s->h, 0, s->pitch);
	}
}

bool MoviePlayer::isMovieLoaded() {
	return _decoder.isVideoLoaded();
}

bool MoviePlayer::isPaused() {
	return _decoder.isPaused();
}

float MoviePlayer::getScaleFactor() {
	if (_decoder.isVideoLoaded())
		return _outputBitmap->getScaleFactorX();
	else
		return 0;
}

void MoviePlayer::setScaleFactor(float scaleFactor) {
	if (_decoder.isVideoLoaded()) {
		_outputBitmap->setScaleFactor(scaleFactor);

		// Ausgabebitmap auf dem Bildschirm zentrieren
		GraphicEngine *gfxPtr = Kernel::GetInstance()->GetGfx();
		_outputBitmap->setX((gfxPtr->GetDisplayWidth() - _outputBitmap->getWidth()) / 2);
		_outputBitmap->setY((gfxPtr->GetDisplayHeight() - _outputBitmap->getHeight()) / 2);
	}
}

double MoviePlayer::getTime() {
	return _decoder.getElapsedTime() / 1000.0;
}

#endif

} // End of namespace Sword25
