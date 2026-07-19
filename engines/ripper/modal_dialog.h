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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_MODAL_DIALOG_H
#define RIPPER_MODAL_DIALOG_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;
class RipperEngine;

class ModalDialogManager {
public:
	enum PresentationStyle {
		kMenubPresentation,
		kWacPresentation
	};

	enum TextEntryResult {
		kTextEntryPending,
		kTextEntryAccepted,
		kTextEntryCancelled,
		kTextEntryFailed
	};

	explicit ModalDialogManager(RipperEngine *engine);

	bool initialize(ResourceManager &resources);
	bool run(uint bodyResourceId, bool retainSceneCursorRegions = false,
		PresentationStyle style = kMenubPresentation);
	bool runText(const Common::String &title, const Common::String &body,
		const char *source, bool retainSceneCursorRegions = false,
		PresentationStyle style = kMenubPresentation);
	bool drawRetainedTextPanel(uint bodyResourceId, const Common::Rect &bounds,
		uint firstVisible, uint &maximumFirstVisible, uint &visibleRows,
		PresentationStyle style);
	bool beginTextEntry(const Common::String &prompt, uint maximumLength,
		uint helpResourceId, const char *source);
	TextEntryResult serviceTextEntry(Common::String &text);

private:
	bool captureDisplay();
	void applyModalPalette();
	void restoreDisplay();
	void wrapText(const Common::String &text, uint maxWidth,
		Common::Array<Common::String> &lines) const;
	uint measureText(const Common::String &text) const;
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const;
	void drawBitmap(byte *screen, uint pitch, const BitmapAssetFrame &bitmap,
		int x, int y) const;
	void drawFrame(byte *screen, uint pitch, const Common::Rect &bounds,
		PresentationStyle style) const;
	void drawOverflowBar(byte *screen, uint pitch, const Common::Rect &bounds,
		PresentationStyle style) const;
	void drawDialog(const Common::String &title,
		const Common::Array<Common::String> &lines, uint firstVisible,
		uint visibleRows, const Common::Rect &bounds,
		PresentationStyle style) const;
	void drawTextEntry(const Common::String &prompt, const Common::String &text,
		uint firstVisible, uint cursorPosition, bool caretVisible,
		const Common::Rect &bounds) const;
	uint textEntryCursorFromPoint(const Common::String &text, uint firstVisible,
		int x, const Common::Rect &bounds) const;
	void updateTextEntryFirstVisible(const Common::Rect &bounds);
	void finishTextEntry(Common::String &text);
	bool runTextInternal(const Common::String &title, const Common::String &body,
		uint bodyResourceId, const char *source, bool retainSceneCursorRegions,
		PresentationStyle style);
	const Common::String &resourceString(uint resourceId) const;

	RipperEngine *_engine;
	BitmapFontAsset _font;
	Common::Array<BitmapAssetFrame> _skin;
	Common::Array<BitmapAssetFrame> _wacSkin;
	Common::Array<byte> _modalPalette;
	Common::Array<Common::String> _gameText;
	Common::Array<byte> _savedPixels;
	Common::Array<byte> _savedPalette;
	Common::String _textEntryPrompt;
	Common::String _textEntryText;
	Common::String _textEntrySource;
	uint _textEntryMaximumLength;
	uint _textEntryHelpResourceId;
	uint _textEntryFirstVisible;
	uint _textEntryCursorPosition;
	uint32 _textEntryNextCaretMillis;
	bool _textEntryOverwrite;
	bool _textEntryCaretVisible;
	bool _textEntryActive;
	bool _textEntryRestoreCursor;
	bool _initialized;
};

} // End of namespace Ripper

#endif // RIPPER_MODAL_DIALOG_H
