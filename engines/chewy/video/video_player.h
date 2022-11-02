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

#ifndef CHEWY_VIDEO_VIDEO_PLAYER_H
#define CHEWY_VIDEO_VIDEO_PLAYER_H

namespace Chewy {

class CfoDecoder;

class VideoPlayer {
public:
	VideoPlayer() { _playCount = 0; }

	/**
	 * @Plays a video file
	 * @param num - the video file number
	 * @param stopMusic - stop music before starting the video
	 * @param disposeMusic - dispose music after stopping the video
	 * @return - true if played, false if skipped
	*/
	bool playVideo(uint num, bool stopMusic = true, bool disposeMusic = true);

private:
	bool handleCustom(uint num, uint frame, CfoDecoder *cfoDecoder);
	int _playCount;
};

} // End of namespace Chewy

#endif
