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

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class Telephone : public ActionRecord, public RenderObject {
public:
	struct PhoneCall {
		Common::Array<byte> phoneNumber; // 0x0, 11 bytes
		Common::String soundName; // 0xB
		Common::String text; // 0x15, 0xC8 bytes
		SceneChangeDescription sceneChange; // 0xDD
		// shouldStopRendering
		EventFlagDescription flag; // 0xE7
	};

	enum CallState { kWaiting, kButtonPress, kRinging, kBadNumber, kCall, kHangUp };

	Telephone() :
		RenderObject(7),
		_callState(kWaiting),
		_selected(0) {}
	virtual ~Telephone() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	Common::String _imageName; // 0x00
	Common::Array<Common::Rect> _srcRects; // 0xA, 12
	Common::Array<Common::Rect> _destRects; // 0xCA, 12
	SoundDescription _genericDialogueSound; // 0x18A
	SoundDescription _genericButtonSound; // 0x1AC
	SoundDescription _ringSound; // 0x1CE
	SoundDescription _dialToneSound; // 0x1F0
	SoundDescription _dialAgainSound; // 0x212
	SoundDescription _hangUpSound; // 0x234
	Common::Array<Common::String> _buttonSoundNames; // 0x256, 12 * 0xA
	Common::String _addressBookString; // 0x2CE, 0xC8 long
	Common::String _dialAgainString; // 0x396
	SceneChangeDescription _reloadScene; // 0x45E
	EventFlagDescription _flagOnReload; // 0x468 ??
	SceneChangeDescription _exitScene; // 0x46C
	EventFlagDescription _flagOnExit; // 0x476
	Common::Rect _exitHotspot; // 0x47A
	// 0x48A numConvos
	Common::Array<PhoneCall> _calls; // 0x48C

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
