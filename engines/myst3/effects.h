/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef EFFECTS_H_
#define EFFECTS_H_

#include "common/hashmap.h"

#include "engines/myst3/directorysubentry.h"

namespace Graphics {
struct Surface;
}

namespace Myst3 {

class Myst3Engine;

class Effect {
public:
	virtual ~Effect();

	virtual bool update() = 0;
	virtual void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst) = 0;

	bool hasFace(uint face) { return _facesMasks.contains(face); }

	// Public and static for use by the debug console
	static Graphics::Surface *loadMask(Common::SeekableReadStream *maskStream);

protected:
	Effect(Myst3Engine *vm);

	bool loadMasks(uint32 id, DirectorySubEntry::ResourceType type);
	void flipVertical(Graphics::Surface *s);

	Myst3Engine *_vm;

	typedef Common::HashMap<uint, Graphics::Surface *> FaceMaskMap;
	FaceMaskMap _facesMasks;
};

class WaterEffect : public Effect {
public:
	static WaterEffect *create(Myst3Engine *vm, uint32 id);
	virtual ~WaterEffect();

	bool update();
	void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst);

protected:
	WaterEffect(Myst3Engine *vm);

	void doStep(float position, bool isFrame);
	void apply(Graphics::Surface *src, Graphics::Surface *dst, Graphics::Surface *mask,
			bool bottomFace, int32 waterEffectAmpl);

	uint32 _lastUpdate;
	int32 _step;

	int8 _bottomDisplacement[640];
	int8 _verticalDisplacement[640];
	int8 _horizontalDisplacements[5][640];

private:
	bool isRunning();
};

class LavaEffect : public Effect {
public:
	static LavaEffect *create(Myst3Engine *vm, uint32 id);
	virtual ~LavaEffect();

	bool update();
	void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst);

protected:
	LavaEffect(Myst3Engine *vm);

	void doStep(int32 position, float ampl);

	uint32 _lastUpdate;
	int32 _step;

	int32 _displacement[256];
};

class MagnetEffect : public Effect {
public:
	static MagnetEffect *create(Myst3Engine *vm, uint32 id);
	virtual ~MagnetEffect();

	bool update();
	void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst);

protected:
	MagnetEffect(Myst3Engine *vm);

	void apply(Graphics::Surface *src, Graphics::Surface *dst, Graphics::Surface *mask, int32 position);

	int32 _lastSoundId;
	Common::MemoryReadStream *_shakeStrength;

	uint32 _lastTime;
	float _position;
	float _lastAmpl;
	int32 _verticalDisplacement[256];
};

class ShakeEffect : public Effect {
public:
	static ShakeEffect *create(Myst3Engine *vm);
	virtual ~ShakeEffect();

	bool update();
	void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst);

	float getPitchOffset() { return _pitchOffset; }
	float getHeadingOffset() { return _headingOffset; }

protected:
	ShakeEffect(Myst3Engine *vm);

	uint32 _lastFrame;
	uint _magnetEffectShakeStep;
	float _pitchOffset;
	float _headingOffset;

};

} // End of namespace Myst3

#endif // EFFECTS_H_
