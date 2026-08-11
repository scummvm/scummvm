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
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/random.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/shared.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"
#include "scumm/insane/rebel2/psx/video.h"

#include <math.h>

namespace Scumm {

#ifdef USE_TINYGL
const int kLevel1FrameRate = 30;

enum {
	kLevel1SfxTieApproachA = 0x19,
	kLevel1SfxTieApproachB = 0x1a,
	kLevel1SfxTieExplode = 0x1b,
	kLevel1SfxTieFire = 0x17,
	kLevel1SfxPlayerFire = 0x18,
	kLevel1SfxPlayerHit = 0x36,
	kLevel1SfxLowShield = 0x2d,
	kLevel1SfxEngineLeft = 0x32,
	kLevel1SfxEngineRight = 0x33,
	kLevel1SfxEngineOutside = 0x41,
	kLevel1SfxExtraLife = 0x44
};

// Outside engine base volume, and the per-frame step of the mix cross-fade.
enum {
	kLevel1OutsideBase = 0x46,
	kLevel1MixStep = 2,
	kLevel1MixMaximum = 0x7f
};

int approachMix(int current, int target) {
	if (current < target)
		return MIN(target, current + kLevel1MixStep);
	return MAX(target, current - kLevel1MixStep);
}

// Points per kill and the extra life thresholds, by difficulty.
const int kLevel1KillScore[3] = { 80, 100, 150 };
const int kLevel1ExtraLife[3][3] = {
	{ 5000, 5000, 10000 },
	{ 5000, 5000, 30000 },
	{ 10000, 20000, 30000 }
};

// The TIEs fly five point Hermite splines in world space; angles run 4096 units to
// the turn and fractions are 1/4096.
enum {
	kLevel1SplinePoints = 5,
	kLevel1SplineStep = 0x40,
	kLevel1LeadFrames = 30,
	kLevel1EnemyCount = 3,
	kLevel1ShotCount = 8,
	kLevel1ShieldFull = kRA2PSXShieldFull,
	kLevel1LowShield = kRA2PSXLowShield,
	// The gauge chases the real value by 40 a frame, and a hit shakes the view.
	kLevel1ShieldStep = 0x28,
	kLevel1ShakeFrames = 6,
	kLevel1FireFacing = 0xf3c,
	kLevel1HitRadius = 0xc000,
	// ra2FirePlayerShot alternates between two diagonal cannon pairs on every trigger.
	kLevel1BoltCount = 2
};

// Ticks the original insists on between single shots, and its auto fire repeat, indexed
// by view.
const int kLevel1FireGap[2] = { 4, 5 };
const int kLevel1AutoFireRepeat[2] = { 12, 14 };
// The cockpit shell drifts with the crosshair by these divisors.
const int kLevel1CockpitDriftX = 0x628b;
const int kLevel1CockpitDriftY = 0x3800;

const int kLevel1SplineDepth[kLevel1SplinePoints] = { 17000, 14000, 11000, -4000, -8000 };

// Bolt damage by view then difficulty, and the far heavier damage from a TIE that rams.
const int kLevel1BoltDamage[2][3] = { { 100, 200, 240 }, { 112, 160, 220 } };
const int kLevel1RamDamage[3] = { 256, 512, 1024 };
// Per cent chance that a shot is aimed at the player instead of fired straight ahead.
const int kLevel1AimChance[2][3] = { { 5, 5, 5 }, { 15, 20, 20 } };
// A TIE that gets this close without being shot collides with the player.
const int kLevel1RamRadius[2] = { 200, 220 };

// ra2InitializeCrosshair's box and start point, in the original's 240 line frame.
enum {
	kLevel1AimLeft = 30,
	kLevel1AimWidth = 260,
	kLevel1AimTop = 48,
	kLevel1AimHeight = 130,
	kLevel1AimStartX = 160,
	kLevel1AimStartY = 113
};

// Swapping views dollies the ship out to 1000 and back to 40 at 20 a tick while the
// cockpit shell zooms away; the original locks the controls until it lands.
enum {
	kLevel1ViewCockpitZ = 40,
	kLevel1ViewOutsideZ = 1000,
	kLevel1ViewStep = 20,
	kLevel1ViewRecentre = 10,
	kLevel1ViewZoomOutZ = 400,
	kLevel1ViewZoomInZ = 300,
	kLevel1CockpitScaleStep = 0xe0,
	kLevel1CockpitScaleMin = 0x400,
	kLevel1CockpitScaleFull = 0x1000,
	kLevel1ShipSwellStep = 200
};

// The original truncates its 1/4096 products toward zero.
int fixedShift12(int value) {
	return (value < 0 ? value + 0xfff : value) >> 12;
}

void normalizeVector12(const int *source, int *result) {
	const int x = source[0];
	const int y = source[1];
	const int z = source[2];
	const double square = (double)x * x + (double)y * y + (double)z * z;
	const int length = MAX(1, (int)sqrt(square));
	result[0] = x * 4096 / length;
	result[1] = y * 4096 / length;
	result[2] = z * 4096 / length;
}

// asin(value / 4096) in the PlayStation's 4096 units to the turn.
int lookupAsinAngle(int value) {
	return (int)(asin(CLIP(value, 0, 4096) / 4096.0) * 651.8986469044033 + 0.5);
}

int signedAsinAngle(int value) {
	return value < 0 ? -lookupAsinAngle(-value) : lookupAsinAngle(value);
}

struct RA2PSXLevel1Spline {
	RA2PSXLevel1Spline() {
		memset(this, 0, sizeof(*this));
		segmentCount = kLevel1SplinePoints;
		step = kLevel1SplineStep;
	}

