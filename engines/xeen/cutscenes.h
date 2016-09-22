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

#ifndef XEEN_CUTSCENES_H
#define XEEN_CUTSCENES_H

#include "xeen/files.h"
#include "xeen/sprites.h"

namespace Xeen {

#define WAIT(time) events.updateGameCounter(); \
	if (events.wait(time)) \
		return false

class XeenEngine;

class Cutscenes {
protected:
	XeenEngine *_vm;
	StringArray _subtitles;
	SpriteResource *_boxSprites;
	uint _timeElapsed;
	Common::String _subtitleLine;
	uint _subtitleLineNum, _subtitleSize;
protected:
	Cutscenes(XeenEngine *vm) : _vm(vm), _timeElapsed(0), _boxSprites(nullptr),
		_subtitleLineNum(0), _subtitleSize(0) {}
	virtual ~Cutscenes() {}

	/**
	 * Resets the subtitles position
	 */
	void resetSubtitles(uint lineNum, uint defaultSize = 1);

	/**
	 * Free subtitles
	 */
	void freeSubtitles();

	/**
	 * Shows subtitles
	 */
	void showSubtitles(uint windowIndex = 0);

	/**
	 * Delays either the specified number of frames, or until
	 * an entire subtitle line is shown if subtitles are on
	 */
	bool subtitlesWait(uint minTime = 0);

	/**
	 * Wait for the end of currently playing sound or subtitles line
	 */
	bool waitForLineOrSound();

	/**
	 * Records the current execution time
	 */
	void recordTime();

	/**
	 * Returns the number of ticks since the last recordTime
	 */
	uint timeElapsed();

	/**
	 * Get a speaking frame from a range
	 */
	uint getSpeakingFrame(uint minFrame, uint maxFrame);

	/**
	 * Draws the scroll in the background
	 * @param rollUp	If true, rolls up the scroll. If false, unrolls.
	 * @param fadeIn	If true, does an initial fade in
	 * @returns		True if key or mouse pressed
	 */
	virtual bool doScroll(bool rollUp, bool fadeIn);
};

} // End of namespace Xeen

#endif /* XEEN_CUTSCENES_H */
