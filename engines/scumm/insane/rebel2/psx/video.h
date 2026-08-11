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
 */

#ifndef SCUMM_INSANE_REBEL2_PSX_VIDEO_H
#define SCUMM_INSANE_REBEL2_PSX_VIDEO_H

#include "video/psx_decoder.h"

namespace Scumm {

class RA2PSXStreamDecoder : public Video::PSXStreamDecoder {
public:
	enum FrameVersion {
		kVersion2 = 2,
		kVersion3 = 3
	};

	explicit RA2PSXStreamDecoder(FrameVersion version, uint32 frameCount = 0);
	bool loadStream(Common::SeekableReadStream *stream) override;

private:
	FrameVersion _version;
};

} // End of namespace Scumm

#endif
