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
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/nebular/rooms/thunks.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

Action _action;
static Scene scene;
Scene *const _scene = &scene;
Globals _globals;
VM _vm;

bool Action::isAction(int word1) const {
	return player_parse(word1, 0);
}

bool Action::isAction(int word1, int word2) const {
	return player_parse(word1, word2, 0);
}

bool Action::isAction(int word1, int word2, int word3) const {
	return player_parse(word1, word2, word3, 0);
}

void Game::Objects::addToInventory(int object_id) {
	inter_give_to_player(object_id);
}

bool Game::Objects::isInRoom(int object_id) const {
	return object_is_here(object_id);
}

bool Game::Objects::isInInventory(int object_id) const {
	return !object_is_here(object_id);
}

void Game::loadQuoteSet(int quote1, ...) {
	va_list va;
	int id = quote1;
	va_start(va, quote1);

	while (id) {
		quote_load(id, 0);
		id = va_arg(va, int);
	}

	va_end(va);
}

char *Game::getQuote(int quote_id) {
	return quote_string(kernel.quotes, quote_id);
}

int16 &Globals::operator[](int idx) {
	return global[idx];
}

char *Resources::formatName(int my_room, char type, int num, int ext, const char *text) {
	return kernel_full_name(my_room, type, num, text, ext);
}

char *Resources::formatAAName(int num) {
	return kernel_interface_name(num);
}

int Scene::Animation::getCurrentFrame() const {
	error("TODO: Animation::getCurrentFrame");
}

int Scene::DynamicHotspots::add(int vocab_id, int verb_id, int auto_sequence, const Common::Rect &r) {
	return kernel_add_dynamic(vocab_id, verb_id, 0, auto_sequence,
		r.left, r.top, r.width(), r.height());
}

void Scene::DynamicHotspots::remove(int dyn_id) {
	kernel_delete_dynamic(dyn_id);
}

void Scene::Hotspots::activate(int hotspot, int active) {
	kernel_flip_hotspot(hotspot, active);
}

int Scene::KernelMessages::add(const Common::Point &pt, uint fontColor, uint8 flags, int endTrigger,
		uint32 timeout, const Common::String &msg) {
	return kernel_message_add(const_cast<char *>(msg.c_str()), pt.x, pt.y, fontColor, timeout, endTrigger, 0);
}

int16 Scene::Sprites::addSprites(const char *name, int load_flags) {
	return kernel_load_series(name, load_flags);
}

int16 Scene::Sequences::addSpriteCycle(int series_id, int mirror, word ticks, word interval_ticks,
	word start_ticks, int expire) {
	return kernel_seq_forward(series_id, mirror, ticks, interval_ticks, start_ticks, expire);
}

int16 Scene::Sequences::addReverseSpriteCycle(int series_id, int mirror, word ticks, word interval_ticks,
		word start_ticks, int expire) {
	return kernel_seq_backward(series_id, mirror, ticks, interval_ticks, start_ticks, expire);
}

int Scene::Sequences::startPingPongCycle(int series_id, int mirror, word ticks, word interval_ticks,
	word start_ticks, int expire) {
	return kernel_seq_pingpong(series_id, mirror, ticks, interval_ticks, start_ticks, expire);
}

void Scene::Sequences::remove(int sequence_id) {
	kernel_seq_delete(sequence_id);
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

int Scene::loadAnimation(const char *name, int trigger_code) {
	return kernel_run_animation(name, trigger_code);
}

void VM::Dialogs::show(int id) {
	text_show(id);
}

void VM::Dialogs::showItem(int object_id, int message) {
	object_examine(object_id, message, 0);
}

void VM::Palette::setEntry(int color, int r, int g, int b) {
	pal_change_color(color, r, g, b);
}

void VM::Sound::command(int num) {
	g_engine->_soundManager->command(num);
}

char *formAnimName(char type, int num) {
	return kernel_name(type, num);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
