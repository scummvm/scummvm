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
 *
 */

#ifndef LASTEXPRESS_BEETLE_H
#define LASTEXPRESS_BEETLE_H

#include "lastexpress/lastexpress.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/helpers.h"

#include "common/array.h"
#include "common/system.h"

namespace LastExpress {

class LastExpressEngine;
struct Seq;
struct Sprite;

class Beetle {
public:

	Beetle(LastExpressEngine *engine);
	~Beetle();

	void update();

	void load();
	void unload();

	bool isLoaded() const;

	bool catchBeetle();
	bool isCatchable() const;

private:
	struct BeetleData {
		Common::Array<Sequence *> sequences;

		uint32 field_74;
		Sequence *currentSequence;
		uint32 currentFrame;
		uint32 index;
		int16 coordOffset;
		int16 field_86;

		int16 coordX;
		int16 coordY;

		uint32 indexes[16];

		uint32 offset;
		SequenceFrame *frame;
		bool isLoaded;
		uint32 field_D5;
		uint32 field_D9;
		uint32 field_DD;

		BeetleData() {
			field_74 = 0;
			currentSequence = NULL;
			currentFrame = 0;
			index = 0;
			coordOffset = 0;

			field_86 = 0;

			coordX = 0;
			coordY = 0;

			memset(indexes, 0, sizeof(indexes));
			offset = 0;

			frame = NULL;
			isLoaded = false;
			field_D5 = 0;
			field_D9 = 0;
			field_DD = 0;
		}

		~BeetleData() {
			for (int i = 0; i < (int)sequences.size(); i++)
				SAFE_DELETE(sequences[i]);

			sequences.clear();
		}
	};

	LastExpressEngine *_engine;

	BeetleData *_data;

	void move();
	void updateFrame(SequenceFrame *frame) const;
	void updateData(uint32 index);
	void drawUpdate();
	void invertDirection();
};

class CBeetle {
public:
	CBeetle(LastExpressEngine *engine);
	~CBeetle();

	void tick();
	void checkMouse();
	void setDirection(int direction);
	bool onTable();
	bool click();

private:
	LastExpressEngine *_engine = nullptr;

	Seq *_sequences[30];
	Seq *_currentSequence = nullptr;
	int _currentFrame = 0;
	int _index = 0;
	int _coordOffset = 0;
	Common::Point _coords;
	int _directions[16];
	int _currentDirectionIndex = 0;
	Sprite *_frame = nullptr;
	bool _loaded = false;
	int _mouseCooldown = 0;
	int _fleeSpeed = 0;
	int _spawnCounter = 0;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_BEETLE_H
