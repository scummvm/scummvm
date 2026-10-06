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

#ifndef NANCY_ACTION_CAMERAACTION_H
#define NANCY_ACTION_CAMERAACTION_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// The photo album of the Nancy14 camera (AR 133): shows one picture at a time,
// with controls to take another, delete this one, and page back and forth. The
// interface art is part of the scene background, so this only draws the display
// area and the sprite of a control being pressed.
class CameraAction : public RenderActionRecord {
public:
	CameraAction() : RenderActionRecord(7) {}
	virtual ~CameraAction() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "CameraAction"; }

	// kPrevious and kNext double as Yes and No while a deletion is confirmed.
	enum Control { kTakePicture = 0, kDelete = 1, kPrevious = 2, kNext = 3, kNumControls = 4 };

	enum Screen { kEmpty = 0, kCameraFull = 1, kViewing = 2, kConfirmDelete = 3, kDeleted = 4 };

	struct Button {
		Common::Rect destRect;	// on-screen hotspot, and where the sprite is drawn
		Common::Rect srcRect;	// pressed sprite inside the interface image
	};

	// The control under the cursor, or -1 if there is none or it is disabled.
	int buttonAtCursor(const Common::Point &mousePos) const;
	bool isButtonEnabled(uint button) const;
	void pressButton(uint button);

	// Drops any message and selects the newest picture.
	void resetToBrowsing();
	void showMessage(Screen screen);
	void deleteCurrentPicture();
	void redraw();

	// -- File data --
	Common::Path _imageName;			// 0x00 - sprite & message sheet
	uint16 _buttonCursorType = 0;		// 0x21 - cursor over an enabled control
	Button _buttons[kNumControls];		// 0x23
	Common::Rect _pictureRect;			// 0xa3 - where a picture is displayed
	Common::Rect _cameraFullRect;		// 0xb3
	Common::Rect _confirmDeleteRect;	// 0xc3
	Common::Rect _deletedRect;			// 0xd3
	RandomSoundBlock _buttonSound;		// 0xe3

	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	Graphics::ManagedSurface _image;
	Screen _screen = kEmpty;
	int _pictureIndex = -1;
	Common::Rect _messageRect;			// plate currently shown, if any
	uint32 _messageEndTime = 0;
	int _pendingButton = -1;			// pressed, not yet acted on
	uint32 _pendingTime = 0;
	bool _takePictureRequested = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CAMERAACTION_H
