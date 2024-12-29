/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SHERLOCK_SCALPEL_3DO_SCREEN_H
#define SHERLOCK_SCALPEL_3DO_SCREEN_H

#include "sherlock/scalpel/scalpel_screen.h"

namespace Sherlock {

class SherlockEngine;

namespace Scalpel {

class Scalpel3DOScreen : public ScalpelScreen {
protected:
	/**
	 * Draws a surface at a given position within this surface with transparency
	 */
	virtual void transBlitFromUnscaled(const Graphics::Surface &src, const Common::Point &pt, bool flipped,
		int overrideColor);
public:
	Scalpel3DOScreen(SherlockEngine *vm);
	~Scalpel3DOScreen() override {}

	/**
	 * Draws a sub-section of a surface at a given position within this surface
	 */
	void rawBlitFrom(const Graphics::Surface &src, const Common::Point &pt);

	/**
	 * Fade backbuffer 1 into screen (3DO RGB!)
	 */
	void fadeIntoScreen3DO(int speed);

	void blitFrom3DOcolorLimit(uint16 color);

	/**
	 * Draws a surface on this surface
	 */
	void SHblitFrom(const Graphics::Surface &src) override;

	/**
	 * Draws a surface at a given position within this surface
	 */
	void SHblitFrom(const Graphics::Surface &src, const Common::Point &destPos) override;

	/**
	 * Draws a sub-section of a surface at a given position within this surface
	 */
	void SHblitFrom(const Graphics::Surface &src, const Common::Point &destPos, const Common::Rect &srcBounds) override;

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	void SHtransBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped = false, int scaleVal = SCALE_THRESHOLD) override;

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	void SHtransBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped = false, int scaleVal = SCALE_THRESHOLD) override;

	/**
	 * Fill a given area of the surface with a given color
	 */
	void SHfillRect(const Common::Rect &r, uint color) override;

	uint16 width() const override;
	uint16 height() const override;
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
