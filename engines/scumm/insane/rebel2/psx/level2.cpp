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

#include "common/events.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/shared.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"

namespace Scumm {

// Level 2 is a cover shooter, and the original runs its three parts as separate levels:
// 2, 0xc9 and 0xca. The same engine drives chapters 11 and 12.
//
// Every trooper slot owns four animations - stepping out, an aimed burst, a wild burst
// and falling - at play script indices 8i to 8i+3, with its bolt at 4+8i.
const RA2PSXLevel2PartInfo kRA2PSXLevel2Parts[kRA2PSXLevel2PartCount] = {
	{ "l2P1bg", "r1L2P1",
		{ { 0x26, 0 }, { 0x68, 0 } }, { { 0, 0 }, { 0, 0 } }, 0,
		{ { "BACK_L",   0, 28, 256, 190, 0 },
		  { "BACK_R", 256, 28,  64, 190, 0 },
		  { "PARA_L",   0, 28,  54, 190, 1 },
		  { "PARA_M",  54, 28, 180,  25, 1 },
		  { "PARA_R", 234, 28, 190, 190, 1 } },
		0, 0x26,
		0x2a, 0x32, 0xcf, 0x8d,
		{ 63, 104, 145, 186, 320 }, { 73, 88, 98, 118, 240 }, 0,
		{
		{ 255, 65, 78, 116 }, { 250, 68, 76, 113 }, { 234, 68, 76, 113 },
		{ 219, 68, 70, 113 }, { 200, 72, 80, 109 }, { 169, 73, 88, 108 },
		{ 169, 73, 88, 108 }, { 167, 73, 90, 108 }, { 165, 73, 92, 108 },
		{ 165, 73, 92, 108 }, { 179, 73, 78, 108 }, { 179, 73, 78, 108 },
		{ 178, 73, 80, 108 }, { 174, 74, 84, 107 }, { 174, 74, 84, 107 },
		{ 181, 73, 78, 109 }, { 181, 73, 78, 109 }, { 181, 73, 78, 108 },
		{ 181, 73, 80, 108 }, { 181, 73, 80, 108 }, { 180, 73, 80, 108 },
		{ 180, 73, 80, 108 }, { 181, 73, 82, 108 }, { 182, 73, 80, 108 },
		{ 182, 73, 80, 108 }, { 181, 73, 78, 109 }, { 181, 73, 78, 109 },
		{ 182, 73, 78, 108 }, { 183, 73, 80, 108 }, { 183, 73, 80, 108 }
		},
		2, { "TRP_0", "TRP_1", nullptr }, { "LAS_0", "LAS_1", nullptr },
		{ { { 7, 0x1f }, { 7, 0x1a }, { 8, 0x0b } },
		  { { 7, 0x36 }, { 7, 0x28 }, { 8, 0x1a } },
		  { { 0, 0 }, { 0, 0 }, { 0, 0 } } } },
	{ "l2P2bg", "r1L2P2",
		{ { 0, 0x28 }, { 0, 0x28 } }, { { 0, 0x0a }, { 0, -20 } }, 1,
		{ { "BACK_L",   0,  28, 256, 200, 0 },
		  { "BACK_R", 256,  28,  64, 200, 0 },
		  { "PARA_L",   0,  28,  64, 230, 1 },
		  { "PARA_R",  64, 151, 256, 107, 1 },
		  { nullptr,    0,   0,   0,   0, 0 } },
		-10, 0x12,
		0x1c, 0x26, 0x11e, 0xa0,
		{ 59, 125, 190, 257, 320 }, { 60, 90, 120, 138, 240 }, 0,
		{
		{ 146, 150, 68, 50 }, { 140, 141, 74, 59 }, { 147, 112, 70, 88 },
		{ 156, 106, 70, 94 }, { 164, 106, 70, 94 }, { 139, 106, 90, 94 },
		{ 134, 113, 94, 87 }, { 132, 114, 96, 86 }, { 131, 115, 96, 85 },
		{ 124, 118, 102, 82 }, { 157, 103, 76, 97 }, { 155, 113, 76, 87 },
		{ 155, 114, 76, 86 }, { 154, 115, 76, 85 }, { 144, 117, 86, 83 },
		{ 160, 103, 72, 97 }, { 159, 109, 74, 91 }, { 161, 111, 72, 89 },
		{ 160, 112, 72, 88 }, { 161, 113, 72, 87 }, { 157, 106, 72, 94 },
		{ 157, 108, 74, 92 }, { 158, 110, 72, 90 }, { 157, 111, 72, 89 },
		{ 156, 112, 72, 88 }, { 159, 106, 72, 94 }, { 158, 112, 76, 88 },
		{ 160, 112, 78, 88 }, { 159, 113, 80, 87 }, { 160, 115, 80, 85 }
		},
		3, { "TRP_0", "TRP_1", "TRP_2" }, { "LAS_0", "LAS_1", "LAS_2" },
		{ { { 0x12, 0x2b }, { 0x0e, 0x24 }, { 0x0e, 0x19 } },
		  { { 0x17, 0x3a }, { 0x15, 0x28 }, { 0x15, 0x1e } },
		  { { 0x0d, 0x3b }, { 0x0d, 0x29 }, { 0x0f, 0x1c } } } },
	{ "l2P3bg", "r1L2P3",
		{ { 0x0f, 0 }, { -114, 0 } }, { { 0x1e, 0 }, { -14, 0 } }, 0,
		{ { "BACK_L",   0, 28, 256, 190, 0 },
		  { "BACK_R", 256, 28,  94, 190, 0 },
		  { "PARA_L",   0, 28,  46, 190, 1 },
		  { "PARA_R",  46, 28,  44, 190, 1 },
		  { nullptr,    0,  0,   0,   0, 0 } },
		0, 0x23,
		0x1a, 0x43, 0xc9, 0x92,
		{ 48, 91, 134, 177, 320 }, { 77, 87, 106, 126, 240 }, 3,
		{
		{ 147, 103, 90, 81 }, { 156, 100, 78, 84 }, { 150, 99, 64, 85 },
		{ 127, 98, 74, 86 }, { 121, 97, 74, 87 }, { 107, 101, 82, 83 },
		{ 74, 101, 96, 83 }, { 81, 92, 86, 92 }, { 91, 95, 74, 89 },
		{ 90, 99, 74, 85 }, { 92, 103, 74, 81 }, { 91, 103, 74, 81 },
		{ 89, 102, 74, 82 }, { 91, 94, 76, 90 }, { 90, 99, 74, 85 },
		{ 88, 103, 74, 81 }, { 89, 103, 74, 81 }, { 87, 102, 74, 82 },
		{ 90, 95, 72, 89 }, { 92, 99, 70, 85 }, { 91, 102, 72, 82 },
		{ 91, 102, 72, 82 }, { 90, 102, 72, 82 }, { 92, 95, 72, 89 },
		{ 92, 100, 76, 84 }, { 89, 103, 78, 81 }, { 91, 103, 78, 81 },
		{ 90, 102, 76, 82 }, { 93, 95, 90, 89 }, { 95, 101, 90, 83 },
		{ 95, 103, 90, 81 }, { 95, 103, 92, 81 }, { 93, 103, 90, 81 }
		},
		3, { "TRP_0", "TRP_1", "TRP_2" }, { "LAS_0", "LAS_1", "LAS_2" },
		{ { { 0, 0 }, { 0, 0 }, { 0, 0 } },
		  { { 0, 0 }, { 0, 0 }, { 0, 0 } },
		  { { 0, 0 }, { 0, 0 }, { 0, 0 } } } }
};

// How many troopers a part sends, as base plus a roll of range.
const int16 kRA2PSXLevel2WaveTable[kRA2PSXLevel2PartCount][3][2] = {
	{ { 4, 0 }, { 6, 2 }, { 6, 4 } },
	{ { 5, 0 }, { 6, 3 }, { 7, 4 } },
	{ { 6, 0 }, { 8, 4 }, { 9, 5 } }
};

// { reappear base, reappear range, aim chance percent, respawn base, respawn range }
const int16 kRA2PSXLevel2SlotTable[kRA2PSXLevel2PartCount][3][5] = {
	{ { 140, 20, 66, 120, 60 }, { 100, 20, 66, 120, 60 }, { 80, 20, 50, 90, 40 } },
	{ { 140, 60, 66, 120, 60 }, { 100, 60, 58, 120, 60 }, { 80, 20, 50, 90, 40 } },
	{ { 140, 80, 66, 120, 60 }, { 100, 80, 50, 120, 60 }, { 80, 20, 50, 90, 40 } }
};

const int16 kRA2PSXLevel2FireTable[2][3][2] = {
	{ { 9, 6 }, { 8, 6 }, { 6, 4 } },
	{ { 9, 6 }, { 10, 6 }, { 6, 4 } }
};

const int16 kRA2PSXLevel2BoltTable[kRA2PSXLevel2PartCount][3][2] = {
	{ { 30, 180 }, { 40, 256 }, { 50, 256 } },
	{ { 30, 180 }, { 40, 256 }, { 50, 256 } },
	{ { 30, 180 }, { 40, 256 }, { 50, 256 } }
};

const int16 kRA2PSXLevel2KillScore[3] = { 80, 100, 150 };

RA2PSXLevel2Scene::RA2PSXLevel2Scene() : _part(0), _difficulty(0), _frame(0), _delay(0),
		_out(false), _moving(false), _tick(0), _remaining(0), _active(0), _clearTicks(0),
		_kills(0), _misses(0) {
	for (int layer = 0; layer < 2; ++layer) {
		for (int axis = 0; axis < 2; ++axis) {
			_scroll[layer][axis] = _scrollTarget[layer][axis] = 0;
			_scrollStep[layer][axis] = _scrollHold[layer][axis] = 0;
		}
	}
}

const RA2PSXLevel2PartInfo &RA2PSXLevel2Scene::info() const {
	return kRA2PSXLevel2Parts[_part];
}

bool RA2PSXLevel2Scene::load(const RA2PSXArchive &archive, int part, int difficulty,
		Common::RandomSource &random) {
	if (part < 0 || part >= kRA2PSXLevel2PartCount)
		return false;

	Common::Array<byte> data;
	_textures.clear();
	if (!archive.getMember(kRA2PSXLevel2Parts[part].sheet, data) || !_textures.append(data))
		return false;

	static const char *const required[] = { "BACK_L", "BACK_R", "PARA_L", "PARA_R" };
	for (uint i = 0; i < ARRAYSIZE(required); ++i) {
		if (!_textures.has(required[i]))
			return false;
	}

	// Every rookie pose is its own member: a format word, a palette and one image. The
	// poses stream into ROOKIE_A's VRAM slot, so they take that texture's CLUT - their
	// own often leaves the backdrop index opaque.
	const Common::Array<uint32> *rookieClut = _textures.palette("ROOKIE_A");
	_rookie.clear();
	const int poseCount = kRA2PSXLevel2Parts[part].aimBase + 30;
	for (int frame = 0; frame < poseCount; ++frame) {
		const Common::String path = Common::String::format("%s/anim%02d",
				kRA2PSXLevel2Parts[part].anims, frame);
		Common::Array<byte> anim;
		Common::Array<RA2PSXTexture> decoded;
		if (!archive.getMember(path, anim) || anim.size() < 8)
			return false;
		const uint16 height = READ_LE_UINT16(anim.data() + 2) & 0xff;
		if (!loadRA2PSXSpriteAnimation(anim, height ? height : 256, decoded, rookieClut) ||
				decoded.empty())
			return false;
		_rookie.push_back(decoded[0]);
	}

	Common::Array<byte> hud;
	_hud.clear();
	if (archive.getMember("trpTex", hud))
		_hud.append(hud);

	Common::Array<byte> script;
	_play.clear();
	if (!archive.getMember(Common::String::format("play%d", part + 1), script) ||
			!loadRA2PSXPlayScript(script, _play))
		return false;

	_part = part;
	_difficulty = CLIP(difficulty, 0, 2);
	_frame = 0;
	_delay = 0;
	_out = false;
	_moving = false;
	const RA2PSXLevel2PartInfo &part_ = kRA2PSXLevel2Parts[part];
	for (int plane = 0; plane < 2; ++plane) {
		for (int axis = 0; axis < 2; ++axis) {
			// Multiply rather than shift: parts two and three scroll to negatives.
			_scroll[plane][axis] = _scrollTarget[plane][axis] =
					part_.coverScroll[plane][axis] * 0x10000;
			_scrollStep[plane][axis] = _scrollHold[plane][axis] = 0;
		}
	}

	const int16 *wave = kRA2PSXLevel2WaveTable[part][_difficulty];
	_tick = 0;
	_remaining = wave[0] + (wave[1] ? (int)random.getRandomNumber(wave[1]) : 0);
	_active = 0;
	_clearTicks = 0;
	_kills = 0;
	_misses = 0;
	for (int i = 0; i < kRA2PSXLevel2TrooperCount; ++i) {
		_troopers[i] = RA2PSXLevel2Actor();
		_bolts[i] = RA2PSXLevel2Actor();
		// Nothing steps out before the part has settled.
		_troopers[i].slotTick = slotDelay(kRA2PSXLevel2SlotTable[part][_difficulty][0],
				kRA2PSXLevel2SlotTable[part][_difficulty][1], random);
	}
	return true;
}

int RA2PSXLevel2Scene::slotDelay(int base, int range, Common::RandomSource &random) const {
	return _tick + base + (range ? (int)random.getRandomNumber(range) : 0);
}

void RA2PSXLevel2Scene::startActor(RA2PSXLevel2Actor &actor, int state, int animation) {
	actor.state = state;
	actor.animation = animation;
	actor.frame = 0;
	actor.hold = kRA2PSXLevel2FrameTicks;
	actor.hit = false;
	actor.fireNext = 0;
	actor.fireEnd = 0;
}

const RA2PSXPlayFrame *RA2PSXLevel2Scene::actorFrame(const RA2PSXLevel2Actor &actor) const {
	if (actor.animation < 0 || (uint)actor.animation >= _play.size())
		return nullptr;
	const RA2PSXPlayAnimation &animation = _play[actor.animation];
	if ((uint)actor.frame >= animation.size())
		return nullptr;
	return &animation[actor.frame];
}

void RA2PSXLevel2Scene::advanceActor(RA2PSXLevel2Actor &actor) {
	const RA2PSXPlayFrame *frame = actorFrame(actor);
	if (!frame || (frame->flags & kRA2PSXPlayLastFrame))
		return;
	++actor.frame;
}

int RA2PSXLevel2Scene::updateEnemies(Common::RandomSource &random) {
	++_tick;
	const RA2PSXLevel2PartInfo &part = info();
	const int16 *slot = kRA2PSXLevel2SlotTable[_part][_difficulty];
	const int16 *bolt = kRA2PSXLevel2BoltTable[_part][_difficulty];
	int damage = 0;

	for (int i = 0; i < part.trooperCount; ++i) {
		RA2PSXLevel2Actor &trooper = _troopers[i];
		RA2PSXLevel2Actor &shot = _bolts[i];

		// The bolt goes first: its flagged frame is the moment the shot lands.
		if (shot.state != kRA2PSXLevel2StateIdle && --shot.hold <= 0) {
			shot.hold = kRA2PSXLevel2FrameTicks;
			const RA2PSXPlayFrame *frame = actorFrame(shot);
			if (frame && (frame->flags & kRA2PSXPlayHitsPlayer) && exposed() &&
					(int)random.getRandomNumber(99) < bolt[0])
				damage += bolt[1];
			if (!frame || (frame->flags & kRA2PSXPlayLastFrame))
				shot.state = kRA2PSXLevel2StateIdle;
			else
				++shot.frame;
		}

		switch (trooper.state) {
		case kRA2PSXLevel2StateIdle:
			// A fresh trooper only steps out while the wave still owes bodies.
			if (_tick >= trooper.slotTick && _remaining - _active > 0) {
				startActor(trooper, kRA2PSXLevel2StateAppear, i * 8);
				++_active;
				trooper.fireNext = part.fireWindows[i][0].start;
				trooper.fireEnd = part.fireWindows[i][0].end;
				if (!trooper.fireEnd)
					startActor(shot, kRA2PSXLevel2StateShot, 4 + i * 8);
			}
			continue;
		case kRA2PSXLevel2StateCover:
			if (_tick >= trooper.slotTick) {
				const bool aimed = (int)random.getRandomNumber(99) < slot[2];
				const int state = aimed ? kRA2PSXLevel2StateAimed : kRA2PSXLevel2StateWild;
				startActor(trooper, state, i * 8 + (aimed ? 1 : 2));
				trooper.fireNext = part.fireWindows[i][aimed ? 1 : 2].start;
				trooper.fireEnd = part.fireWindows[i][aimed ? 1 : 2].end;
				if (!trooper.fireEnd)
					startActor(shot, kRA2PSXLevel2StateShot, i * 8 + (aimed ? 5 : 6));
			}
			continue;
		default:
			break;
		}

		// Parts one and two shoot on a frame window inside the animation instead.
		if (trooper.fireEnd && trooper.frame >= trooper.fireNext &&
				trooper.frame <= trooper.fireEnd) {
			const int16 *cadence = kRA2PSXLevel2FireTable[MIN(_part, 1)][_difficulty];
			startActor(shot, kRA2PSXLevel2StateShot, 4 + i * 8);
			trooper.fireNext = trooper.frame + cadence[0] +
					(int)random.getRandomNumber(cadence[1]);
		}

		if (--trooper.hold > 0)
			continue;
		trooper.hold = kRA2PSXLevel2FrameTicks;
		const RA2PSXPlayFrame *frame = actorFrame(trooper);
		if (frame && !(frame->flags & kRA2PSXPlayLastFrame)) {
			++trooper.frame;
			continue;
		}

		if (trooper.state == kRA2PSXLevel2StateDie) {
			trooper.state = kRA2PSXLevel2StateIdle;
			--_active;
			--_remaining;
			trooper.slotTick = slotDelay(slot[3], slot[4], random);
			continue;
		}
		// It ducked back untouched, which the original scores as a miss.
		++_misses;
		trooper.state = kRA2PSXLevel2StateCover;
		trooper.slotTick = slotDelay(slot[0], slot[1], random);
	}

	for (int i = 0; i < kRA2PSXLevel2ShotCount; ++i) {
		if (!_shots[i].step)
			continue;
		_shots[i].step += kRA2PSXLevel2ShotStep;
		if (_shots[i].step > kRA2PSXLevel2ShotEnd)
			_shots[i].step = 0;
	}

	if (_remaining <= 0)
		++_clearTicks;
	return damage;
}

void RA2PSXLevel2Scene::projectShot(const RA2PSXLevel2Shot &shot, int step,
		int &x, int &y) const {
	step = CLIP<int>(step, 0, 4096);
	// The bolt is a straight 3D line from the gun at z 1000 to the crosshair at z 18000.
	// Projecting that line back through its own endpoints cancels the focal length, so
	// the screen position is a perspective weighted blend of the two screen points.
	const int nearZ = kRA2PSXLevel2ShotNearZ;
	const int farZ = kRA2PSXLevel2ShotFarZ;
	const int z = nearZ + (farZ - nearZ) * step / 4096;
	const int deltaX = shot.targetX * farZ - shot.muzzleX * nearZ;
	const int deltaY = shot.targetY * farZ - shot.muzzleY * nearZ;
	x = (int)((shot.muzzleX * nearZ + (int)(((int64)deltaX * step) >> 12)) / z);
	y = (int)((shot.muzzleY * nearZ + (int)(((int64)deltaY * step) >> 12)) / z);
}

int RA2PSXLevel2Scene::shoot(int aimX, int aimY) {
	if (!outOfCover())
		return 0;

	const RA2PSXLevel2PartInfo &part = info();
	// The bolt leaves the pistol the rookie is holding, which the pose box locates: he
	// aims across the frame, so the gun sits at the leading edge of his sprite.
	const RA2PSXLevel2Pose &pose = part.poses[CLIP<int>(_frame, 0, kRA2PSXLevel2FrameCount - 1)];
	for (int i = 0; i < kRA2PSXLevel2ShotCount; ++i) {
		if (_shots[i].step)
			continue;
		_shots[i].step = kRA2PSXLevel2ShotStep;
		_shots[i].muzzleX = pose.x + part.rookieOffsetX + pose.width / 8;
		_shots[i].muzzleY = pose.y + part.rookieOffsetY + pose.height / 6;
		_shots[i].targetX = aimX;
		_shots[i].targetY = aimY;
		break;
	}

	// The crosshair is in screen space; a frame's box is where the backdrop puts it.
	const int shotX = aimX + (_scroll[0][0] >> 16);
	const int shotY = aimY - kRA2PSXLevel2SceneTop + (_scroll[0][1] >> 16);
	for (int i = 0; i < part.trooperCount; ++i) {
		RA2PSXLevel2Actor &trooper = _troopers[i];
		if (trooper.state == kRA2PSXLevel2StateIdle ||
				trooper.state == kRA2PSXLevel2StateCover ||
				trooper.state == kRA2PSXLevel2StateDie)
			continue;
		const RA2PSXPlayFrame *frame = actorFrame(trooper);
		if (!frame || !(frame->flags & kRA2PSXPlayTargetable))
			continue;
		if (shotX < frame->boxLeft || shotX > frame->boxRight ||
				shotY < frame->boxTop || shotY > frame->boxBottom)
			continue;

		startActor(trooper, kRA2PSXLevel2StateDie, i * 8 + 3);
		_bolts[i].state = kRA2PSXLevel2StateIdle;
		++_kills;
		return kRA2PSXLevel2KillScore[_difficulty];
	}
	return 0;
}

void RA2PSXLevel2Scene::setScrollTarget(const int16 target[2][2]) {
	const RA2PSXLevel2PartInfo &part = info();
	for (int plane = 0; plane < 2; ++plane) {
		for (int axis = 0; axis < 2; ++axis) {
			_scrollTarget[plane][axis] = target[plane][axis] * 0x10000;
			_scrollStep[plane][axis] =
					(_scrollTarget[plane][axis] - _scroll[plane][axis]) / kRA2PSXLevel2ScrollSteps;
			// Only the axis the part actually slides along eases out.
			_scrollHold[plane][axis] =
					axis == part.scrollAxis ? kRA2PSXLevel2ScrollHold : 1000;
		}
	}
}

void RA2PSXLevel2Scene::toggleCover() {
	if (_moving)
		return;
	_moving = true;
	setScrollTarget(_out ? info().coverScroll : info().openScroll);
}

int RA2PSXLevel2Scene::aimFrame(int aimX, int aimY) const {
	const RA2PSXLevel2PartInfo &part = info();
	int column = 1;
	while (column <= kRA2PSXLevel2AimColumns && part.aimColumns[column - 1] < aimX)
		++column;
	int row = 0;
	while (row < kRA2PSXLevel2AimRows && part.aimRows[row] < aimY)
		++row;
	return CLIP<int>(column * kRA2PSXLevel2AimColumns + row + part.aimBase,
			coverFrames(), part.aimBase + 29);
}

int RA2PSXLevel2Scene::coverFrames() const {
	return info().aimBase + kRA2PSXLevel2CoverFrames;
}

void RA2PSXLevel2Scene::update(int aimX, int aimY) {
	for (int layer = 0; layer < 2; ++layer) {
		for (int axis = 0; axis < 2; ++axis) {
			int &value = _scroll[layer][axis];
			int &step = _scrollStep[layer][axis];
			const int target = _scrollTarget[layer][axis];
			if (value == target || !step)
				continue;
			if (_scrollHold[layer][axis] > 0)
				--_scrollHold[layer][axis];
			else
				step = (step >> 8) * (kRA2PSXLevel2ScrollDamping >> 8);
			value += step;
			if ((step < 0 && value <= target) || (step > 0 && value >= target))
				value = target;
		}
	}

	if (_moving) {
		if (--_delay >= 0)
			return;
		_delay = kRA2PSXLevel2RookieDelay;
		// Leaning out walks 0 to 4, ducking back walks it down again.
		if (!_out) {
			if (_frame < coverFrames() - 1) {
				++_frame;
			} else {
				_moving = false;
				_out = true;
				_frame = aimFrame(aimX, aimY);
			}
		} else {
			if (_frame >= coverFrames())
				_frame = coverFrames() - 1;
			if (_frame > 0) {
				--_frame;
			} else {
				_moving = false;
				_out = false;
			}
		}
		return;
	}

	if (_out)
		_frame = aimFrame(aimX, aimY);
}

void RA2PSXLevel2Scene::drawLayer(Graphics::Surface &surface,
		const RA2PSXLevel2Layer &layer, int left, int top) const {
	if (!layer.name || !_textures.has(layer.name))
		return;
	_textures.draw(surface, layer.name,
			left + layer.x - (_scroll[layer.plane][0] >> 16),
			top + layer.y - (_scroll[layer.plane][1] >> 16),
			Common::Rect(0, 0, layer.width, layer.height));
}

void RA2PSXLevel2Scene::drawFrame(Graphics::Surface &surface, const RA2PSXTexture &frame,
		int x, int y) const {
	for (int row = 0; row < frame.height; ++row) {
		const int destY = y + row;
		if (destY < 0 || destY >= surface.h)
			continue;
		for (int column = 0; column < frame.width; ++column) {
			const int destX = x + column;
			if (destX < 0 || destX >= surface.w)
				continue;
			const uint32 pixel = frame.pixels[row * frame.width + column];
			if (!(pixel & 0x01000000))
				continue;
			surface.setPixel(destX, destY, surface.format.RGBToColor(
					(pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff));
		}
	}
}

void RA2PSXLevel2Scene::drawPlayFrame(Graphics::Surface &surface,
		const RA2PSXPlayFrame &frame, const Common::Array<uint32> &palette,
		int left, int top) const {
	for (int row = 0; row < frame.height; ++row) {
		const int destY = top + frame.y + row;
		if (destY < 0 || destY >= surface.h)
			continue;
		for (int column = 0; column < frame.width; ++column) {
			const int destX = left + frame.x + column;
			if (destX < 0 || destX >= surface.w)
				continue;
			const byte index = frame.pixels[row * frame.width + column];
			const uint32 pixel = index < palette.size() ? palette[index] : 0;
			if (!(pixel & 0x01000000))
				continue;
			surface.setPixel(destX, destY, surface.format.RGBToColor(
					(pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff));
		}
	}
}

void RA2PSXLevel2Scene::draw(Graphics::Surface &surface, int aimX, int aimY) const {
	const RA2PSXLevel2PartInfo &part = info();
	const int left = (surface.w - 320) / 2;
	const int top = (surface.h - 240) / 2;
	surface.fillRect(Common::Rect(surface.w, surface.h), 0);

	for (int i = 0; i < kRA2PSXLevel2LayerCount; ++i) {
		if (!part.layers[i].plane)
			drawLayer(surface, part.layers[i], left, top);
	}

	// Troopers ride the backdrop, so ducking both slides them along and puts the near
	// wall in front of them.
	const int actorX = left - (_scroll[0][0] >> 16);
	const int actorY = top + kRA2PSXLevel2SceneTop - (_scroll[0][1] >> 16);
	for (int i = 0; i < part.trooperCount; ++i) {
		const Common::Array<uint32> *palette = _textures.palette(part.trooperPalettes[i]);
		if (!palette)
			continue;
		if (_troopers[i].state != kRA2PSXLevel2StateIdle &&
				_troopers[i].state != kRA2PSXLevel2StateCover) {
			const RA2PSXPlayFrame *frame = actorFrame(_troopers[i]);
			if (frame)
				drawPlayFrame(surface, *frame, *palette, actorX, actorY);
		}
	}

	for (int i = 0; i < kRA2PSXLevel2LayerCount; ++i) {
		if (part.layers[i].plane)
			drawLayer(surface, part.layers[i], left, top);
	}

	// A bolt flies past the near wall on its way to the camera, so it sits in front of
	// it - but only while the rookie is out; cover stops the shot, as the DOS handler
	// does by refusing to draw a beam at all while he is ducked.
	if (exposed()) {
		for (int i = 0; i < part.trooperCount; ++i) {
			const Common::Array<uint32> *bolt = _textures.palette(part.boltPalettes[i]);
			if (!bolt || _bolts[i].state == kRA2PSXLevel2StateIdle)
				continue;
			const RA2PSXPlayFrame *frame = actorFrame(_bolts[i]);
			if (frame)
				drawPlayFrame(surface, *frame, *bolt, actorX, actorY);
		}
	}

	// The rookie stands nearest of all, in front of his own cover.
	if ((uint)_frame < _rookie.size()) {
		const RA2PSXLevel2Pose &pose = part.poses[_frame];
		drawFrame(surface, _rookie[_frame], left + pose.x + part.rookieOffsetX,
				top + pose.y + part.rookieOffsetY);
	}

	// The player's own bolts are nearest of all, and stop with him behind cover.
	static const byte kShotHead[3] = { 0xff, 0xf0, 0xc0 };
	static const byte kShotTail[3] = { 0xc0, 0x30, 0x00 };
	for (int i = 0; exposed() && i < kRA2PSXLevel2ShotCount; ++i) {
		if (!_shots[i].step || _shots[i].step >= kRA2PSXLevel2ShotDraw)
			continue;
		int headX, headY, tailX, tailY;
		projectShot(_shots[i], _shots[i].step, headX, headY);
		projectShot(_shots[i], _shots[i].step - kRA2PSXLevel2ShotStep, tailX, tailY);
		for (int thickness = 0; thickness < 2; ++thickness) {
			drawRA2PSXGouraudLine(surface, left + tailX, top + tailY + thickness,
					left + headX, top + headY + thickness, kShotTail, kShotHead);
		}
	}

	if (outOfCover() && _hud.has("CROSS"))
		_hud.draw(surface, "CROSS", left + aimX - 4, top + aimY - 3,
				Common::Rect(0, 0, 8, 7));
}

// The PSX release composites its waves from sprite sheets instead of playing a SMUSH
// segment, but the level's shape is the same, so it rides runRebel2Level2 as well. Each
// wave is one pass at the part's troopers; the part is over once its budget is spent.
class Rebel2PSX::Level2Handler : public Rebel2Level2Handler {
public:
	Level2Handler(Rebel2PSX &psx, const RA2PSXArchive &archive) :
		_psx(psx), _archive(archive), _phaseState(0), _shield(kRA2PSXShieldFull),
		_score(0), _kills(0), _misses(0), _lives(3), _skipped(false) {
	}

	bool shouldQuit() const override { return _psx._vm->shouldQuit(); }

	// The briefing, then the long cinematic that drops the rookie into the corridor.
	bool playOpening() override {
		return _psx.playVideo("S1/L02_INTR.STR", 1, false) &&
				_psx.playVideo("S1/L02_CUT1.STR", 1, false);
	}

	void beginAttempt() override {
		_shield = kRA2PSXShieldFull;
		_kills = 0;
		_misses = 0;
	}

	void beginPhase(int phase, bool) override {
		_phaseState = 0;
		_skipped = false;
		if (!_scene.load(_archive, phase - 1, _psx._settings.difficulty, _psx._vm->_rnd))
			warning("Rebel Assault II: could not load level 2 part %d", phase);
	}

	// One wave per trooper the part scripts, so the shared loop runs the whole part.
	int16 waveBudget(int phase) override {
		const int16 *wave = kRA2PSXLevel2WaveTable[CLIP(phase - 1, 0, 2)]
				[CLIP(_psx._settings.difficulty, 0, 2)];
		return (int16)(wave[0] + wave[1]);
	}

	bool playBackgroundWave(int phase) override { return showScene(phase); }
	bool playWave(int phase, uint16) override { return showScene(phase); }

	// The PSX part is one continuous scene rather than a chain of movie waves, so a pass
	// normally settles the phase outright. The budget only matters if one is cut short.
	WaveCredit creditWave(int16 mask, int16 *budget, int16) override {
		WaveCredit credit;
		if (_scene.cleared() || playerDead() || _skipped) {
			_phaseState = (uint16)mask;
			credit.stop = true;
			return credit;
		}
		if (budget && *budget > 0) {
			--*budget;
			credit.bits = 2;
		} else {
			credit.stop = true;
		}
		return credit;
	}

	// The two short links between the parts; CUT1 belongs to the opening.
	bool playPhaseEnd(int phase) override {
		static const char *const cutscenes[] = {
			"S1/L02_CUT2.STR", "S1/L02_CUT3.STR"
		};
		return _psx.playVideo(cutscenes[phase - 1], 1, false);
	}

	uint16 phaseState() const override { return _phaseState; }
	bool playerDead() const override { return _shield <= 0; }
	void accumulateKills() override { _kills += _scene.kills(); }
	void accumulateMisses() override { _misses += _scene.misses(); }

	// A death costs a life and restarts the level from its first part.
	bool handleDeath(int, Result &result) override {
		if (!_psx.playVideo("S1/L02_DIE.STR", 1, false)) {
			result = shouldQuit() ? kQuit : kError;
			return false;
		}
		if (--_lives > 0)
			return true;
		_psx.playVideo("S1/L02_OVER.STR", 1, false);
		result = kGameOver;
		return false;
	}

	void playComplete(int) override {
		_psx.playVideo("S1/L02_EXTR.STR", 1, false);
	}

private:
	// Runs one pass at the part: the rookie leans out and ducks, the scripted troopers
	// step out and trade fire, and the pass ends when the wave or the shield runs out.
	bool showScene(int phase) {
		const RA2PSXLevel2PartInfo &part = kRA2PSXLevel2Parts[phase - 1];
		const bool cursorWasVisible = CursorMan.isVisible();
		CursorMan.showMouse(false);

		int aimX = (part.aimLeft + part.aimRight) / 2;
		int aimY = (part.aimTop + part.aimBottom) / 2;
		bool left = false, right = false, up = false, down = false;
		bool running = true;
		// The fire press has to outlive the poll that saw it: ticks run at 60Hz while
		// events are drained several times as often, so a per pass flag loses presses.
		bool firePressed = false;
		bool fireEdge = false;
		bool fireWasPressed = false;
		int flashFrame = kRA2PSXHitFlashFrames;
		int tick = -1;
		const uint32 startTime = g_system->getMillis();
		const int viewX = ((int)_psx._vm->_screenWidth - 320) / 2;
		const int viewY = ((int)_psx._vm->_screenHeight - 240) / 2;

		while (running && !shouldQuit()) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				const bool pressed = event.type == Common::EVENT_KEYDOWN;
				if (pressed || event.type == Common::EVENT_KEYUP) {
					switch (event.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
						// Stands in for the pause menu, which is not ported yet.
						if (pressed) {
							_skipped = true;
							running = false;
						}
						break;
					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_a:
						left = pressed;
						break;
					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_d:
						right = pressed;
						break;
					case Common::KEYCODE_UP:
					case Common::KEYCODE_w:
						up = pressed;
						break;
					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_s:
						down = pressed;
						break;
					case Common::KEYCODE_SPACE:
					case Common::KEYCODE_RETURN:
					case Common::KEYCODE_KP_ENTER:
						if (pressed && !firePressed && !event.kbdRepeat)
							fireEdge = true;
						firePressed = pressed;
						break;
					case Common::KEYCODE_TAB:
					case Common::KEYCODE_LCTRL:
					case Common::KEYCODE_RCTRL:
						if (pressed && !event.kbdRepeat)
							_scene.toggleCover();
						break;
					default:
						break;
					}
				} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START ||
						event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
					const bool started = event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START;
					if (event.customType == kScummActionInsaneAttack) {
						if (started && !firePressed)
							fireEdge = true;
						firePressed = started;
					} else if (started && event.customType == kScummActionInsaneSwitch) {
						_scene.toggleCover();
					}
				} else if (event.type == Common::EVENT_MOUSEMOVE) {
					aimX = event.mouse.x - viewX;
					aimY = event.mouse.y - viewY;
				} else if (event.type == Common::EVENT_RBUTTONDOWN) {
					_scene.toggleCover();
				} else if (event.type == Common::EVENT_LBUTTONDOWN) {
					if (!firePressed)
						fireEdge = true;
					firePressed = true;
				} else if (event.type == Common::EVENT_LBUTTONUP) {
					firePressed = false;
				} else if (event.type == Common::EVENT_QUIT ||
						event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					_psx._vm->quitGame();
				}
			}

			const uint32 elapsed = g_system->getMillis() - startTime;
			const int target = (int)((uint64)elapsed * kRA2PSXLevel2TickRate / 1000);
			bool redraw = false;
			while (tick < target && running) {
				++tick;
				redraw = true;
				if (_scene.outOfCover()) {
					aimX += (right ? 2 : 0) - (left ? 2 : 0);
					aimY += (down ? 2 : 0) - (up ? 2 : 0);
				}
				aimX = CLIP<int>(aimX, part.aimLeft, part.aimRight);
				aimY = CLIP<int>(aimY, part.aimTop, part.aimBottom);
				_scene.update(aimX, aimY);

				const int damage = _scene.updateEnemies(_psx._vm->_rnd);
				if (damage) {
					_shield = MAX(0, _shield - damage);
					flashFrame = 0;
				}
				// Holding fire keeps shooting, the way the original polls its pad.
				const bool shoot = fireEdge || (firePressed && fireWasPressed &&
						!(tick % kRA2PSXLevel2FireRepeat));
				fireWasPressed = firePressed;
				fireEdge = false;
				if (shoot)
					_score += _scene.shoot(aimX, aimY);
				if (flashFrame < kRA2PSXHitFlashFrames)
					++flashFrame;
				if (_shield <= 0 || _scene.cleared())
					running = false;
			}

			if (redraw) {
				Graphics::Surface output;
				output.create(_psx._vm->_screenWidth, _psx._vm->_screenHeight,
						g_system->getScreenFormat());
				_scene.draw(output, aimX, aimY);
				drawRA2PSXHitFlash(output, flashFrame);
				g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0,
						output.w, output.h);
				output.free();
				g_system->updateScreen();
			}
			g_system->delayMillis(5);
		}

		CursorMan.showMouse(cursorWasVisible);
		return !shouldQuit();
	}

	Rebel2PSX &_psx;
	const RA2PSXArchive &_archive;
	RA2PSXLevel2Scene _scene;
	uint16 _phaseState;
	int _shield;
	int _score;
	int _kills;
	int _misses;
	int _lives;
	bool _skipped;
};

Common::Error Rebel2PSX::runLevel2() {
	Common::SeekableReadStream *stream = openResource(2);
	if (!stream)
		return Common::Error(Common::kReadingFailed,
				_("Could not open the PlayStation Level 2 resources"));
	RA2PSXArchive archive;
	const bool loaded = archive.load(*stream);
	delete stream;
	if (!loaded)
		return Common::Error(Common::kReadingFailed,
				_("Could not read the PlayStation Level 2 resources"));

	Level2Handler handler(*this, archive);
	if (runRebel2Level2(handler, _vm->_rnd) == Rebel2Level2Handler::kError)
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation Level 2 videos"));
	return Common::kNoError;
}

} // End of namespace Scumm
