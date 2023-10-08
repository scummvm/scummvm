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

#include "m4/burger/rooms/room.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

HotSpotRec Room::_wilburHotspot;
char Room::_wilburName[16];
char Room::_wilburVerb = 0;

void Room::setWilburHotspot() {
	Common::strcpy_s(_wilburName, "Wilbur");

	_wilburHotspot.clear();
	_wilburHotspot.vocab = _wilburName;
	_wilburHotspot.verb = &_wilburVerb;
	_wilburHotspot.feet_x = 0x7fff;
	_wilburHotspot.feet_y = 0x7fff;
	_wilburHotspot.cursor_number = kArrowCursor;
}

HotSpotRec *Room::custom_hotspot_which(int32 x, int32 y) {
	if (!_G(player).walker_in_this_scene || !_G(player).walker_visible)
		return nullptr;

	player_update_info();
	int y2 = _G(player_info).y - (_G(player_info).scale * 75 / 100);
	int y1 = _G(player_info).y - (_G(player_info).scale * 150 / 100);
	int xSize = _G(player_info).scale * 22 / 100;

	if (y > y2 || y <= y1 || imath_abs(x - _G(player_info).x) >= xSize)
		return nullptr;

	return &_wilburHotspot;
}

void Room::npc_say(const char *digiName, int trigger, const char *seriesName,
		int layer, bool shadow, int firstFrame, int lastFrame, int digiSlot, int digiVol) {
	term_message("npc_say: %s     npc_series: %s     npc_trigger: %d",
		digiName, seriesName, trigger);

	_G(npcTrigger) = trigger;

	if (seriesName) {
		_G(npcSpeech1) = series_play(seriesName, layer, 4, -1, 6, -1, 100, 0, 0, firstFrame, lastFrame);

		if (shadow) {
			char temp[20];
			Common::strcpy_s(temp, 20, seriesName);
			Common::strcat_s(temp, 20, "s");
			_G(npcSpeech2) = series_play(temp, layer + 1, 4, -1, 6, -1, 100, 0, 0, firstFrame, lastFrame);
		}
	}

	kernel_trigger_dispatch_now(kNPC_SPEECH_STARTED);
	KernelTriggerType oldMode = _G(kernel).trigger_mode;

	_G(kernel).trigger_mode = KT_DAEMON;
	digi_play(digiName, digiSlot, digiVol, kNPC_SPEECH_FINISHED);

	_G(kernel).trigger_mode = oldMode;
}

void Room::npc_say(int trigger, const char *seriesName, int layer, bool shadow,
		int firstFrame, int lastFrame, int digiSlot, int digiVol) {
	npc_say(conv_sound_to_play(), trigger, seriesName, layer, shadow,
		firstFrame, lastFrame, digiSlot, digiVol);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
