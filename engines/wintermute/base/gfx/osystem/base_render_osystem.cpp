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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/base/base_registry.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "common/system.h"
#include "engines/wintermute/graphics/transparent_surface.h"
#include "common/queue.h"

namespace WinterMute {

RenderTicket::RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY) : _owner(owner),
	_srcRect(*srcRect), _dstRect(*dstRect), _drawNum(0), _isValid(true), _wantsDraw(true), _hasAlpha(true) {
	_colorMod = 0;
		_mirror = TransparentSurface::FLIP_NONE;
		if (mirrorX)
			_mirror |= TransparentSurface::FLIP_V;
		if (mirrorY)
			_mirror |= TransparentSurface::FLIP_H;
	if (surf) {
		_surface = new Graphics::Surface();
		_surface->create((uint16)srcRect->width(), (uint16)srcRect->height(), surf->format);
		assert(_surface->format.bytesPerPixel == 4);
		// Get a clipped copy of the surface
		for (int i = 0; i < _surface->h; i++) {
			memcpy(_surface->getBasePtr(0, i), surf->getBasePtr(srcRect->left, srcRect->top + i), srcRect->width() * _surface->format.bytesPerPixel);
		}
		// Then scale it if necessary
		if (dstRect->width() != srcRect->width() || dstRect->height() != srcRect->height()) {
			TransparentSurface src(*_surface, false);
			Graphics::Surface *temp = src.scale(dstRect->width(), dstRect->height());
			_surface->free();
			delete _surface;
			_surface = temp;
		}
	} else {
		_surface = NULL;
	}
}

RenderTicket::~RenderTicket() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

bool RenderTicket::operator==(RenderTicket &t) {
	if ((t._srcRect != _srcRect) ||
		(t._dstRect != _dstRect) ||
		(t._mirror != _mirror) ||
		(t._owner != _owner) ||
		(t._hasAlpha != _hasAlpha) ||
		(t._colorMod != _colorMod)) {
			return false;
		}
	return true;
}

BaseRenderer *makeOSystemRenderer(BaseGame *inGame) {
	return new BaseRenderOSystem(inGame);
}

// TODO: Redo everything here.

//////////////////////////////////////////////////////////////////////////
BaseRenderOSystem::BaseRenderOSystem(BaseGame *inGame) : BaseRenderer(inGame) {
	_renderSurface = new Graphics::Surface();
	_drawNum = 1;
	_needsFlip = true;

	_borderLeft = _borderRight = _borderTop = _borderBottom = 0;
	_ratioX = _ratioY = 1.0f;
	setAlphaMod(255);
	setColorMod(255, 255, 255);
	_dirtyRect = NULL;
	_disableDirtyRects = true;
}

