/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRAPHICS_TINYGL_ZRECT_H_
#define GRAPHICS_TINYGL_ZRECT_H_

#include "common/rect.h"
#include "graphics/tinygl/zgl.h"

namespace Graphics {

	enum DrawCallType {
		DrawCall_Rasterization,
		DrawCall_Blitting
	};

	class DrawCall {
	public:
		DrawCall(DrawCallType type);
		virtual ~DrawCall() { }
		bool operator==(const DrawCall &other) const;
		virtual void execute() const = 0;
		virtual void execute(const Common::Rect &clippingRectangle) const = 0;
		DrawCallType getType() const { return _type; }
	private:
		DrawCallType _type;
	};

	class RasterizationDrawCall : public DrawCall {
	public:
		RasterizationDrawCall(TinyGL::GLContext *c);
		virtual void execute() const;
		virtual void execute(const Common::Rect &clippingRectangle) const;
	};

	class BlittingDrawCall : public DrawCall {
		BlittingDrawCall(TinyGL::GLContext *c);
		virtual void execute() const;
		virtual void execute(const Common::Rect &clippingRectangle) const;
	};
}

#endif
