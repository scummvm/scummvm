/* ScummVM - Graphic Adventure Engine */

#ifndef RIPPER_DIALOGUE_H
#define RIPPER_DIALOGUE_H

#include "common/array.h"
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
class CompiledScript;

class DialogueChooser {
public:
	bool initialize(ResourceManager &resources, bool loadScrollArrows);
	bool execute(const CompiledScript &script, const ScriptCommand &command, bool includeChoice = true);
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

private:
	struct Choice {
		Common::String text;
		uint16 result;
	};
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
	ChooserModel _chooser;
	int _hoveredArrow = 0;
	BitmapFontAsset _font;
};

} // End of namespace Ripper

#endif // RIPPER_DIALOGUE_H
