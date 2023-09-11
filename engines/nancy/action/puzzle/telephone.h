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

#ifndef NANCY_ACTION_TELEPHONE_H
#define NANCY_ACTION_TELEPHONE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class Telephone : public RenderActionRecord {
public:
	struct PhoneCall {
		Common::Array<byte> phoneNumber;
		Common::String soundName;
		Common::String text;
		SceneChangeWithFlag sceneChange;
	};

	enum CallState { kWaiting, kButtonPress, kRinging, kBadNumber, kCall, kHangUp };

	Telephone() :
		RenderActionRecord(7),
		_callState(kWaiting),
		_selected(0) {}
	virtual ~Telephone() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	Common::String _imageName;
	Common::Array<Common::Rect> _srcRects;
	Common::Array<Common::Rect> _destRects;
	SoundDescription _genericDialogueSound;
	SoundDescription _genericButtonSound;
	SoundDescription _ringSound;
	SoundDescription _dialToneSound;
	SoundDescription _dialAgainSound;
	SoundDescription _hangUpSound;
	Common::Array<Common::String> _buttonSoundNames;
	Common::String _addressBookString;
	Common::String _dialAgainString;
	SceneChangeWithFlag _reloadScene;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;
	Common::Array<PhoneCall> _calls;

	Common::Array<byte> _calledNumber;
	Graphics::ManagedSurface _image;
	CallState _callState;
	uint _selected;

protected:
	Common::String getRecordTypeName() const override { return "Telephone"; }
	bool isViewportRelative() const override { return true; }

	void drawButton(uint id);
	void undrawButton(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TELEPHONE_H
