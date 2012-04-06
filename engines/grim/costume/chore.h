/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/animation.h"

#include "engines/grim/pool.h"

namespace Grim {

class Costume;
class Animation;
class Component;
class TextSplitter;

struct TrackKey {
	int time, value;
};

struct ChoreTrack {
	int compID;
	int numKeys;
	TrackKey *keys;
	Component *component;
};

class Chore {
public:
	Chore();
	virtual ~Chore();
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
	bool isLooping() { return _looping; }

	virtual int getId() { return _choreId; }
	
	void setOwner(Costume *owner) { _owner = owner; }
	void createTracks(int num);

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
	friend class EMICostume;
};

class PoolChore : public PoolObject<PoolChore>, public Chore {
public:
	virtual int getId() { return PoolObject<PoolChore>::getId(); }
	static int32 getStaticTag() { return MKTAG('C', 'H', 'O', 'R'); }
};

} // end of namespace Grim

#endif
