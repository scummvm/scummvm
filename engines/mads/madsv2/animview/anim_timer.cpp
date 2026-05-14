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
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

static int normalTimer1;
static int slow_frames, fast_frames, iffy_frames;

void anim_timer_init() {
	normalTimer1 = 0;
	slow_frames = fast_frames = iffy_frames = 0;
}

void anim_timer() {
	bool flag = false;
	int currTimer = g_system->getMillis();

	if (current_error_code || speechNum)
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

		}
	}

block1:
	warning("TODO: block1");
block2:
	warning("TODO: block2");

done:
	cycle_colors();
}

} // namespace AnimView
} // namespace MADSV2
} // namespace MADS
