/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SLUDGE_TIMING_H
#define SLUDGE_TIMING_H

namespace Sludge {

class Timer {
private:
	int _desiredfps; // desired frames per second
	uint32 _starttime, _endtime;
	uint32 _desired_frame_time;

public:
	void setDesiredfps(int t) { _desiredfps = t; }
	void init(void);
	void initSpecial(int t);
	void waitFrame(void);

	Timer():_desiredfps(300), _starttime(0), _endtime(0), _desired_frame_time(0){}
};

} // End of namespace Sludge

#endif
