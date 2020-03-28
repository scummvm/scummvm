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
#include "common/rect.h"

#include "engines/myst3/archive.h"

namespace Graphics {
struct Surface;
}

namespace Myst3 {

class Myst3Engine;

class Effect {
public:
	struct FaceMask {
		FaceMask();
		~FaceMask();

		static Common::Rect getBlockRect(uint x, uint y);

		Graphics::Surface *surface;
		bool block[10][10];
	};

	virtual ~Effect();

	virtual bool update() = 0;
	virtual void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst) = 0;

	bool hasFace(uint face) { return _facesMasks.contains(face); }
	Common::Rect getUpdateRectForFace(uint face);

	// Public and static for use by the debug console
	static FaceMask *loadMask(Common::SeekableReadStream *maskStream);

protected:
	Effect(Myst3Engine *vm);

	bool loadMasks(const Common::String &room, uint32 id, Archive::ResourceType type);

	Myst3Engine *_vm;

	typedef Common::HashMap<uint, FaceMask *> FaceMaskMap;
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
	Common::SeekableReadStream *_shakeStrength;

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

	uint32 _lastTick;
	uint _magnetEffectShakeStep;
	float _pitchOffset;
	float _headingOffset;

};

class RotationEffect : public Effect {
public:
	static RotationEffect *create(Myst3Engine *vm);
	virtual ~RotationEffect();

	bool update();
	void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst);

	float getHeadingOffset() { return _headingOffset; }

protected:
	RotationEffect(Myst3Engine *vm);

	uint32 _lastUpdate;
	float _headingOffset;

};

class ShieldEffect : public Effect {
public:
	static ShieldEffect *create(Myst3Engine *vm, uint32 id);
	virtual ~ShieldEffect();

	bool update();
	void applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst);

protected:
	ShieldEffect(Myst3Engine *vm);
	bool loadPattern();

	uint32 _lastTick;
	float _amplitude;
	float _amplitudeIncrement;

	uint8 _pattern[4096];
	int32 _displacement[256];
};

} // End of namespace Myst3

#endif // EFFECTS_H_
