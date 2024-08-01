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

#ifndef QDENGINE_QDCORE_QD_GAME_OBJECT_STATIC_H
#define QDENGINE_QDCORE_QD_GAME_OBJECT_STATIC_H

#include "qdengine/qdcore/qd_sprite.h"
#include "qdengine/qdcore/qd_game_object.h"


namespace QDEngine {

class qdGameObjectStatic : public qdGameObject {
public:
	qdGameObjectStatic();
	~qdGameObjectStatic();

	int named_object_type() const {
		return QD_NAMED_OBJECT_STATIC_OBJ;
	}

	const qdSprite *get_sprite() const {
		return &_sprite;
	}
	qdSprite *get_sprite() {
		return &_sprite;
	}

	void redraw(int offs_x = 0, int offs_y = 0) const;
	void debug_redraw() const { }
	void draw_contour(uint32 color) const;
	void draw_shadow(int offs_x, int offs_y, uint32 color, int alpha) const { }

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
		return false;
	}

	bool load_resources();
	void free_resources();

	Vect2s screen_size() const {
		return Vect2s(_sprite.size_x(), _sprite.size_y());
	}

	bool hit(int x, int y) const;

protected:

	bool load_script_body(const xml::tag *p);
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;

private:
	qdSprite _sprite;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_OBJECT_STATIC_H
