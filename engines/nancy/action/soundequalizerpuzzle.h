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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_SOUNDEQUALIZERPUZZLE_H
#define NANCY_ACTION_SOUNDEQUALIZERPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace UI {
class Scrollbar;
}

namespace Nancy {

struct SoundEqualizerPuzzleData;

namespace Action {

class ViewportScrollbar;

class SoundEqualizerPuzzle: public RenderActionRecord {
public:
	SoundEqualizerPuzzle() : RenderActionRecord(7) {}
	virtual ~SoundEqualizerPuzzle();

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	Common::String _imageName;

	Common::Rect _buttonSrc;
	Common::Rect _buttonDest;
	Common::Rect _sliderSrc;

	Common::Array<uint16> _sliderX;
	Common::Array<uint16> _sliderYMin;
	Common::Array<uint16> _sliderYMax;

	Common::Rect _lightSrc;
	Common::Array<Common::Array<Common::Rect>> _lightDests;

	Common::Array<uint16> _sliderInitialPositions;

	Common::Array<SoundDescription> _sounds;

	Common::Array<uint16> _minVolume;
	Common::Array<uint16> _maxVolume;
	Common::Array<uint16> _minRate;
	Common::Array<uint16> _maxRate;

	uint16 _solveChannelID = 0;
	uint16 _solveMinVolume = 0;
	uint16 _solveMaxVolume = 0;
	uint16 _solveMinRate = 0;
	uint16 _solveMaxRate = 0;

	SceneChangeDescription _exitScene;
	SoundDescription _exitSound;

	FlagDescription _solveFlag;

	Graphics::ManagedSurface _image;
	Common::Array<ViewportScrollbar *> _sliders;

	SoundEqualizerPuzzleData *_puzzleState = nullptr;

protected:
	Common::String getRecordTypeName() const override { return "SoundEqualizerPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void updateSlider(uint sliderID);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SOUNDEQUALIZERPUZZLE_H
