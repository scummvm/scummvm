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

#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/transparent_surface.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/event.h"
#include "sludge/fileset.h"
#include "sludge/graphics.h"
#include "sludge/imgloader.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/people.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sprites.h"
#include "sludge/zbuffer.h"

namespace Sludge {

// This function is only used to kill text font
void GraphicsManager::forgetSpriteBank(SpriteBank &forgetme) {
	// kill the sprite bank
	if (forgetme.myPalette.pal) {
		delete[] forgetme.myPalette.pal;
		forgetme.myPalette.pal = NULL;
		delete[] forgetme.myPalette.r;
		forgetme.myPalette.r = NULL;
		delete[] forgetme.myPalette.g;
		forgetme.myPalette.g = NULL;
		delete[] forgetme.myPalette.b;
		forgetme.myPalette.b = NULL;
	}

	if (forgetme.sprites) {
		for (int i = 0; i < forgetme.total; ++i) {
			forgetme.sprites[i].surface.free();
			forgetme.sprites[i].burnSurface.free();
		}

		delete []forgetme.sprites;
		forgetme.sprites = NULL;
	}
}

bool GraphicsManager::reserveSpritePal(SpritePalette &sP, int n) {
	if (sP.pal) {
		delete[] sP.pal;
		delete[] sP.r;
		delete[] sP.g;
		delete[] sP.b;
	}

	sP.pal = new uint16[n];
	if (!checkNew(sP.pal))
		return false;

	sP.r = new byte[n];
	if (!checkNew(sP.r))
		return false;
	sP.g = new byte[n];
	if (!checkNew(sP.g))
		return false;
	sP.b = new byte[n];
	if (!checkNew(sP.b))
		return false;
	sP.total = n;
	return (bool)(sP.pal != NULL) && (sP.r != NULL) && (sP.g != NULL) && (sP.b != NULL);
}

bool GraphicsManager::loadSpriteBank(int fileNum, SpriteBank &loadhere, bool isFont) {

	int total, spriteBankVersion = 0, howmany = 0, startIndex = 0;
	byte *data;

	setResourceForFatal(fileNum);
	if (!g_sludge->_resMan->openFileFromNum(fileNum))
		return fatal("Can't open sprite bank / font");

	loadhere.isFont = isFont;

	Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();
	total = readStream->readUint16BE();
	if (!total) {
		spriteBankVersion = readStream->readByte();
		if (spriteBankVersion == 1) {
			total = 0;
		} else {
			total = readStream->readUint16BE();
		}
	}

	if (total <= 0)
		return fatal("No sprites in bank or invalid sprite bank file");
	if (spriteBankVersion > 3)
		return fatal("Unsupported sprite bank file format");

	loadhere.total = total;
	loadhere.sprites = new Sprite[total];
	if (!checkNew(loadhere.sprites))
		return false;
	byte **spriteData = new byte *[total];
	if (!checkNew(spriteData))
		return false;

	// version 1, 2, read how many now
	if (spriteBankVersion && spriteBankVersion < 3) {
		howmany = readStream->readByte();
		startIndex = 1;
	}

	// version 3, sprite is png
	if (spriteBankVersion == 3) {
		debugC(2, kSludgeDebugGraphics, "png sprite");
		for (int i = 0; i < total; i++) {
			loadhere.sprites[i].xhot = readStream->readSint16LE();
			loadhere.sprites[i].yhot = readStream->readSint16LE();
			if (!ImgLoader::loadPNGImage(readStream, &loadhere.sprites[i].surface, false)) {
				return fatal("fail to read png sprite");
			}
		}
		g_sludge->_resMan->finishAccess();
		setResourceForFatal(-1);
		return true;
	}

	// version 0, 1, 2
	for (int i = 0; i < total; i++) {
		uint picwidth, picheight;
		// load sprite width, height, relative position
		if (spriteBankVersion == 2) {
			picwidth = readStream->readUint16BE();
			picheight = readStream->readUint16BE();
			loadhere.sprites[i].xhot = readStream->readSint16LE();
			loadhere.sprites[i].yhot = readStream->readSint16LE();
		} else {
			picwidth = (byte)readStream->readByte();
			picheight = (byte)readStream->readByte();
			loadhere.sprites[i].xhot = readStream->readByte();
			loadhere.sprites[i].yhot = readStream->readByte();
		}

		// init data
		loadhere.sprites[i].surface.create(picwidth, picheight, *g_sludge->getScreenPixelFormat());
		if (isFont) {
			loadhere.sprites[i].burnSurface.create(picwidth, picheight, *g_sludge->getScreenPixelFormat());
		}
		data = (byte *)new byte[picwidth * (picheight + 1)];
		if (!checkNew(data))
			return false;
		memset(data + picwidth * picheight, 0, picwidth);
		spriteData[i] = data;

		// read color
		if (spriteBankVersion == 2) { // RUN LENGTH COMPRESSED DATA
			uint size = picwidth * picheight;
			uint pip = 0;

			while (pip < size) {
				byte col = readStream->readByte();
				int looper;
				if (col > howmany) {
					col -= howmany + 1;
					looper = readStream->readByte() + 1;
				} else
					looper = 1;

				while (looper--) {
					data[pip++] = col;
				}
			}
		} else { // RAW DATA
			uint bytes_read = readStream->read(data, picwidth * picheight);
			if (bytes_read != picwidth * picheight && readStream->err()) {
				warning("Reading error in loadSpriteBank.");
			}
		}
	}

	// read howmany for version 0
	if (!spriteBankVersion) {
		howmany = readStream->readByte();
		startIndex = readStream->readByte();
	}

	// Make palette for version 0, 1, 2
	if (!reserveSpritePal(loadhere.myPalette, howmany + startIndex))
		return false;
	for (int i = 0; i < howmany; i++) {
		loadhere.myPalette.r[i + startIndex] = (byte)readStream->readByte();
		loadhere.myPalette.g[i + startIndex] = (byte)readStream->readByte();
		loadhere.myPalette.b[i + startIndex] = (byte)readStream->readByte();
		loadhere.myPalette.pal[i + startIndex] =
				(uint16)g_sludge->getOrigPixelFormat()->RGBToColor(
						loadhere.myPalette.r[i + startIndex],
						loadhere.myPalette.g[i + startIndex],
						loadhere.myPalette.b[i + startIndex]);
	}
	loadhere.myPalette.originalRed = loadhere.myPalette.originalGreen = loadhere.myPalette.originalBlue = 255;

	// convert
	for (int i = 0; i < total; i++) {
		int fromhere = 0;
		int transColour = -1;
		int size = loadhere.sprites[i].surface.w * loadhere.sprites[i].surface.h;
		while (fromhere < size) {
			byte s = spriteData[i][fromhere++];
			if (s) {
				transColour = s;
				break;
			}
		}
		fromhere = 0;
		for (int y = 0; y < loadhere.sprites[i].surface.h; y++) {
			for (int x = 0; x < loadhere.sprites[i].surface.w; x++) {
				byte *target = (byte *)loadhere.sprites[i].surface.getBasePtr(x, y);
				byte s = spriteData[i][fromhere++];
				if (s) {
					target[0] = (byte)255;
					target[1] = (byte)loadhere.myPalette.b[s];
					target[2] = (byte)loadhere.myPalette.g[s];
					target[3] = (byte)loadhere.myPalette.r[s];
					transColour = s;
				} else if (transColour >= 0) {
					target[0] = (byte)0;
					target[1] = (byte)loadhere.myPalette.b[transColour];
					target[2] = (byte)loadhere.myPalette.g[transColour];
					target[3] = (byte)loadhere.myPalette.r[transColour];
				}
				if (isFont) {
					target = (byte *)loadhere.sprites[i].burnSurface.getBasePtr(x, y);
					if (s)
						target[0] = loadhere.myPalette.r[s];
					target[1] = (byte)255;
					target[2] = (byte)255;
					target[3] = (byte)255;
				}
			}
		}
		delete[] spriteData[i];
	}
	delete[] spriteData;
	spriteData = NULL;

	g_sludge->_resMan->finishAccess();

	setResourceForFatal(-1);

	return true;
}

// pasteSpriteToBackDrop uses the colour specified by the setPasteColour (or setPasteColor)
void GraphicsManager::pasteSpriteToBackDrop(int x1, int y1, Sprite &single, const SpritePalette &fontPal) {
	// kill zBuffer
	if (_zBuffer->originalNum >= 0 && _zBuffer->sprites) {
		int num = _zBuffer->originalNum;
		killZBuffer();
		_zBuffer->originalNum = num;
	}

	//TODO: shader: useLightTexture
	x1 -= single.xhot;
	y1 -= single.yhot;
	Graphics::TransparentSurface tmp(single.surface, false);
	tmp.blit(_backdropSurface, x1, y1, Graphics::FLIP_NONE, nullptr,
			TS_RGB(fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue));
}

// burnSpriteToBackDrop adds text in the colour specified by setBurnColour
// using the differing brightness levels of the font to achieve an anti-aliasing effect.
void GraphicsManager::burnSpriteToBackDrop(int x1, int y1, Sprite &single, const SpritePalette &fontPal) {
	// kill zBuffer
	if (_zBuffer->originalNum >= 0 && _zBuffer->sprites) {
		int num = _zBuffer->originalNum;
		killZBuffer();
		_zBuffer->originalNum = num;
	}

	//TODO: shader: useLightTexture
	x1 -= single.xhot;
	y1 -= single.yhot - 1;
	Graphics::TransparentSurface tmp(single.surface, false);
	tmp.blit(_backdropSurface, x1, y1, Graphics::FLIP_NONE, nullptr,
			TS_RGB(_currentBurnR, _currentBurnG, _currentBurnB));
}

void GraphicsManager::fontSprite(bool flip, int x, int y, Sprite &single, const SpritePalette &fontPal) {
	float x1 = (float)x - (float)single.xhot / _cameraZoom;
	float y1 = (float)y - (float)single.yhot / _cameraZoom;

	// Use Transparent surface to scale and blit
	Graphics::TransparentSurface tmp(single.surface, false);
	tmp.blit(_renderSurface, x1, y1, (flip ? Graphics::FLIP_H : Graphics::FLIP_NONE), 0, TS_RGB(fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue));

	if (single.burnSurface.getPixels() != nullptr) {
		Graphics::TransparentSurface tmp2(single.burnSurface, false);
		tmp2.blit(_renderSurface, x1, y1, (flip ? Graphics::FLIP_H : Graphics::FLIP_NONE), 0, TS_RGB(fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue));

	}
}

void GraphicsManager::fontSprite(int x, int y, Sprite &single, const SpritePalette &fontPal) {
	fontSprite(false, x, y, single, fontPal);
}

void GraphicsManager::flipFontSprite(int x, int y, Sprite &single, const SpritePalette &fontPal) {
	fontSprite(true, x, y, single, fontPal);
}

Graphics::Surface *GraphicsManager::duplicateSurface(Graphics::Surface *surface) {
	Graphics::Surface *res = new Graphics::Surface();
	res->copyFrom(*surface);
	return res;
}

void GraphicsManager::blendColor(Graphics::Surface *blitted, uint32 color, Graphics::TSpriteBlendMode mode) {
	Graphics::TransparentSurface tmp;
	tmp.create(blitted->w, blitted->h, blitted->format);
	tmp.fillRect(Common::Rect(0, 0, tmp.w, tmp.h), color);
	tmp.blit(*blitted, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB((uint)255, (uint)255, (uint)255, (uint)255), (int)blitted->w, (int)blitted->h, mode);
	tmp.free();
}

Graphics::Surface *GraphicsManager::applyLightmapToSprite(Graphics::Surface *&blitted, OnScreenPerson *thisPerson, bool mirror, int x, int y, int x1, int y1, int diffX, int diffY) {
	Graphics::Surface * toDetele = nullptr;

	// if light map is used
	bool light = !(thisPerson->extra & EXTRA_NOLITE);

	// apply light map and set light map color
	byte curLight[3];
	if (light && _lightMap.getPixels()) {
		if (_lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx = x + _cameraX;
			int ly = y + _cameraY;
			if (lx < 0 || ly < 0 || lx >= (int)_sceneWidth || ly >= (int)_sceneHeight) {
				curLight[0] = curLight[1] = curLight[2] = 255;
			} else {
				byte *target = (byte *)_lightMap.getBasePtr(lx, ly);
				curLight[0] = target[3];
				curLight[1] = target[2];
				curLight[2] = target[1];
			}
		} else if (_lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;

			toDetele = blitted = duplicateSurface(blitted);

			// apply light map texture
			Graphics::TransparentSurface tmp(_lightMap, false);
			Common::Rect rect_none(x1, y1, x1 + diffX, y1 + diffY);
			Common::Rect rect_h(_sceneWidth - x1 - diffX, y1, _sceneWidth - x1, y1 + diffY);
			tmp.blit(*blitted, 0, 0,
					(mirror ? Graphics::FLIP_H : Graphics::FLIP_NONE),
					(mirror ? &rect_h : &rect_none),
					TS_ARGB((uint)255, (uint)255, (uint)255, (uint)255),
					(int)blitted->w, (int)blitted->h, Graphics::BLEND_MULTIPLY);
		} else {
			curLight[0] = curLight[1] = curLight[2] = 255;
		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}

	// calculate light map color
	float fr, fg, fb;
	fr = fg = fb = 0.0F;
	if (thisPerson->colourmix) {
		fr = curLight[0]*thisPerson->r * thisPerson->colourmix / 65025 / 255.0F;
		fg = curLight[1]*thisPerson->g * thisPerson->colourmix / 65025 / 255.0F;
		fb = curLight[2]*thisPerson->b * thisPerson->colourmix / 65025 / 255.0F;
	}

	uint32 primaryColor = TS_ARGB((uint8)(255 - thisPerson->transparency),
			(uint8)(fr + curLight[0] * (255 - thisPerson->colourmix) / 255.f),
			(uint8)(fg + curLight[1] * (255 - thisPerson->colourmix) / 255.f),
			(uint8)(fb + curLight[2] * (255 - thisPerson->colourmix) / 255.f));

	uint32 secondaryColor = TS_ARGB(0, (uint8)(fr * 255), (uint8)(fg * 255), (uint8)(fb * 255));

	// apply primary color
	if (primaryColor != (uint32)TS_ARGB(255, 255, 255, 255)) {
		if (!toDetele) {
			toDetele = blitted = duplicateSurface(blitted);
			blendColor(blitted, primaryColor, Graphics::BLEND_MULTIPLY);
		}
	}

	// apply secondary light map color
	if (secondaryColor != 0x0) {
		if (!toDetele) {
			toDetele = blitted = duplicateSurface(blitted);
		}
		blendColor(blitted, secondaryColor, Graphics::BLEND_ADDITIVE);
	}
	return toDetele;
}

bool GraphicsManager::scaleSprite(Sprite &single, const SpritePalette &fontPal, OnScreenPerson *thisPerson, bool mirror) {
	float x = thisPerson->x;
	float y = thisPerson->y;

	float scale = thisPerson->scale;
	bool useZB = !(thisPerson->extra & EXTRA_NOZB);

	if (scale <= 0.05)
		return false;

	int diffX = (int)(((float)single.surface.w) * scale);
	int diffY = (int)(((float)single.surface.h) * scale);

	float x1, y1, x2, y2;

	if (thisPerson->extra & EXTRA_FIXTOSCREEN) {
		x = x / _cameraZoom;
		y = y / _cameraZoom;
		if (single.xhot < 0)
			x1 = x - (int)((mirror ? (float)(single.surface.w - single.xhot) : (float)(single.xhot + 1)) * scale / _cameraZoom);
		else
			x1 = x - (int)((mirror ? (float)(single.surface.w - (single.xhot + 1)) : (float)single.xhot) * scale / _cameraZoom);
		y1 = y - (int)((single.yhot - thisPerson->floaty) * scale / _cameraZoom);
		x2 = x1 + (int)(diffX / _cameraZoom);
		y2 = y1 + (int)(diffY / _cameraZoom);
	} else {
		x -= _cameraX;
		y -= _cameraY;
		if (single.xhot < 0)
			x1 = x - (int)((mirror ? (float)(single.surface.w - single.xhot) : (float)(single.xhot + 1)) * scale);
		else
			x1 = x - (int)((mirror ? (float)(single.surface.w - (single.xhot + 1)) : (float)single.xhot) * scale);
		y1 = y - (int)((single.yhot - thisPerson->floaty) * scale);
		x2 = x1 + diffX;
		y2 = y1 + diffY;
	}

	Graphics::Surface *blitted = &single.surface;
	Graphics::Surface *ptr = applyLightmapToSprite(blitted, thisPerson, mirror, x, y, x1, y1, diffX, diffY);

	// Use Transparent surface to scale and blit
	if (!_zBuffer->numPanels) {
		Graphics::TransparentSurface tmp(*blitted, false);
		tmp.blit(_renderSurface, x1, y1, (mirror ? Graphics::FLIP_H : Graphics::FLIP_NONE), nullptr, TS_ARGB((uint)255, (uint)255, (uint)255, (uint)255), diffX, diffY);
		if (ptr) {
			ptr->free();
			delete ptr;
			ptr = nullptr;
		}
	} else {
		int d = useZB ? y + _cameraY : (y + _cameraY > _sceneHeight * 0.6 ? _sceneHeight + 1 : 0);
		addSpriteDepth(blitted, d, x1, y1, (mirror ? Graphics::FLIP_H : Graphics::FLIP_NONE), diffX, diffY, ptr);
	}

	// Are we pointing at the sprite?
	if (_vm->_evtMan->mouseX() >= x1 && _vm->_evtMan->mouseX() <= x2
			&& _vm->_evtMan->mouseY() >= y1 && _vm->_evtMan->mouseY() <= y2) {
		if (thisPerson->extra & EXTRA_RECTANGULAR)
			return true;

		// check if point to non transparent part
		int pixelx = (int)(single.surface.w * (_vm->_evtMan->mouseX() - x1) / (x2 - x1));
		int pixely = (int)(single.surface.h * (_vm->_evtMan->mouseY() - y1) / (y2 - y1));
		uint32 *colorPtr = (uint32 *)single.surface.getBasePtr(pixelx, pixely);

		uint8 a, r, g, b;
		g_sludge->getScreenPixelFormat()->colorToARGB(*colorPtr, a, r, g, b);
		return a != 0;
	}
	return false;
}

void GraphicsManager::resetSpriteLayers(ZBufferData *pz, int x, int y, bool upsidedown) {
	if (_spriteLayers->numLayers > 0)
		killSpriteLayers();
	_spriteLayers->numLayers = pz->numPanels;
	debugC(3, kSludgeDebugZBuffer, "%i zBuffer layers", _spriteLayers->numLayers);
	for (int i = 0; i < _spriteLayers->numLayers; ++i) {
		SpriteDisplay *node = new SpriteDisplay(x, y, (upsidedown ? Graphics::FLIP_V : Graphics::FLIP_NONE), &pz->sprites[i], pz->sprites[i].w, pz->sprites[i].h);
		_spriteLayers->layer[i].push_back(node);
		debugC(3, kSludgeDebugZBuffer, "Layer %i is of depth %i", i, pz->panel[i]);
	}
}

void GraphicsManager::addSpriteDepth(Graphics::Surface *ptr, int depth, int x, int y, Graphics::FLIP_FLAGS flip, int width, int height, bool freeAfterUse) {
	int i;
	for (i = 1; i < _zBuffer->numPanels; ++i) {
		if (_zBuffer->panel[i] >= depth) {
			break;
		}
	}
	--i;
	debugC(3, kSludgeDebugZBuffer, "Add sprite of Y-value : %i in layer %i", depth, i);

	SpriteDisplay *node = new SpriteDisplay(x, y, flip, ptr, width, height, freeAfterUse);
	_spriteLayers->layer[i].push_back(node);
}

void GraphicsManager::displaySpriteLayers() {
	for (int i = 0; i < _spriteLayers->numLayers; ++i) {
		debugC(3, kSludgeDebugGraphics, "Display layer %i with %i sprites", i, _spriteLayers->layer[i].size());
		SpriteLayer::iterator it;
		for (it = _spriteLayers->layer[i].begin(); it != _spriteLayers->layer[i].end(); ++it) {
			Graphics::TransparentSurface tmp(*(*it)->surface, false);
			tmp.blit(_renderSurface, (*it)->x, (*it)->y, (*it)->flip, nullptr, TS_ARGB((uint)255, (uint)255, (uint)255, (uint)255), (*it)->width, (*it)->height);
		}
	}
	killSpriteLayers();
}

void GraphicsManager::killSpriteLayers() {
	for (int i = 0; i < _spriteLayers->numLayers; ++i) {
		SpriteLayer::iterator it;
		for (it = _spriteLayers->layer[i].begin(); it != _spriteLayers->layer[i].end(); ++it) {
			if ((*it)->freeAfterUse) {
				(*it)->surface->free();
				delete (*it)->surface;
				(*it)->surface = nullptr;
			}
			delete (*it);
			(*it) = nullptr;
		}
		_spriteLayers->layer[i].clear();
	}
	_spriteLayers->numLayers = 0;
}

// Paste a scaled sprite onto the backdrop
void GraphicsManager::fixScaleSprite(int x, int y, Sprite &single, const SpritePalette &fontPal, OnScreenPerson *thisPerson, int camX, int camY, bool mirror) {

	float scale = thisPerson->scale;
	bool useZB = !(thisPerson->extra & EXTRA_NOZB);

	if (scale <= 0.05)
		return;

	int diffX = (int)(((float)single.surface.w) * scale);
	int diffY = (int)(((float)single.surface.h) * scale);
	int x1;
	if (single.xhot < 0)
		x1 = x - (int)((mirror ? (float)(single.surface.w - single.xhot) : (float)(single.xhot + 1)) * scale);
	else
		x1 = x - (int)((mirror ? (float)(single.surface.w - (single.xhot + 1)) : (float)single.xhot) * scale);
	int y1 = y - (int)((single.yhot - thisPerson->floaty) * scale);

	Graphics::Surface *blitted = &single.surface;
	Graphics::Surface *ptr = applyLightmapToSprite(blitted, thisPerson, mirror, x, y, x1, y1, diffX, diffY);

	// draw backdrop
	drawBackDrop();

	// draw zBuffer
	if (_zBuffer->numPanels) {
		drawZBuffer((int)(x1 + camX), (int)(y1 + camY), false);
	}

	// draw sprite
	if (!_zBuffer->numPanels) {
		Graphics::TransparentSurface tmp(single.surface, false);
		tmp.blit(_renderSurface, x1, y1, (mirror ? Graphics::FLIP_H : Graphics::FLIP_NONE), nullptr, TS_ARGB((uint)255, (uint)255, (uint)255, (uint)255), diffX, diffY);
		if (ptr) {
			ptr->free();
			delete ptr;
			ptr = nullptr;
		}
	} else {
		int d = useZB ? y + _cameraY : (y + _cameraY > _sceneHeight * 0.6 ? _sceneHeight + 1 : 0);
		addSpriteDepth(&single.surface, d, x1, y1, (mirror ? Graphics::FLIP_H : Graphics::FLIP_NONE), diffX, diffY, ptr);
	}

	// draw all
	displaySpriteLayers();

	// copy screen to backdrop
	_backdropSurface.copyFrom(_renderSurface);
}

} // End of namespace Sludge
