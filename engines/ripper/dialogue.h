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

#ifndef RIPPER_DIALOGUE_H
#define RIPPER_DIALOGUE_H

#include "common/array.h"
#include "common/noncopyable.h"
#include "common/rect.h"
#include "common/str.h"
#include "ripper/controls.h"
#include "ripper/resources.h"

namespace Common {
class Serializer;
}

namespace Ripper {

struct ScriptCommand;
struct MouseState;

class DialogueChooser : private Common::NonCopyable {
public:
	struct Choice {
		Common::String text;
		uint16 result;
	};

	struct SaveState {
		Common::Array<Choice> choices;
		ChooserModel chooser;
		bool pending = false;
	};

	bool initialize(ResourceManager &resources, bool retailPresentation);
	bool execute(const ScriptCommand &command, bool includeChoice = true);
	bool service(const MouseState &mouse, uint &result);
	bool serviceKeyboard(uint16 command, uint &result);
	void appendChoice(const Common::String &text, uint16 result);
	bool activateChoices(const char *source);
	void updateHover(const Common::Point &point);
	bool contains(const Common::Point &point) const;
	void draw(bool captureBacking = false);
	void rebuildPresentationBands(const char *reason) const;
	bool isPending() const { return _pending; }
	bool hasChoices() const { return !_choices.empty(); }
	void clearPending();
	void dismissForSceneTransition(const char *reason);
	bool syncGame(Common::Serializer &serializer);
	void captureSaveState(SaveState &state) const;
	bool syncGame(Common::Serializer &serializer, SaveState &state) const;
	void applySaveState(SaveState &state);

private:
	uint measureText(const Common::String &text) const;
	bool hasScrollArrows() const;
	uint maximumVisibleChoiceCount() const;
	void updateLayout();
	void drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const;
	Common::Rect visualBounds() const;
	bool restoreBacking();
	bool selectChoice(uint choiceIndex, uint &result, const char *source);

	Common::Array<Choice> _choices;
	Common::Array<BitmapAssetFrame> _arrowFrames;
	Common::Rect _chooserBounds;
	Common::Rect _upArrowBounds;
	Common::Rect _downArrowBounds;
	Common::Rect _backingBounds;
	Common::Array<byte> _backingPixels;
	Common::Array<byte> _renderedChoicePixels;
	bool _pending = false;
	bool _visualDirty = false;
	byte _normalBackgroundColor = 0;
	byte _normalTextColor = 0;
	byte _selectedBackgroundColor = 0;
	byte _selectedTextColor = 0;
	ChooserModel _chooser;
	int _hoveredArrow = 0;
	BitmapFontAsset _font;
};

} // End of namespace Ripper

#endif // RIPPER_DIALOGUE_H