	int control[kLevel1SplinePoints][3];
	int tangentA[3];
	int tangentB[3];
	int previous[3];
	int coefficient[3][4];
	int position[3];
	int segment;
	int segmentCount;
	int t;
	int step;
};

void updateSplineTangents(RA2PSXLevel1Spline &spline, int segment) {
	for (int axis = 0; axis < 3; ++axis)
		spline.tangentA[axis] = spline.tangentB[axis];

	if (segment == 0) {
		for (int axis = 0; axis < 3; ++axis) {
			spline.tangentB[axis] = spline.control[1][axis] - spline.control[0][axis];
			spline.previous[axis] = spline.tangentB[axis];
			spline.tangentA[axis] = spline.control[0][axis];
		}
		// The original only overwrites x and y, leaving z at the first control point.
		spline.tangentA[0] = 1;
		spline.tangentA[1] = 1;
	} else if (segment == spline.segmentCount - 1) {
		for (int axis = 0; axis < 3; ++axis)
			spline.tangentB[axis] = spline.previous[axis];
	} else {
		for (int axis = 0; axis < 3; ++axis) {
			const int old = spline.previous[axis];
			spline.previous[axis] = spline.control[segment + 1][axis] -
					spline.control[segment][axis];
			spline.tangentB[axis] = (old + spline.previous[axis]) / 2;
		}
	}
}

// Hermite basis, one row per source term and one column per power of t.
const int kLevel1SplineBasis[4][4] = {
	{ 2, -3, 0, 1 },
	{ -2, 3, 0, 0 },
	{ 1, -2, 1, 0 },
	{ 1, -1, 0, 0 }
};

void buildSplineSegmentCoefficients(RA2PSXLevel1Spline &spline, int segment) {
	if (segment < 1 || segment >= kLevel1SplinePoints)
		return;
	for (int axis = 0; axis < 3; ++axis) {
		const int source[4] = {
			spline.control[segment - 1][axis], spline.control[segment][axis],
			spline.tangentA[axis], spline.tangentB[axis]
		};
		for (int power = 0; power < 4; ++power) {
			int value = 0;
			for (int term = 0; term < 4; ++term)
				value += source[term] * kLevel1SplineBasis[term][power];
			spline.coefficient[axis][power] = value;
		}
	}
}

void advanceSplineObject(RA2PSXLevel1Spline &spline) {
	const int t1 = spline.t;
	const int t2 = t1 * t1 >> 12;
	const int t3 = t1 * t2 >> 12;
	for (int axis = 0; axis < 3; ++axis)
		spline.position[axis] = (int16)(((spline.coefficient[axis][0] * t3 +
				spline.coefficient[axis][1] * t2 +
				spline.coefficient[axis][2] * t1) >> 12) + spline.coefficient[axis][3]);

	spline.t += spline.step;
	if (spline.t <= 0xfff)
		return;
	spline.t -= 0x1000;
	if (++spline.segment == spline.segmentCount)
		return;
	updateSplineTangents(spline, spline.segment);
	buildSplineSegmentCoefficients(spline, spline.segment + 1);
}

void randomizeTieSplineControlPoints(RA2PSXLevel1Spline &spline, int mode,
		int spread, int base, Common::RandomSource &random) {
	if (mode < 1 || mode > 3)
		return;
	const int offset = base / 2;
	if (mode != 1) {
		for (int axis = 0; axis < 2; ++axis)
			spline.control[1][axis] += (int)random.getRandomNumber(spread * 2 - 1) -
					spread + offset;
	}
	if (mode != 2) {
		for (int axis = 0; axis < 2; ++axis)
			spline.control[2][axis] += (int)random.getRandomNumber(spread * 2 - 1) -
					spread + offset;
	}
	if (mode == 1)
		return;
	for (int axis = 0; axis < 2; ++axis)
		spline.control[3][axis] += (int)random.getRandomNumber(spread * 2 - 1) - spread + offset;
}

// Head on attack run: the far and near ends are picked first, the rest strung between them.
void initTieSplinePatternA(RA2PSXLevel1Spline &spline, Common::RandomSource &random) {
	spline = RA2PSXLevel1Spline();

	int last = (int)random.getRandomNumber(349) + 500;
	if (!random.getRandomBit())
		last = -last;
	spline.control[4][0] = last;
	last = (int)random.getRandomNumber(349) + 500;
	if (!random.getRandomBit())
		last = -last;
	spline.control[4][1] = last;
	spline.control[4][2] = kLevel1SplineDepth[4];

	const int lead = spline.control[4][0] > 0 ? -kLevel1SplineDepth[0] : kLevel1SplineDepth[0];
	spline.control[0][0] = (int)random.getRandomNumber(179) * lead / 640;
	spline.control[0][1] = spline.control[4][1] < 1 ? 0 :
			(int)random.getRandomNumber(139) * -kLevel1SplineDepth[0] / 640;
	spline.control[0][2] = kLevel1SplineDepth[0];

	int direction[3];
	for (int axis = 0; axis < 3; ++axis)
		direction[axis] = spline.control[0][axis] - spline.control[4][axis];
	normalizeVector12(direction, direction);
	for (int point = 3; point >= 1; --point) {
		const int depth = kLevel1SplineDepth[point];
		spline.control[point][0] = fixedShift12(direction[0] * depth) + spline.control[4][0];
		spline.control[point][1] = fixedShift12(direction[1] * depth) + spline.control[4][1];
		spline.control[point][2] = depth;
	}

	randomizeTieSplineControlPoints(spline, (int)random.getRandomNumber(2),
			(int)random.getRandomNumber(499) + 1000, 800, random);
	updateSplineTangents(spline, 0);
	buildSplineSegmentCoefficients(spline, 1);
}

// Loose weave: every control point is scattered across the view at its own depth.
void initTieSplinePatternB(RA2PSXLevel1Spline &spline, Common::RandomSource &random) {
	spline = RA2PSXLevel1Spline();

	for (int point = 0; point < kLevel1SplinePoints - 1; ++point) {
		const int depth = kLevel1SplineDepth[point];
		spline.control[point][2] = depth;
		spline.control[point][0] = ((int)random.getRandomNumber(259) - 130) * depth / 640;
		spline.control[point][1] = ((int)random.getRandomNumber(179) - 90) * depth / 640;
	}

	const int depth = kLevel1SplineDepth[kLevel1SplinePoints - 1];
	spline.control[4][2] = depth;
	int offset = (int)random.getRandomNumber(299) + 800;
	if ((int)random.getRandomNumber(1999) > 1000)
		offset = -offset;
	spline.control[4][0] = offset * depth / 640;
	offset = (int)random.getRandomNumber(299) + 600;
	if ((int)random.getRandomNumber(1999) > 1000)
		offset = -offset;
	spline.control[4][1] = offset * depth / 640;

	spline.control[0][0] += (int)random.getRandomNumber(2999) - 1500;
	spline.control[0][1] += (int)random.getRandomNumber(1499) - 750;
	updateSplineTangents(spline, 0);
	buildSplineSegmentCoefficients(spline, 1);
}

struct RA2PSXLevel1Enemy {
	RA2PSXLevel1Enemy() : active(false), previousYaw(0), facing(0), fireCountdown(0) {
		rotation[0] = rotation[1] = rotation[2] = 0;
	}

	bool active;
	RA2PSXLevel1Spline path;
	// A copy of the path run 30 frames ahead; the gap gives the heading.
	RA2PSXLevel1Spline lead;
	int rotation[3];
	int previousYaw;
	int facing;
	int fireCountdown;
};

// The fireball billboard is scaled about eight times its 68x56 texture, drifts toward
// the camera and runs one frame per tick; only five may burn at once.
enum {
	kLevel1ExplosionSlots = 5,
	kLevel1ExplosionScale = 0x7fff,
	kLevel1ExplosionDrift = 0x32,
	kLevel1DebrisCount = 16,
	kLevel1DebrisLife = 30
};

struct RA2PSXLevel1Explosion {
	RA2PSXLevel1Explosion() : frame(-1), rotation(0) {
		position[0] = position[1] = position[2] = 0;
	}

	int frame;
	int rotation;
	int position[3];
};

struct RA2PSXLevel1Debris {
	RA2PSXLevel1Debris() : life(0), model(0), puffCountdown(0) {
		for (int axis = 0; axis < 3; ++axis)
			position[axis] = velocity[axis] = rotation[axis] = spin[axis] = 0;
	}

	int life;
	int model;
	int puffCountdown;
	int position[3];
	int velocity[3];
	int rotation[3];
	int spin[3];
};

void spawnLevel1Explosion(RA2PSXLevel1Explosion *explosions, const int *position,
		Common::RandomSource &random) {
	int slot = -1;
	int oldest = -1;
	for (int i = 0; i < kLevel1ExplosionSlots; ++i) {
		if (explosions[i].frame < 0) {
			slot = i;
			break;
		}
		if (explosions[i].frame > oldest) {
			oldest = explosions[i].frame;
			slot = i;
		}
	}

	RA2PSXLevel1Explosion &explosion = explosions[slot];
	explosion.frame = 0;
	explosion.rotation = (int)random.getRandomNumber(15) * 0x100;
	for (int axis = 0; axis < 3; ++axis)
		explosion.position[axis] = position[axis];
}

void updateLevel1Explosions(RA2PSXLevel1Explosion *explosions, int frameCount) {
	for (int i = 0; i < kLevel1ExplosionSlots; ++i) {
		if (explosions[i].frame < 0)
			continue;
		if (++explosions[i].frame >= frameCount) {
			explosions[i].frame = -1;
			continue;
		}
		explosions[i].position[2] -= kLevel1ExplosionDrift;
	}
}

void renderLevel1Explosions(RA2PSXTinyGLRenderer &renderer,
		const Common::Array<RA2PSXTexture> &frames, const RA2PSXLevel1Explosion *explosions) {
	for (int i = 0; i < kLevel1ExplosionSlots; ++i) {
		const RA2PSXLevel1Explosion &explosion = explosions[i];
		if (explosion.frame < 0 || (uint)explosion.frame >= frames.size())
			continue;
		const RA2PSXTexture &frame = frames[explosion.frame];
		const float scale = kLevel1ExplosionScale / 4096.0f;
		renderer.renderSprite(frame, (float)explosion.position[0], (float)explosion.position[1],
				(float)explosion.position[2], frame.width * 0.5f * scale,
				frame.height * 0.5f * scale, explosion.rotation);
	}
}

// Each shard gets a tumble and a shove back toward the camera; one spin axis is dropped
// so the pieces do not all rotate the same way.
int debrisImpulse(Common::RandomSource &random) {
	const int magnitude = (int)random.getRandomNumber(31) + 16;
	return random.getRandomBit() ? magnitude : -magnitude;
}

void spawnLevel1Debris(RA2PSXLevel1Debris *debris, const int *position,
		const int *rotation, int count, int modelCount, Common::RandomSource &random) {
	for (int piece = 0; piece < count; ++piece) {
		int slot = -1;
		for (int i = 0; i < kLevel1DebrisCount; ++i) {
			if (!debris[i].life) {
				slot = i;
				break;
			}
		}
		if (slot < 0)
			return;

		RA2PSXLevel1Debris &shard = debris[slot];
		shard = RA2PSXLevel1Debris();
		shard.life = kLevel1DebrisLife;
		shard.model = modelCount ? (int)random.getRandomNumber(modelCount - 1) : 0;
		for (int axis = 0; axis < 3; ++axis) {
			shard.position[axis] = position[axis];
			shard.rotation[axis] = rotation[axis];
			shard.spin[axis] = debrisImpulse(random);
		}
		shard.velocity[0] = debrisImpulse(random);
		shard.velocity[1] = debrisImpulse(random);
		shard.velocity[2] = -((int)random.getRandomNumber(63) + 32);
		shard.spin[(int)random.getRandomNumber(2)] = 0;
	}
}

void rotateVector(const RA2PSXMatrix &transform, const int *source, int *result) {
	for (int row = 0; row < 3; ++row) {
		float value = 0.0f;
		for (int column = 0; column < 3; ++column)
			value += transform.rotation[row][column] * source[column];
		result[row] = (int)value;
	}
}

// Each shard drags a smoke trail: a SMALLEX cell that spins and dims three steps a frame
// from the grey the original picks on spawn. The emitter counts down 5 and is stepped
// twice a frame, so a puff leaves every two and a half frames, and each one is thrown
// along an upward base velocity turned through up to a quarter turn on all three axes.
enum {
	kLevel1PuffCount = 64,
	kLevel1PuffSpin = 0x10,
	kLevel1PuffFade = 3,
	kLevel1PuffReload = 5,
	kLevel1PuffStepsPerFrame = 2,
	kLevel1PuffHalfSize = 64,
	kLevel1PuffSpread = 0x400
};

struct RA2PSXLevel1Puff {
	RA2PSXLevel1Puff() : brightness(0), rotation(0), spin(0) {
		for (int axis = 0; axis < 3; ++axis)
			position[axis] = velocity[axis] = 0;
	}

