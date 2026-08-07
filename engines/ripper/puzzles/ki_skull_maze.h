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

#ifndef RIPPER_PUZZLES_KI_SKULL_MAZE_H
#define RIPPER_PUZZLES_KI_SKULL_MAZE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/puzzles/ki_skull_maze_model.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class KiSkullMazePuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kTrapped,
		kLoadFailed
	};

	explicit KiSkullMazePuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	struct Config {
		uint randomizer;
		uint frameRate;
		uint doorDelayTicks;
		uint startPosition;
		uint initialHazardDelayTicks;
		uint hazardsPerDecrease;
		uint hazardDelayDecrementTicks;
		uint minimumHazardDelayTicks;

		Config();
	};

	struct Animation {
		Common::Array<BitmapAssetFrame> frames;
		uint16 width;
		uint16 height;

		Animation() : width(0), height(0) {}
	};

	bool loadConfiguration();
	bool loadAssets();
	bool loadBitmap(const Common::String &path, BitmapAssetFrame &frame,
		bool pcx);
	bool loadAnimation(const Common::String &path, Animation &animation);
	void applyPalette() const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y, byte transparentColor) const;
	void render() const;
	Common::Point playerPosition(uint direction, uint originCell) const;
	int findCell(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool startMove(uint targetCell, uint32 now);
	bool advanceAnimation(uint32 now);
	bool advanceToggle(uint32 now);
	bool spawnHazard(uint32 now);
	void beginTerminalState(uint state, uint32 now);
	bool handleKeyword(uint16 command, uint completionFlag, Result &result);
	bool complete(uint completionFlag, const char *source);
	void playCue(uint cue);
	void stopAudio();
	void cleanup(uint savedSelectionIndex, bool savedCursorVisible);

	RipperEngine *_engine;
	Config _config;
	KiSkullMazeModel _model;
	IndexedDisplaySnapshot _incomingDisplay;
	BitmapAssetFrame _background;
	BitmapAssetFrame _lids[5];
	Animation _skullAnimation;
	Animation _manAnimations[5];
	Audio::SoundHandle _audioHandles[3];
	uint _keywordIndex;
	int _hoveredCell;
	uint _movementDirection;
	uint _movementOriginCell;
	uint _targetCell;
	uint _movementFrame;
	uint _skullFrame;
	uint _toggleStep;
	uint _terminalState;
	uint _hazardsSinceDecrease;
	uint _hazardDelayTicks;
	bool _movementCuePlayed;
	uint32 _nextFrameMillis;
	uint32 _nextToggleMillis;
	uint32 _nextHazardMillis;
	uint32 _terminalDeadlineMillis;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KI_SKULL_MAZE_H
