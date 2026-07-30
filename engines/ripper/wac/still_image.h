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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_WAC_STILL_IMAGE_H
#define RIPPER_WAC_STILL_IMAGE_H

#include "common/rect.h"
#include "common/str.h"

#include "ripper/resources.h"

namespace Ripper {

class WacDatabaseSession;

class WacStillImageViewer {
public:
	explicit WacStillImageViewer(WacDatabaseSession *database);

	uint16 run(byte entryIndex, const Common::String &entryLabel,
		const Common::String &path);

private:
	bool load(const Common::String &path);
	void draw() const;
	void drawScrollControls() const;
	int findScrollControl(const Common::Point &point) const;
	void scroll(int delta);

	WacDatabaseSession *_database;
	BitmapAssetFrame _image;
	uint _scrollOffset;
	int _scrollControl;
};

} // End of namespace Ripper

#endif // RIPPER_WAC_STILL_IMAGE_H
