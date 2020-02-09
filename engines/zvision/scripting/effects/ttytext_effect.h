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

#ifndef ZVISION_TTYTEXT_NODE_H
#define ZVISION_TTYTEXT_NODE_H

#include "common/rect.h"
#include "graphics/surface.h"

#include "zvision/scripting/scripting_effect.h"
#include "zvision/text/text.h"
#include "zvision/text/truetype_font.h"

namespace Common {
class String;
}

namespace ZVision {
class ttyTextNode : public ScriptingEffect {
public:
	ttyTextNode(ZVision *engine, uint32 key, const Common::String &file, const Common::Rect &r, int32 delay);
	~ttyTextNode() override;

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis) override;
private:
	Common::Rect _r;

	TextStyleState _state;
	StyledTTFont _fnt;
	Common::String _txtbuf;
	uint32 _txtpos;

	int32 _delay;
	int32 _nexttime;
	Graphics::Surface _img;
	int16 _dx;
	int16 _dy;
private:

	void newline();
	void scroll();
	void outchar(uint16 chr);
};

} // End of namespace ZVision

#endif
