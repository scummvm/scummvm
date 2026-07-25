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

#include "common/textconsole.h"
#include "mads/core/global.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/player.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

Scene _scene;

int Scene::DynamicHotspots::add(int vocab_id, int verb_id, int auto_sequence, const Common::Rect &r) {
	return kernel_add_dynamic(vocab_id, verb_id, 0, auto_sequence,
		r.left, r.top, r.width(), r.height());
}

void Scene::DynamicHotspots::remove(int dyn_id) {
	kernel_delete_dynamic(dyn_id);
}

int Scene::DynamicHotspots::setPosition(int id, const Common::Point &pt, int facing) {
	kernel_dynamic_walk(id, pt.x, pt.y, facing);
	return id;
}

int Scene::DynamicHotspots::setCursor(int id, int cursorNum) {
	return kernel_dynamic_cursor(id, cursorNum);
}

Scene::DynamicHotspot Scene::DynamicHotspots::operator[](int idx) {
	return Scene::DynamicHotspot(idx);
}

Scene::DynamicHotspot::DynamicHotspot(int index) : _articleNumber(kernel_dynamic_hot[index].prep) {
}

void Scene::Hotspots::activate(int hotspot, int active) {
	kernel_flip_hotspot(hotspot, active);
}

int Scene::Rails::getNext() const {
	return player.next_special_code;
}

void Scene::Rails::resetNext() {
	player.next_special_code = 0;
}

void Scene::SpriteSlots::clear() {
	image_marker = 0;
}

void Scene::SpriteSlots::fullRefresh() {
	matte_refresh_work();
}

void Scene::UserInterface::emptyConversationList() {
	inter_reset_dialog();
}

void Scene::UserInterface::selectObject(int item_id) {
	inter_set_active_inven(item_id);
}

void Scene::UserInterface::setup(int inputMode) {
	kernel_set_interface_mode(inputMode);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
