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
#include "gui/ThemeEngine.h"
#include "gui/gui-manager.h"
#include "image/png.h"
#include <common/file.h>
#include <backends/graphics/surfacesdl/surfacesdl-graphics.h>

namespace Networking {

CloudIcon::CloudIcon(): _frame(0), _wasVisible(false), _iconsInited(false) {
	initIcons();
}

CloudIcon::~CloudIcon() {}

void CloudIcon::initIcons() {
	if (_iconsInited) return;

	Image::PNGDecoder decoder;
	Common::ArchiveMemberList members;
	Common::File file;
	if (!file.open("cloudicon.png")) warning("failed");
	Common::SeekableReadStream *stream = &file;	
	if (stream) {			
		if (!decoder.loadStream(*stream))
			error("Error decoding PNG");

		Graphics::TransparentSurface *s = new Graphics::TransparentSurface(*decoder.getSurface(), true);
		if (s) {
			Graphics::PixelFormat f = g_system->getOSDFormat();
			//f.bytesPerPixel = 4;
			debug("%d in osd vs %d in s", f.bytesPerPixel, s->format.bytesPerPixel);
			Graphics::TransparentSurface *s2 = s->convertTo(f);
			if (s2) _icon.copyFrom(*s2);
			else warning("failed converting");
		}
		else warning("failed reading");
	}
	_iconsInited = true;
}

void CloudIcon::draw() {
	initIcons();
	_frame++;

	Cloud::Storage *storage = CloudMan.getCurrentStorage();	
	if (storage && storage->isWorking()) {
		if (g_system) {
			if (!_wasVisible) {
				g_system->clearOSD();
				_wasVisible = true;
			}
		} else {
			_wasVisible = false;
		}
	} else {
		_wasVisible = false;
	}

	if (g_system) {
		if (_icon.getPixels()) {
			int x = g_system->getOverlayWidth() - _icon.w - 10, y = 10;
			g_system->copyRectToOSD(_icon.getPixels(), _icon.pitch, x, y, _icon.w, _icon.h);
		}
	}
}

} // End of namespace Networking