//////////////////////////////////////////////////////////////////////////
BaseRenderOSystem::~BaseRenderOSystem() {
	_renderSurface->free();
	delete _renderSurface;
#if 0
	if (_renderer) SDL_DestroyRenderer(_renderer);
	if (_win) SDL_DestroyWindow(_win);
	SDL_Quit();
#endif
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::initRenderer(int width, int height, bool windowed) {
	//if (SDL_Init(SDL_INIT_VIDEO) < 0) return STATUS_FAILED;

#if 0
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
#endif
	_width = width;
	_height = height;
	_renderRect.setWidth(_width);
	_renderRect.setHeight(_height);

	_realWidth = width;
	_realHeight = height;


	// find suitable resolution
/*#ifdef __IPHONEOS__
	_realWidth = 480;
	_realHeight = 320;

	int numModes = SDL_GetNumDisplayModes(0);
	for (int i = 0; i < numModes; i++) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, i, &mode);

		if (mode.w > mode.h) {
			_realWidth = mode.w;
			_realHeight = mode.h;
			break;
		}
	}
#else*/
	_realWidth = _gameRef->_registry->readInt("Debug", "ForceResWidth", _width);
	_realHeight = _gameRef->_registry->readInt("Debug", "ForceResHeight", _height);
//#endif

	/*
	_realWidth = 480;
	_realHeight = 320;
	*/


	float origAspect = (float)_width / (float)_height;
	float realAspect = (float)_realWidth / (float)_realHeight;

	float ratio;
	if (origAspect < realAspect) {
		// normal to wide
		ratio = (float)_realHeight / (float)_height;
	} else {
		// wide to normal
		ratio = (float)_realWidth / (float)_width;
	}

	_borderLeft = (int)((_realWidth - (_width * ratio)) / 2);
	_borderRight = (int)(_realWidth - (_width * ratio) - _borderLeft);

	_borderTop = (int)((_realHeight - (_height * ratio)) / 2);
	_borderBottom = (int)(_realHeight - (_height * ratio) - _borderTop);



	_ratioX = (float)(_realWidth - _borderLeft - _borderRight) / (float)_width;
	_ratioY = (float)(_realHeight - _borderTop - _borderBottom) / (float)_height;

#if 0
	Uint32 flags = SDL_WINDOW_SHOWN;
#endif
#ifdef __IPHONEOS__
	//flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
#endif

	//_windowed = _gameRef->_registry->readBool("Video", "Windowed", true);
//	if (!windowed) flags |= SDL_WINDOW_FULLSCREEN;

	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	g_system->beginGFXTransaction();
	g_system->initSize(_width, _height, &format);
	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError != OSystem::kTransactionSuccess) {
		warning("Couldn't setup GFX-backend for %dx%dx%d", _width, _height, format.bytesPerPixel * 8);
		return STATUS_FAILED;
	}
#if 0
	_win = SDL_CreateWindow("WME Lite",
	                        SDL_WINDOWPOS_UNDEFINED,
	                        SDL_WINDOWPOS_UNDEFINED,
	                        _realWidth, _realHeight,
	                        flags);

	if (!_win) return STATUS_FAILED;
#endif

	g_system->showMouse(false);

#ifdef __IPHONEOS__
	// SDL defaults to OGL ES2, which doesn't work on old devices
	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles");
#else
	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
#if 0
	_renderer = SDL_CreateRenderer(_win, -1, 0);

	if (!_renderer) return STATUS_FAILED;
#endif
	_renderSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_active = true;

	_clearColor = _renderSurface->format.ARGBToColor(255, 0, 0, 0);

	return STATUS_OK;
}

void BaseRenderOSystem::setAlphaMod(byte alpha) {
	byte r = RGBCOLGetR(_colorMod);
	byte g = RGBCOLGetB(_colorMod);
	byte b = RGBCOLGetB(_colorMod);
	_colorMod = BS_ARGB(alpha, r, g, b);
}

