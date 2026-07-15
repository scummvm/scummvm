/* ScummVM - Graphic Adventure Engine */

#ifndef RIPPER_DIALOGUE_H
#define RIPPER_DIALOGUE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "ripper/resources.h"

namespace Ripper {

struct ScriptCommand;
struct MouseState;
class CompiledScript;

class DialogueManager {
public:
	bool initialize(ResourceManager &resources);
	bool execute(const CompiledScript &script, const ScriptCommand &command, bool includeChoice = true);
	bool service(const MouseState &mouse, uint &result);
	void updateHover(const Common::Point &point);
	bool contains(const Common::Point &point) const;
	void draw() const;
	void rebuildPresentationBands(const char *reason) const;
	bool isPending() const { return _pending; }
	bool hasChoices() const { return !_choices.empty(); }
	void clearPending();

private:
	struct Choice {
		Common::String text;
		uint16 result;
	};
	uint measureText(const Common::String &text) const;
	void updateLayout();
	void drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const;

	Common::Array<Choice> _choices;
	Common::Array<BitmapAssetFrame> _arrowFrames;
	Common::Rect _chooserBounds;
	Common::Rect _upArrowBounds;
	Common::Rect _downArrowBounds;
	bool _pending = false;
	uint _selectedChoice = 0;
	uint _firstVisibleChoice = 0;
	int _hoveredArrow = 0;
	BitmapFontAsset _font;
};

} // End of namespace Ripper

#endif // RIPPER_DIALOGUE_H