	int brightness;
	int rotation;
	int spin;
	// Kept in 1/4096 like the original's particles, so the drift stays smooth.
	int position[3];
	int velocity[3];
};

void spawnLevel1Puff(RA2PSXLevel1Puff *puffs, const int *position,
		Common::RandomSource &random) {
	int slot = -1;
	int dimmest = 0x7fffffff;
	for (int i = 0; i < kLevel1PuffCount; ++i) {
		if (!puffs[i].brightness) {
			slot = i;
			break;
		}
		if (puffs[i].brightness < dimmest) {
			dimmest = puffs[i].brightness;
			slot = i;
		}
	}

	RA2PSXLevel1Puff &puff = puffs[slot];
	puff.brightness = (int)random.getRandomNumber(31) + 0x60;
	puff.rotation = (int)random.getRandomNumber(0xfff);
	puff.spin = 0;
	for (int axis = 0; axis < 3; ++axis)
		puff.position[axis] = position[axis] * 4096;

	RA2PSXMatrix spin;
	spin.preRotateX((int)random.getRandomNumber(kLevel1PuffSpread - 1) - kLevel1PuffSpread / 2);
	spin.preRotateY((int)random.getRandomNumber(kLevel1PuffSpread - 1) - kLevel1PuffSpread / 2);
	spin.preRotateZ((int)random.getRandomNumber(kLevel1PuffSpread - 1) - kLevel1PuffSpread / 2);
	const int base[3] = {
		(int)random.getRandomNumber(499) - 250,
		-((int)random.getRandomNumber(299) + 0xb6a),
		0
	};
	rotateVector(spin, base, puff.velocity);
	for (int axis = 0; axis < 3; ++axis)
		puff.velocity[axis] *= 16;
}

void updateLevel1Puffs(RA2PSXLevel1Puff *puffs) {
	for (int i = 0; i < kLevel1PuffCount; ++i) {
		if (!puffs[i].brightness)
			continue;
		for (int axis = 0; axis < 3; ++axis)
			puffs[i].position[axis] += puffs[i].velocity[axis];
		puffs[i].spin += kLevel1PuffSpin;
		puffs[i].brightness = MAX(0, puffs[i].brightness - kLevel1PuffFade);
	}
}

void renderLevel1Puffs(RA2PSXTinyGLRenderer &renderer, const RA2PSXTexture &texture,
		const RA2PSXLevel1Puff *puffs) {
	if (texture.pixels.empty())
		return;
	for (int i = 0; i < kLevel1PuffCount; ++i) {
		const RA2PSXLevel1Puff &puff = puffs[i];
		if (!puff.brightness)
			continue;
		renderer.renderSprite(texture, puff.position[0] / 4096.0f, puff.position[1] / 4096.0f,
				puff.position[2] / 4096.0f, kLevel1PuffHalfSize, kLevel1PuffHalfSize,
				(int16)(puff.rotation + puff.spin), puff.brightness);
	}
}

void updateLevel1Debris(RA2PSXLevel1Debris *debris, RA2PSXLevel1Puff *puffs,
		Common::RandomSource &random) {
	for (int i = 0; i < kLevel1DebrisCount; ++i) {
		if (!debris[i].life)
			continue;
		debris[i].puffCountdown -= kLevel1PuffStepsPerFrame;
		while (debris[i].puffCountdown <= 0) {
			debris[i].puffCountdown += kLevel1PuffReload;
			spawnLevel1Puff(puffs, debris[i].position, random);
		}
		for (int axis = 0; axis < 3; ++axis) {
			debris[i].position[axis] += debris[i].velocity[axis];
			debris[i].rotation[axis] = (int16)(debris[i].rotation[axis] + debris[i].spin[axis]);
		}
		if (--debris[i].life && debris[i].position[2] < 0)
			debris[i].life = 0;
	}
}

void renderLevel1Debris(RA2PSXTinyGLRenderer &renderer,
		const Common::Array<RA2PSXModel> &models, const RA2PSXLevel1Debris *debris) {
	if (models.empty())
		return;
	for (int i = 0; i < kLevel1DebrisCount; ++i) {
		if (!debris[i].life)
			continue;
		RA2PSXMatrix transform;
		transform.setRotationZ(debris[i].rotation[2]);
		transform.preRotateY(debris[i].rotation[1]);
		transform.preRotateX(debris[i].rotation[0]);
		transform.setTranslation(debris[i].position[0], debris[i].position[1],
				debris[i].position[2]);
		renderer.renderTransformedModel(models[debris[i].model % models.size()], transform, false);
	}
}

// The gameplay frames are wider and taller than the screen; the view slides around
// inside them and tilts, following a twenty frame average of where the player is aiming.
enum {
	kLevel1ViewHistory = 20,
	kLevel1ViewCenterX = 0x34,
	kLevel1ViewCenterY = 0x13,
	kLevel1ViewTiltBase = 0x10
};

struct RA2PSXLevel1ViewTracker {
	RA2PSXLevel1ViewTracker() : next(0), filled(0) {
		for (int i = 0; i < kLevel1ViewHistory; ++i)
			historyX[i] = historyY[i] = 0;
	}

	void push(int x, int y) {
		historyX[next] = x;
		historyY[next] = y;
		next = (next + 1) % kLevel1ViewHistory;
		filled = MIN(filled + 1, (int)kLevel1ViewHistory);
	}

	void average(int &x, int &y) const {
		int totalX = 0;
		int totalY = 0;
		for (int i = 0; i < kLevel1ViewHistory; ++i) {
			totalX += historyX[i];
			totalY += historyY[i];
		}
		x = totalX / kLevel1ViewHistory;
		y = totalY / kLevel1ViewHistory;
	}

	int historyX[kLevel1ViewHistory];
	int historyY[kLevel1ViewHistory];
	int next;
	int filled;
};

// The original scales by 0x400000 then shifts back down by 11, rounding away from zero.
int scaleViewOffset(int value, int divisor) {
	const int scaled = (int)((int64)value * 0x400000 / divisor);
	return (scaled + (scaled < 0 ? -0x400 : 0x400)) >> 11;
}

void getLevel1BackgroundView(const RA2PSXLevel1ViewTracker &tracker,
		const Graphics::Surface &background, int width, int height,
		RA2PSXBackgroundView &view) {
	int averageX = 0;
	int averageY = 0;
	tracker.average(averageX, averageY);

	const int tilt = scaleViewOffset(averageX, 0x4f00);
	view.tiltLeft = kLevel1ViewTiltBase - tilt;
	view.tiltRight = kLevel1ViewTiltBase + tilt;
	// ra2SetProjectionOffset gets the same figure negated, so the 3D scene rides the pan.
	view.sceneX = -scaleViewOffset(averageX, 5220);
	view.sceneY = -scaleViewOffset(averageY, 0x1ce3);
	view.panX = kLevel1ViewCenterX - view.sceneX;
	// The band sits eight rows below the top of the port's cropped frame.
	view.panY = kLevel1ViewCenterY - view.sceneY - 8;

	const int slack = MAX(0, (int)background.h - height) - MAX(view.tiltLeft, view.tiltRight);
	view.panX = CLIP(view.panX, 0, MAX(0, (int)background.w - width));
	view.panY = CLIP(view.panY, 0, MAX(0, slack));
}

struct RA2PSXLevel1TieShot {
	RA2PSXLevel1TieShot() : active(false), distance(0), length(0) {}

	bool active;
	int origin[2][3];
	int direction[2][3];
	int yaw[2];
	int pitch[2];
	int distance;
	int length;
};

struct RA2PSXLevel1Shot {
	RA2PSXLevel1Shot() : active(false), progress(0), targetX(0), targetY(0), targetZ(0) {}

	bool active;
	int progress;
	float start[kLevel1BoltCount][3];
	float roll[kLevel1BoltCount];
	float targetX;
	float targetY;
	float targetZ;
	// The line the player aims along, sampled once per logic frame for the hit test.
	float traceStart[3];
	float trace[3];
	float previousTrace[3];
};

struct RA2PSXLevel1Ship {
	RA2PSXLevel1Ship() : x(0), y(0), z(1000), velocityX(0), velocityY(0) {}

