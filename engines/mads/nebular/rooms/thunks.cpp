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

int16 Scene::Sequences::addSpriteCycle(int series_id, int mirror, word ticks, word expire,
	word start_ticks, int interval_ticks) {
	// ***WARNING***: Last 3 params reversed
	return kernel_seq_forward(series_id, mirror, ticks, interval_ticks, start_ticks, expire);
}

int16 Scene::Sequences::addReverseSpriteCycle(int series_id, int mirror, word ticks, word expire,
		word start_ticks, int interval_ticks) {
	// ***WARNING***: Last 3 params reversed
	return kernel_seq_backward(series_id, mirror, ticks, interval_ticks, start_ticks, expire);
}

int Scene::Sequences::startPingPongCycle(int series_id, int mirror, word ticks, word expire,
	word start_ticks, int interval_ticks) {
	// ***WARNING***: Last 3 params reversed
	return kernel_seq_pingpong(series_id, mirror, ticks, interval_ticks, start_ticks, expire);
}

void Scene::Sequences::remove(int sequence_id) {
	kernel_seq_delete(sequence_id);
}

Scene::SequenceProxy Scene::Sequences::operator[](uint sequence_id) {
	return SequenceProxy(&sequence_list[sequence_id]);
}

void Scene::Sequences::setDepth(int sequence_id, int depth) {
	kernel_seq_depth(sequence_id, depth);
}

void Scene::Sequences::setScale(int sequence_id, int scale) {
	kernel_seq_scale(sequence_id, scale);
}

void Scene::Sequences::setAnimRange(int sequence_id, int first, int last) {
	kernel_seq_range(sequence_id, first, last);
}

void Scene::Sequences::addTimer(int ticks, int trigger_code) {
	kernel_timing_trigger(ticks, trigger_code);
}

int16 Scene::Sequences::addSubEntry(int sequence_id, int trigger_type, int trigger_sprite, int trigger_code) {
	return kernel_seq_trigger(sequence_id, trigger_type, trigger_sprite, trigger_code);
}

void Scene::Sequences::setMsgLayout(int sequence_id) {
	kernel_seq_player(sequence_id, false);
}

void Scene::Sequences::setPosition(int sequence_id, const Common::Point &pt) {
	kernel_seq_loc(sequence_id, pt.x, pt.y);
}

void Scene::Sequences::setMotion(int sequence_id, int flags,
		int delta_x_times_100, int delta_y_times_100) {
	kernel_seq_motion(sequence_id, flags, delta_x_times_100, delta_y_times_100);
}

void Scene::Sequences::updateTimeout(int new_sequence_id, int old_sequence_id) {
	// ***WARNING***: params reversed
	kernel_seq_timeout(old_sequence_id, new_sequence_id);
}

void Scene::Sequences::scan() {
	kernel_seq_full_update();
}

int Scene::Sequences::startCycle(int srcSpriteIdx, bool flipped, int cycleIndex) {
	return kernel_seq_stamp(srcSpriteIdx, flipped, cycleIndex);
}

void Scene::Sequences::setDone(int sequence_id) {
	kernel_player_expire(sequence_id);
}

int Scene::Sequences::findByTrigger(int trigger) {
	return kernel_seq_find_by_trigger(trigger);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
