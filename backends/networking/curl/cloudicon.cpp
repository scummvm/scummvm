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

#include "backends/networking/curl/cloudicon.h"
#include "backends/cloud/cloudmanager.h"
#include "common/memstream.h"
#include "gui/ThemeEngine.h"
#include "gui/gui-manager.h"
#include "image/png.h"

namespace Networking {

const float CloudIcon::ALPHA_STEP = 0.025;
const float CloudIcon::ALPHA_MAX = 1;
const float CloudIcon::ALPHA_MIN = 0.6;

CloudIcon::CloudIcon():
	_wasVisible(false), _iconsInited(false), _showingDisabled(false),
	_currentAlpha(0), _alphaRising(true), _disabledFrames(0) {
	initIcons();
}

CloudIcon::~CloudIcon() {}

bool CloudIcon::draw() {
	bool stop = false;
	initIcons();
	
	if (CloudMan.isWorking() || _disabledFrames > 0) {		
		if (g_system) {
			if (!_wasVisible) {
				g_system->clearOSD();
				_wasVisible = true;
			}
			--_disabledFrames;
			if (_alphaRising) {
				if (_currentAlpha < ALPHA_MIN)
					_currentAlpha += 5 * ALPHA_STEP;
				else
					_currentAlpha += ALPHA_STEP;
				if (_currentAlpha > ALPHA_MAX) {
					_currentAlpha = ALPHA_MAX;
					_alphaRising = false;
				}
			} else {
				_currentAlpha -= ALPHA_STEP;
				if (_currentAlpha < ALPHA_MIN) {
					_currentAlpha = ALPHA_MIN;
					_alphaRising = true;
				}
			}
		} else {
			_wasVisible = false;
		}
	} else {
		_wasVisible = false;
		_currentAlpha -= 5 * ALPHA_STEP;
		if (_currentAlpha <= 0) {
			_currentAlpha = 0;
			stop = true;
		}
	}

	if (g_system) {
		Graphics::TransparentSurface *surface = &_icon;
		makeAlphaIcon((_showingDisabled? _disabledIcon:_icon), _currentAlpha);
		if (_alphaIcon.getPixels()) surface = &_alphaIcon;
		if (surface && surface->getPixels()) {
			int x = g_system->getOverlayWidth() - surface->w - 10, y = 10;
			g_system->copyRectToOSD(surface->getPixels(), surface->pitch, x, y, surface->w, surface->h);
		}
	}

	if (stop) _showingDisabled = false;
	return stop;
}

void CloudIcon::showDisabled() {
	_showingDisabled = true;
	_disabledFrames = 20 * 3; //3 seconds 20 fps
}

#include "backends/networking/curl/cloudicon_data.h"
#include "backends/networking/curl/cloudicon_disabled_data.h"

void CloudIcon::initIcons() {
	if (_iconsInited) return;
	loadIcon(_icon, cloudicon_data, ARRAYSIZE(cloudicon_data));
	loadIcon(_disabledIcon, cloudicon_disabled_data, ARRAYSIZE(cloudicon_disabled_data));
	_iconsInited = true;
}

void CloudIcon::loadIcon(Graphics::TransparentSurface &icon, byte *data, uint32 size) {	
	Image::PNGDecoder decoder;	
	Common::MemoryReadStream stream(data, size);
	if (!decoder.loadStream(stream))
		error("CloudIcon::loadIcon: error decoding PNG");

	Graphics::TransparentSurface *s = new Graphics::TransparentSurface(*decoder.getSurface(), true);
	if (s) {
		Graphics::PixelFormat f = g_system->getOSDFormat();
		if (f != s->format) {
			Graphics::TransparentSurface *s2 = s->convertTo(f);
			if (s2) icon.copyFrom(*s2);
			else warning("CloudIcon::loadIcon: failed converting TransparentSurface");
			delete s2;
		} else {
			icon.copyFrom(*s);
		}
		delete s;
	} else warning("CloudIcon::loadIcon: failed reading TransparentSurface from PNGDecoder");
}

void CloudIcon::makeAlphaIcon(Graphics::TransparentSurface &icon, float alpha) {
	_alphaIcon.copyFrom(icon);

	byte *pixels = (byte *)_alphaIcon.getPixels();
	for (int y = 0; y < _alphaIcon.h; y++) {
		byte *row = pixels + y * _alphaIcon.pitch;
		for (int x = 0; x < _alphaIcon.w; x++) {
			uint32 srcColor;
			if (_alphaIcon.format.bytesPerPixel == 2)
				srcColor = READ_UINT16(row);
			else if (_alphaIcon.format.bytesPerPixel == 3)
				srcColor = READ_UINT24(row);
			else
				srcColor = READ_UINT32(row);

			// Update color's alpha
			byte r, g, b, a;
			_alphaIcon.format.colorToARGB(srcColor, a, r, g, b);
			a = (byte)(a * alpha);
			uint32 color = _alphaIcon.format.ARGBToColor(a, r, g, b);

			if (_alphaIcon.format.bytesPerPixel == 2)
				*((uint16 *)row) = color;
			else
				*((uint32 *)row) = color;

			row += _alphaIcon.format.bytesPerPixel;
		}
	}
}

} // End of namespace Networking
