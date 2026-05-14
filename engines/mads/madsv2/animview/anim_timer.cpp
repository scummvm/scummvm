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

#include "mads/madsv2/animview/anim_timer.h"
#include "mads/madsv2/animview/animview.h"
#include "mads/madsv2/animview/functions.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

static const byte FX_TIMES[16] = {
	0, 110, 110, 64, 64, 64, 64, 64, 64, 64, 64, 0, 0, 0, 0, 0
};
constexpr int sound_var1 = 49;

constexpr int MESSAGES_COUNT = 8;
static int messageHandle[MESSAGES_COUNT];

static int paletteHandle;
static int palIndex1, palIndex2;
static int matteId;
static int normalTimer1, messageCount;
static int frameViewX, frameViewY;
static int currentViewX, currentViewY;

void anim_timer_init() {
	paletteHandle = 0;
	palIndex1 = palIndex2 = 0;
	matteId = 0;
	normalTimer1 = messageCount = 0;
	frameViewX = frameViewY = 0;
	currentViewX = currentViewY = 0;
	normalTimer1 = 0;
}

void anim_timer() {
	bool flag = false;
	uint32 currTimer = g_system->getMillis();
	Speech *speech;
	int sound, count;

	if (current_error_code || speechStream)
		goto done;
	if (currentFrame < 0 || currentFrame >= maxFrame)
		goto done;
	if (foundSeries && (currentFrame <= seriesMinFrame || currentFrame > seriesMaxFrame))
		goto done;

	if (timerFlag1) {
		timerFlag1 = false;
		flag = g_engine->isSpeechPlaying();
		goto block1;
	}
	if (normalTimer1)
		goto block2;

	if (runFx == 0) {
		if (peelFlag && timer2 >= currTimer && timer1 <= currTimer) {
			anim_peel();

			timer2 += current_anim->misc_peel_rate;

			if (timer1 <= currTimer) {
				matte_frame(runFx, false);
			}
		}

		if (timer1 < currTimer)
			goto done;
	} else {
		uint32 time = timer1 - FX_TIMES[runFx];
		if (time > currTimer)
			goto done;
	}

	sound = current_anim->frame[currentFrame].sound;
	if (sound && runFx == 0)
		g_engine->_soundManager->command(sound);

	if (speechIndex != -1)
		goto block2;
	speechIndex = current_anim->frame[currentFrame].speech;
	if (speechIndex == -1)
		goto block2;

	speech = &current_anim->speech[speechIndex];
	flag = (speech->display_condition & 0x4000) &&
		(speech->display_condition & 0x800) &&
		(speech->resource_id >= 0);

	if (speech->sound && (!flag || sound_var1 == 49)) {
		g_engine->_soundManager->command(speech->sound);
	}

	if (flag) {
		speechStream = speech->speech;
		speechFlags = speech->flags;
		goto done;
	}

block1:
	speechLoops = flag ? current_anim->speech[speechIndex].speech_loops :
		current_anim->speech[speechIndex].non_speech_loops;
	runVal6 = 0;
	runVal7 = -1;

	speech = &current_anim->speech[speechIndex];
	if (!loadFontFlag && (speech->display_condition & 0x8000) &&
			(speech->display_condition & 0x2000)) {
		paletteHandle = pal_alloc_color(-1, 0, &palIndex1, &speech->color[0]);
		paletteHandle = pal_alloc_color(paletteHandle, -1, &palIndex2, &speech->color[1]);

		mcga_setpal_range(&master_palette, MIN(palIndex1, palIndex2), ABS(palIndex2 - palIndex1));
		runVal8 = -1;
		matteId = matte_add_message(current_anim->font, speech->text,
			speech->x, speech->y, (palIndex2 << 8) | palIndex1,
			current_anim->font_auto_spacing);
		messageCount = 0;

		for (count = 0; count < current_anim->num_speech; ++count) {
			Speech *s = &current_anim->speech[count];
			if (count != speechIndex && messageCount < MESSAGES_COUNT &&
					s->first_frame <= currentFrame && s->last_frame > currentFrame) {
				messageHandle[messageCount++] = matte_add_message(current_anim->font, s->text,
					s->x, s->y, (palIndex2 << 8) | palIndex1,
					current_anim->font_auto_spacing);
			}
		}
	}

block2:
	frameViewX = current_anim->frame[currentFrame].view_x;
	frameViewY = current_anim->frame[currentFrame].view_y;

	if (frameViewX != currentViewX || frameViewY != currentViewY) {
		if (!picture_map.one_to_one) {
			tile_pan(&picture_map, frameViewX, frameViewY);
			tile_pan(&depth_map, frameViewX, frameViewY);

		}

		image_marker = 1;
		image_list[0].flags = IMAGE_REFRESH;
		image_list[0].segment_id = 0xff;
		currentViewX = frameViewX;
		currentViewY = frameViewY;
	} else {
		for (count = 0; count < image_marker; ++count) {
			if (image_list[count].flags == IMAGE_REFRESH)
				image_list[count].flags = IMAGE_ERASE;
		}
	}

	// TODO: More block 2 stuff
	warning("TODO: block2");

done:
	cycle_colors();
}

} // namespace AnimView
} // namespace MADSV2
} // namespace MADS
