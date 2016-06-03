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

namespace Networking {

CloudIcon::CloudIcon(): _frame(0) {}

CloudIcon::~CloudIcon() {}

void CloudIcon::draw() {	 
	Cloud::Storage *storage = CloudMan.getCurrentStorage();
	bool working = (storage && storage->isWorking());
	if (working) {
		//buf = animation frame;
		if (g_system) {
			const Graphics::Surface *s = g_gui.theme()->getImageSurface(GUI::ThemeEngine::kImageLogoSmall);
			int x = 10, y = 10;
			g_system->copyRectToOSD(s->getPixels(), s->pitch, x, y, s->w, s->h);
		}
	} else {
		//buf = empty;
		//TODO: put empty piece to OSD?
	}
}

} // End of namespace Networking
