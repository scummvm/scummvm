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

#ifndef NEVERHOOD_MODULES_MODULE2800_SPRITES_H
#define NEVERHOOD_MODULES_MODULE2800_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class AsScene2803LightCord : public AnimatedSprite {
public:
	AsScene2803LightCord(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y);
	void stPulled();
	void stIdle();
	void setFileHashes(uint32 fileHash1, uint32 fileHash2);
protected:
	Scene *_parentScene;
	uint32 _fileHash1, _fileHash2;
	bool _isPulled, _isBusy;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPulled(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2803TestTubeOne : public AnimatedSprite {
public:
	AsScene2803TestTubeOne(NeverhoodEngine *vm, uint32 fileHash1, uint32 fileHash2);
protected:
	uint32 _fileHash1, _fileHash2;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2803Rope : public AnimatedSprite {
public:
	AsScene2803Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmReleased(int messageNum, const MessageParam &param, Entity *sender);
	void stReleased();
	void stHide();
};

class Scene2804;

class SsScene2804RedButton : public StaticSprite {
public:
	SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene);
protected:
	Scene2804 *_parentScene;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2804LightCoil : public StaticSprite {
public:
	SsScene2804LightCoil(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2804BeamCoilBody : public StaticSprite {
public:
	SsScene2804BeamCoilBody(NeverhoodEngine *vm);
};

class SsScene2804LightTarget : public StaticSprite {
public:
	SsScene2804LightTarget(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2804Flash : public StaticSprite {
public:
	SsScene2804Flash(NeverhoodEngine *vm);
	void show();
};

class AsScene2804CrystalWaves : public AnimatedSprite {
public:
	AsScene2804CrystalWaves(NeverhoodEngine *vm, uint crystalIndex);
	void show();
	void hide();
protected:
	uint _crystalIndex;
};

class AsScene2804Crystal : public AnimatedSprite {
public:
	AsScene2804Crystal(NeverhoodEngine *vm, AsScene2804CrystalWaves *asCrystalWaves, uint crystalIndex);
	void show();
	void hide();
	void activate();
	int16 getColorNum() const { return _colorNum; }
protected:
	AsScene2804CrystalWaves *_asCrystalWaves;
	uint _crystalIndex;
	int16 _colorNum;
	bool _isLightOn;
	bool _isShowing;
};

class SsScene2804CrystalButton : public StaticSprite {
public:
	SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex);
protected:
	Scene2804 *_parentScene;
	AsScene2804Crystal *_asCrystal;
	uint _crystalIndex;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2804BeamCoil : public AnimatedSprite {
public:
	AsScene2804BeamCoil(NeverhoodEngine *vm, Scene *parentScene, SsScene2804BeamCoilBody *ssBeamCoilBody);
	~AsScene2804BeamCoil() override;
protected:
	Scene *_parentScene;
	SsScene2804BeamCoilBody *_ssBeamCoilBody;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void show();
	void hide();
	void stBeaming();
	uint32 hmBeaming(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2804BeamTarget : public AnimatedSprite {
public:
	AsScene2804BeamTarget(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2806Spew : public AnimatedSprite {
public:
	AsScene2806Spew(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2808Dispenser : public StaticSprite {
public:
	SsScene2808Dispenser(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum, int testTubeIndex);
	void startCountdown(int index);
protected:
	Scene *_parentScene;
	int _countdown;
	int _testTubeSetNum, _testTubeIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808TestTube : public AnimatedSprite {
public:
	AsScene2808TestTube(NeverhoodEngine *vm, int testTubeSetNum, int testTubeIndex, SsScene2808Dispenser *ssDispenser);
	void fill();
	void flush();
	uint32 getFillLevel() const { return _fillLevel; }
protected:
	SsScene2808Dispenser *_ssDispenser;
	int _testTubeSetNum;
	uint32 _fillLevel;
	int _testTubeIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808Handle : public AnimatedSprite {
public:
	AsScene2808Handle(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum);
	void activate();
	void stActivated();
protected:
	Scene *_parentScene;
	int _testTubeSetNum;
	bool _isActivated;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmActivating(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808Flow : public AnimatedSprite {
public:
	AsScene2808Flow(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum);
	void start();
	void stKeepFlowing();
protected:
	Scene *_parentScene;
	int _testTubeSetNum;
	uint32 hmFlowing(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808LightEffect : public AnimatedSprite {
public:
	AsScene2808LightEffect(NeverhoodEngine *vm, int which);
protected:
	int _countdown;
	void update();
};

class AsScene2809Spew : public AnimatedSprite {
public:
	AsScene2809Spew(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2810Rope : public AnimatedSprite {
public:
	AsScene2810Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2812Winch : public AnimatedSprite {
public:
	AsScene2812Winch(NeverhoodEngine *vm);
	~AsScene2812Winch() override;
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2812Rope : public AnimatedSprite {
public:
	AsScene2812Rope(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRopingDown(int messageNum, const MessageParam &param, Entity *sender);
	void stRopingDown();
};

class AsScene2812TrapDoor : public AnimatedSprite {
public:
	AsScene2812TrapDoor(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene2801 : public Klaymen {
public:
	KmScene2801(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2803 : public Klaymen {
public:
	KmScene2803(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2803Small : public Klaymen {
public:
	KmScene2803Small(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stShrink();
	uint32 hmShrink(int messageNum, const MessageParam &param, Entity *sender);

	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2805 : public Klaymen {
public:
	KmScene2805(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2806 : public Klaymen {
public:
	KmScene2806(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
		bool needsLargeSurface, NRect *clipRects, uint clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2809 : public Klaymen {
public:
	KmScene2809(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
		bool needsLargeSurface, NRect *clipRects, uint clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2810Small : public Klaymen {
public:
	KmScene2810Small(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2810 : public Klaymen {
public:
	KmScene2810(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
		NRect *clipRects, uint clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2812 : public Klaymen {
public:
	KmScene2812(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2800_SPRITES_H */
