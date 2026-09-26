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

#ifndef NANCY_ACTION_PUZZLERECORD_H
#define NANCY_ACTION_PUZZLERECORD_H

#include "common/path.h"

#include "graphics/managed_surface.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Common base for all puzzle action records. Holds the scene changes and
// give-up hotspot that nearly every puzzle has.
class PuzzleRecord : public RenderActionRecord {
public:
	PuzzleRecord(uint zOrder) : RenderActionRecord(zOrder) {}
	virtual ~PuzzleRecord() {}

	Common::String getRecordExtraInfo() const override;

protected:
	// Loads _imageName into _image, keyed to the draw surface's transparent color
	void loadImage();

	// Creates a transparent, viewport-sized draw surface and shows it over the viewport
	void initViewportSurface();

	// The ids in the AR data are raw cursor types, which is what the "set from script" path expects
	void setDataCursor(uint16 cursorType, bool hotspotVariant = true) const;

	// Plays a random sound from the block and shows its caption, looked up by the sound's
	// name, first in AUTOTEXT, then in CONVO. Returns the sound's description, which is
	// left as "NO SOUND" when the block has nothing to play.
	SoundDescription playSoundBlock(const RandomSoundBlock &block);
	bool isSoundBlockPlaying(const RandomSoundBlock &block) const;

	// Reads the Nancy 13+ count-prefixed give-up hotspot array into the exit fields,
	// keeping only the first record.
	void readExitHotspot(Common::SeekableReadStream &stream);

	bool isExitHotspotHovered(const NancyInput &input) const;
	void setExitCursor() const;

	// Shows the exit cursor and returns true while the mouse is over the give-up hotspot
	bool hoverExitHotspot(const NancyInput &input) const;

	// A solve sound named "NO SOUND" or left empty counts as absent: it never plays,
	// and is never reported as playing.
	bool hasSolveSound() const;
	void playSolveSound();
	bool isSolveSoundPlaying() const;

	// The puzzle's sprite sheet
	Common::Path _imageName;
	Graphics::ManagedSurface _image;

	// Scene the puzzle moves to when solved; its flag is set on solve
	SceneChangeWithFlag _solveScene;
	SoundDescription _solveSound;
	uint16 _solveSoundDelay = 0; // units differ per puzzle

	// Scene the puzzle moves to when left unsolved, e.g. via the give-up hotspot
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	bool _exitCursorFromData = false; // otherwise the generic puzzle exit cursor is used
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PUZZLERECORD_H
