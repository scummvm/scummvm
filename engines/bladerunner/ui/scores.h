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

#ifndef BLADERUNNER_SCORES_H
#define BLADERUNNER_SCORES_H

#include "common/array.h"

namespace Common {
struct KeyState;
}

namespace BladeRunner {

class BladeRunnerEngine;
class Font;
class Shape;
class SaveFileReadStream;
class SaveFileWriteStream;
class TextResource;
class VQAPlayer;
class UIImagePicker;

class Scores {
	BladeRunnerEngine *_vm;
	bool               _isOpen;
	bool               _isLoaded;
	VQAPlayer         *_vqaPlayer;
	int                _scores[7];
	int                _scorers[7];

	int                _lastScoreId;
	int                _lastScoreValue;

	Font              *_font;
	TextResource      *_txtScorers;

public:
	Scores(BladeRunnerEngine *vm);
	~Scores();

	void open();
	bool isOpen() const;
	void close();

	int query(int index) { return _scores[index]; }
	void set(int index, int value);

	void handleKeyDown(const Common::KeyState &kbd);
	int handleMouseUp(int x, int y);
	int handleMouseDown(int x, int y);

	void tick();
	void fill();

	void reset();
	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
