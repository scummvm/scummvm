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

#ifndef M4_SUBTITLES_H
#define M4_SUBTITLES_H

#include "common/hashmap.h"
#include "video/subtitles.h"

namespace M4 {

class M4Subtitles : Video::Subtitles {
public:
	M4Subtitles();
	~M4Subtitles();

	Common::String getSubtitle(const Common::String &audioFile) const;
	bool drawSubtitle(const Common::String &audioFile) const;
	void clearSubtitle() const;
	void updateSubtitleOverlay() const;

private:
	void setupSubtitles();
	bool loadSubtitles(const Common::String &filename);

	Common::HashMap<Common::String, Common::String> _subtitles;
};

} // namespace M4

#endif /* M4_SUBTITLES_H */
