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

#include "graphics/surface.h"

namespace Cloud {

class CloudIcon {
public:
	CloudIcon();
	~CloudIcon();

	/**
	 * The type of cloud icon to show
	 */
	enum Type {
		kNone,     /** Hide the currently shown icon if any */
		kSyncing,  /** Cloud syncing icon */
		kDisabled  /** Cloud syncing not available icon */
	};

	/**
	 * Select the icon to show on the OSD
	 *
	 * @param icon Icon type to show. Use kNone to hide the current icon if any.
	 * @param duration Duration in milliseconds the icon stays visible on screen. 0 means the icon stays indefinitely.
	 */
	void show(Type icon, int duration = 0);

	/** The currently visible icon. kNone means no icon is shown. */
	Type getShownType() const;

	/** Returns true if the icon state needs to be checked for changes */
	bool needsUpdate() const;

	/** Update the icon visible on the OSD */
	void update();

private:
	static const float ALPHA_SPEED, ALPHA_MAX, ALPHA_MIN;
	static const int UPDATE_DELAY_MIN_MILLIS = 10;

	enum State {
		kHidden,
		kShown,
		kGoingToHide
	};

	State _state;
	Type _type;

	Graphics::Surface _icon, _disabledIcon, _alphaIcon;
	float _currentAlpha;
	bool _alphaRising;

	uint32 _hideTime;
	uint32 _lastUpdateTime;

	void initIcons();
	void loadIcon(Graphics::Surface &icon, byte *data, uint32 size);
	void makeAlphaIcon(const Graphics::Surface &icon, float alpha);

	void hide();
};

} // End of namespace Cloud

#endif