void BaseRenderOSystem::setColorMod(byte r, byte g, byte b) {
	byte alpha = RGBCOLGetA(_colorMod);
	_colorMod = BS_ARGB(alpha, r, g, b);
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::flip() {
	if (!_disableDirtyRects) {
		drawTickets();
	}
	if (_needsFlip || _disableDirtyRects) {
		if (_disableDirtyRects) {
			g_system->copyRectToScreen((byte *)_renderSurface->pixels, _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
		}
	//	g_system->copyRectToScreen((byte *)_renderSurface->pixels, _renderSurface->pitch, _dirtyRect->left, _dirtyRect->top, _dirtyRect->width(), _dirtyRect->height());
		delete _dirtyRect;
		_dirtyRect = NULL;
		g_system->updateScreen();
		_needsFlip = false;
	}
	_drawNum = 1;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::fill(byte r, byte g, byte b, Common::Rect *rect) {
	//SDL_SetRenderDrawColor(_renderer, r, g, b, 0xFF);
	//SDL_RenderClear(_renderer);
	_clearColor = _renderSurface->format.ARGBToColor(0xFF, r, g, b);
	if (!_disableDirtyRects)
		return STATUS_OK;
	if (!rect) {
		rect = &_renderRect;
	}
	_renderSurface->fillRect(*rect, _clearColor);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::fade(uint16 Alpha) {
	uint32 dwAlpha = 255 - Alpha;
	return fadeToColor(dwAlpha << 24);
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::fadeToColor(uint32 Color, Common::Rect *rect) {
	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("BaseRenderOSystem::FadeToColor - Breaks when using dirty rects");
		warning("Implement BaseRenderOSystem::FadeToColor"); // TODO.
		hasWarned = true;
	}

	Common::Rect fillRect;

	if (rect) {
		fillRect.left = rect->left;
		fillRect.top = rect->top;
		fillRect.setWidth(rect->width());
		fillRect.setHeight(rect->height());
	} else {
		Rect32 rc;
		_gameRef->getCurrentViewportRect(&rc);
		fillRect.left = (int16)rc.left;
		fillRect.top = (int16)rc.top;
		fillRect.setWidth((int16)(rc.right - rc.left));
		fillRect.setHeight((int16)(rc.bottom - rc.top));
	}
	modTargetRect(&fillRect);

	byte r = RGBCOLGetR(Color);
	byte g = RGBCOLGetG(Color);
	byte b = RGBCOLGetB(Color);
	byte a = RGBCOLGetA(Color);

	//TODO: This is only here until I'm sure about the final pixelformat
	uint32 col = _renderSurface->format.ARGBToColor(a, r, g, b);
	if (_disableDirtyRects)
		_renderSurface->fillRect(fillRect, col);
	else {
		setAlphaMod(a);
		setColorMod(r, g, b);
		Graphics::Surface surf;
		surf.create((uint16)fillRect.width(), (uint16)fillRect.height(), _renderSurface->format);
		Common::Rect sizeRect(fillRect);
		sizeRect.translate(-fillRect.top, -fillRect.left);
		surf.fillRect(fillRect, col);
		drawSurface(NULL, &surf, &sizeRect, &fillRect, false, false);
		surf.free();
		_clearColor = col;
	}
	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	//SDL_RenderFillRect(_renderer, &fillRect);

	return STATUS_OK;
}

void BaseRenderOSystem::drawSurface(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY) {
	if (_disableDirtyRects) {
		RenderTicket renderTicket(owner, surf, srcRect, dstRect, mirrorX, mirrorY);
		// HINT: The surface-data contains other info than it should.
	//	drawFromSurface(renderTicket._surface, srcRect, dstRect, NULL, mirrorX, mirrorY);
		drawFromSurface(renderTicket.getSurface(), &renderTicket._srcRect, &renderTicket._dstRect, NULL, renderTicket._mirror);
		return;
	}
	// Skip rects that are completely outside the screen:
	if ((dstRect->left < 0 && dstRect->right < 0) || (dstRect->top < 0 && dstRect->bottom < 0)) {
		return;
	}
	
	RenderTicket compare(owner, NULL, srcRect, dstRect, mirrorX, mirrorY); 
	compare._colorMod = _colorMod;
	RenderQueueIterator it;
	for (it = _renderQueue.begin(); it != _renderQueue.end(); it++) {
		if ((*it)->_owner == owner && *(*it) == compare && (*it)->_isValid) {
			(*it)->_colorMod = _colorMod;
			drawFromTicket(*it);
			return;
		}
	}
	RenderTicket *ticket = new RenderTicket(owner, surf, srcRect, dstRect, mirrorX, mirrorY);
	ticket->_colorMod = _colorMod;
	drawFromTicket(ticket);
}

void BaseRenderOSystem::invalidateTicket(RenderTicket *renderTicket) {
	addDirtyRect(renderTicket->_dstRect);
	renderTicket->_isValid = false;
//	renderTicket->_canDelete = true; // TODO: Maybe readd this, to avoid even more duplicates.
}

void BaseRenderOSystem::invalidateTicketsFromSurface(BaseSurfaceOSystem *surf) {
	RenderQueueIterator it;
	for (it = _renderQueue.begin(); it != _renderQueue.end(); it++) {
		if ((*it)->_owner == surf) {
			invalidateTicket(*it);
		}
	}
}

void BaseRenderOSystem::drawFromTicket(RenderTicket *renderTicket) {
	renderTicket->_wantsDraw = true;
	// A new item always has _drawNum == 0
	if (renderTicket->_drawNum == 0) {
		// In-order
		if (_renderQueue.empty() || _drawNum > (_renderQueue.back())->_drawNum) {
			renderTicket->_drawNum = _drawNum++;
			_renderQueue.push_back(renderTicket);
			addDirtyRect(renderTicket->_dstRect);
		} else {
			// Before something
			Common::List<RenderTicket*>::iterator pos;
			for (pos = _renderQueue.begin(); pos != _renderQueue.end(); pos++) {
				if ((*pos)->_drawNum >= _drawNum) {
					break;
				}
			}
			_renderQueue.insert(pos, renderTicket);
			Common::List<RenderTicket*>::iterator it;
			renderTicket->_drawNum = _drawNum++;
			// Increment the following tickets, so they still are in line
			for (it = pos; it != _renderQueue.end(); it++) {
				(*it)->_drawNum++;
				(*it)->_wantsDraw = false;
			}
			addDirtyRect(renderTicket->_dstRect);
		}
	} else {
		// Was drawn last round, still in the same order
		if (_drawNum == renderTicket->_drawNum) {
			_drawNum++;
		} else {
			// Remove the ticket from the list
			RenderQueueIterator it = _renderQueue.begin();
			while (it != _renderQueue.end()) {
				if ((*it) == renderTicket) {
					it = _renderQueue.erase(it);
					break;
				} else {
					it++;
				}
			}
			if (it != _renderQueue.end()) {
				// Decreement the following tickets.
				for (; it != _renderQueue.end(); it++) {
					(*it)->_drawNum--;
				} 
			}
			// Is not in order, so readd it as if it was a new ticket
			renderTicket->_drawNum = 0;
			drawFromTicket(renderTicket);
		}
	}
}

void BaseRenderOSystem::addDirtyRect(const Common::Rect &rect) {
	if (!_dirtyRect) {
		_dirtyRect = new Common::Rect(rect);
	} else {
		_dirtyRect->extend(rect);
	}
	_dirtyRect->clip(_renderRect);
//	warning("AddDirtyRect: %d %d %d %d", rect.left, rect.top, rect.right, rect.bottom);
}

void BaseRenderOSystem::drawTickets() {
	RenderQueueIterator it = _renderQueue.begin();
	// Clean out the old tickets
	int decrement = 0;
	while (it != _renderQueue.end()) {
		if ((*it)->_wantsDraw == false || (*it)->_isValid == false) {
			RenderTicket* ticket = *it;
			addDirtyRect((*it)->_dstRect);
			//warning("Discarding Rect: %d %d %d %d Width: %d Height: %d", (*it)->_dstRect.left, (*it)->_dstRect.top, (*it)->_dstRect.right, (*it)->_dstRect.bottom, (*it)->_dstRect.width() , (*it)->_dstRect.height());
			it = _renderQueue.erase(it);
			delete ticket;
			decrement++;
		} else {
			(*it)->_drawNum -= decrement;
			it++;
		}
	}
	if (!_dirtyRect || _dirtyRect->width() == 0 || _dirtyRect->height() == 0)
		return;
	// The color-mods are stored in the RenderTickets on add, since we set that state again during
	// draw, we need to keep track of what it was prior to draw.
	uint32 oldColorMod = _colorMod;
//	warning("DirtyRect: %d %d %d %d Width: %d Height: %d", _dirtyRect->left, _dirtyRect->top, _dirtyRect->right, _dirtyRect->bottom, _dirtyRect->width(), _dirtyRect->height());
	
	// Apply the clear-color to the dirty rect.
	_renderSurface->fillRect(*_dirtyRect, _clearColor);
	_drawNum = 1;
	for (it = _renderQueue.begin(); it != _renderQueue.end(); it++) {
		RenderTicket *ticket = *it;
		assert(ticket->_drawNum == _drawNum++);
		if (ticket->_isValid && ticket->_dstRect.intersects(*_dirtyRect)) {
			// dstClip is the area we want redrawn.
			Common::Rect dstClip(ticket->_dstRect);
			// reduce it to the dirty rect
			dstClip.clip(*_dirtyRect);
			// we need to keep track of the position to redraw the dirty rect
			Common::Rect pos(dstClip);
			int16 offsetX = ticket->_dstRect.left;
			int16 offsetY = ticket->_dstRect.top;
			// convert from screen-coords to surface-coords.
			dstClip.translate(-offsetX, -offsetY);

			_colorMod = ticket->_colorMod;
			drawFromSurface(ticket->getSurface(), &ticket->_srcRect, &pos, &dstClip, ticket->_mirror);
			_needsFlip = true;
		}
		// Some tickets want redraw but don't actually clip the dirty area (typically the ones that shouldnt become clear-color)
		ticket->_wantsDraw = false;
	}
	g_system->copyRectToScreen((byte *)_renderSurface->getBasePtr(_dirtyRect->left, _dirtyRect->top), _renderSurface->pitch, _dirtyRect->left, _dirtyRect->top, _dirtyRect->width(), _dirtyRect->height());

	// Revert the colorMod-state.
	_colorMod = oldColorMod;
}

// Replacement for SDL2's SDL_RenderCopy
void BaseRenderOSystem::drawFromSurface(const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, Common::Rect *clipRect, uint32 mirror) {
	TransparentSurface src(*surf, false);
	bool doDelete = false;
	if (!clipRect) {
		doDelete = true;
		clipRect = new Common::Rect();
		clipRect->setWidth(surf->w);
		clipRect->setHeight(surf->h);
	}

	src.blit(*_renderSurface, dstRect->left, dstRect->top, mirror, clipRect, _colorMod, clipRect->width(), clipRect->height());
	if (doDelete)
		delete clipRect;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("BaseRenderOSystem::DrawLine - not fully ported yet");
		hasWarned = true;
	}
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);
	byte a = RGBCOLGetA(color);

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

	Point32 point1, point2;
	point1.x = x1;
	point1.y = y1;
	pointToScreen(&point1);

	point2.x = x2;
	point2.y = y2;
	pointToScreen(&point2);

	// TODO: This thing is mostly here until I'm sure about the final color-format.
	uint32 colorVal = _renderSurface->format.ARGBToColor(a, r, g, b);
	_renderSurface->drawLine(point1.x, point1.y, point2.x, point2.y, colorVal);
	//SDL_RenderDrawLine(_renderer, point1.x, point1.y, point2.x, point2.y);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseImage *BaseRenderOSystem::takeScreenshot() {
// TODO: Fix this
	warning("BaseRenderOSystem::TakeScreenshot() - not ported yet");
	BaseImage *screenshot = new BaseImage(_gameRef->_fileManager);
	screenshot->copyFrom(_renderSurface);
	return screenshot;
#if 0
	SDL_Rect viewport;

	SDL_RenderGetViewport(_renderer, &viewport);

	SDL_Surface *surface = SDL_CreateRGBSurface(0, viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0x00000000);
	if (!surface) return NULL;

	if (SDL_RenderReadPixels(_renderer, NULL, surface->format->format, surface->pixels, surface->pitch) < 0) return NULL;

	FIBITMAP *dib = FreeImage_Allocate(viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);

	for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
		byte *bits = FreeImage_GetScanLine(dib, y);
		byte *src = (byte *)surface->pixels + (viewport.h - y - 1) * surface->pitch;
		memcpy(bits, src, bytespp * viewport.w);
	}

	return new BaseImage(_gameRef,  dib);
#endif
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::switchFullscreen() {
	/*if (_windowed) SDL_SetWindowFullscreen(_win, SDL_TRUE);
	else SDL_SetWindowFullscreen(_win, SDL_FALSE);

	_windowed = !_windowed;
	*/
	_gameRef->_registry->writeBool("Video", "Windowed", _windowed);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
const char *BaseRenderOSystem::getName() {
	if (_name.empty()) {
#if 0
		if (_renderer) {
			SDL_RendererInfo info;
			SDL_GetRendererInfo(_renderer, &info);
			_name = AnsiString(info.name);
		}
#endif
	}
	return _name.c_str();
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::setViewport(int left, int top, int right, int bottom) {
	Common::Rect rect;
	// TODO: Hopefully this is the same logic that ScummVM uses.
	rect.left = (int16)(left + _borderLeft);
	rect.top = (int16)(top + _borderTop);
	rect.right = (int16)((right - left) * _ratioX);
	rect.bottom = (int16)((bottom - top) * _ratioY);

	// TODO fix this once viewports work correctly in SDL/landscape
#ifndef __IPHONEOS__
	//SDL_RenderSetViewport(GetSdlRenderer(), &rect);
#endif
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::modTargetRect(Common::Rect *rect) {
#if 0
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	rect->x = MathUtil::Round(rect->x * _ratioX + _borderLeft - viewportRect.x);
	rect->y = MathUtil::Round(rect->y * _ratioY + _borderTop - viewportRect.y);
	rect->w = MathUtil::RoundUp(rect->w * _ratioX);
	rect->h = MathUtil::RoundUp(rect->h * _ratioY);
#endif
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::pointFromScreen(Point32 *point) {
#if 0
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	point->x = point->x / _ratioX - _borderLeft / _ratioX + viewportRect.x;
	point->y = point->y / _ratioY - _borderTop / _ratioY + viewportRect.y;
#endif
}


//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::pointToScreen(Point32 *point) {
#if 0
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	point->x = MathUtil::RoundUp(point->x * _ratioX) + _borderLeft - viewportRect.x;
	point->y = MathUtil::RoundUp(point->y * _ratioY) + _borderTop - viewportRect.y;
#endif
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::dumpData(const char *filename) {
	warning("BaseRenderOSystem::DumpData(%s) - not reimplemented yet", filename); // TODO
#if 0
	FILE *f = fopen(filename, "wt");
	if (!f) return;

	BaseSurfaceStorage *Mgr = _gameRef->_surfaceStorage;

	int TotalKB = 0;
	int TotalLoss = 0;
	fprintf(f, "Filename;Usage;Size;KBytes\n");
	for (int i = 0; i < Mgr->_surfaces.getSize(); i++) {
		BaseSurfaceOSystem *Surf = (BaseSurfaceOSystem *)Mgr->_surfaces[i];
		if (!Surf->_filename) continue;
		if (!Surf->_valid) continue;

		fprintf(f, "%s;%d;", Surf->_filename, Surf->_referenceCount);
		fprintf(f, "%dx%d;", Surf->getWidth(), Surf->getHeight());

		int kb = Surf->getWidth() * Surf->getHeight() * 4 / 1024;

		TotalKB += kb;
		fprintf(f, "%d;", kb);
		fprintf(f, "\n");
	}
	fprintf(f, "Total %d;;;%d\n", Mgr->_surfaces.getSize(), TotalKB);


	fclose(f);
	_gameRef->LOG(0, "Texture Stats Dump completed.");
	_gameRef->QuickMessage("Texture Stats Dump completed.");
#endif
}

BaseSurface *BaseRenderOSystem::createSurface() {
	return new BaseSurfaceOSystem(_gameRef);
}

} // end of namespace WinterMute
