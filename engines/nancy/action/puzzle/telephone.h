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

class Font;

namespace Action {

class Telephone : public RenderActionRecord {
public:
	struct PhoneCall {
		Common::Array<byte> phoneNumber;
		Common::String soundName;
		Common::String text;
		SceneChangeWithFlag sceneChange;

		// NewPhone members
		int16 eventFlagCondition = -1;
		Common::Rect displaySrc;
	};

	enum CallState { kWaiting, kButtonPress, kRinging, kBadNumber, kPreCall, kCall, kHangUp };

	Telephone(bool isNewPhone) :
		RenderActionRecord(7),
		_callState(kWaiting),
		_buttonLastPushed(-1),
		_selected(-1),
		_checkNumbers(false),
		_font(nullptr),
		_animIsStopped(false),
		_isNewPhone(isNewPhone) {}
	virtual ~Telephone() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return _isNewPhone ? "NewPhone" : "Telephone"; }
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;
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

	// NewPhone properties
	bool _hasDisplay = false;
	uint16 _displayFont = 0;
	Common::Path _displayAnimName;
	uint32 _displayAnimFrameTime = 0;
	Common::Array<Common::Rect> _displaySrcs;
	Common::Rect _displayDest;

	bool _dialAutomatically = true;

	Common::Rect _dirHighlightSrc;
	Common::Rect _dialHighlightSrc;

	int16 _upDirButtonID = -1;
	int16 _downDirButtonID = -1;
	int16 _dialButtonID = -1;
	int16 _dirButtonID = -1;

	Common::Rect _displayDialingSrc;

	SoundDescription _preCallSound;

	Common::Array<byte> _calledNumber;
	Graphics::ManagedSurface _image;
	Graphics::ManagedSurface _animImage;
	CallState _callState;
	int _buttonLastPushed;
	int _selected;
	bool _checkNumbers;
	bool _animIsStopped;

	uint32 _displayAnimEnd = 0;
	uint16 _displayAnimFrame = 0;
	int16 _displayedDirectory = 0;
	bool _isShowingDirectory = false;

	const Font *_font;

	bool _isNewPhone;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TELEPHONE_H
