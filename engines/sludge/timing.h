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
#ifndef SLUDGE_TIMING_H
#define SLUDGE_TIMING_H

namespace Sludge {

class Timer {
public:
	Timer();

	void setDesiredFPS(int t) { _desiredFPS = t; }
	void reset(void);
	void init(void);
	void initSpecial(int t);
	void waitFrame(void);

	int getLastFps() const { return _lastFPS; }

private:
	int _desiredFPS; // desired frames per second
	uint32 _startTime, _endTime;
	uint32 _desiredFrameTime;
	uint32 _addNextTime;

	// FPS stats
	void updateFpsStats();
	int _lastFPS;
	int _thisFPS;
	uint32 _lastSeconds;
};

} // End of namespace Sludge

#endif
