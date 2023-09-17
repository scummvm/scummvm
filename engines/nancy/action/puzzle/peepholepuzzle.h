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

#ifndef NANCY_ACTION_PEEPHOLEPUZZLE_H
#define NANCY_ACTION_PEEPHOLEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Action record that, despite what its name suggests, is mostly used
// to render Nancy's diary in nancy6 and up.
class PeepholePuzzle : public RenderActionRecord {
public:
	PeepholePuzzle() : RenderActionRecord(7) {}
	virtual ~PeepholePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "PeepholePuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawInner();
	void checkButtons();

	Common::Path _innerImageName;
	Common::Path _buttonsImageName;

	uint16 _transparency = 0;

	Common::Rect _innerBounds;
	Common::Rect _startSrc;
	Common::Rect _dest;

	// Order: up, down, left, right
	Common::Array<Common::Rect> _buttonDests;
	Common::Array<Common::Rect> _buttonSrcs;
	Common::Array<Common::Rect> _buttonDisabledSrcs;

	byte _pixelsToScroll = 0;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _innerImage;
	Graphics::ManagedSurface _buttonsImage;

	Common::Rect _currentSrc;
	int _pressedButton = -1;
	uint32 _pressStart = 0;
	Common::Array<bool> _disabledButtons = Common::Array<bool>(4, false);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PEEPHOLEPUZZLE_H
