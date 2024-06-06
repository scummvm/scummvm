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

#ifndef __QD_ANIMATION_MAKER_H__
#define __QD_ANIMATION_MAKER_H__


namespace QDEngine {

typedef void (*maker_progress_fnc)(int percents_loaded, void *data);

class qdAnimationMaker {
public:
	qdAnimationMaker();
	~qdAnimationMaker();

	bool insert_frame(class qdAnimation *p, const char *fname, int &insert_pos, bool insert_after = true, bool dupe_check = false);
	bool insert_frames(class qdAnimation *p, const char *folder, int &insert_pos, bool insert_after = true);

	maker_progress_fnc set_callback(maker_progress_fnc p, void *data = 0);

	void set_default_frame_length(float len) {
		default_frame_length_ = len;
	}

private:
	float default_frame_length_;    // длительность кадра по умолчанию (в секундах)

	void *callback_data_;
	maker_progress_fnc progress_callback_;
};

} // namespace QDEngine

#endif /* __QD_ANIMATION_MAKER_H__ */