	int x;
	int y;
	int z;
	int velocityX;
	int velocityY;
};

const float kLevel1LaserStart[2][kLevel1BoltCount][3] = {
	{ { -600.0f, 200.0f, 400.0f }, { 600.0f, -100.0f, 400.0f } },
	{ { -600.0f, -100.0f, 400.0f }, { 600.0f, 200.0f, 400.0f } }
};

const float kLevel1LaserRoll[kLevel1BoltCount] = { -45.0f, 45.0f };

const float kLevel1ShipLaserStart[kLevel1BoltCount][3] = {
	{ -93.0f, 11.0f, -139.0f },
	{ 93.0f, 11.0f, -139.0f }
};

// The two cannon mounts, in the TIE's own space.
const int kLevel1TieMuzzle[2][3] = { { -60, 10, -50 }, { 60, 10, -50 } };

void spawnLevel1Enemy(RA2PSXLevel1Enemy &enemy, Common::RandomSource &random) {
	enemy = RA2PSXLevel1Enemy();
	enemy.active = true;
	if (random.getRandomNumber(999) < 500)
		initTieSplinePatternA(enemy.path, random);
	else
		initTieSplinePatternB(enemy.path, random);
	enemy.lead = enemy.path;
	enemy.lead.t += enemy.path.step * kLevel1LeadFrames;
	enemy.rotation[1] = 0x800;
	enemy.previousYaw = 0x800;
	enemy.fireCountdown = (int)random.getRandomNumber(14) + 8;
}

// The TIE points down its own path and banks into the turn; facing measures how squarely
// that path runs at the player, which is what lets it open fire.
void orientTieAlongSpline(RA2PSXLevel1Enemy &enemy, int playerX, int playerY) {
	int delta[3];
	for (int axis = 0; axis < 3; ++axis)
		delta[axis] = enemy.path.position[axis] - enemy.lead.position[axis];

	int vector[3] = { 0, delta[1], delta[2] };
	normalizeVector12(vector, vector);
	enemy.rotation[0] = signedAsinAngle(vector[1]);

	vector[0] = delta[0];
	vector[1] = 0;
	vector[2] = delta[2];
	normalizeVector12(vector, vector);
	const int yaw = (int16)(-signedAsinAngle(vector[0]) + 0x800);
	enemy.rotation[2] = (int16)fixedShift12(((yaw - enemy.previousYaw) * 2 +
			enemy.rotation[2]) * 0xf00);
	enemy.rotation[1] = yaw;
	enemy.previousYaw = yaw;

	for (int axis = 0; axis < 3; ++axis)
		delta[axis] = enemy.lead.position[axis] - enemy.path.position[axis];
	normalizeVector12(delta, delta);
	int toPlayer[3] = {
		playerX - enemy.path.position[0],
		playerY - enemy.path.position[1],
		-enemy.path.position[2]
	};
	normalizeVector12(toPlayer, toPlayer);
	enemy.facing = fixedShift12(delta[0] * toPlayer[0]) + fixedShift12(delta[1] * toPlayer[1]) +
			fixedShift12(delta[2] * toPlayer[2]);
}

// pan = (x * 640 / z) / 2 + 0x40, which saturates well before the screen edges.
int getLevel1WorldPan(const int *position) {
	if (!position[2])
		return 0x40;
	return CLIP<int>((position[0] * 640 / position[2]) / 2 + 0x40, 0, 0x7f);
}

bool spawnLevel1TieShot(RA2PSXLevel1TieShot *shots, const RA2PSXLevel1Enemy &enemy,
		bool aimed, bool outsideView, const int *shipPosition, Common::RandomSource &random) {
	int slot = -1;
	for (int i = 0; i < kLevel1ShotCount; ++i) {
		if (!shots[i].active) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return false;

	RA2PSXLevel1TieShot &shot = shots[slot];
	shot = RA2PSXLevel1TieShot();
	shot.active = true;
	shot.distance = 0x380;
	shot.length = 0x100;

	RA2PSXMatrix transform;
	transform.preRotateX(-enemy.rotation[0]);
	transform.preRotateY(enemy.rotation[1]);
	transform.preRotateZ(enemy.rotation[2]);

	// Aimed shots converge on the player; in the cockpit that is a point scattered
	// across the canopy rather than the camera itself.
	int target[3];
	if (outsideView) {
		for (int axis = 0; axis < 3; ++axis)
			target[axis] = shipPosition[axis];
	} else {
		target[0] = (int)random.getRandomNumber(999) - 500;
		target[1] = (int)random.getRandomNumber(799) - 400;
		target[2] = 0x280;
	}

	for (int bolt = 0; bolt < 2; ++bolt) {
		rotateVector(transform, kLevel1TieMuzzle[bolt], shot.origin[bolt]);
		for (int axis = 0; axis < 3; ++axis)
			shot.origin[bolt][axis] += enemy.path.position[axis];

		int *direction = shot.direction[bolt];
		if (aimed) {
			for (int axis = 0; axis < 3; ++axis)
				direction[axis] = enemy.path.position[axis] - target[axis];
			normalizeVector12(direction, direction);
		} else {
			const int forward[3] = { 0, 0, -0x1000 };
			rotateVector(transform, forward, direction);
		}

		int vector[3] = { direction[0], 0, direction[2] };
		normalizeVector12(vector, vector);
		shot.yaw[bolt] = -signedAsinAngle(vector[0]);
		vector[0] = 0;
		vector[1] = direction[1];
		vector[2] = direction[2];
		normalizeVector12(vector, vector);
		shot.pitch[bolt] = signedAsinAngle(vector[1]);
	}
	return true;
}

void getLevel1TieShotPosition(const RA2PSXLevel1TieShot &shot, int bolt, int *position) {
	for (int axis = 0; axis < 3; ++axis)
		position[axis] = shot.origin[bolt][axis] - (shot.direction[bolt][axis] * shot.distance >> 12);
}

// Returns false once the bolt is spent; hit is set only when it reaches the player.
bool updateLevel1TieShot(RA2PSXLevel1TieShot &shot, bool outsideView,
		const int *shipPosition, bool &hit) {
	int position[3];
	getLevel1TieShotPosition(shot, 1, position);
	shot.length = MIN(0x1000, shot.length + 0x400);
	shot.distance += 0x380;
	hit = false;

	if (!outsideView) {
		if (position[2] > 0x280)
			return true;
		hit = position[0] * position[0] + position[1] * position[1] < 450000;
		return false;
	}

	const int dx = position[0] - shipPosition[0];
	const int dy = position[1] - shipPosition[1];
	const int dz = position[2] - shipPosition[2];
	if ((int)sqrt((double)dx * dx + (double)dy * dy + (double)dz * dz) < 500) {
		hit = true;
		return false;
	}
	return position[2] > 200;
}

void renderLevel1TieShots(RA2PSXTinyGLRenderer &renderer, const RA2PSXModel &bolt,
		const RA2PSXLevel1TieShot *shots) {
	for (int i = 0; i < kLevel1ShotCount; ++i) {
		if (!shots[i].active)
			continue;
		for (int index = 0; index < 2; ++index) {
			int position[3];
			getLevel1TieShotPosition(shots[i], index, position);
			RA2PSXMatrix transform;
			transform.setScale(0x1000, 0x1000, shots[i].length);
			transform.preRotateY(shots[i].yaw[index]);
			transform.preRotateX(shots[i].pitch[index]);
			transform.setTranslation(position[0], position[1], position[2]);
			renderer.renderTransformedModel(bolt, transform, false);
		}
	}
}

void updateLevel1Aim(int &x, int &y, int &velocityX, int &velocityY,
		int &directionX, int &directionY, bool left, bool right, bool up, bool down,
		const Common::Rect &bounds) {
	if (left && right)
		left = right = false;
	if (up && down)
		up = down = false;

	if (!left && !right) {
		directionX = 0;
		velocityX /= 2;
	} else {
		if (left) {
			if (velocityX > 0)
				velocityX = -velocityX / 4;
			velocityX = MAX(-4096, velocityX - 448);
			directionX = -1;
		}
		if (right) {
			if (velocityX < 0)
				velocityX = -velocityX / 4;
			velocityX = MIN(4096, velocityX + 448);
			directionX = 1;
		}
		if ((up || down) && ABS(velocityX) < ABS(velocityY) / 2)
			velocityX = ABS(velocityY) * directionX / 2;
	}
	x = CLIP<int>(x + velocityX / 512, bounds.left, bounds.right);

	if (!up && !down) {
		directionY = 0;
		velocityY /= 2;
	} else {
		if (up) {
			if (velocityY > 0)
				velocityY = -velocityY / 4;
			velocityY = MAX(-4096, velocityY - 448);
			directionY = -1;
		}
		if (down) {
			if (velocityY < 0)
				velocityY = -velocityY / 4;
			velocityY = MIN(4096, velocityY + 448);
			directionY = 1;
		}
		if ((left || right) && ABS(velocityX) / 2 > ABS(velocityY))
			velocityY = ABS(velocityX) * directionY / 2;
	}
	y = CLIP<int>(y + velocityY / 512, bounds.top, bounds.bottom);
}

// The camera swap. ra2UpdateExternalViewInput steps this twice per frame while it runs,
// and keeps the pad dead throughout.
struct RA2PSXLevel1ViewSwap {
	RA2PSXLevel1ViewSwap() : direction(0), cockpitScale(kLevel1CockpitScaleFull),
			shipScale(kLevel1CockpitScaleFull), showCockpit(true) {}

	int direction;
	int cockpitScale;
	int shipScale;
	bool showCockpit;
};

// Returns true once the swap lands, with outside telling the caller which view it landed in.
bool stepLevel1ViewSwap(RA2PSXLevel1ViewSwap &swap, RA2PSXLevel1Ship &ship, bool &outside) {
	if (swap.direction > 0) {
		ship.z += kLevel1ViewStep;
		swap.showCockpit = ship.z <= kLevel1ViewZoomOutZ;
		if (swap.showCockpit)
			swap.cockpitScale = MAX<int>(kLevel1CockpitScaleMin,
					swap.cockpitScale - kLevel1CockpitScaleStep);
		if (ship.z >= kLevel1ViewOutsideZ) {
			ship.z = kLevel1ViewOutsideZ;
			swap.direction = 0;
			outside = true;
			return true;
		}
		return false;
	}

	swap.shipScale += kLevel1ShipSwellStep;
	ship.z -= kLevel1ViewStep;
	ship.x -= CLIP(ship.x, -kLevel1ViewRecentre, (int)kLevel1ViewRecentre);
	ship.y -= CLIP(ship.y, -kLevel1ViewRecentre, (int)kLevel1ViewRecentre);
	swap.showCockpit = ship.z <= kLevel1ViewZoomInZ;
	if (swap.showCockpit) {
		swap.cockpitScale = MIN<int>(kLevel1CockpitScaleFull,
				swap.cockpitScale + kLevel1CockpitScaleStep);
		if (swap.cockpitScale == kLevel1CockpitScaleFull)
			ship.z = kLevel1ViewCockpitZ;
	}
	if (ship.z <= kLevel1ViewCockpitZ) {
		ship.z = kLevel1ViewOutsideZ;
		swap.cockpitScale = kLevel1CockpitScaleFull;
		swap.shipScale = kLevel1CockpitScaleFull;
		swap.showCockpit = true;
		swap.direction = 0;
		outside = false;
		return true;
	}
	return false;
}

void updateLevel1Ship(RA2PSXLevel1Ship &ship,
		bool left, bool right, bool up, bool down) {
	if (left == right) {
		ship.velocityX = ship.velocityX * 3 / 4;
	} else if (left) {
		ship.velocityX = MAX(-4096, ship.velocityX - 400);
	} else {
		ship.velocityX = MIN(4096, ship.velocityX + 400);
	}

	if (up == down) {
		ship.velocityY = ship.velocityY * 3 / 4;
	} else if (up) {
		ship.velocityY = MAX(-4096, ship.velocityY - 400);
	} else {
		ship.velocityY = MIN(4096, ship.velocityY + 400);
	}

	ship.x = CLIP<int>(ship.x + ship.velocityX / 16 / 25, -282, 282);
	ship.y = CLIP<int>(ship.y + ship.velocityY * 10 / 4096, -142, 157);
}

// The nose follows both sticks, as the original's ship does: banking yaws it and
// climbing pitches it, and the guns fire along that nose.
void getLevel1ShipOrientation(const RA2PSXLevel1Ship &ship,
		float &forwardX, float &forwardY, float &forwardZ, float &roll) {
	const float bank = (ship.velocityX / 16) * 360.0f / 4096.0f;
	const float climb = (ship.velocityY / 16) * 360.0f / 4096.0f;
	const float yaw = -bank * 0.5f * 0.017453292519943295f;
	const float pitch = climb * 0.5f * 0.017453292519943295f;
	const float pitchCosine = cosf(pitch);
	forwardX = sinf(yaw) * pitchCosine;
	forwardY = -sinf(pitch);
	forwardZ = -cosf(yaw) * pitchCosine;
	roll = bank;
}

void transformLevel1ShipPoint(const RA2PSXLevel1Ship &ship,
		float localX, float localY, float localZ,
		float &worldX, float &worldY, float &worldZ) {
	float forwardX;
	float forwardY;
	float forwardZ;
	float roll;
	getLevel1ShipOrientation(ship, forwardX, forwardY, forwardZ, roll);

	const float angle = roll * 0.017453292519943295f;
	const float cosine = cosf(angle);
	const float sine = sinf(angle);
	worldX = ship.x + forwardZ * cosine * localX - forwardZ * sine * localY +
			forwardX * localZ;
	worldY = ship.y + sine * localX + cosine * localY + forwardY * localZ;
	worldZ = ship.z - forwardX * cosine * localX + forwardX * sine * localY +
			forwardZ * localZ;
}

void traceLevel1Shot(RA2PSXLevel1Shot &shot) {
	const float progress = shot.progress / 4096.0f;
	shot.trace[0] = shot.traceStart[0] + (shot.targetX - shot.traceStart[0]) * progress;
	shot.trace[1] = shot.traceStart[1] + (shot.targetY - shot.traceStart[1]) * progress;
	shot.trace[2] = shot.traceStart[2] + (shot.targetZ - shot.traceStart[2]) * progress;
}

bool spawnLevel1Shot(RA2PSXLevel1Shot *shots, int aimX, int aimY,
		int centerX, int centerY, int focal, const RA2PSXLevel1Ship *ship, int pair,
		int sceneX, int sceneY) {
	int slot = -1;
	for (int i = 0; i < kLevel1ShotCount; ++i) {
		if (!shots[i].active) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return false;

	RA2PSXLevel1Shot &shot = shots[slot];
	shot = RA2PSXLevel1Shot();
	shot.active = true;
	// The original launches from the muzzle in the cockpit and only skips ahead outside.
	shot.progress = ship ? 400 : 0;
	if (!ship) {
		for (int i = 0; i < kLevel1BoltCount; ++i) {
			for (int axis = 0; axis < 3; ++axis)
				shot.start[i][axis] = kLevel1LaserStart[pair][i][axis];
			shot.roll[i] = kLevel1LaserRoll[i];
		}
		shot.targetX = (float)(aimX - centerX - sceneX) * 18000.0f / focal;
		shot.targetY = (float)(aimY - centerY - sceneY) * 18000.0f / focal;
		shot.targetZ = 18000.0f;
		// Midway between the two cannon the original alternates between.
		shot.traceStart[0] = 0.0f;
		shot.traceStart[1] = 50.0f;
		shot.traceStart[2] = 400.0f;
	} else {
		float forwardX;
		float forwardY;
		float forwardZ;
		float shipRoll;
		getLevel1ShipOrientation(*ship, forwardX, forwardY, forwardZ, shipRoll);
		for (int i = 0; i < kLevel1BoltCount; ++i) {
			transformLevel1ShipPoint(*ship, kLevel1ShipLaserStart[i][0],
					kLevel1ShipLaserStart[i][1], kLevel1ShipLaserStart[i][2],
					shot.start[i][0], shot.start[i][1], shot.start[i][2]);
			shot.roll[i] = kLevel1LaserRoll[i] + shipRoll;
		}
		transformLevel1ShipPoint(*ship, 0.0f, -70.0f, -100.0f,
				shot.traceStart[0], shot.traceStart[1], shot.traceStart[2]);
		shot.targetX = shot.traceStart[0];
		shot.targetY = shot.traceStart[1];
		shot.targetZ = shot.traceStart[2];
		shot.targetX -= forwardX * 18000.0f;
		shot.targetY -= forwardY * 18000.0f;
		shot.targetZ -= forwardZ * 18000.0f;
	}

	traceLevel1Shot(shot);
	for (int axis = 0; axis < 3; ++axis)
		shot.previousTrace[axis] = shot.trace[axis];
	return true;
}

void updateLevel1Shots(RA2PSXLevel1Shot *shots) {
	for (int i = 0; i < kLevel1ShotCount; ++i) {
		if (!shots[i].active)
			continue;
		for (int axis = 0; axis < 3; ++axis)
			shots[i].previousTrace[axis] = shots[i].trace[axis];
		shots[i].progress += 200;
		if (shots[i].progress > 4399)
			shots[i].active = false;
		else
			traceLevel1Shot(shots[i]);
	}
}

// ra2FindPlayerShotHit: once the bolt's midpoint is past the TIE it keeps testing every
// frame, so aiming inside a TIE still scores as the bolt sweeps outwards.
bool level1ShotHitsEnemy(const RA2PSXLevel1Shot &shot, const int *position) {
	const float toPrevious = position[2] - shot.previousTrace[2];
	const float toCurrent = position[2] - shot.trace[2];
	if (toPrevious * toPrevious >= toCurrent * toCurrent)
		return false;
	const float dx = shot.trace[0] - position[0];
	const float dy = shot.trace[1] - position[1];
	return dx * dx + dy * dy <= (float)kLevel1HitRadius;
}

void renderLevel1Shots(RA2PSXTinyGLRenderer &renderer, const RA2PSXModel &laser,
		const RA2PSXLevel1Shot *shots) {
	for (int shotIndex = 0; shotIndex < kLevel1ShotCount; ++shotIndex) {
		const RA2PSXLevel1Shot &shot = shots[shotIndex];
		if (!shot.active || shot.progress >= 4000)
			continue;
		const float progress = shot.progress / 4096.0f;
		for (int laserIndex = 0; laserIndex < kLevel1BoltCount; ++laserIndex) {
			const float *start = shot.start[laserIndex];
			const float directionX = shot.targetX - start[0];
			const float directionY = shot.targetY - start[1];
			const float directionZ = shot.targetZ - start[2];
			renderer.renderPerspectiveModel(laser,
					start[0] + directionX * progress,
					start[1] + directionY * progress,
					start[2] + directionZ * progress,
					directionX, directionY, directionZ, shot.roll[laserIndex], false);
		}
	}
}

#endif

Rebel2PSX::Level1Result Rebel2PSX::playLevel1(const RA2PSXModel &enemyModel,
		const RA2PSXModel &shipModel, const RA2PSXModel &crosshair,
		const RA2PSXModel &laser, const RA2PSXModel &tieLaser,
		const Common::Array<RA2PSXModel> &debrisModels, const RA2PSXLevel1UI &ui,
		int &lives, int &score) {
#ifndef USE_TINYGL
	(void)enemyModel;
	(void)shipModel;
	(void)crosshair;
	(void)laser;
	(void)tieLaser;
	(void)debrisModels;
	(void)ui;
	(void)lives;
	(void)score;
	return kLevel1Error;
#else
	Common::SeekableReadStream *stream = openRawFile("S1/L01_PLAY.STR", 1);
	if (!stream)
		return kLevel1Error;

	RA2PSXStreamDecoder decoder(RA2PSXStreamDecoder::kVersion2);
	if (!decoder.loadStream(stream) || !decoder.setOutputPixelFormat(g_system->getScreenFormat())) {
		decoder.close();
		return kLevel1Error;
	}

	RA2PSXTinyGLRenderer renderer;
	if (!renderer.init(_vm->_screenWidth, _vm->_screenHeight)) {
		decoder.close();
		return kLevel1Error;
	}

	// The renderer's own projection, so aiming and the world agree.
	const int centerX = _vm->_screenWidth / 2;
	const int centerY = _vm->_screenHeight / 2;
	const int focal = _vm->_screenWidth * 2;

	RA2PSXLevel1Enemy enemies[kLevel1EnemyCount];
	RA2PSXLevel1Explosion explosions[kLevel1ExplosionSlots];
	RA2PSXLevel1Debris debris[kLevel1DebrisCount];
	RA2PSXLevel1Puff puffs[kLevel1PuffCount];
	RA2PSXLevel1ViewTracker viewTracker;
	RA2PSXLevel1Shot shots[kLevel1ShotCount];
	RA2PSXLevel1TieShot tieShots[kLevel1ShotCount];
	RA2PSXLevel1Ship ship;
	RA2PSXSoundPlayer soundPlayer(_vm, _soundBank);
	RA2PSXSoundPlayer::SoundId approachSounds[kLevel1EnemyCount] = {};
	// A hard left/right pair for the cockpit and a centred one for outside,
	// cross-faded as the view changes.
	const RA2PSXSoundPlayer::SoundId engineLeft =
			soundPlayer.play(kLevel1SfxEngineLeft, 0x28, 0x00);
	const RA2PSXSoundPlayer::SoundId engineRight =
			soundPlayer.play(kLevel1SfxEngineRight, 0x28, 0x7f);
	const RA2PSXSoundPlayer::SoundId engineOutside =
			soundPlayer.play(kLevel1SfxEngineOutside, kLevel1OutsideBase, 0x40);
	RA2PSXSoundPlayer::SoundId lowShieldAlarm = RA2PSXSoundPlayer::kInvalidSoundId;
	int cockpitMix = -1;
	int outsideMix = -1;
	const int difficulty = CLIP(_settings.difficulty, 0, 2);
	int extraLifeStage = 0;
	int nextExtraLife = kLevel1ExtraLife[difficulty][0];
	// The crosshair box lives in the original's 240 line frame; the port crops 20 rows.
	const int viewOffsetX = (_vm->_screenWidth - 320) / 2;
	const int viewOffsetY = (_vm->_screenHeight - 240) / 2;
	const Common::Rect aimBounds(kLevel1AimLeft + viewOffsetX, kLevel1AimTop + viewOffsetY,
			kLevel1AimLeft + kLevel1AimWidth + viewOffsetX,
			kLevel1AimTop + kLevel1AimHeight + viewOffsetY);
	int aimX = kLevel1AimStartX + viewOffsetX;
	int aimY = kLevel1AimStartY + viewOffsetY;
	int aimVelocityX = 0;
	int aimVelocityY = 0;
	int aimDirectionX = 0;
	int aimDirectionY = 0;
	int shield = kLevel1ShieldFull;
	int shieldDisplayed = kLevel1ShieldFull;
	int shakeFrames = 0;
	int flashFrame = -1;
	int shakeX = 0;
	int shakeY = 0;
	int spawnDelay = 0;
	int spawnRange = 80;
	int spawnBase = 60;
	int nextSpawnAdjustment = 0;
	int logicFrame = -1;
	int videoFrame = -1;
	int fireTick = 0;
	int lastFireTick = -kLevel1FireGap[1];
	int autoFireCounter = 0;
	int cannonPair = 0;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool actionLeft = false;
	bool actionRight = false;
	bool actionUp = false;
	bool actionDown = false;
	int joystickAxisX = 0;
	int joystickAxisY = 0;
	bool mouseFire = false;
	bool keyFire = false;
	bool actionFire = false;
	bool fireRequested = false;
	bool fireWasPressed = false;
	bool thirdPersonView = false;
	RA2PSXLevel1ViewSwap viewSwap;
	RA2PSXBackgroundView backgroundView;
	Level1Result result = kLevel1Complete;
	const int joystickDeadzone = MIN<int>(Common::JOYAXIS_MAX,
			MAX(0, ConfMan.getInt("joystick_deadzone")) * 1000);
	const bool rapidFire = ConfMan.hasKey("rebel2_rapid_fire") ?
			ConfMan.getBool("rebel2_rapid_fire") : true;

	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(false);
	g_system->warpMouse(160, 120);
	decoder.setVolume(_settings.videoVolume());
	decoder.start();
	const uint32 gameplayStartTime = g_system->getMillis();
	const Graphics::Surface *background = nullptr;

	while (!_vm->shouldQuit() && !decoder.endOfVideo()) {
		bool redraw = false;
		bool toggleViewRequested = false;
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				if (thirdPersonView) {
					ship.x = CLIP<int>((event.mouse.x - centerX) * ship.z / focal, -282, 282);
					ship.y = CLIP<int>((event.mouse.y - centerY) * ship.z / focal, -142, 157);
					ship.velocityX = ship.velocityY = 0;
				} else {
					aimX = CLIP<int>(event.mouse.x, aimBounds.left, aimBounds.right);
					aimY = CLIP<int>(event.mouse.y, aimBounds.top, aimBounds.bottom);
					aimVelocityX = aimVelocityY = 0;
					aimDirectionX = aimDirectionY = 0;
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (!thirdPersonView) {
					aimX = CLIP<int>(event.mouse.x, aimBounds.left, aimBounds.right);
					aimY = CLIP<int>(event.mouse.y, aimBounds.top, aimBounds.bottom);
				}
				mouseFire = true;
				fireRequested = true;
				break;
			case Common::EVENT_LBUTTONUP:
				mouseFire = false;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					result = kLevel1Quit;
				} else if (event.kbd.keycode == Common::KEYCODE_TAB && !event.kbdRepeat) {
					toggleViewRequested = true;
				} else if (event.kbd.keycode == Common::KEYCODE_LEFT ||
						event.kbd.keycode == Common::KEYCODE_a) {
					moveLeft = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RIGHT ||
						event.kbd.keycode == Common::KEYCODE_d) {
					moveRight = true;
				} else if (event.kbd.keycode == Common::KEYCODE_UP ||
						event.kbd.keycode == Common::KEYCODE_w) {
					moveUp = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN ||
						event.kbd.keycode == Common::KEYCODE_s) {
					moveDown = true;
				} else if (event.kbd.keycode == Common::KEYCODE_SPACE ||
						event.kbd.keycode == Common::KEYCODE_RETURN) {
					if (!keyFire && !event.kbdRepeat)
						fireRequested = true;
					keyFire = true;
				}
				break;
			case Common::EVENT_KEYUP:
				if (event.kbd.keycode == Common::KEYCODE_LEFT || event.kbd.keycode == Common::KEYCODE_a)
					moveLeft = false;
				else if (event.kbd.keycode == Common::KEYCODE_RIGHT || event.kbd.keycode == Common::KEYCODE_d)
					moveRight = false;
				else if (event.kbd.keycode == Common::KEYCODE_UP || event.kbd.keycode == Common::KEYCODE_w)
					moveUp = false;
				else if (event.kbd.keycode == Common::KEYCODE_DOWN || event.kbd.keycode == Common::KEYCODE_s)
					moveDown = false;
				else if (event.kbd.keycode == Common::KEYCODE_SPACE ||
						event.kbd.keycode == Common::KEYCODE_RETURN)
					keyFire = false;
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			case Common::EVENT_CUSTOM_ENGINE_ACTION_END: {
				const bool pressed = event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START;
				switch (event.customType) {
				case kScummActionInsaneLeft:
					actionLeft = pressed;
					break;
				case kScummActionInsaneRight:
					actionRight = pressed;
					break;
				case kScummActionInsaneUp:
					actionUp = pressed;
					break;
				case kScummActionInsaneDown:
					actionDown = pressed;
					break;
				case kScummActionInsaneAttack:
					actionFire = pressed;
					if (pressed)
						fireRequested = true;
					break;
				case kScummActionInsaneSwitch:
					if (pressed)
						toggleViewRequested = true;
					break;
				case kScummActionInsaneBack:
					if (pressed)
						result = kLevel1Quit;
					break;
				default:
					break;
				}
				break;
			}
			case Common::EVENT_CUSTOM_BACKEND_ACTION_AXIS: {
				const int axisPosition = event.joystick.position == Common::JOYAXIS_MIN ?
						Common::JOYAXIS_MAX : event.joystick.position;
				switch (event.customType) {
				case kScummBackendActionRebel2AxisUp:
					if (event.joystick.position != 0 || joystickAxisY <= 0)
						joystickAxisY = -axisPosition;
					break;
				case kScummBackendActionRebel2AxisDown:
					if (event.joystick.position != 0 || joystickAxisY >= 0)
						joystickAxisY = axisPosition;
					break;
				case kScummBackendActionRebel2AxisLeft:
					if (event.joystick.position != 0 || joystickAxisX <= 0)
						joystickAxisX = -axisPosition;
					break;
				case kScummBackendActionRebel2AxisRight:
					if (event.joystick.position != 0 || joystickAxisX >= 0)
						joystickAxisX = axisPosition;
					break;
				default:
					break;
				}
				break;
			}
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				_vm->quitGame();
				result = kLevel1Quit;
				break;
			default:
				break;
			}
		}
		if (toggleViewRequested && !viewSwap.direction) {
			viewSwap.direction = thirdPersonView ? -1 : 1;
			ship.velocityX = ship.velocityY = 0;
			aimVelocityX = aimVelocityY = 0;
			aimDirectionX = aimDirectionY = 0;
			if (viewSwap.direction > 0)
				ship.z = kLevel1ViewCockpitZ;
			redraw = true;
		}
		if (result == kLevel1Quit)
			break;

		while (decoder.needsUpdate()) {
			background = decoder.decodeNextFrame();
			if (!background) {
				result = kLevel1Error;
				break;
			}
			videoFrame = decoder.getCurFrame();
		}
		if (result == kLevel1Error)
			break;
		soundPlayer.update();

		const uint32 elapsed = g_system->getMillis() - gameplayStartTime;
		const int targetLogicFrame = (int)((uint64)elapsed * kLevel1FrameRate / 1000);
		while (logicFrame < targetLogicFrame && shield > 0) {
			++logicFrame;
			redraw = true;
			// Controls are dead while the camera moves, exactly as the original does.
			const bool steering = viewSwap.direction == 0;
			const bool left = steering && (moveLeft || actionLeft ||
					joystickAxisX < -joystickDeadzone);
			const bool right = steering && (moveRight || actionRight ||
					joystickAxisX > joystickDeadzone);
			const bool up = steering && (moveUp || actionUp || joystickAxisY < -joystickDeadzone);
			const bool down = steering && (moveDown || actionDown ||
					joystickAxisY > joystickDeadzone);
			if (viewSwap.direction) {
				for (int step = 0; step < 2 && viewSwap.direction; ++step) {
					if (stepLevel1ViewSwap(viewSwap, ship, thirdPersonView) &&
							!thirdPersonView) {
						aimX = kLevel1AimStartX + viewOffsetX;
						aimY = kLevel1AimStartY + viewOffsetY;
					}
				}
			} else if (thirdPersonView) {
				updateLevel1Ship(ship, left, right, up, down);
			} else {
				updateLevel1Aim(aimX, aimY, aimVelocityX, aimVelocityY,
						aimDirectionX, aimDirectionY,
						left, right, up, down, aimBounds);
			}

			const int cockpitTarget = thirdPersonView ? 0 : kLevel1MixMaximum;
			const int outsideTarget = thirdPersonView ? kLevel1MixMaximum : 0;
			if (cockpitMix != cockpitTarget || outsideMix != outsideTarget) {
				cockpitMix = cockpitMix < 0 ? cockpitTarget :
						approachMix(cockpitMix, cockpitTarget);
				outsideMix = outsideMix < 0 ? outsideTarget :
						approachMix(outsideMix, outsideTarget);
				soundPlayer.setVolume(engineLeft, cockpitMix);
				soundPlayer.setVolume(engineRight, cockpitMix);
				soundPlayer.setVolume(engineOutside, outsideMix);
			} else if (thirdPersonView) {
				// Outside, the engine tracks how hard the ship is turning.
				const int turn = MAX(ABS(ship.velocityX), ABS(ship.velocityY));
				soundPlayer.setPitch(engineOutside, 0x2000 + turn);
				soundPlayer.setVolume(engineOutside, kLevel1OutsideBase +
						turn / (0xa0 - kLevel1OutsideBase));
				soundPlayer.setPan(engineOutside,
						CLIP<int>(ship.x * 4096 / 0x479f + 64, 0, 127));
			}

			if (shieldDisplayed != shield) {
				shieldDisplayed = shieldDisplayed < shield ?
						MIN(shield, shieldDisplayed + kLevel1ShieldStep) :
						MAX(shield, shieldDisplayed - kLevel1ShieldStep);
				shieldDisplayed = MAX(1, shieldDisplayed);
			}
			if (flashFrame >= 0 && ++flashFrame >= kRA2PSXHitFlashFrames)
				flashFrame = -1;
			if (shakeFrames > 0) {
				--shakeFrames;
				shakeX = (int)_vm->_rnd.getRandomNumber(3);
				shakeY = (int)_vm->_rnd.getRandomNumber(3);
			} else {
				shakeX = shakeY = 0;
			}

			// Started once, then left running, as in the original.
			if (shieldDisplayed < kLevel1LowShield &&
					lowShieldAlarm == RA2PSXSoundPlayer::kInvalidSoundId)
				lowShieldAlarm = soundPlayer.play(kLevel1SfxLowShield, 0x50, 0x40);

			if (logicFrame >= nextSpawnAdjustment) {
				nextSpawnAdjustment = logicFrame + 20;
				spawnRange = MAX(40, spawnRange - 1);
				spawnBase = MAX(20, spawnBase - 1);
			}

			const int view = thirdPersonView ? 1 : 0;
			const int shipPosition[3] = { ship.x, ship.y, ship.z };
			// The cockpit aims from the camera, so the fire gate uses the origin there.
			const int playerX = thirdPersonView ? ship.x : 0;
			const int playerY = thirdPersonView ? ship.y : 0;

			int activeEnemies = 0;
			for (int i = 0; i < kLevel1EnemyCount; ++i)
				activeEnemies += enemies[i].active ? 1 : 0;
			--spawnDelay;
			if (videoFrame < 1599 && activeEnemies < kLevel1EnemyCount && spawnDelay <= 0) {
				for (int i = 0; i < kLevel1EnemyCount; ++i) {
					if (!enemies[i].active) {
						spawnLevel1Enemy(enemies[i], _vm->_rnd);
						break;
					}
				}
				spawnDelay = spawnBase + _vm->_rnd.getRandomNumber(spawnRange - 1);
			}

			updateLevel1Shots(shots);
			updateLevel1Explosions(explosions, _explosionFrames.size());
			updateLevel1Debris(debris, puffs, _vm->_rnd);
			updateLevel1Puffs(puffs);
			viewTracker.push(thirdPersonView ? ship.x * focal / MAX(1, ship.z) : aimX - centerX,
					thirdPersonView ? ship.y * focal / MAX(1, ship.z) : aimY - centerY);
			if (background)
				getLevel1BackgroundView(viewTracker, *background, _vm->_screenWidth,
						_vm->_screenHeight, backgroundView);
			for (int i = 0; i < kLevel1ShotCount; ++i) {
				if (!tieShots[i].active)
					continue;
				bool hit = false;
				if (!updateLevel1TieShot(tieShots[i], thirdPersonView, shipPosition, hit))
					tieShots[i].active = false;
				if (hit) {
					shield = MAX(0, shield - kLevel1BoltDamage[view][difficulty]);
					shakeFrames = kLevel1ShakeFrames;
					flashFrame = 0;
					soundPlayer.play(kLevel1SfxPlayerHit, 0x7f, 0x40);
				}
			}

			for (int i = 0; i < kLevel1EnemyCount; ++i) {
				if (!enemies[i].active)
					continue;

				RA2PSXLevel1Enemy &enemy = enemies[i];
				advanceSplineObject(enemy.lead);
				advanceSplineObject(enemy.path);
				orientTieAlongSpline(enemy, playerX, playerY);

				const int *position = enemy.path.position;
				const int soundPan = getLevel1WorldPan(position);
				bool destroyed = false;

				// Close in, a TIE that has not been shot down collides with the player.
				if (position[2] <= 1499) {
					bool rammed;
					if (thirdPersonView) {
						const int dx = position[0] - ship.x;
						const int dy = position[1] - ship.y;
						const int dz = position[2] - ship.z;
						rammed = (int)sqrt((double)dx * dx + (double)dy * dy +
								(double)dz * dz) < kLevel1RamRadius[1];
					} else {
						rammed = (int)sqrt((double)position[0] * position[0] +
								(double)position[1] * position[1]) < kLevel1RamRadius[0];
					}
					if (rammed) {
						shield = MAX(0, shield - kLevel1RamDamage[difficulty]);
						shakeFrames = kLevel1ShakeFrames;
						flashFrame = 0;
						destroyed = true;
					}
				}

				if (!destroyed && position[2] < 0) {
					soundPlayer.stop(approachSounds[i]);
					approachSounds[i] = RA2PSXSoundPlayer::kInvalidSoundId;
					enemy.active = false;
					continue;
				}

				if (!destroyed) {
					if (position[2] < 10000 &&
							approachSounds[i] == RA2PSXSoundPlayer::kInvalidSoundId) {
						const uint16 sfx = _vm->_rnd.getRandomNumber(999) < 800 ?
								kLevel1SfxTieApproachA : kLevel1SfxTieApproachB;
						const int pitch = 0x1c18 + _vm->_rnd.getRandomNumber(1999);
						approachSounds[i] = soundPlayer.play(sfx, 0x5e, 0x40, pitch);
					}
					soundPlayer.setPan(approachSounds[i], soundPan);

					for (int shotIndex = 0; shotIndex < kLevel1ShotCount && !destroyed; ++shotIndex) {
						if (shots[shotIndex].active &&
								level1ShotHitsEnemy(shots[shotIndex], position)) {
							shots[shotIndex].active = false;
							destroyed = true;
							score = MIN(9999999, score + kLevel1KillScore[difficulty]);
							if (score >= nextExtraLife) {
								++lives;
								soundPlayer.play(kLevel1SfxExtraLife, 0x7f, 0x40);
								extraLifeStage = MIN(extraLifeStage + 1, 2);
								nextExtraLife += kLevel1ExtraLife[difficulty][extraLifeStage];
							}
						}
					}
				}

				// A TIE only shoots while it is still out at range and running square at
				// the player; most shots are sprayed ahead rather than aimed.
				if (!destroyed && position[2] < 16000 && --enemy.fireCountdown < 0 &&
						enemy.facing >= kLevel1FireFacing && position[2] >= 2000) {
					enemy.fireCountdown = (int)_vm->_rnd.getRandomNumber(44) + 8;
					const bool aimed = (int)_vm->_rnd.getRandomNumber(99) <
							kLevel1AimChance[view][difficulty];
					if (spawnLevel1TieShot(tieShots, enemy, aimed, thirdPersonView,
							shipPosition, _vm->_rnd))
						soundPlayer.play(kLevel1SfxTieFire, 0x4e, soundPan);
				}

				if (destroyed) {
					const int pitch = _vm->_rnd.getRandomNumber(0x3fff);
					soundPlayer.play(kLevel1SfxTieExplode, 0x5a, soundPan, pitch);
					soundPlayer.stop(approachSounds[i]);
					approachSounds[i] = RA2PSXSoundPlayer::kInvalidSoundId;
					enemy.active = false;
					spawnLevel1Explosion(explosions, position, _vm->_rnd);
					spawnLevel1Debris(debris, position, enemy.rotation,
							(int)_vm->_rnd.getRandomNumber(4) + 1, debrisModels.size(), _vm->_rnd);
				}
			}

			// A press only counts once the minimum gap has passed; with auto fire the
			// press just re-arms the repeat, which then fires on the same tick.
			++fireTick;
			const bool heldFire = steering && (mouseFire || keyFire || actionFire);
			const bool fireEdge = heldFire && (fireRequested || !fireWasPressed);
			fireWasPressed = heldFire;
			fireRequested = false;
			bool shootRequested = false;
			if (fireEdge && fireTick - lastFireTick >= kLevel1FireGap[view]) {
				lastFireTick = fireTick;
				autoFireCounter = 0;
				shootRequested = !rapidFire;
			}
			if (rapidFire && heldFire && --autoFireCounter < 0) {
				autoFireCounter = kLevel1AutoFireRepeat[view];
				shootRequested = true;
			}
			if (shootRequested && spawnLevel1Shot(shots, aimX, aimY, centerX, centerY, focal,
					thirdPersonView ? &ship : nullptr, cannonPair,
					backgroundView.sceneX, backgroundView.sceneY)) {
				cannonPair ^= 1;
				soundPlayer.play(kLevel1SfxPlayerFire, 0x3f, 0x40);
			}
		}

		if (shield <= 0) {
			result = kLevel1Death;
			break;
		}

		if (background && redraw) {
			renderer.setViewOffset(backgroundView.sceneX, backgroundView.sceneY);
			renderer.beginFrame(*background, backgroundView);
			// Everything shares one painter's pass, farthest first, as the original's
			// ordering table does.
			int order[kLevel1EnemyCount] = { 0, 1, 2 };
			for (int i = 0; i < kLevel1EnemyCount; ++i) {
				for (int j = i + 1; j < kLevel1EnemyCount; ++j) {
					if (enemies[order[j]].path.position[2] > enemies[order[i]].path.position[2])
						SWAP(order[i], order[j]);
				}
			}
			for (int i = 0; i < kLevel1EnemyCount; ++i) {
				const RA2PSXLevel1Enemy &enemy = enemies[order[i]];
				if (!enemy.active)
					continue;
				RA2PSXMatrix transform;
				transform.setRotationZ(enemy.rotation[2]);
				transform.preRotateY(enemy.rotation[1]);
				transform.preRotateX(enemy.rotation[0]);
				transform.setTranslation(enemy.path.position[0], enemy.path.position[1],
						enemy.path.position[2]);
				renderer.renderTransformedModel(enemyModel, transform, false);
			}
			renderLevel1TieShots(renderer, tieLaser, tieShots);
			renderLevel1Shots(renderer, laser, shots);
			renderLevel1Debris(renderer, debrisModels, debris);
			renderLevel1Puffs(renderer, _smokeTexture, puffs);
			renderLevel1Explosions(renderer, _explosionFrames, explosions);
			if (thirdPersonView || viewSwap.direction) {
				float forwardX;
				float forwardY;
				float forwardZ;
				float shipRoll;
				getLevel1ShipOrientation(ship, forwardX, forwardY, forwardZ, shipRoll);
				renderer.renderPerspectiveModel(shipModel, ship.x, ship.y, ship.z,
						forwardX, forwardY, forwardZ, shipRoll, false, viewSwap.shipScale);
			} else {
				renderer.renderModel(crosshair, aimX, aimY, 31.0f,
						0.0f, 0.0f, 0.0f, false);
			}
			Graphics::Surface output;
			renderer.finishFrame(output);
			drawRA2PSXHitFlash(output, flashFrame);
			if (viewSwap.showCockpit && (!thirdPersonView || viewSwap.direction))
				ui.drawCockpit(output, viewSwap.cockpitScale,
						scaleViewOffset(aimX - (kLevel1AimStartX + viewOffsetX),
								kLevel1CockpitDriftX),
						scaleViewOffset(aimY - (kLevel1AimStartY + viewOffsetY),
								kLevel1CockpitDriftY));
			ui.drawHUD(output, score, lives, shieldDisplayed, logicFrame);
			g_system->copyRectToScreen(output.getPixels(), output.pitch, shakeX, shakeY,
					output.w - shakeX, output.h - shakeY);
			g_system->updateScreen();
		}
		g_system->delayMillis(5);
	}

	soundPlayer.stopAll();
	decoder.close();
	CursorMan.showMouse(cursorWasVisible);
	return _vm->shouldQuit() ? kLevel1Quit : result;
#endif
}

} // End of namespace Scumm
