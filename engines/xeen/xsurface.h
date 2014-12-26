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

#ifndef XEEN_XSURFACE_H
#define XEEN_XSURFACE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Xeen {

class XSurface: public Graphics::Surface {
public:
	virtual void transBlitFrom(const XSurface &src, const Common::Point &destPos);

	virtual void blitFrom(const XSurface &src, const Common::Point &destPos);
public:
	XSurface();
	XSurface(int w, int h);
	~XSurface();

	void create(uint16 w, uint16 h);

	void transBlitFrom(const XSurface &src);

	void blitFrom(const XSurface &src);

	bool empty() const { return getPixels() == nullptr; }
};

} // End of namespace Xeen

#endif /* XEEN_XSURFACE_H */
