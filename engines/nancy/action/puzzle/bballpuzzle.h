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

#ifndef NANCY_ACTION_BBALLPUZZLE_H
#define NANCY_ACTION_BBALLPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {
namespace Action {

class BBallPuzzle : public RenderActionRecord {
public:
	BBallPuzzle() : RenderActionRecord(7) {}
	virtual ~BBallPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "BBallPuzzle"; };
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;

	uint16 _positions = 0;
	uint16 _powers = 0;
	uint16 _angles = 0;

	Common::Array<Common::Point> _correctVals;

	Common::Rect _shootButtonDest;
	Common::Rect _minusButtonDest;
	Common::Rect _plusButtonDest;

	Common::Rect _playerDest;
	Common::Rect _powerDest;
	Common::Rect _angleDest;
	Common::Array<Common::Rect> _angleSliderHotspots;

	Common::Rect _shootButtonSrc;
	Common::Rect _minusButtonSrc;
	Common::Rect _plusButtonSrc;

	Common::Array<Common::Rect> _playerSrcs;
	Common::Array<Common::Rect> _powerSrcs;
	Common::Array<Common::Rect> _anglesSrcs;

	SoundDescription _shootSound;
	SoundDescription _minusSound;
	SoundDescription _plusSound;

	SceneChangeDescription _shootSceneChange;

	Common::Array<int16> _badShootFlags;
	int16 _goodShootFlag = 0;
	Common::Array<int16> _playerPositionFlags;

	int16 _winFlag = 0;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	uint _curAngle = 0;
	uint _curPower = 0;
	uint _curPosition = 0;

	bool _pressedButton = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BBALLPUZZLE_H
