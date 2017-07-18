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
#include "sludge/newfatal.h"
#include "sludge/fileset.h"
#include "sludge/backdrop.h"
#include "sludge/moreio.h"
#include "sludge/statusba.h"
#include "sludge/talk.h"
#include "sludge/zbuffer.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/variable.h"
#include "sludge/imgloader.h"

namespace Sludge {

extern inputType input;
extern Graphics::Surface renderSurface;

bool freeze();
void unfreeze(bool);    // Because FREEZE.H needs a load of other includes

bool backdropExists = false;
extern int zBufferToSet;

Graphics::Surface lightMap;
Graphics::Surface OrigBackdropSurface;
Graphics::Surface backdropSurface;
Graphics::Surface snapshotSurface;

float snapTexW = 1.0;
float snapTexH = 1.0;

uint winWidth, winHeight;
int lightMapMode = LIGHTMAPMODE_PIXEL;
parallaxLayer *parallaxStuff = NULL;
int cameraPX = 0, cameraPY = 0;

uint sceneWidth, sceneHeight;
int lightMapNumber;
uint currentBlankColour = TS_ARGB(255, 0, 0, 0);

extern int cameraX, cameraY;
extern float cameraZoom;

void nosnapshot() {
	if (snapshotSurface.getPixels())
		snapshotSurface.free();
}

void saveSnapshot(Common::WriteStream *stream) {
	if (snapshotSurface.getPixels()) {
		stream->writeByte(1);               // 1 for snapshot follows
		Image::writePNG(*stream, snapshotSurface);
	} else {
		stream->writeByte(0);
	}
}

bool snapshot() {
	nosnapshot();
	if (!freeze())
		return false;

	// draw snapshot to backdrop
	displayBase();
	viewSpeech(); // ...and anything being said
	drawStatusBar();

	// copy backdrop to snapshot
	snapshotSurface.copyFrom(backdropSurface);

	unfreeze(false);
	return true;
}

bool restoreSnapshot(Common::SeekableReadStream *stream) {
	if (!(ImgLoader::loadImage(stream, &snapshotSurface))) {
		return false;
	}
	return true;
}

void killBackDrop() {
	if (backdropSurface.getPixels())
		backdropSurface.free();
	backdropExists = false;
}

void killLightMap() {
	if (lightMap.getPixels()) {
		lightMap.free();
	}
	lightMapNumber = 0;
}

void killParallax() {
	while (parallaxStuff) {
		parallaxLayer *k = parallaxStuff;
		parallaxStuff = k->next;
		k->surface.free();
		delete k;
		k = NULL;
	}
}

bool reserveBackdrop() {
	cameraX = 0;
	cameraY = 0;
	input.mouseX = (int)((float)input.mouseX * cameraZoom);
	input.mouseY = (int)((float)input.mouseY * cameraZoom);
	cameraZoom = 1.0;
	input.mouseX = (int)((float)input.mouseX / cameraZoom);
	input.mouseY = (int)((float)input.mouseY / cameraZoom);

	return true;
}

void killAllBackDrop() {
	killLightMap();
	killBackDrop();
	killParallax();
	killZBuffer();
}

bool resizeBackdrop(int x, int y) {
	sceneWidth = x;
	sceneHeight = y;
	return reserveBackdrop();
}

bool killResizeBackdrop(int x, int y) {
	killAllBackDrop();
	return resizeBackdrop(x, y);
}

void loadBackDrop(int fileNum, int x, int y) {
	debug(kSludgeDebugGraphics, "Load back drop");
	setResourceForFatal(fileNum);
	if (!g_sludge->_resMan->openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!loadHSI(g_sludge->_resMan->getData(), x, y, false)) {
		Common::String mess = Common::String::format("Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i", x, y, sceneWidth, sceneHeight);
		fatal(mess);
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);

	// set zBuffer if it's not set
	if (zBufferToSet >= 0) {
		setZBuffer(zBufferToSet);
		zBufferToSet = -1;
	}
}

void mixBackDrop(int fileNum, int x, int y) {
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

void blankScreen(int x1, int y1, int x2, int y2) {
	// in case of no backdrop added at all
	if (!backdropSurface.getPixels()) {
		return;
	}

	if (y1 < 0)
		y1 = 0;
	if (x1 < 0)
		x1 = 0;
	if (x2 > (int)sceneWidth)
		x2 = (int)sceneWidth;
	if (y2 > (int)sceneHeight)
		y2 = (int)sceneHeight;

	backdropSurface.fillRect(Common::Rect(x1, y1, x2, y2), currentBlankColour);
}

// This function is very useful for scrolling credits, but very little else
void hardScroll(int distance) {
	// scroll 0 distance, return
	if (!distance)
		return;

	// blank screen
	blankScreen(0, 0, sceneWidth, sceneHeight);

	// scroll more than backdrop height, screen stay blank
	if (ABS(distance) >= (int)sceneHeight) {
		return;
	}

	// copy part of the backdrop to it
	if (distance > 0) {
		backdropSurface.copyRectToSurface(OrigBackdropSurface, 0, 0,
				Common::Rect(0, distance, backdropSurface.w, backdropSurface.h));
	} else {
		backdropSurface.copyRectToSurface(OrigBackdropSurface, 0, -distance,
				Common::Rect(0, 0, backdropSurface.w, backdropSurface.h + distance));
	}
}

void drawVerticalLine(uint x, uint y1, uint y2) {
	backdropSurface.drawLine(x, y1, x, y2, backdropSurface.format.ARGBToColor(255, 0, 0, 0));
}

void drawHorizontalLine(uint x1, uint y, uint x2) {
	backdropSurface.drawLine(x1, y, x2, y, backdropSurface.format.ARGBToColor(255, 0, 0, 0));
}

void darkScreen() {
	Graphics::TransparentSurface tmp(backdropSurface, false);
	tmp.blit(backdropSurface, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB(0, 255 >> 1, 0, 0));
}

inline int sortOutPCamera(int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ? (sceneMax ? ((cX * boxMax) / sceneMax) : 0) : ((cX * fX) / 100);
}

void drawBackDrop() {
	if (!backdropExists)
		return;

	// TODO: apply lightmap shader

	// draw parallaxStuff
	if (parallaxStuff) {
		// TODO: simulate image repeating effect
		warning("Drawing parallaxStuff");
#if 0
		parallaxLayer *ps = parallaxStuff;

		// go to the parallax at bottom
		while (ps->next) ps = ps->next;

		// draw parallax one by one
		while (ps) {
			ps->cameraX = sortOutPCamera(cameraX, ps->fractionX, (int)(sceneWidth - (float)winWidth / cameraZoom), (int)(ps->surface.w - (float)winWidth / cameraZoom));
			ps->cameraY = sortOutPCamera(cameraY, ps->fractionY, (int)(sceneHeight - (float)winHeight / cameraZoom), (int)(ps->surface.h - (float)winHeight / cameraZoom));

			uint w = (ps->wrapS) ? sceneWidth : ps->surface.w;
			uint h = (ps->wrapT) ? sceneHeight : ps->surface.h;

			const GLfloat vertices[] = {
				(GLfloat) - ps->cameraX, (GLfloat) - ps->cameraY, 0.1f,
				w - ps->cameraX, (GLfloat) - ps->cameraY, 0.1f,
				(GLfloat) - ps->cameraX, h - ps->cameraY, 0.1f,
				w - ps->cameraX, h - ps->cameraY, 0.1f
			};

			const GLfloat texCoords[] = {
				0.0f, 0.0f,
				texw, 0.0f,
				0.0f, texh,
				texw, texh
			};
			drawQuad(shader.smartScaler, vertices, 1, texCoords);

			ps = ps->prev;
		}
#endif
	}

	// draw backdrop
	Graphics::TransparentSurface tmp(backdropSurface, false);
	tmp.blit(renderSurface, 0, 0);
}

bool loadLightMap(int v) {
	setResourceForFatal(v);
	if (!g_sludge->_resMan->openFileFromNum(v))
		return fatal("Can't open light map.");

	killLightMap();
	lightMapNumber = v;

	if (!ImgLoader::loadImage(g_sludge->_resMan->getData(), &lightMap))
		return false;

	if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
		if (lightMap.w != sceneWidth || lightMap.h != sceneHeight) {
			return fatal("Light map width and height don't match scene width and height. That is required for lightmaps in HOTSPOT mode.");
		}
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);

	return true;
}

bool loadParallax(uint16 v, uint16 fracX, uint16 fracY) {
	setResourceForFatal(v);
	if (!g_sludge->_resMan->openFileFromNum(v))
		return fatal("Can't open parallax image");

	parallaxLayer *nP = new parallaxLayer;
	if (!checkNew(nP))
		return false;

	nP->next = parallaxStuff;
	parallaxStuff = nP;
	if (nP->next) {
		nP->next->prev = nP;
	}
	nP->prev = NULL;

	if (!ImgLoader::loadImage(g_sludge->_resMan->getData(), &nP->surface, 0))
		return false;

	nP->fileNum = v;
	nP->fractionX = fracX;
	nP->fractionY = fracY;

	// 65535 is the value of AUTOFIT constant in Sludge
	if (fracX == 65535) {
		nP->wrapS = false;
		if (nP->surface.w < winWidth) {
			fatal("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
			return false;
		}
	} else {
		nP->wrapS = true;
	}

	if (fracY == 65535) {
		nP->wrapT = false;
		if (nP->surface.h < winHeight) {
			fatal("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
			return false;
		}
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

bool loadHSI(Common::SeekableReadStream *stream, int x, int y, bool reserve) {
	debug(kSludgeDebugGraphics, "Load HSI");
	if (reserve) {
		killAllBackDrop(); // kill all
	}

	if (!ImgLoader::loadImage(stream, &backdropSurface, (int)reserve))
		return false;

	uint realPicWidth = backdropSurface.w;
	uint realPicHeight = backdropSurface.h;

	// resize backdrop
	if (reserve) {
		if (!resizeBackdrop(realPicWidth, realPicHeight))
			return false;
	}

	if (x == IN_THE_CENTRE)
		x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) {
		debug(kSludgeDebugGraphics, "Illegal back drop size");
		return false;
	}

	OrigBackdropSurface.copyFrom(backdropSurface);
	backdropExists = true;

	return true;
}

bool mixHSI(Common::SeekableReadStream *stream, int x, int y) {
	debug(kSludgeDebugGraphics, "Load mixHSI");
	Graphics::Surface mixSurface;
	if (!ImgLoader::loadImage(stream, &mixSurface, 0))
		return false;

	uint realPicWidth = mixSurface.w;
	uint realPicHeight = mixSurface.h;

	if (x == IN_THE_CENTRE)
		x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight)
		return false;

	Graphics::TransparentSurface tmp(mixSurface, false);
	tmp.blit(backdropSurface, x, y, Graphics::FLIP_NONE, nullptr, TS_ARGB(255, 255 >> 1, 255, 255));
	mixSurface.free();

	return true;
}

void saveHSI(Common::WriteStream *stream) {
	Image::writePNG(*stream, backdropSurface);
}

void saveParallaxRecursive(parallaxLayer *me, Common::WriteStream *stream) {
	if (me) {
		saveParallaxRecursive(me->next, stream);
		stream->writeByte(1);
		stream->writeUint16BE(me->fileNum);
		stream->writeUint16BE(me->fractionX);
		stream->writeUint16BE(me->fractionY);
	}
}

bool getRGBIntoStack(uint x, uint y, stackHandler *sH) {
	if (x >= sceneWidth || y >= sceneHeight) {
		return fatal("Co-ordinates are outside current scene!");
	}

	variable newValue;

	newValue.varType = SVT_NULL;

	byte *target = (byte *)renderSurface.getBasePtr(x, y);

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
