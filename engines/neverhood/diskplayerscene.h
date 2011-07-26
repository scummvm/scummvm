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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NEVERHOOD_DISKPLAYERSCENE_H
#define NEVERHOOD_DISKPLAYERSCENE_H

#include "neverhood/neverhood.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"
#include "neverhood/smackerplayer.h"

namespace Neverhood {

class DiskplayerScene;

class Class494 : public AnimatedSprite {
public:
	Class494(NeverhoodEngine *vm);
	void sub43BE20();
protected:	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub43BE00();
};

class DiskplayerPlayButton : public StaticSprite {
public:
	DiskplayerPlayButton(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene);
	void press();
	void release();
protected:
	DiskplayerScene *_diskplayerScene;	
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	bool _isPlaying;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class DiskplayerSlot : public Entity {
public:
	DiskplayerSlot(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene, int elementIndex, int value);
	void activate();
	void stop();
	void appear();
	void play();
	void setFlag2(bool value) { _flag2 = value; }
protected:
	DiskplayerScene *_diskplayerScene;
	SoundResource _soundResource;
	Sprite *_inactiveSlot;
	Sprite *_appearSlot;
	Sprite *_activeSlot;
	int _elementIndex;
	int _initialCountdown;
	int _countdown;
	bool _flag2;
	int _value;
	bool _flag;
	void update();	
};

class DiskplayerScene : public Scene {
public:
	DiskplayerScene(NeverhoodEngine *vm, Module *parentModule, int which);
	bool getFlag3() const { return _flag3; }
protected:
	SmackerPlayer *_smackerPlayer;
	DiskplayerPlayButton *_playButton;
	Class494 *_class494;
	DiskplayerSlot *_diskSlots[20];
	DiskplayerSlot *_class650;
	int _updateStatus;
	byte _diskAvailable[20];
	bool _flag4;
	int _which;
	int _diskIndex;
	int _appearCountdown;
	int _tuneInCountdown;
	bool _fullFlag;
	bool _inputDisabled;
	bool _flag3;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stop();
	void tuneIn();
	void playDisk();
	void playStatic();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_DISKPLAYERSCENE_H */
