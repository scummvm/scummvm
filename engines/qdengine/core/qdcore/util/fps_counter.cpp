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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/util/fps_counter.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

fpsCounter::fpsCounter(int period) : start_time_(0.0f),
	prev_time_(0.0f),
	period_(period),
	frame_count_(0),
	value_(-1.0f),
	value_min_(0.0f),
	value_max_(0.0f),
	min_frame_time_(1000.0f),
	max_frame_time_(0.0f) {
}

bool fpsCounter::quant() {
	float time = float(xclock());

	if (min_frame_time_ > time - prev_time_)
		min_frame_time_ = time - prev_time_;

	if (max_frame_time_ < time - prev_time_)
		max_frame_time_ = time - prev_time_;

	frame_count_++;
	prev_time_ = time;

	if (period_) {
		if (prev_time_ - start_time_ >= period_) {
			value_ = float(frame_count_) / (prev_time_ - start_time_) * 1000.0f;

			value_min_ = 1000.0f / max_frame_time_;
			value_max_ = 1000.0f / min_frame_time_;

			frame_count_ = 0;
			start_time_ = prev_time_;

			min_frame_time_ = 10000.0f;
			max_frame_time_ = 0.0f;

			return true;
		}
		return false;
	} else {
		value_ = float(frame_count_) / (float(xclock()) - start_time_) * 1000.0f;
		return true;
	}
}

void fpsCounter::reset() {
	prev_time_ = start_time_ = float(xclock());
	frame_count_ = 0;
	value_ = -1.0f;
	value_min_ = value_max_ = 0.0f;

	min_frame_time_ = 10000.0f;
	max_frame_time_ = 0.0f;
}

} // namespace QDEngine
