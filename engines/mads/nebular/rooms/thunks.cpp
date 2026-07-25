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

Globals _globals;
Scene _scene;

int16 &Globals::operator[](int idx) {
	return global[idx];
}

static uint16 dummy_num_images;
Scene::Animation::Animation(int anim_id) : _id(anim_id),
		_currentFrame(kernel_anim[anim_id].frame),
		_repeatFlag(kernel_anim[anim_id].repeat),
		_spriteListIndexes(kernel_anim[anim_id].anim ? &kernel_anim[anim_id].anim->series_id[0] : nullptr),
		_oldFrameEntry(kernel_anim[anim_id].image),
		_frameEntriesCount(kernel_anim[anim_id].anim ? kernel_anim[anim_id].anim->num_images : dummy_num_images),
		_frameEntries(kernel_anim[anim_id].anim ? kernel_anim[anim_id].anim->image : nullptr) {
}

int Scene::Animation::getCurrentFrame() const {
	return _currentFrame;
}

void Scene::Animation::setNextFrameTimer(long time) {
	kernel_anim[_id].next_clock = time;
}

long Scene::Animation::getNextFrameTimer() {
	return 	kernel_anim[_id].next_clock;
}

void Scene::Animation::setCurrentFrame(int frameNum) {
	kernel_reset_animation(_id, frameNum);
}

void Scene::Animation::resetSpriteSetsCount() {
	kernel_anim[_id].anim->num_series = 0;
}

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

Scene::KernelMessages::TalkFont &Scene::KernelMessages::TalkFont::operator=(FontPtr font) {
	kernel_message_font = font;
	return *this;
}

int Scene::KernelMessages::TalkFont::getWidth(const Common::String &message, int spacing) const {
	return font_string_width(kernel_message_font, message.c_str(), spacing);
}

int Scene::KernelMessages::add(const Common::Point &pt, uint fontColor, uint8 flags, int endTrigger,
		uint32 timeout, const char *msg) {
	return kernel_message_add(const_cast<char *>(msg), pt.x, pt.y, fontColor, timeout, endTrigger, flags);
}

int Scene::KernelMessages::addQuote(int quoteId, int endTrigger, uint32 timeout) {
	return kernel_message_player(quoteId, timeout, endTrigger);
}

void Scene::KernelMessages::remove(int msgIndex) {
	kernel_message_delete(msgIndex);
}

void Scene::KernelMessages::reset() {
	kernel_message_purge();
}

void Scene::KernelMessages::setQuoted(int msgIndex, int numTicks, bool quoted) {
	kernel_message_teletype(msgIndex, numTicks, quoted);
}

void Scene::KernelMessages::setSeqIndex(int msgIndex, int seqIndex) {
	kernel_message_attach(msgIndex, seqIndex);
}

void Scene::KernelMessages::initRandomMessages(int maxSimultaneousMessages,
		const Common::Rect &bounds, int minYSpacing, int scrollRate,
		int color, int duration, int quoteId, ...) {
	va_list marker;
	va_start(marker, quoteId);
	kernel_random_messages_vinit(maxSimultaneousMessages, bounds.left, bounds.right,
		bounds.top, bounds.bottom, minYSpacing, scrollRate, color, duration, quoteId, marker);
	va_end(marker);
}

void Scene::KernelMessages::randomServer() {
	kernel_random_message_server();
}

int Scene::KernelMessages::checkRandom() {
	return kernel_check_random();
}

bool Scene::KernelMessages::generateRandom(int major, int minor) {
	return kernel_generate_random_message(major, minor);
}

Scene::KernelMessages::KernelMessageProxy Scene::KernelMessages::Entries::operator[](uint handle) {
	return KernelMessageProxy(&kernel_message[handle]);
}

int Scene::Rails::getNext() const {
	return player.next_special_code;
}

void Scene::Rails::resetNext() {
	player.next_special_code = 0;
}

int16 Scene::Sprites::addSprites(const char *name, int load_flags) {
	return kernel_load_series(name, load_flags);
}

void Scene::Sprites::remove(int sprite_id) {
	matte_deallocate_series(sprite_id, true);
}

Scene::Sprite Scene::Sprites::operator[](int idx) {
	return Scene::Sprite(series_list[idx]);
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

int Scene::loadAnimation(const char *name, int trigger_code) {
	return kernel_run_animation(name, trigger_code);
}

void Scene::freeAnimation() {
	kernel_abort_animation(0);
}

void Scene::changeVariant(int num) {
	kernel_load_variant(num);
}

void Scene::drawElements(int transitionType, bool surfaceFlag) {
	matte_frame(transitionType, surfaceFlag);
}

void Scene::resetScene() {
	kernel_dump_all();
}

void Scene::clearSequenceList() {
	kernel_seq_init();
}

void Scene::addActiveVocab(int vocab_id) {
	vocab_make_active(vocab_id);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
