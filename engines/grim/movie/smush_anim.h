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

#ifndef GRIM_MOVIE_SMUSH_ANIM_H
#define GRIM_MOVIE_SMUSH_ANIM_H

#include "common/scummsys.h"

namespace Grim {

class SmushTimer;
class SmushSound;

class SmushAnimClient {
public:
	virtual ~SmushAnimClient() {}

	virtual int animatorRender(int flags) = 0;
	virtual void resetFrameCounter() = 0;
	virtual int getFrameCounter() = 0;
};

class SmushAnimPlay {

private:
	SmushTimer *_timer;
	SmushSound *_sound;
	int _frameTimer;
	int _syncTimer;
	int _flags;
	int _finished;
	int _framesDue;
	int _lateTicks;
	int _nominalDelay;
	int _currentDelay;
	int _skipFlag;
	int _state;
	int _started;
	int _syncPaused;

	static void frameTimer(int timer, void *context);
	static void syncTimer(int timer, void *context);

	int anmRenderBody(SmushAnimClient *client);

public:
	SmushAnimPlay(SmushTimer *timer, SmushSound *sound);
	~SmushAnimPlay();

	int anmStart(int fps, int flags);
	void anmStop();
	int anmRender(SmushAnimClient *client);
	void anmSetRate(int fps);
	void anmSyncUp();
	bool anmRenderQuery();
	int anmGetTime();

};

} // End of namespace Grim
#endif
