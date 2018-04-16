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

#ifndef NEVERHOOD_DISKPLAYERSCENE_H
#define NEVERHOOD_DISKPLAYERSCENE_H

#include "neverhood/neverhood.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"

namespace Neverhood {

class DiskplayerScene;
class SmackerPlayer;

class AsDiskplayerSceneKey : public AnimatedSprite {
public:
	AsDiskplayerSceneKey(NeverhoodEngine *vm);
	void stDropKey();
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stDropKeyDone();
};

class DiskplayerPlayButton : public StaticSprite {
public:
	DiskplayerPlayButton(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene);
	void press();
	void release();
protected:
	DiskplayerScene *_diskplayerScene;
	bool _isPlaying;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class DiskplayerSlot : public Entity {
public:
	DiskplayerSlot(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene, int slotIndex, bool isAvailable);
	void activate();
	void stop();
	void appear();
	void play();
	void setLocked(bool isLocked) { _isLocked = isLocked; }
protected:
	DiskplayerScene *_diskplayerScene;
	Sprite *_inactiveSlot;
	Sprite *_appearSlot;
	Sprite *_activeSlot;
	int _initialBlinkCountdown;
	int _blinkCountdown;
	bool _isLocked;
	bool _isBlinking;
	void update();
};

enum {
	kUSStopped		= 0,
	kUSTuningIn		= 1,
	kUSPlaying		= 2,
	kUSPlayingFinal	= 3
};

class DiskplayerScene : public Scene {
public:
	DiskplayerScene(NeverhoodEngine *vm, Module *parentModule, int paletteIndex);
	bool getDropKey() const { return _dropKey; }
protected:
	SmackerPlayer *_diskSmackerPlayer;
	DiskplayerPlayButton *_ssPlayButton;
	AsDiskplayerSceneKey *_asKey;
	DiskplayerSlot *_diskSlots[20];
	DiskplayerSlot *_finalDiskSlot;
	int _updateStatus;
	bool _diskAvailable[20];
	int _diskIndex;
	int _appearCountdown;
	int _tuneInCountdown;
	bool _hasAllDisks;
	bool _inputDisabled;
	bool _dropKey;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void openSmacker(uint32 fileHash, bool keepLastFrame);
	void stop();
	void tuneIn();
	void playDisk();
	void playStatic();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_DISKPLAYERSCENE_H */
