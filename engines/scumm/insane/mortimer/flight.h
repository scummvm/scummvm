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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_MORTIMER_FLIGHT_H
#define SCUMM_INSANE_MORTIMER_FLIGHT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

class SmushDeltaBlocksDecoder;

struct MortimerSprite {
	int x, y, width, height;
	Common::Array<byte> pixels;
};

struct MortimerSound {
	uint frame = 0;
	uint rate = 11025;
	byte volume = 255;
	int8 balance = 0;
	bool music = false;
	Common::Array<byte> samples;
};

bool loadMortimerSprites(Common::SeekableReadStream &stream, Common::Array<MortimerSprite> &sprites);
bool loadMortimerSound(Common::SeekableReadStream &stream, MortimerSound &sound);

// The Windows demo flights. No ScummEngine, InsaneRebel or SmushPlayer
// state is used here; only the existing, stateless RLE helpers and a private
// codec37 decoder are shared with the other SMUSH games.
class MortimerFlight {
public:
	enum { kWidth = 384, kHeight = 288 };
	enum Weapon { kRevitascope, kSlimeGun };
	enum ShotResult { kMiss, kAnimalRescued, kHornetHit, kShellRestored };

	MortimerFlight();
	~MortimerFlight();
	bool load(Common::SeekableReadStream &stream);
	void reset();
	bool advance();
	ShotResult shoot(const Common::Point &point);
	bool hitsObstacle(const Common::Point &point) const;

	const byte *pixels() const { return _pixels.begin(); }
	const byte *palette() const { return _palette; }
	uint frame() const { return _nextFrame ? _nextFrame - 1 : 0; }
	uint frameCount() const { return _frames.size(); }
	uint frameRate() const { return _frameRate; }
	uint rescuedCount() const { return _rescuedCount; }
	uint rescuedSpecies() const { return _rescuedSpecies; }
	Weapon weapon() const { return _weapon; }
	const Common::String &nextFlight() const { return _nextFlight; }
	const Common::Array<MortimerSound> &sounds() const { return _sounds; }
	const Common::String &errorMessage() const { return _error; }

private:
	struct Frame {
		uint32 offset, size;
	};
	struct Object {
		uint32 offset, size;
		uint16 skipA, skipB;
	};
	struct Target {
		uint16 id, toggle, remapToggle;
		uint rescuedAt;
		Common::Rect rect;
		bool active, hit;
	};
	struct Obstacle {
		Common::Point corners[4];
	};

	bool fail(const char *message);
	void handleAction(const byte *data, uint32 size);
	void handlePalette(const byte *data, uint32 size);
	void render();
	Target *findTarget(uint16 id);

	Common::Array<byte> _data, _background, _pixels, _scratch;
	Common::Array<Frame> _frames;
	Common::Array<Object> _objects;
	Common::Array<Target> _targets;
	Common::Array<Obstacle> _obstacles;
	Common::Array<MortimerSound> _sounds;
	SmushDeltaBlocksDecoder *_decoder;
	Common::String _error, _nextFlight;
	byte _palette[768], _initialPalette[768], _remap[256];
	int16 _paletteDelta[768];
	int32 _shiftedPalette[768];
	bool _toggles[256];
	Weapon _weapon;
	uint _nextFrame, _frameRate, _rescuedCount, _rescuedSpecies;
};

} // End of namespace Scumm

#endif
