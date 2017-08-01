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

#include "common/debug.h"
#include "common/rect.h"
#include "image/png.h"
#include "graphics/surface.h"
#include "graphics/transparent_surface.h"
#include "graphics/palette.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/event.h"
#include "sludge/fileset.h"
#include "sludge/graphics.h"
#include "sludge/imgloader.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/statusba.h"
#include "sludge/zbuffer.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/talk.h"
#include "sludge/variable.h"
#include "sludge/version.h"

namespace Sludge {

Parallax::Parallax() {
	_parallaxLayers.clear();
}

Parallax::~Parallax() {
	kill();
}

void Parallax::kill() {
	ParallaxLayers::iterator it;
	for (it = _parallaxLayers.begin(); it != _parallaxLayers.end(); ++it) {
		(*it)->surface.free();
		delete (*it);
		(*it) = nullptr;
	}
	_parallaxLayers.clear();
}

bool Parallax::add(uint16 v, uint16 fracX, uint16 fracY) {
	setResourceForFatal(v);
	if (!g_sludge->_resMan->openFileFromNum(v))
		return fatal("Can't open parallax image");

	ParallaxLayer *nP = new ParallaxLayer;
	if (!checkNew(nP))
		return false;

	_parallaxLayers.push_back(nP);

	if (!ImgLoader::loadImage(g_sludge->_resMan->getData(), &nP->surface, 0))
		return false;

	nP->fileNum = v;
	nP->fractionX = fracX;
	nP->fractionY = fracY;

	// 65535 is the value of AUTOFIT constant in Sludge
	if (fracX == 65535) {
		nP->wrapS = false;
//		if (nP->surface.w < _winWidth) {
//			fatal("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
//			return false;
//		}
	} else {
		nP->wrapS = true;
	}

	if (fracY == 65535) {
		nP->wrapT = false;
//		if (nP->surface.h < _winHeight) {
//			fatal("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
//			return false;
//		}
	} else {
		nP->wrapT = true;
	}

	// TODO: reinterpret this part
#if 0
	if (nP->wrapS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	if (nP->wrapT)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);
	return true;
}

void Parallax::draw() {
	// draw parallaxStuff
	if (!_parallaxLayers.empty()) {
		// TODO: simulate image repeating effect
		warning("Drawing parallaxStuff");
#if 0
		// display parallax from bottom to top
		ParallaxLayers::iterator it;
		for (it = _parallax.begin(); it != _parallax.end(); ++it) {
			(*it)->cameraX = sortOutPCamera(cameraX, (*it)->fractionX, (int)(sceneWidth - (float)winWidth / cameraZoom), (int)((*it)->surface.w - (float)winWidth / cameraZoom));
			(*it)->cameraY = sortOutPCamera(cameraY, (*it)->fractionY, (int)(sceneHeight - (float)winHeight / cameraZoom), (int)((*it)->surface.h - (float)winHeight / cameraZoom));

			uint w = ((*it)->wrapS) ? sceneWidth : (*it)->surface.w;
			uint h = ((*it)->wrapT) ? sceneHeight : (*it)->surface.h;

			const GLfloat vertices[] = {
				(GLfloat) - (*it)->cameraX, (GLfloat) - (*it)->cameraY, 0.1f,
				w - (*it)->cameraX, (GLfloat) - (*it)->cameraY, 0.1f,
				(GLfloat) - (*it)->cameraX, h - (*it)->cameraY, 0.1f,
				w - (*it)->cameraX, h - (*it)->cameraY, 0.1f
			};

			const GLfloat texCoords[] = {
				0.0f, 0.0f,
				texw, 0.0f,
				0.0f, texh,
				texw, texh
			};
			drawQuad(shader.smartScaler, vertices, 1, texCoords);

		}
#endif
	}
}

void Parallax::save(Common::WriteStream *stream) {
	ParallaxLayers::iterator it;
	for (it = _parallaxLayers.begin(); it != _parallaxLayers.end(); ++it) {
		stream->writeByte(1);
		stream->writeUint16BE((*it)->fileNum);
		stream->writeUint16BE((*it)->fractionX);
		stream->writeUint16BE((*it)->fractionY);
	}
}

void GraphicsManager::nosnapshot() {
	if (_snapshotSurface.getPixels())
		_snapshotSurface.free();
}

void GraphicsManager::saveSnapshot(Common::WriteStream *stream) {
	if (_snapshotSurface.getPixels()) {
		stream->writeByte(1);               // 1 for snapshot follows
		Image::writePNG(*stream, _snapshotSurface);
	} else {
		stream->writeByte(0);
	}
}

bool GraphicsManager::snapshot() {
	nosnapshot();
	if (!freeze())
		return false;

	// draw snapshot to rendersurface
	displayBase();
	viewSpeech(); // ...and anything being said
	drawStatusBar();

	// copy backdrop to snapshot
	_snapshotSurface.copyFrom(_renderSurface);

	unfreeze(false);
	return true;
}

bool GraphicsManager::restoreSnapshot(Common::SeekableReadStream *stream) {
	if (!(ImgLoader::loadImage(stream, &_snapshotSurface))) {
		return false;
	}
	return true;
}

void GraphicsManager::killBackDrop() {
	if (_backdropSurface.getPixels())
		_backdropSurface.free();
	_backdropExists = false;
}

void GraphicsManager::killLightMap() {
	if (_lightMap.getPixels()) {
		_lightMap.free();
	}
	_lightMapNumber = 0;
}

bool GraphicsManager::reserveBackdrop() {
	_cameraX = 0;
	_cameraY = 0;
	_vm->_evtMan->mouseX() = (int)((float)_vm->_evtMan->mouseX() * _cameraZoom);
	_vm->_evtMan->mouseY() = (int)((float)_vm->_evtMan->mouseY() * _cameraZoom);
	_cameraZoom = 1.0;
	_vm->_evtMan->mouseX() = (int)((float)_vm->_evtMan->mouseX() / _cameraZoom);
	_vm->_evtMan->mouseY() = (int)((float)_vm->_evtMan->mouseY() / _cameraZoom);

	return true;
}

void GraphicsManager::killAllBackDrop() {
	killLightMap();
	killBackDrop();
	g_sludge->_gfxMan->killParallax();
	killZBuffer();
}

bool GraphicsManager::resizeBackdrop(int x, int y) {
	debug(kSludgeDebugGraphics, "Load HSI");
	_sceneWidth = x;
	_sceneHeight = y;
	return reserveBackdrop();
}

bool GraphicsManager::killResizeBackdrop(int x, int y) {
	killAllBackDrop();
	return resizeBackdrop(x, y);
}

void GraphicsManager::loadBackDrop(int fileNum, int x, int y) {
	debug(kSludgeDebugGraphics, "Load back drop of num %i at position %i, %i", fileNum, x, y);
	setResourceForFatal(fileNum);
	if (!g_sludge->_resMan->openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!loadHSI(g_sludge->_resMan->getData(), x, y, false)) {
		Common::String mess = Common::String::format("Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i", x, y, _sceneWidth, _sceneHeight);
		fatal(mess);
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);

	// reset zBuffer
	if (_zBuffer->originalNum >= 0) {
		setZBuffer(_zBuffer->originalNum);
	}
}

void GraphicsManager::mixBackDrop(int fileNum, int x, int y) {
	debug(kSludgeDebugGraphics, "Mix back drop of num %i at position %i, %i", fileNum, x, y);
	setResourceForFatal(fileNum);
	if (!g_sludge->_resMan->openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!mixHSI(g_sludge->_resMan->getData(), x, y)) {
		fatal("Can't paste overlay image outside screen dimensions");
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);
}

void GraphicsManager::blankScreen(int x1, int y1, int x2, int y2) {
	// in case of no backdrop added at all, create it
	if (!_backdropSurface.getPixels()) {
		_backdropSurface.create(_winWidth, _winHeight, _renderSurface.format);
	}

	if (y1 < 0)
		y1 = 0;
	if (x1 < 0)
		x1 = 0;
	if (x2 > (int)_sceneWidth)
		x2 = (int)_sceneWidth;
	if (y2 > (int)_sceneHeight)
		y2 = (int)_sceneHeight;

	_backdropSurface.fillRect(Common::Rect(x1, y1, x2, y2), _currentBlankColour);
}

void GraphicsManager::blankAllScreen() {
	blankScreen(0, 0, _sceneWidth, _sceneHeight);
}

// This function is very useful for scrolling credits, but very little else
void GraphicsManager::hardScroll(int distance) {
	// scroll 0 distance, return
	if (!distance)
		return;

	// blank screen
	blankAllScreen();

	// scroll more than backdrop height, screen stay blank
	if (ABS(distance) >= (int)_sceneHeight) {
		return;
	}

	// copy part of the backdrop to it
	if (distance > 0) {
		_backdropSurface.copyRectToSurface(_origBackdropSurface, 0, 0,
				Common::Rect(0, distance, _backdropSurface.w, _backdropSurface.h));
	} else {
		_backdropSurface.copyRectToSurface(_origBackdropSurface, 0, -distance,
				Common::Rect(0, 0, _backdropSurface.w, _backdropSurface.h + distance));
	}
}

void GraphicsManager::drawLine(uint x1, uint y1, uint x2, uint y2) {
	_backdropSurface.drawLine(x1, y1, x2, y2, _backdropSurface.format.ARGBToColor(255, 0, 0, 0));
}

void GraphicsManager::drawVerticalLine(uint x, uint y1, uint y2) {
	drawLine(x, y1, x, y2);
}

void GraphicsManager::drawHorizontalLine(uint x1, uint y, uint x2) {
	drawLine(x1, y, x2, y);
}

void GraphicsManager::darkScreen() {
	Graphics::TransparentSurface tmp(_backdropSurface, false);
	tmp.blit(_backdropSurface, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB(0, 255 >> 1, 0, 0));
}

void GraphicsManager::drawBackDrop() {
	// TODO: apply lightmap shader
	drawParallax();

	if (!_backdropExists)
		return;
	// draw backdrop
	Graphics::TransparentSurface tmp(_backdropSurface, false);
	tmp.blit(_renderSurface, 0, 0);
}

bool GraphicsManager::loadLightMap(int v) {
	setResourceForFatal(v);
	if (!g_sludge->_resMan->openFileFromNum(v))
		return fatal("Can't open light map.");

	killLightMap();
	_lightMapNumber = v;

	if (!ImgLoader::loadImage(g_sludge->_resMan->getData(), &_lightMap))
		return false;

	if (_lightMapMode == LIGHTMAPMODE_HOTSPOT) {
		if (_lightMap.w != _sceneWidth || _lightMap.h != _sceneHeight) {
			return fatal("Light map width and height don't match scene width and height. That is required for lightmaps in HOTSPOT mode.");
		}
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);

	return true;
}

void GraphicsManager::saveLightMap(Common::WriteStream *stream) {
	if (_lightMap.getPixels()) {
		stream->writeByte(1);
		stream->writeUint16BE(_lightMapNumber);
	} else {
		stream->writeByte(0);
	}
	stream->writeByte(_lightMapMode);
}

bool GraphicsManager::loadLightMap(int ssgVersion, Common::SeekableReadStream *stream) {
	if (stream->readByte()) {
		if (!loadLightMap(stream->readUint16BE()))
			return false;
	}

	if (ssgVersion >= VERSION(1, 4)) {
		_lightMapMode = stream->readByte() % 3;
	}

	return true;
}

bool GraphicsManager::loadHSI(Common::SeekableReadStream *stream, int x, int y, bool reserve) {
	debug(kSludgeDebugGraphics, "Load HSI");
	if (reserve) {
		killAllBackDrop(); // kill all
	}

	Graphics::Surface tmp;

	if (!ImgLoader::loadImage(stream, &tmp, (int)reserve))
		return false;

	uint realPicWidth = tmp.w;
	uint realPicHeight = tmp.h;

	// resize backdrop
	if (reserve) {
		if (!resizeBackdrop(realPicWidth, realPicHeight))
			return false;
	}

	if (x == IN_THE_CENTRE)
		x = (_sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (_sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > _sceneWidth || y < 0 || y + realPicHeight > _sceneHeight) {
		debug(kSludgeDebugGraphics, "Illegal back drop size");
		return false;
	}

	// copy surface loaded to backdrop
	_backdropSurface.copyRectToSurface(tmp.getPixels(), tmp.pitch, x, y, tmp.w, tmp.h);
	tmp.free();

	_origBackdropSurface.copyFrom(_backdropSurface);
	_backdropExists = true;

	return true;
}

bool GraphicsManager::mixHSI(Common::SeekableReadStream *stream, int x, int y) {
	debug(kSludgeDebugGraphics, "Load mixHSI");
	Graphics::Surface mixSurface;
	if (!ImgLoader::loadImage(stream, &mixSurface, 0))
		return false;

	uint realPicWidth = mixSurface.w;
	uint realPicHeight = mixSurface.h;

	if (x == IN_THE_CENTRE)
		x = (_sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (_sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > _sceneWidth || y < 0 || y + realPicHeight > _sceneHeight)
		return false;

	Graphics::TransparentSurface tmp(mixSurface, false);
	tmp.blit(_backdropSurface, x, y, Graphics::FLIP_NONE, nullptr, TS_ARGB(255, 255 >> 1, 255, 255));
	mixSurface.free();

	return true;
}

void GraphicsManager::saveHSI(Common::WriteStream *stream) {
	Image::writePNG(*stream, _backdropSurface);
}


bool GraphicsManager::getRGBIntoStack(uint x, uint y, StackHandler *sH) {
	if (x >= _sceneWidth || y >= _sceneHeight) {
		return fatal("Co-ordinates are outside current scene!");
	}

	Variable newValue;

	newValue.varType = SVT_NULL;

	byte *target = (byte *)_renderSurface.getBasePtr(x, y);

	setVariable(newValue, SVT_INT, target[1]);
	if (!addVarToStackQuick(newValue, sH->first)) return false;
	sH->last = sH->first;

	setVariable(newValue, SVT_INT, target[2]);
	if (!addVarToStackQuick(newValue, sH->first)) return false;

	setVariable(newValue, SVT_INT, target[3]);
	if (!addVarToStackQuick(newValue, sH->first)) return false;

	return true;
}

} // End of namespace Sludge
