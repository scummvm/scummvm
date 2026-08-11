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

#ifndef TEXT_LAYOUT_H
#define TEXT_LAYOUT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace AGDS {

class AGDSEngine;
class Process;

class TextLayout {
	int _fontId;
	Common::Point _pos;
	bool _npc;
	bool _valid;

	struct Line {
		Common::Point pos;
		Common::String text;
		Common::Point size;
	};

	Common::Array<Line> _lines;

	Common::String _process;
	Common::String _charNotifyVar;
	Common::String _charDirectionNotifyVar;
	Common::String _npcNotifyVar;

public:
	TextLayout() : _fontId(-1), _npc(true), _valid(false) {}

	bool valid() const {
		return _valid;
	}

	void reset(AGDSEngine &engine);

	void setCharNotifyVar(const Common::String &name) {
		_charNotifyVar = name;
	}

	void setCharDirectionNotifyVar(const Common::String &name) {
		_charDirectionNotifyVar = name;
	}

	void setNPCNotifyVar(const Common::String &name) {
		_npcNotifyVar = name;
	}

	void paint(AGDSEngine &engine, Graphics::Surface &backbuffer);
	void layout(AGDSEngine &engine, Process &process, const Common::String &text, Common::Point pos, int fontId, bool npc);
};

} // End of namespace AGDS

#endif /* AGDS_TEXT_LAYOUT_H */
