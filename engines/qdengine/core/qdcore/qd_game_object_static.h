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

#ifndef QDENGINE_CORE_QDCORE_QD_GAME_OBJECT_STATIC_H
#define QDENGINE_CORE_QDCORE_QD_GAME_OBJECT_STATIC_H

#include "qdengine/core/qdcore/qd_sprite.h"
#include "qdengine/core/qdcore/qd_game_object.h"


namespace QDEngine {

class XStream;

class qdGameObjectStatic : public qdGameObject {
public:
	qdGameObjectStatic();
	~qdGameObjectStatic();

	int named_object_type() const {
		return QD_NAMED_OBJECT_STATIC_OBJ;
	}

	const qdSprite *get_sprite() const {
		return &sprite_;
	}
	qdSprite *get_sprite() {
		return &sprite_;
	}

	void redraw(int offs_x = 0, int offs_y = 0) const;
	void debug_redraw() const { }
	void draw_contour(unsigned color) const;
	void draw_shadow(int offs_x, int offs_y, unsigned color, int alpha) const { }

	bool load_script(const xml::tag *p);
	bool save_script(XStream &fh, int indent = 0) const;
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
		return false;
	}

	bool load_resources();
	void free_resources();

	Vect2s screen_size() const {
		return Vect2s(sprite_.size_x(), sprite_.size_y());
	}

	bool hit(int x, int y) const;

#ifdef _QUEST_EDITOR
	bool remove_sprite_edges();
#endif //_QUEST_EDITOR
protected:

	bool load_script_body(const xml::tag *p);
	bool save_script_body(XStream &fh, int indent = 0) const;
	bool save_script_body(Common::SeekableWriteStream &fh, int indent = 0) const;

private:
	qdSprite sprite_;
};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_GAME_OBJECT_STATIC_H
