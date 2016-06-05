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

#ifndef BACKENDS_NETWORKING_CURL_CLOUDICON_H
#define BACKENDS_NETWORKING_CURL_CLOUDICON_H

#include "graphics/transparent_surface.h"

namespace Networking {

class CloudIcon {
	static const float ALPHA_STEP, ALPHA_MAX, ALPHA_MIN;

	bool _wasVisible, _iconsInited, _showingDisabled;
	Graphics::TransparentSurface _icon, _disabledIcon, _alphaIcon;
	float _currentAlpha;
	bool _alphaRising;
	int _disabledFrames;

	void initIcons();
	void loadIcon(Graphics::TransparentSurface &icon, const char *filename);
	void makeAlphaIcon(Graphics::TransparentSurface &icon, float alpha);

public:
	CloudIcon();
	~CloudIcon();

	/**
	 * This method is called from ConnectionManager every time
	 * its own timer calls the handle() method. The primary
	 * responsibility of this draw() method is to draw cloud icon
	 * on ScummVM's OSD when current cloud Storage is working.
	 *
	 * As we don't want ConnectionManager to work when no
	 * Requests are running, we'd like to stop the timer. But then
	 * this icon wouldn't have time to disappear smoothly. So,
	 * in order to do that, ConnectionManager stop its timer
	 * only when this draw() method returns true, indicating that
	 * the CloudIcon has disappeared and the timer could be stopped.
	 *
	 * @return true if ConnMan's timer could be stopped.
	 */
	bool draw();

	/** Draw a "cloud disabled" icon instead of "cloud syncing" one. */
	void showDisabled();
};

} // End of namespace Networking

#endif
