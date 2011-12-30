/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_CHORE_H
#define GRIM_CHORE_H

#include "engines/grim/textsplit.h"
#include "engines/grim/animation.h"

#include "engines/grim/pool.h"

namespace Grim {

class Costume;
class Animation;

struct TrackKey {
	int time, value;
};

struct ChoreTrack {
	int compID;
	int numKeys;
	TrackKey *keys;
};

class Chore {
public:
	Chore();
	~Chore();
	void load(int id, Costume *owner, TextSplitter &ts);
	void play();
	void playLooping();
	void setLooping(bool val) { _looping = val; }
	void stop();
	void update(uint time);
	void setLastFrame();
	void fadeIn(uint msecs);
	void fadeOut(uint msecs);
	void cleanup();

	bool isPlaying() { return _playing; }

	virtual int getId() { return _choreId; }

private:
	void setKeys(int startTime, int stopTime);
	void fade(Animation::FadeMode, uint msecs);

	Costume *_owner;

	int _choreId;
	int _length;
	int _numTracks;
	ChoreTrack *_tracks;
	char _name[32];

	bool _hasPlayed, _playing, _looping;
	int _currTime;

	friend class Costume;
};

class PoolChore : public PoolObject<PoolChore, MKTAG('C', 'H', 'O', 'R')>, public Chore {
public:
	virtual int getId() { return PoolObject<PoolChore, MKTAG('C', 'H', 'O', 'R')>::getId(); }
};

} // end of namespace Grim

#endif
